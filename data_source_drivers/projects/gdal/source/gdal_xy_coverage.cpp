/******************************************************************************
gdal_xy_coverage.cpp

begin      : 9/25/2007 2:27:16 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_GDAL_SOURCE
#include "gdal_xy_coverage.h"

// local includes
#include "exceptions.h"
#include "pyx_shared_gdal_data_set.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/reference_sphere.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/derm/wgs84.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/srs.h"
#include "pyxis/procs/user_credentials.h"
#include "pyxis/sampling/spatial_reference_system.h"
#include "pyxis/sampling/xy_bounds_geometry.h"
#include "pyxis/sampling/xy_utils.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"

// Windows includes
// #include <direct.h>

// GDAL includes
#include "cpl_string.h"
#include "gdal_priv.h"

// boost includes
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include <queue>

// standard includes
#include <algorithm>
#include <cassert>
#include <numeric>
#include <regex>

// Properties
const std::string GDALXYCoverage::kstrScope = "GDALDataSetReader"; 
const int GDALXYCoverage::knMaxRetryCount = 10;
const int GDALXYCoverage::knWaitBetweenRetries = 1000; //Time to sleep thread for in miliseconds.

//! The minimum index of a raster band
static const int knMinBandOffset = 1;

// {F68A284C-0875-4874-B677-E069F2356DF1}
PYXCOM_DEFINE_CLSID(GDALXYCoverage, 
					0xf68a284c, 0x875, 0x4874, 0xb6, 0x77, 0xe0, 0x69, 0xf2, 0x35, 0x6d, 0xf1);
PYXCOM_CLASS_INTERFACES(GDALXYCoverage, IXYCoverage::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid);

namespace
{

	/*!
	Map GDAL data types to PYXIS value types.

	\param	nType		The GDAL data type.

	\return The mapped PYXIS value type.
	*/
	PYXValue::eType mapType(GDALDataType nType)
	{
		switch (nType)
		{
		case GDT_Byte:
			return PYXValue::knUInt8;

		case GDT_UInt16:
			return PYXValue::knUInt16;

		case GDT_Int16:
			return PYXValue::knInt16;

		case GDT_UInt32:
			return PYXValue::knUInt32;

		case GDT_Int32:
			return PYXValue::knInt32;

		case GDT_Float32:
			return PYXValue::knFloat;

		case GDT_Float64:
			return PYXValue::knDouble;

		case GDT_CInt16:
			return PYXValue::knInt16;

		case GDT_CInt32:
			return PYXValue::knInt32;

		case GDT_CFloat32:
			return PYXValue::knFloat;

		case GDT_CFloat64:
			return PYXValue::knDouble;

		default:
			assert(false && "Unsupported data type.");
			return PYXValue::knNull;
		}
	}

	/*!
	Get the number of bytes required by a GDAL data type. This method takes into
	account the fact that complex types require twice the number of bytes.

	\param	nType	The GDAL data type

	\return	The number of bytes required.
	*/
	int bytesForType(GDALDataType nType)
	{
		switch (nType)
		{
		case GDT_Byte:
			return 1;

		case GDT_UInt16:
		case GDT_Int16:
			return 2;

		case GDT_UInt32:
		case GDT_Int32:
		case GDT_Float32:
			return 4;

		case GDT_Float64:
			return 8;

		case GDT_CInt16:
			return 4;

		case GDT_CInt32:
		case GDT_CFloat32:
			return 8;

		case GDT_CFloat64:
			return 16;

		default:
			assert(false && "Unsupported data type.");
			return 0;
		}
	}

}

//////////////////////////////////////////////////////////////////////////////
// PYXMapReduce
//////////////////////////////////////////////////////////////////////////////
template<class T,class K,class V>
class PYXMapReduce
{
public:
	typedef typename T Item;
	typedef typename std::map<K,V> Map;
	virtual void map(const Item & item,Map & result) = 0;
	virtual void reduce(Map & item,Map & result) = 0;

private:
	PYXTaskGroupWithLocalStorage<Map> m_mapActions;

	void mapFunc(Map & result,Item * item)
	{
		std::auto_ptr<Item> safeItem(item);

		map(*safeItem,result);
	}

	void reduceFunc()
	{
		for(int i=1;i<m_mapActions.getLocalStorageCount();i++)
		{
			reduce(m_mapActions.getLocalStorage(i),m_mapActions.getLocalStorage(0));
		}
	}

public:
	void addItems(const boost::function<void(Map & result)> & func)
	{
		m_mapActions.addTask(func);
	}

	void addItem(const T & item)
	{
		map(item,m_mapActions.getLocalStorage(0));
		//m_mapActions.addTask(boost::bind(&PYXMapReduce::mapFunc,this,_1,new T(item)));
	}

	const Map & getResult()
	{
		m_mapActions.joinAll();

		reduceFunc();

		return m_mapActions.getLocalStorage(0);
	}
};

////////////////////////////////////////////////////////////////////////////////
// GDALXYAsyncValueGetter
////////////////////////////////////////////////////////////////////////////////
Tester<GDALXYAsyncValueGetter> gTester;
struct XYValueRequestResult
{
public:
	PYXIcosIndex m_index;
	PYXCoord2DInt m_raster;
	PYXCoord2DDouble m_native;
};

class GDALXYAsyncValueGetter : public XYAsyncValueGetter
{
public:
	typedef std::vector<XYValueRequestResult> ListOfRequests;
	typedef std::map<std::pair<int,int>,ListOfRequests> MapOfRequests;


	static const int ChunkSize = 1024;
	//static const int ChunkSize = 512;


	//! Unit test method
	static void test()
	{
		{// test X close to min X
			PYXCoord2DInt center (1,10);
			int width = 4;
			PYXCoord2DInt otherCenter;
			TEST_ASSERT(findCenterOnOtherSide(center,width,0,20,otherCenter));
			TRACE_INFO(otherCenter);
			TEST_ASSERT(otherCenter == PYXCoord2DInt(22,10));
		}
		{// test X close to max X
			PYXCoord2DInt center (18,10);
			int width = 4;
			PYXCoord2DInt otherCenter;
			TEST_ASSERT(findCenterOnOtherSide(center,width,0,20,otherCenter));
			TRACE_INFO(otherCenter);
			TEST_ASSERT(otherCenter == PYXCoord2DInt(-3,10));
		}
		{// test X is in the middle
			PYXCoord2DInt center (10,10);
			int width = 4;
			PYXCoord2DInt otherCenter;
			TEST_ASSERT(!findCenterOnOtherSide(center,width,0,20,otherCenter));
			TRACE_INFO(otherCenter);
		}
	}

public:
	static PYXPointer<GDALXYAsyncValueGetter> create(const GDALXYCoverage & coverage,const XYAsyncValueConsumer & consumer, int width,int height)
	{
		return PYXNEW(GDALXYAsyncValueGetter, coverage, consumer, width, height);
	}

	GDALXYAsyncValueGetter(const GDALXYCoverage & coverage,const XYAsyncValueConsumer & consumer, int width,int height)
		:	m_coverage(coverage), 
		m_consumer(consumer), 
		m_width(width), 
		m_height(height),
		m_canAddRequests(true),
		m_failed(false),
		m_mapReduce(coverage,consumer)
	{
	}

	//addRequest - add a get value request with a given index
	virtual void addAsyncRequests(const PYXTile & tile)
	{	
		if (!m_canAddRequests)
		{
			PYXTHROW(PYXException,"can't add requests once wait was called");
		}

		m_mapReduce.addItems(boost::bind(&GDALXYAsyncValueGetter::doAddRequests,this,_1,tile));
	}

	//addRequest - add a get value request with a given index
	virtual void addAsyncRequest(const PYXIcosIndex & index)
	{	
		if (!m_canAddRequests)
		{
			PYXTHROW(PYXException,"can't add requests once wait was called");
		}

		m_mapReduce.addItem(index);
	}

	//waits until all asyncRequests were completed.
	virtual bool join()
	{
		const MapReduce::Map & result = m_mapReduce.getResult();
		readBuffers(result);
		m_readingTasks.joinAll();

		return !m_failed;
	}

private:
	PYXPointer<GDALXYCoverage::Buffer> readBuffer(std::pair<int,int>& chunkPosition)
	{
		int offset = std::max(m_width,m_height)/2;
		PYXRect2DInt rect(
			chunkPosition.first*ChunkSize - offset,
			chunkPosition.second*ChunkSize - offset,
			(chunkPosition.first+1)*ChunkSize + offset,
			(chunkPosition.second+1)*ChunkSize + offset);

		rect.clip(m_coverage.m_dataRasterBounds);
		return m_coverage.getReadBuffer(rect);
	}
	void readBuffers(const MapOfRequests & requests)
	{
		try
		{
			int offset = std::max(m_width,m_height)/2;
			int count = requests.size();

			for(MapOfRequests::const_iterator mapIt = requests.begin();mapIt != requests.end(); mapIt++)
			{
				std::pair<int,int> chunk = mapIt->first;

				auto buffer = readBuffer(chunk);
				if (!buffer)
				{
					m_failed = true;
					return;
				}

				for(size_t i=0;i<mapIt->second.size();i+=100)
				{
					m_readingTasks.addTask(boost::bind(&GDALXYAsyncValueGetter::performRequests,this,buffer,const_cast<XYValueRequestResult*>(&(mapIt->second[i])),std::min((size_t)100,mapIt->second.size()-i)));
				}			
			}
		}
		catch(...)
		{
			m_failed = true;
		}
	}

	static bool findCenterOnOtherSide(const PYXCoord2DInt &center,int width,int xMin,int xMax , PYXCoord2DInt &result )
	{
		int distanceToMinBorder = center.x() - xMin;
		int distanceToMaxBorder = xMax - center.x() ;

		if(distanceToMinBorder < width)
		{
			result = PYXCoord2DInt(xMax + distanceToMinBorder + 1,center.y());
			return true;
		}
		else if(distanceToMaxBorder < width)
		{
			result = PYXCoord2DInt(xMin - distanceToMaxBorder - 1,center.y());
			return true;
		}
		return false;
	}

	void performRequests(PYXPointer<GDALXYCoverage::Buffer> buffer,XYValueRequestResult * requests,size_t size)
	{
		try
		{
			int windowSize = m_width*m_height;
			std::vector<PYXValue> values(windowSize);
			std::vector<char> hasValues(windowSize);

			std::vector<PYXValue> otherValues(windowSize);
			std::vector<char> hasOtherValues(windowSize);


			for(size_t i=0;i<size;++i)
			{
				XYValueRequestResult & request = requests[i];
				m_coverage.getMatrixOfValues(buffer,request.m_raster,&(values[0]),(bool*)&(hasValues[0]),m_width,m_height);

				// check if we need the otherside!
				PYXCoord2DInt otherCenter;	
				if(m_coverage.m_wrapX && findCenterOnOtherSide(request.m_raster,m_width,m_coverage.m_dataRasterBounds.xMin(),m_coverage.m_dataRasterBounds.xMax(),otherCenter))
				{
					std::pair<int,int> chunk(otherCenter.x() / ChunkSize, otherCenter.y() / ChunkSize);
					auto otherBuffer = readBuffer(chunk);

					m_coverage.getMatrixOfValues(otherBuffer,otherCenter,&(otherValues[0]),(bool*)&(hasOtherValues[0]),m_width,m_height);

					for(int j = 0; j < windowSize; j++)
					{
						if(values[j].isNull() && !otherValues[j].isNull())
						{
							values[j] = otherValues[j];
						}
					}
				}
				//end finding the other side
				m_consumer.onRequestCompleted(request.m_index,request.m_native,(bool*)&(hasValues[0]),&(values[0]),m_width,m_height);
			}
		}
		catch(...)
		{
			m_failed = true;
		}
	}

private:
	const GDALXYCoverage & m_coverage;
	const XYAsyncValueConsumer & m_consumer;

	PYXTaskGroup m_readingTasks;

	bool m_canAddRequests;
	bool m_failed;

	int m_width;
	int m_height;

	class MapReduce : public PYXMapReduce<PYXIcosIndex,std::pair<int,int>,ListOfRequests>
	{
	private:
		const GDALXYCoverage & m_coverage;
		const XYAsyncValueConsumer & m_consumer;

	public:
		MapReduce(const GDALXYCoverage & coverage,const XYAsyncValueConsumer & consumer) : m_coverage(coverage), m_consumer(consumer)
		{
		}

	public:
		virtual void map(const PYXIcosIndex & index,Map & result)
		{
			XYValueRequestResult request;
			request.m_index = index;

			//convert into native coordinates
			if (!m_coverage.getCoordConverter()->tryPyxisToNative(index,&request.m_native))
			{
				//we are complete out of the valid native range
				m_consumer.onRequestCompleted(index,PYXCoord2DDouble(),0,0,0,0);
				return;
			}

			//convert into raster coordinates
			m_coverage.getMetaDataGDAL()->nativeToRaster(request.m_native,&request.m_raster);

			//check if raster coordinate is valid to read from
			if (!m_coverage.m_dataRasterBounds.inside(request.m_raster))
			{
				//its not - report we got no values
				m_consumer.onRequestCompleted(index,PYXCoord2DDouble(),0,0,0,0);
				return;
			}

			//find the right chunk to add
			std::pair<int,int> chunk(request.m_raster.x() / ChunkSize, request.m_raster.y() / ChunkSize);

			//add the requests to the request list
			MapOfRequests::iterator mapIt = result.find(chunk);

			if (mapIt == result.end())
			{
				result.insert(std::make_pair(chunk,ListOfRequests()));
				mapIt = result.find(chunk);
			}

			mapIt->second.push_back(request);
		}

		virtual void reduce(Map & item,Map & result)
		{
			for(MapOfRequests::iterator itemMapIt = item.begin();
				itemMapIt != item.end();
				++itemMapIt)
			{
				MapOfRequests::iterator mapIt = result.find(itemMapIt->first);

				if (mapIt == result.end())
				{
					result.insert(std::make_pair(itemMapIt->first,ListOfRequests()));
					mapIt = result.find(itemMapIt->first);
				}

				for(ListOfRequests::iterator it = itemMapIt->second.begin();
					it != itemMapIt->second.end();
					++it)
				{
					mapIt->second.push_back(*it);
				}
			}
		}
	};

	MapReduce m_mapReduce;

	void doAddRequests(MapReduce::Map & map,PYXTile tile)
	{
		for (PYXExhaustiveIterator it(tile.getRootIndex(), tile.getCellResolution()); !it.end(); it.next())
		{
			m_mapReduce.map(it.getIndex(),map);
		}
	}
};


////////////////////////////////////////////////////////////////////////////////
// XYCoverageValueGetterNoCLUT
////////////////////////////////////////////////////////////////////////////////

//! helper class XYCoverageValueGetterNoCLUT
XYCoverageValueGetterNoCLUT::XYCoverageValueGetterNoCLUT()
{
}

void STDMETHODCALLTYPE XYCoverageValueGetterNoCLUT::setXYCoverageValue (GDALXYCoverage* pXYCoverage)
{
	m_pXYCoverage = pXYCoverage;
}

bool STDMETHODCALLTYPE XYCoverageValueGetterNoCLUT::getCoverageValue(const PYXCoord2DDouble& native,
																	 PYXValue* pValue) const
{
	return m_pXYCoverage->getCoverageValueNoCLUT(native, pValue);
}

int GDALXYCoverage::knBufferLimiter = 100000000;

/*!
Constructor.
*/
GDALXYCoverage::GDALXYCoverage() :
	m_pNullValue(0),
	m_nValueByteSize(0),
	m_pGDALBuffer(0),
	m_pColourTable(0),
	m_nDataType(PYXValue::knNull),
	m_nOverview(-1),
	m_limitSize(true),
	m_axisLimit(-1),
	m_wrapX(false)
{
	m_getterNoCLUT.setXYCoverageValue(this);
}

/*!
Destructor.
*/
GDALXYCoverage::~GDALXYCoverage()
{
	// destroy the buffers.
	delete[] m_pGDALBuffer;
	delete[] m_pNullValue;
}

////////////////////////////////////////////////////////////////////////////////
// IXYCoverage
////////////////////////////////////////////////////////////////////////////////

XYCoverageValueGetter* STDMETHODCALLTYPE GDALXYCoverage::getCoverageValueGetter() const
{
	// TODO: optimize for CLUT.

	if (getContext() != PYXFieldDefinition::knContextCLUT)
	{
		return &m_getterNoCLUT;
	}

	return &m_getter;
}


PYXPointer<XYAsyncValueGetter> STDMETHODCALLTYPE GDALXYCoverage::getAsyncCoverageValueGetter(
	const XYAsyncValueConsumer & consumer,
	int matrixWidth,
	int matrixHeight
	) const
{
	return GDALXYAsyncValueGetter::create(*this,consumer,matrixWidth,matrixHeight);
}

/*!
Get the coverage value at the specified native coordinates.
This call MUST be thread-safe.

\param	native		The native coordinates.
\param  pValue      an pointer to the PYXValue to be filled in

\return	True if a data value was retrieved, false if there was no value to return.
*/
bool GDALXYCoverage::getCoverageValue(	const PYXCoord2DDouble& native,
									  PYXValue* pValue) const
{
	// this routine is a Critical Section in terms of thread safety
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	// get the value at raster coordinates
	PYXCoord2DInt raster;
	m_metaDataGDAL.nativeToRaster(native, &raster);
	// TODO: decide if we need to make XYUtils use the same sort of pixel bounds definition
	// as GDAL and then enforce that all over the place.
	//XYUtils::nativeToNearestRaster(getBounds(), getStepSize(), native, &raster);
	return getValue(raster, pValue);
}

/*!
Get the coverage value at the specified native coordinates.
This call MUST be thread-safe.

\param	native		The native coordinates.
\param  pValue      an pointer to the PYXValue to be filled in

\return	True if a data value was retrieved, false if there was no value to return.
*/
bool GDALXYCoverage::getCoverageValueNoCLUT(	const PYXCoord2DDouble& native,
											PYXValue* pValue) const
{
	// this routine is a Critical Section in terms of thread safety
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	// get the value at raster coordinates
	PYXCoord2DInt raster;
	m_metaDataGDAL.nativeToRaster(native, &raster);

	if (!m_dataRasterBounds.inside(raster))
	{
		// out of bounds so return no data
		return false;
	}

	bool bGotBuffer = false;
	{
		// lock the buffer access while we look at them
		boost::recursive_mutex::scoped_lock lock(m_bufferMutex);

		auto buffer = m_readBuffers.findContainingBuffer(raster);
		bGotBuffer = buffer->read(raster,(byte*)m_pGDALBuffer);		
	}

	if (!bGotBuffer)
	{
		// get it the old way...
		int nError = CE_None;
		// fetch the value from the raster
		int nPixelSpace = GDALGetDataTypeSize( m_nGDALDataType ) / 8;

		GByte *pabyBandData = (GByte *) m_pGDALBuffer;
		for (int nBand = 1; nBand <= getBandCount(); ++nBand)
		{
			GDALRasterBand *poBand = getRasterBand(nBand);
			assert((poBand != 0) && "GDAL raster band is null.");

			nError = poBand->RasterIO( GF_Read, raster.x(), raster.y(), 1, 1, 
				(void *) pabyBandData, 1, 1,
				m_nGDALDataType, nPixelSpace, nPixelSpace );
			if (nError != CE_None)
			{
				TRACE_ERROR("GDAL error '" << nError << 
					"' occurred during value extraction at coord '" <<
					raster << "'.");

				// attempt will be made to retrieve this coverage value again until success
				// TODO[kabiraman]: We should probably throw an exception here and do something intelligent.  
				// In case of a WCS, we'd be hitting the server continuously over the same missing coverage value.  
				// For local files, we don't know yet if/how this code path will be reached.
				return false;
			}
			pabyBandData += nPixelSpace;
		}
	}

	// Determine if the null value is equal to the retrieved value.
	if (m_pNullValue && memcmp(m_pGDALBuffer, m_pNullValue, m_nValueByteSize) == 0)
	{
		return false;
	}

	// Determine if the forced null value is equal to the retrieved value.
	if (m_forcedNullValue.getType() != PYXValue::knNull && memcmp(m_pGDALBuffer, m_forcedNullValue.getPtr(0), m_nValueByteSize) == 0)
	{
		return false;
	}

	memcpy(pValue->getPtr(0), m_pGDALBuffer, m_nValueByteSize);
	return true;
}

/*!
Get the field values around the specified native coordinate.
The origin of the matrix of returned values will be at the grid point 
in the mesh that is lower or equal to the nativeCentre point and adjusted 
left (or down) by trunc().  

If the values to be returned fall outside of the current data set, then 
the edges of the data set will be duplicated and returned.

\param	nativeCentre	The native coordinate.
\param  pValues         an array of PYXValue to be filled in with field values
that are centered on the point requested.
\param  sizeX           width of PYXValue array
\param  sizeY           height of PYXValue array
\param	nFieldIndex		The field index.
*/
void STDMETHODCALLTYPE GDALXYCoverage::getMatrixOfValues(	const PYXCoord2DDouble& nativeCentre,
														 PYXValue* pValues,
														 int sizeX,
														 int sizeY) const
{
	// this routine is a Critical Section in terms of thread safety
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	assert(pValues != 0 && "validate the pointer to the array.");
	assert(sizeX > 0 && "we want an array size that is positive");
	assert(sizeY > 0 && "we want an array size that is positive");

	// calculate the lower left corner in raster coordinates.
	PYXCoord2DInt rasterBaseLL;
	m_metaDataGDAL.nativeToRaster(nativeCentre, &rasterBaseLL);
	// XYUtils::nativeToLowerRaster(getBounds(), getStepSize(), nativeCentre, &rasterBaseLL);
	rasterBaseLL.setX(rasterBaseLL.x() - static_cast<int>((sizeX - 1) / 2.0));
	rasterBaseLL.setY(rasterBaseLL.y() - static_cast<int>((sizeY - 1) / 2.0));

	// find the maximum bounds for the data source in native coordinatees.
	PYXCoord2DDouble nativeMax;
	if (getStepSize().x() < 0)
	{
		nativeMax.setX(getBounds().xMin());
	}
	else
	{
		nativeMax.setX(getBounds().xMax());
	}
	if (getStepSize().y() < 0)
	{
		nativeMax.setY(getBounds().yMin());
	}
	else
	{
		nativeMax.setY(getBounds().yMax());
	}

	// translate the native maximum to raster coords.
	PYXCoord2DInt rasterMax;
	m_metaDataGDAL.nativeToRaster(nativeMax, &rasterMax);
	//XYUtils::nativeToLowerRaster(getBounds(), getStepSize(), nativeMax, &rasterMax);
	rasterMax.setX(rasterMax.x() - 1);
	rasterMax.setY(rasterMax.y() - 1);

	PYXValue pv = m_coverageValueSpec;

	PYXCoord2DInt raster;  // the position of the current point we are retrieving.
	for (int x = 0; x < sizeX; ++x)
	{
		// set the x coordinate of the raster.
		raster.setX(rasterBaseLL.x() + x);

		for (int y = 0; y < sizeY; y++)
		{
			// set the y coordinate of the raster.
			raster.setY(rasterBaseLL.y() + y);
			if (getValue(raster, &pv))
			{
				pValues[x * sizeX + y] = pv;
			}
			else
			{
				pValues[x * sizeX +y] = PYXValue();
			}

		}
	}
}


void STDMETHODCALLTYPE GDALXYCoverage::getMatrixOfValues(const PYXPointer<Buffer> & buffer,
														 const PYXCoord2DInt& raster,
														 PYXValue* pValues,
														 bool* pHasValues,
														 int sizeX,
														 int sizeY) const
{
	PYXValue pv = m_coverageValueSpec;

	PYXCoord2DInt readCoord;  // the position of the current point we are retrieving.
	for (int x = 0; x < sizeX; ++x)
	{
		// set the x coordinate of the raster.
		readCoord.setX(raster.x() + x - ((sizeX-1)/2));

		for (int y = 0; y < sizeY; y++)
		{
			// set the y coordinate of the raster.
			readCoord.setY(raster.y() + y - ((sizeY-1)/2));
			if (getValue(buffer, readCoord, &pv))
			{
				pValues[x * sizeY + y] = pv;
				pHasValues[x * sizeY + y] = true;
			}
			else
			{
				pValues[x * sizeY +y] = PYXValue();
				pHasValues[x * sizeY + y] = false;
			}
		}
	}
}

/*! 
Set spatial reference system for the reader.

\param spSRS	The new spatial reference system for the object.
*/
void GDALXYCoverage::setSpatialReferenceSystem(boost::intrusive_ptr<ISRS> spSRS)
{
	assert(m_bHasSRS == false);

	if (!m_coordConverter)
	{
		m_coordConverter = new CoordConverterImpl();
	}
	m_coordConverter->initialize(*spSRS->getSRS());
	m_coordConverterExternal = m_coordConverter;
	m_bHasSRS = true;
}

/*!
Calculate the spatial precision. Determine the coordinate system of the 
datasource and call the appropriate method.

\return	The spatial precision in metres or -1.0 if not known.
*/
double GDALXYCoverage::getSpatialPrecision() const
{
	if (m_bHasSRS)
	{
		// determine if it is a projected system or a geographic system.
		if (m_coordConverter->isProjected())
		{
			return calcSpatialPrecisionProjected();
		}
		else
		{
			return calcSpatialPrecisionGeographical(WGS84::getInstance());
		}
	}

	return -1.0;
}

bool GDALXYCoverage::loadBuffer (const PYXPointer<Buffer> & buffer) const
{
	PYXRect2DInt readBounds = buffer->getReadBounds();
	byte * pBuffer = buffer->getBuffer();
	int nPixelBytes = buffer->getPixelSize();
	int nLineBytes = buffer->getLineSize();
	int nBandBytes = buffer->getBandSize();

	if ((m_axisLimit > 0) && ((readBounds.height() > m_axisLimit) || (readBounds.width() > m_axisLimit)))
	{
		for (int xBlock = 0; xBlock <= (int) (readBounds.width() / m_axisLimit); ++xBlock)
		{
			for (int yBlock = 0; yBlock <= (int) (readBounds.height() / m_axisLimit); ++yBlock)
			{
				PYXRect2DInt currentReadBounds( readBounds);
				currentReadBounds.setYMin( readBounds.yMin() + yBlock * m_axisLimit);
				currentReadBounds.setYMax( currentReadBounds.yMin() + m_axisLimit - 1);
				if (currentReadBounds.yMax() > readBounds.yMax())
				{
					currentReadBounds.setYMax( readBounds.yMax());
				}
				currentReadBounds.setXMin( readBounds.xMin() + xBlock * m_axisLimit);
				currentReadBounds.setXMax( currentReadBounds.xMin() + m_axisLimit - 1);
				if (currentReadBounds.xMax() > readBounds.xMax())
				{
					currentReadBounds.setXMax( readBounds.xMax());
				}

				byte *currentBuffer = 0;
				PYXRect2DInt currentBufferBounds;
				int width = currentReadBounds.width();
				int height = currentReadBounds.height();
				//Trace_debug( "About to read " << currentReadBounds.width() << " by " << currentReadBounds.height());
				loadBuffer( &currentBuffer, &currentBufferBounds, currentReadBounds);
				//Trace_debug( "Finished reading " << currentReadBounds.width() << " by " << currentReadBounds.height());

				if (currentBuffer == 0)
					return false;

				// Now copy the data into the output buffer....
				for (int x = currentReadBounds.xMin(); x < currentReadBounds.xMax(); ++x)
					for (int y = currentReadBounds.yMin(); y < currentReadBounds.yMax(); ++y)
					{
						memcpy( 
							pBuffer + nPixelBytes * (x - readBounds.xMin() + (readBounds.width() + 1) * (y - readBounds.yMin())), // destination
							currentBuffer + nPixelBytes * (x - currentReadBounds.xMin() + (currentReadBounds.width() + 1) * (y - currentReadBounds.yMin())),
							nPixelBytes);
					}

					delete [] currentBuffer;
			}
		}		
	}
	else
	{
		// fill in the buffers
		int nError = fillBuffer(nBandBytes, nLineBytes, nPixelBytes, (GByte*)pBuffer, readBounds);
		int nRetryCount = 0;

		// Retry on error.
		while (nError != CE_None && nRetryCount < knMaxRetryCount)
		{
			Sleep(knWaitBetweenRetries);
			nError = fillBuffer(nBandBytes, nLineBytes, nPixelBytes, (GByte*)pBuffer, readBounds);
			nRetryCount++;
		}

		if (nError != CE_None && nRetryCount == knMaxRetryCount)
		{
			std::string errorMessage = CPLGetLastErrorMsg();
			PYXTHROW(	GDALProcessException, "GDAL error, " << errorMessage);
		}
	}

	return true;
}

void GDALXYCoverage::loadBuffer (byte** ppBuffer,	PYXRect2DInt* bufferBounds, const PYXRect2DInt& readBounds) const
{
	// lock access to the buffers.
	boost::recursive_mutex::scoped_lock lock(m_bufferMutex);
	// the size of one data value in a band.
	int nPixelBytes = GDALGetDataTypeSize( m_nGDALDataType ) / 8;
	int nLineBytes = nPixelBytes * (readBounds.width() + 1);
	int nBandBytes = nLineBytes * (readBounds.height() + 1);

	// TODO: Adjust the size of the reading buffers to be a multiple of the offset
	// in the gdal dataset.  gdal says this can be a good idea for speed.

	// allocate m_nBandCount arrays of reading buffers of sufficient size.
	// Check for a reasonable size buffer to load
	byte* pTempBuffer;
	if (m_limitSize && nBandBytes > knBufferLimiter)
	{
		return;
	}
	try
	{
		size_t nBufSize = nBandBytes * getBandCount();
		pTempBuffer = new byte[nBufSize];
	}
	catch (std::bad_alloc&)
	{
		// If we didn't get the memory just don't buffer and we will get data one
		// piece at a time.
		return;
	}

	if ((m_axisLimit > 0) && ((readBounds.height() > m_axisLimit) || (readBounds.width() > m_axisLimit)))
	{
		for (int xBlock = 0; xBlock <= (int) (readBounds.width() / m_axisLimit); ++xBlock)
			for (int yBlock = 0; yBlock <= (int) (readBounds.height() / m_axisLimit); ++yBlock)
			{
				PYXRect2DInt currentReadBounds( readBounds);
				currentReadBounds.setYMin( readBounds.yMin() + yBlock * m_axisLimit);
				currentReadBounds.setYMax( currentReadBounds.yMin() + m_axisLimit - 1);
				if (currentReadBounds.yMax() > readBounds.yMax())
				{
					currentReadBounds.setYMax( readBounds.yMax());
				}
				currentReadBounds.setXMin( readBounds.xMin() + xBlock * m_axisLimit);
				currentReadBounds.setXMax( currentReadBounds.xMin() + m_axisLimit - 1);
				if (currentReadBounds.xMax() > readBounds.xMax())
				{
					currentReadBounds.setXMax( readBounds.xMax());
				}

				byte *currentBuffer = 0;
				PYXRect2DInt currentBufferBounds;
				int width = currentReadBounds.width();
				int height = currentReadBounds.height();
				//Trace_debug( "About to read " << currentReadBounds.width() << " by " << currentReadBounds.height());
				loadBuffer( &currentBuffer, &currentBufferBounds, currentReadBounds);
				//Trace_debug( "Finished reading " << currentReadBounds.width() << " by " << currentReadBounds.height());

				// Now copy the data into the output buffer....				
				for (int x = currentReadBounds.xMin(); x < currentReadBounds.xMax(); ++x)
					for (int y = currentReadBounds.yMin(); y < currentReadBounds.yMax(); ++y)
					{
						memcpy( 
							pTempBuffer + nPixelBytes * (x - readBounds.xMin() + (readBounds.width() + 1) * (y - readBounds.yMin())), // destination
							currentBuffer + nPixelBytes * (x - currentReadBounds.xMin() + (currentReadBounds.width() + 1) * (y - currentReadBounds.yMin())),
							nPixelBytes);
					}

					delete [] currentBuffer;
			}
	}
	else
	{
		// fill in the buffers
		int nError = fillBuffer(nBandBytes, nLineBytes, nPixelBytes, (GByte*)pTempBuffer, readBounds);
		int nRetryCount = 0;

		// Retry on error.
		while (nError != CE_None && nRetryCount < knMaxRetryCount)
		{
			Sleep(knWaitBetweenRetries);
			nError = fillBuffer(nBandBytes, nLineBytes, nPixelBytes, (GByte*)pTempBuffer, readBounds);
			nRetryCount++;
		}

		if (nError != CE_None && nRetryCount == knMaxRetryCount)
		{
			std::string errorMessage = CPLGetLastErrorMsg();
			delete[] pTempBuffer;
			PYXTHROW(	GDALProcessException, "GDAL error, " << errorMessage);
		}
	}

	// Get rid of the old one.
	if (*ppBuffer != 0)
	{
		delete[] *ppBuffer;
		*ppBuffer = 0;
	}

	// Remember what data is in the buffer.
	*bufferBounds = readBounds;
	*ppBuffer = pTempBuffer;
}

void GDALXYCoverage::readBuffer (byte* pReadBuffer, const PYXRect2DInt& bufferBounds, const PYXCoord2DInt& raster) const
{
	byte* pMoveto = (byte *)m_pGDALBuffer;

	int nPixelBytes = GDALGetDataTypeSize( m_nGDALDataType ) / 8;
	int nLineBytes = nPixelBytes * (bufferBounds.width() + 1);
	int nBandBytes = nLineBytes * (bufferBounds.height() + 1);

	byte* pMoveFrom = pReadBuffer;

	// the x offset
	pMoveFrom += (raster.x() - bufferBounds.xMin()) * nPixelBytes;

	// the Y offset
	pMoveFrom += (raster.y() - bufferBounds.yMin()) * nLineBytes;

	for (int nBand = 1; nBand <= getBandCount(); ++nBand)
	{
		memcpy(pMoveto, pMoveFrom, nPixelBytes);
		pMoveto += nPixelBytes;
		pMoveFrom += nBandBytes;
	}
}


void GDALXYCoverage::tileLoadHint (const PYXTile& tile) const
{
	return;
}

int GDALXYCoverage::fillBuffer(int nBandBytes, int nLineBytes, int nPixelBytes, GByte* pabyBandData, 
							   const PYXRect2DInt& readBounds) const
{
		int result = CE_None;
	for (int nBand = 0; nBand < getBandCount(); nBand ++)
		{
			result = getRasterBand(nBand+1)->RasterIO(GF_Read, readBounds.xMin(), readBounds.yMin(),
				readBounds.width() + 1, readBounds.height() + 1, (void *) (pabyBandData + nBandBytes*nBand),
				readBounds.width() + 1, readBounds.height() + 1,
				m_nGDALDataType, nPixelBytes, nLineBytes);

			if (result != CE_None)
				break;
		}
		return result;
	}

void GDALXYCoverage::tileLoadDoneHint (const PYXTile& tile) const
{
	clearBuffers();
}

void GDALXYCoverage::geometryHint(PYXPointer<PYXGeometry> spGeom)
{
	m_limitSize = false;
}

//!  Frees up all cached data (large reads).
void GDALXYCoverage::clearBuffers() const
{
	// lock access to the buffers.
	boost::recursive_mutex::scoped_lock lock(m_bufferMutex);
	m_readBuffers.freeLargeBuffers();

}

void GDALXYCoverage::endGeometryHint(PYXPointer<PYXGeometry> spGeom)
{
	clearBuffers();
	m_limitSize = true;
}

PYXPointer<GDALXYCoverage::Buffer> GDALXYCoverage::findContainingBuffer( const PYXRect2DInt& bufferBounds) const
{
	return m_readBuffers.findContainingBuffer(bufferBounds);
}

PYXPointer<GDALXYCoverage::Buffer> GDALXYCoverage::getReadBuffer( const PYXRect2DInt& bufferBounds) const
{	
	boost::recursive_mutex::scoped_lock lock(m_bufferMutex);

	PYXPointer<Buffer> buffer = findContainingBuffer(bufferBounds);

	if (!buffer)
	{
		//if buffer allocations failed - the returned value is null
		buffer = Buffer::create(bufferBounds, m_nGDALDataType, getBandCount());

		if (buffer && loadBuffer(buffer))
		{
			m_readBuffers.add(buffer);
		}
	}

	return buffer;
}



////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

/*!
Get a pointer to the raster band specified for the overview being used for this data source.

\param nBand	Number of the band to be returned [1..n].

\return	Pointer to raster band (based on the overview specified for this data source).
*/
GDALRasterBand* GDALXYCoverage::getRasterBand(int nBand) const
{
	if (!m_vecSelectedBands.empty())
	{
		nBand = m_vecSelectedBands[nBand - 1];
	}

	// if no overview specified - do default handling.
	if (m_nOverview == -1)
	{
		return m_pGDALDataSource->get()->GetRasterBand(nBand);
	}

	return m_pGDALDataSource->get()->GetRasterBand(nBand)->GetOverview(m_nOverview);
}

bool GDALXYCoverage::hasOverview()
{
	int count = m_pGDALDataSource->get()->GetRasterBand(1)->GetOverviewCount();
	return count > 0 && m_nOverview + 1 < count;
}

//! open the next overview for this coverage.
PYXPointer<GDALXYCoverage> GDALXYCoverage::openOverview()
{
	PYXPointer<GDALXYCoverage> overview(new GDALXYCoverage());

	overview->m_spCovDefn = PYXTableDefinition::create();
	overview->m_spDefn = PYXTableDefinition::create();
	overview->setOverview(m_nOverview+1);
	overview->m_coordConverter = m_coordConverter;
	overview->m_bHasSRS = true;
	overview->open(m_pGDALDataSource, std::vector<int>());
	overview->m_metaDataGDAL.initialize(*m_pGDALDataSource->get(), overview->m_nOverview);
	overview->readFile();

	return overview;
}

/*!
Open the dataset.

\param pGDALDataSource	The data source to be opened.
\param vecBands			The bands to be opened.
*/
void GDALXYCoverage::open(PYXPointer<PYXSharedGDALDataSet> pGDALDataSource, const std::vector<int>& vecBands)
{
	TRACE_DEBUG("Opening GDAL dataset with GDALDatasetReader.");

	/*
	Extract the first band out of the passed data set and base all member
	variables on the assumption that the bands are uniform in size and type.
	Use the first band to extract the properties.
	*/
	m_pGDALDataSource = pGDALDataSource;

	m_vecSelectedBands.clear();
	if (vecBands.empty())
	{
		int nBands = m_pGDALDataSource->get()->GetRasterCount();
		switch (nBands)
		{
		case 3:
		case 4:
			// take bands as rgb(a)
			m_vecSelectedBands.resize(nBands);
			std::iota(m_vecSelectedBands.begin(), m_vecSelectedBands.end(), 1);
			break;

		default:
			// take the first band
			m_vecSelectedBands.push_back(1);
			break;
		}
	}
	else if (3 == vecBands.size() || 4 == vecBands.size())
	{
		m_vecSelectedBands = vecBands;
	}
	else
	{
		// take the first of the selected bands
		m_vecSelectedBands.push_back(vecBands[0]);
	}

	GDALRasterBand* pBand = getRasterBand(knMinBandOffset);
	if (pBand == 0)
	{
		PYXTHROW(GDALProcessException, "Invalid band.");
	}

	// store relevant data source information.
	m_nBandXSize = pBand->GetXSize();
	m_nBandYSize = pBand->GetYSize();
	pBand->GetBlockSize(&m_nBlockXSize, &m_nBlockYSize);
	m_dataRasterBounds = PYXRect2DInt(0, 0, m_nBandXSize - 1, m_nBandYSize - 1);

	m_nGDALDataType = pBand->GetRasterDataType();
	m_nDataType = mapType(pBand->GetRasterDataType());
	m_nContext = calcContext(m_pGDALDataSource, pBand, getBandCount());

	if (getBandCount() == 1)
	{
		m_pColourTable = pBand->GetColorTable();
	}
	else
	{
		m_pColourTable = 0;
	}

	// After assuming that all the bands were uniform, verify the assumption
	try
	{
		verifyCompatibleBands();
	}
	catch (PYXException& e)
	{
		PYXRETHROW(	e, GDALProcessException, "Bands in GDAL Raster source are not the same.");
	}

	// initialize the metadata - this may change the band count
	initMetaData();

	// create the buffer for data values
	m_nValueByteSize = bytesForType(m_nGDALDataType) * getBandCount();
	m_pGDALBuffer = new char[m_nValueByteSize];
	memset(m_pGDALBuffer, 0, m_nValueByteSize);

	if (m_pNullValue == 0)
	{
		// determine the null value for the source and store it
		determineNullValue();
	}
}

/*! 
Verify that all bands in the data set are compatible with the stored values.  
If the bands are not compatible an exception is thrown.
*/
void GDALXYCoverage::verifyCompatibleBands()
{
	assert(m_pGDALDataSource->get() != 0 && "Reader not initialized.");

	// cycle through each of the bands
	for (int nBand = 1; nBand <= getBandCount(); ++nBand)
	{
		// verify band existance
		GDALRasterBand* pBand = getRasterBand(nBand);
		if (pBand == 0)
		{
			PYXTHROW(	GDALProcessException, "Band '" << nBand << "' of '" << 
				m_pGDALDataSource->get()->GetRasterCount() << "' is invalid."	);
		}

		// band sizes
		if (m_nBandXSize != pBand->GetXSize())
		{
			PYXTHROW(	GDALProcessException, "Band '" << nBand << "' with size '" <<
				pBand->GetXSize() << 
				"' not compatible with other bands of size '" << 
				m_nBandXSize << "'."	);
		}
		if (m_nBandYSize != pBand->GetYSize())
		{
			PYXTHROW(	GDALProcessException, "Band '" << nBand << "' with size '" <<
				pBand->GetYSize() << 
				"' not compatible with other bands of size '" << 
				m_nBandYSize << "'."	);
		}

		// verify the gdal data type
		if (m_nGDALDataType != pBand->GetRasterDataType())
		{
			PYXTHROW(	GDALProcessException, "Band '" << nBand << 
				"' with GDAL type '" << 
				static_cast<int>(pBand->GetRasterDataType()) <<
				"' not compatible with other bands of type '" << 
				static_cast<int>(pBand->GetRasterDataType()) << "'."	);	

		}

		// band data type
		PYXValue::eType nType = mapType(pBand->GetRasterDataType());
		if (m_nDataType != nType)
		{
			PYXTHROW(	GDALProcessException, "Band '" << nBand << 
				"' with data eAttributeType '" << 
				static_cast<int>(nType) <<
				"' not compatible with other bands of type '" << 
				static_cast<int>(m_nDataType) << "'."	);	
		}

		/*
		Do not need to re-calculate the context since it is based on the data
		type.  As long as the data type is the same the context is the same.
		*/
	}
}

/*!
Determine and store the null value for the data set
*/
void GDALXYCoverage::determineNullValue()
{
	assert(m_pGDALDataSource->get() != 0 && "Reader not initialized.");
	assert(m_nDataType != PYXValue::knNull && "Data type not known.");
	assert(getBandCount() != 0 && "No bands in dataset");
	assert(m_pNullValue == 0 && "Null value already initialized");

	// allocate a buffer for the null value
	m_pNullValue = new char[m_nValueByteSize];

	// cycle through each of the bands
	for (int nBand = 1; nBand <= getBandCount(); ++nBand)
	{
		// verify band existance
		GDALRasterBand* pBand = getRasterBand(nBand);
		if (pBand == 0)
		{
			PYXTHROW(	GDALProcessException, 
				"Invalid band retrieved from dataset."	);
		}

		// extract the null value
		int nSuccess = 0;
		double fNullValue = pBand->GetNoDataValue(&nSuccess);
		if (nSuccess != 0)
		{
			// store the null value in the null value buffer
			storeNullValueForBand(fNullValue, nBand);
		}
		else
		{
			delete[] m_pNullValue;
			m_pNullValue = 0;
		}
	}
}

void GDALXYCoverage::forceNullValue(const PYXValue & nullValue)
{
	m_forcedNullValue = nullValue;	
}

//! Tracks limit on download size, in axis units.  Used for EOX's weird WCS implementation. -1 means no such limit.
void GDALXYCoverage::setAxisLimit( int limit)
{
	m_axisLimit = limit;
}

/*!
Store the null value for an individual band in the dataset.  The value is 
stored in the correct format within the allocated null value buffer.

\param	fNullValue	The null value
\param	nBand		The band (1-based)
*/
void GDALXYCoverage::storeNullValueForBand(double fNullValue, int nBand)
{
	assert(m_pNullValue != 0 && "Null value buffer not allocated.");
	assert(0 < nBand && nBand <= getBandCount() && "Invalid band count.");

	// get 0-based index for band
	int nIndex = nBand - 1;

	switch (m_nGDALDataType)
	{
	case GDT_Byte:
		{
			// convert value from double to unsigned char.
			unsigned char cValue = static_cast<unsigned char>(fNullValue);
			unsigned char* cArray = static_cast<unsigned char*>(m_pNullValue);
			cArray[nIndex] = cValue;
			break;
		}

	case GDT_UInt16:
		{
			// convert value from double to int.
			unsigned short nValue = static_cast<unsigned short>(fNullValue);
			unsigned short* nArray = static_cast<unsigned short*>(m_pNullValue);
			nArray[nIndex] = nValue;
			break;
		}

	case GDT_Int16:
		{
			// convert value from double to int.
			short nValue = static_cast<short>(fNullValue);
			short* nArray = static_cast<short*>(m_pNullValue);
			nArray[nIndex] = nValue;
			break;
		}

	case GDT_UInt32:
		{
			// convert value from double to unsigned int.
			unsigned int nValue = static_cast<unsigned int>(fNullValue);
			unsigned int* nArray = static_cast<unsigned int*>(m_pNullValue);
			nArray[nIndex] = nValue;
			break;
		}

	case GDT_Int32:
		{
			// convert value from double to int.
			int nValue = static_cast<int>(fNullValue);
			int* nArray = static_cast<int*>(m_pNullValue);
			nArray[nIndex] = nValue;
			break;
		}

	case GDT_Float32:
		{
			// convert value from double to float.
			float fValue = static_cast<float>(fNullValue);
			float* fArray = static_cast<float*>(m_pNullValue);
			fArray[nIndex] = fValue;
			break;	
		}

	case GDT_Float64:
		{
			double fValue = fNullValue;
			double* fArray = static_cast<double*>(m_pNullValue);
			fArray[nIndex] = fValue;
			break;	
		}

	case GDT_CInt16:
		{
			// convert value from double to int.
			short nValue = static_cast<short>(fNullValue);
			short* nArray = static_cast<short*>(m_pNullValue);
			nArray[nIndex * 2] = nValue;
			nArray[nIndex * 2 + 1] = nValue;
			break;
		}

	case GDT_CInt32:
		{
			// convert value from double to int.
			int nValue = static_cast<int>(fNullValue);
			int* nArray = static_cast<int*>(m_pNullValue);
			nArray[nIndex * 2] = nValue;
			nArray[nIndex * 2 + 1] = nValue;
			break;
		}

	case GDT_CFloat32:
		{
			// convert value from double to float.
			float fValue = static_cast<float>(fNullValue);
			float* fArray = static_cast<float*>(m_pNullValue);
			fArray[nIndex * 2] = fValue;
			fArray[nIndex * 2 + 1] = fValue;
			break;	
		}

	case GDT_CFloat64:
		{
			double fValue = fNullValue;
			double* fArray = static_cast<double*>(m_pNullValue);
			fArray[nIndex * 2] = fValue;
			fArray[nIndex * 2 + 1] = fValue;
			break;	
		}

	default:
		{
			PYXTHROW(	GDALProcessException,
				"Unsupported data type: '" << m_nDataType << "'."	);
		}
	}
}

/*!
Get the value for the given raster coordinates.

\param	raster	The raster coordinates.
\param  pValue  A pointer to the PYXValue to be filled in.

\return	True if a data value was retrieved, false if there was no value to return.
*/
bool GDALXYCoverage::getDataValue(const PYXCoord2DInt& raster, PYXValue* pValue) const
{
	if (!m_dataRasterBounds.inside(raster))
	{
		// out of bounds so return no data
		return false;
	}

	bool bGotBuffer = false;
	{
		// lock the buffer access while we look at them
		boost::recursive_mutex::scoped_lock lock(m_bufferMutex);

		auto buffer = m_readBuffers.findContainingBuffer(raster);
		bGotBuffer = buffer->read(raster,(byte*)m_pGDALBuffer);		
	}

	if (!bGotBuffer)
	{
		// get it the old way...
		int nError = CE_None;
		// fetch the value from the raster
		int nPixelSpace = GDALGetDataTypeSize( m_nGDALDataType ) / 8;

		GByte *pabyBandData = (GByte *) m_pGDALBuffer;
		for (int nBand = 1; nBand <= getBandCount(); ++nBand)
		{
			GDALRasterBand *poBand = getRasterBand(nBand);
			assert((poBand != 0) && "GDAL raster band is null.");

			nError = poBand->RasterIO( GF_Read, raster.x(), raster.y(), 1, 1, 
				(void *) pabyBandData, 1, 1,
				m_nGDALDataType, nPixelSpace, nPixelSpace );
			if (nError != CE_None)
			{
				std::string errMsg(CPLGetLastErrorMsg());
				PYXTHROW(	GDALProcessException, "GDAL error '" << nError << 
					"' occurred during value extraction at coord '" <<
					raster << "'."	);
			}
			pabyBandData += nPixelSpace;
		}
	}

	// Determine if the null value is equal to the retrieved value.
	if (m_pNullValue && memcmp(m_pGDALBuffer, m_pNullValue, m_nValueByteSize) == 0)
	{
		return false;
	}

	// Determine if the forced null value is equal to the retrieved value.
	if (m_forcedNullValue.getType() != PYXValue::knNull && memcmp(m_pGDALBuffer, m_forcedNullValue.getPtr(0), m_nValueByteSize) == 0)
	{
		return false;
	}

	memcpy(pValue->getPtr(0), m_pGDALBuffer, m_nValueByteSize);
	return true;
}

/*!
Determine the data type for a specific band of the data source.  If the data 
type can not be determined an exception is thrown.

\param	pGDALDataSource	The GDAL data source.
\param	pBand			The band to investigate for data type.
\param	nBandCount		The number of bands in the source.

\return The context for the band.
*/
PYXFieldDefinition::eContextType GDALXYCoverage::calcContext(
	PYXPointer<PYXSharedGDALDataSet> pGDALDataSource,
	GDALRasterBand* pBand,
	int nBandCount	)
{
	assert(pBand != NULL && "Band must be non-null.");

	// ensure that we have at least one band
	if (nBandCount <= 0)
	{
		PYXTHROW(GDALProcessException, "Data set has no bands.");
	}

	// the context to return to the caller
	PYXFieldDefinition::eContextType nContext = PYXFieldDefinition::knContextNone;

	switch (nBandCount)
	{
		/* 
		If it has only one band then it is either Grey Scale or
		it is using a color table or it is elevation data.
		*/
	case 1:
		{
			// determine if it is an elevation data set by checking the GDAL driver name
			if (pGDALDataSource->isElevation())
			{
				nContext = PYXFieldDefinition::knContextElevation;
			}
			else
			{
				// Determine if a colour table is being used.
				GDALColorTable* pColourTable = pBand->GetColorTable();
				if (0 == pColourTable || GDT_Byte != pBand->GetRasterDataType())
				{
					nContext = PYXFieldDefinition::knContextGreyScale;
				}
				else
				{
					// get the palette interpretation
					GDALPaletteInterp nPalette = pColourTable->GetPaletteInterpretation();

					// TODO: this area of code needs testing.
					switch (nPalette)
					{
					case GPI_Gray:
					case GPI_RGB:
						{
							nContext = PYXFieldDefinition::knContextCLUT;
							break;
						}

					case GPI_CMYK:
					case GPI_HLS:
					default:
						{
							PYXTHROW(GDALProcessException, "Unsupported color palette.");
							break;
						}
					}
				}
			}
			break;
		}

		/* If it has three bands then it can be either RGB, or HLS
		the band must be sampled to determine what it is 
		*/
	case 3:
		{
			GDALColorInterp nColor = pBand->GetColorInterpretation();
			switch (nColor)
			{
				// Check to see if it is RGB
			case GCI_RedBand:
			case GCI_GreenBand:
			case GCI_BlueBand:
			case GCI_Undefined:
				{
					nContext = PYXFieldDefinition::knContextRGB;
					break;
				}

				// Check to see if it is HLS
			case GCI_HueBand:
			case GCI_LightnessBand:
			case GCI_SaturationBand:
			default:
				{
					PYXTHROW(GDALProcessException, "Unsupported color palette.");
					break;
				}
			}
			break;
		}

		/* If it has four bands then it can be either CMYK, or RGBA
		the band must be sampled to determine what it is. 
		*/
	case 4:
		{
			GDALColorInterp nColor = pBand->GetColorInterpretation();
			switch (nColor)
			{
				// Check to see if it is RGB
			case GCI_RedBand:
			case GCI_GreenBand:
			case GCI_BlueBand:
			case GCI_Undefined:
				{
					TRACE_INFO("Guessing that this datasource is an RGB");
					nContext = PYXFieldDefinition::knContextRGB;
					break;
				}

			case GCI_CyanBand:
			case GCI_MagentaBand:
			case GCI_YellowBand:
			case GCI_BlackBand:
			default:
				{
					PYXTHROW(GDALProcessException, "Unsupported color palette.");
				}
			}
			break;
		}

	default:
		{
			PYXTHROW(GDALProcessException, "Invalid number of bands.");
		}
	}

	return nContext;
}

/*!
Calculate spatial precision for projection coordinates.

\return	The spatial precision in metres.
*/
double GDALXYCoverage::calcSpatialPrecisionProjected() const
{	
	double fPixelWidth = fabs(m_metaDataGDAL.getPixelWidth());
	double fPixelHeight = fabs(m_metaDataGDAL.getPixelHeight());
	double fSpatialAccuracy = -1.0;

	if (fPixelWidth > fPixelHeight)
	{
		fSpatialAccuracy = fPixelWidth / 2.0;
	}
	else
	{
		fSpatialAccuracy = fPixelHeight / 2.0;
	}

	return fSpatialAccuracy;
}

/*!
Calculate the spatial precision by taking half the worst case distance between
posts.

\param pDatum	The horizontal datum used to generate the data.

\return The spatial precision in metres.
*/
double GDALXYCoverage::calcSpatialPrecisionGeographical(HorizontalDatum const * pDatum) const
{
	double fDelta;
	CoordLatLon pt1;
	CoordLatLon pt2;

	// the worst case latitude distance occurs at the most extreme latitude
	double fPixelHeight = fabs(m_metaDataGDAL.getPixelHeight());
	fDelta = MathUtils::degreesToRadians(fPixelHeight);

	if (fabs(m_metaDataGDAL.getSouthWest().y()) > fabs(m_metaDataGDAL.getNorthEast().y()))
	{
		// data set is mostly in southern hemisphere
		pt1 = CoordLatLon(m_metaDataGDAL.getSouthWest().y(), m_metaDataGDAL.getSouthWest().x());
		pt2 = CoordLatLon(pt1.lat() + fDelta, pt1.lon());
	}
	else
	{
		// data set is mostly in northern hemisphere
		pt1 = CoordLatLon(m_metaDataGDAL.getNorthEast().y(), m_metaDataGDAL.getNorthEast().x());
		pt2 = CoordLatLon(pt1.lat() - fDelta, pt1.lon());
	}

	double fLatDistance = pDatum->calcDistance(pt1, pt2);

	// the largest longitude distance occurs at the smallest latitude
	double fPixelWidth = fabs(m_metaDataGDAL.getPixelWidth());
	fDelta = MathUtils::degreesToRadians(fPixelWidth);
	if (m_metaDataGDAL.getSouthWest().y() >= 0.0)
	{
		/* 
		data set is in northern hemisphere
		get the point closest to the equator
		*/
		pt1 = CoordLatLon(m_metaDataGDAL.getSouthWest().y(), m_metaDataGDAL.getSouthWest().x());
		pt2 = CoordLatLon(pt1.lat(), pt1.lon() + fDelta);
	}
	else if (m_metaDataGDAL.getNorthEast().y() <= 0.0)
	{
		/* 
		data set is in southern hemisphere
		get the point closest to the equator
		*/
		pt1 = CoordLatLon(m_metaDataGDAL.getNorthEast().y(), m_metaDataGDAL.getNorthEast().x());
		pt2 = CoordLatLon(pt1.lat(), pt1.lon() + fDelta);
	}
	else
	{
		// data set straddles the equator
		// use the equator for the largest longitudinal distance
		pt1.setLat(0.0);
		pt1.setLon(0.0);

		pt2 = CoordLatLon(pt1.lat(), pt1.lon() + fDelta);
	}

	double fLonDistance = pDatum->calcDistance(pt1, pt2);

	return std::max(fLatDistance, fLonDistance) / 2.0;
}

/*!
Get the value at the specified native integer raster coordinate.

\param	raster			The native raster coordinate.
\param  pValue      an pointer to the PYXValue to be filled in

\return	True if a data value was retrieved, false if there was no value to return.
*/
bool GDALXYCoverage::getValue(const PYXCoord2DInt& raster, PYXValue* pValue) const
{
	try
	{
		if (getContext() !=
			PYXFieldDefinition::knContextCLUT)
		{
			return getDataValue(raster, pValue);
		}

		// TODO: it would be good to only create this value once and reuse it
		// instead of creating it every time.
		PYXValue clutValue = PYXValue::create(mapType(m_nGDALDataType),
			0, getBandCount(), 0);

		if (!getDataValue(raster, &clutValue))
		{
			return false;
		}

		// Return value looked up in table.
		return lookupValue(clutValue.getInt(), pValue);
	}
	catch (PYXException& e)
	{
		PYXRETHROW(e, GDALProcessException, "Unable to read file.");
	}

	return false;
}



bool GDALXYCoverage::getValue(const PYXPointer<Buffer> & buffer, const PYXCoord2DInt& raster, PYXValue* pValue) const
{
	int tempRawBufferSize = bytesForType(m_nGDALDataType) * getBandCount();
	boost::scoped_array<char> rawBuffer(new char[tempRawBufferSize]);
	memset(rawBuffer.get() , 0, tempRawBufferSize);

	try
	{
		if (!buffer->read(raster, (byte*)rawBuffer.get()))
		{
			return false;
		}

		// Determine if the null value is equal to the retrieved value.
		if (m_pNullValue && memcmp(rawBuffer.get(), m_pNullValue, m_nValueByteSize) == 0)
		{
			return false;
		}

		// Determine if the forced null value is equal to the retrieved value.
		if (m_forcedNullValue.getType() != PYXValue::knNull && memcmp(rawBuffer.get(), m_forcedNullValue.getPtr(0), m_nValueByteSize) == 0)
		{
			return false;
		}

		if (getContext() !=
			PYXFieldDefinition::knContextCLUT)
		{
			memcpy(pValue->getPtr(0), rawBuffer.get(), m_nValueByteSize);
			return true;
		}

		// TODO: it would be good to only create this value once and reuse it
		// instead of creating it every time.
		PYXValue clutValue = PYXValue::create(mapType(m_nGDALDataType),
			0, getBandCount(), 0);

		memcpy(clutValue.getPtr(0), rawBuffer.get(), m_nValueByteSize);

		// Return value looked up in table.
		return lookupValue(clutValue.getInt(), pValue);
	}
	catch (PYXException& e)
	{
		PYXRETHROW(e, GDALProcessException, "Unable to read file.");
	}

	return false;
}


/*! 
This method uses the GDAL reader associated with the data source to convert
a value into an array of 3 characters that can be interpreted as an RGB.

\param nIndex	The table index.
\param pValue	A pointer to the PYXValue to be filled in

\return	true if a data value was retrieved, false if there was no value to return.
*/
bool GDALXYCoverage::lookupValue(int nIndex, PYXValue* pValue) const
{
	assert(	getContext() == PYXFieldDefinition::knContextCLUT && 
		"Invalid field count for lookup"	);

	// get the colour table
	GDALColorTable* pColourTable = getColourTable();
	if (pColourTable == 0)
	{
		PYXTHROW(	GDALProcessException,
			"Colour table not available for source '" << 
			getID() << "'."	);
	}

	const GDALColorEntry* pColourEntry = pColourTable->GetColorEntry(nIndex);
	if (pColourEntry != 0)
	{
		// interpret the colour table result
		switch (pColourTable->GetPaletteInterpretation())
		{
			// Interpret the lookup as rgb
		case GPI_RGB:
			{
				assert(0 <= pColourEntry->c1 && pColourEntry->c1 <= 255);
				assert(0 <= pColourEntry->c2 && pColourEntry->c2 <= 255);
				assert(0 <= pColourEntry->c3 && pColourEntry->c3 <= 255);
				pValue->setUInt8(0, static_cast<uint8_t>(pColourEntry->c1));
				pValue->setUInt8(1, static_cast<uint8_t>(pColourEntry->c2));
				pValue->setUInt8(2, static_cast<uint8_t>(pColourEntry->c3));
				break;
			}

			// Interpret the lookup as greyscale
		case GPI_Gray:
			{
				assert(0 <= pColourEntry->c1 && pColourEntry->c1 <= 255);
				pValue->setUInt8(0, static_cast<uint8_t>(pColourEntry->c1));
				pValue->setUInt8(1, static_cast<uint8_t>(pColourEntry->c1));
				pValue->setUInt8(2, static_cast<uint8_t>(pColourEntry->c1));
				break;
			}

			// Interpretation is not supported
		default:
			{
				assert(false && "Invalid colour palette interpretaion.");
				PYXTHROW(	GDALProcessException,
					"Unsupported colour palette interpretation '"
					<< pColourTable->GetPaletteInterpretation()
					<< "'."	);
			}
		}

		// return with the lookup value intact
		return true;
	}

	// the value could not be resolved
	return false;
}

/*!
Open the GDAL data file.  

There are two ways that failure can occur:
1. It is not a valid GDAL data set, and
2. It is a valid GDAL data set but it had some problem opening it.

If it is not a valid GDAL data set, then the method will return false.
If it is valid but failed to open it, then an exception will be thrown.
If it is valid and was successfully opened, then the method will return true.

\param strFileName	The file to open.
\param spSRS		The spatial reference system for the data source. Can be null.
\param bands		The bands to be opened.
\param strLayerName	The layer to be opened.

\returns false if it is not a valid GDAL data set, true if it opens successfully.
*/
bool GDALXYCoverage::open(
	const std::string& strFileName,
	boost::intrusive_ptr<ISRS> spSRS,
	const std::vector<int>& bands,
	const std::string& strLayerName)
{
	m_spCovDefn = PYXTableDefinition::create();
	m_spDefn = PYXTableDefinition::create();

	std::string strTheFileName(strFileName);

	// If we specify an overview - get the number and remove from the string name for opening.
	size_t nPos = strTheFileName.find("<O=");

	int nOverview = -1;

	if (nPos != std::string::npos)
	{
		strTheFileName = strFileName.substr(0, nPos); 

		// get the overview from the URI and set it in the dataset reader.
		nOverview = atoi(strFileName.substr(nPos + 3).c_str());
		setOverview(nOverview);
	}

	// multilayer netCDF file
	std::string strExt = FileUtils::getExtensionNoDot(strFileName);
	if (	!strLayerName.empty() && (
			boost::iequals(strExt, "cdf") ||
			boost::iequals(strExt, "nc") ||
			boost::iequals(strExt, "nc2") ||
			boost::iequals(strExt, "nc3") ||
			boost::iequals(strExt, "nc4")	)	)
	{
		strTheFileName = "NETCDF:\"" + strFileName + "\":" + strLayerName;
		m_wrapX = true;
	}

	m_pGDALDataSource = PYXSharedGDALDataSet::createRaster(strTheFileName);

	// see if the GDALFileProcess was passed an SRS.
	if (spSRS)
	{
		setSpatialReferenceSystem(spSRS);
	}

	//TRACE_INFO("Opening GDAL data set '" << strFileName << "'.");

	try
	{		
		m_strID = strFileName;

		/*
		Convert the handle to a GDALDataset object pointer. This pointer must be
		deleted in order to close the file. This object will be passed into the
		GDALDatasetReader object and will be deleted by that object.
		*/

		// open the DatasetReader.  Ownership is retained by GDALDataSource
		open(m_pGDALDataSource, bands);

		// initialize the MetaData object.
		m_metaDataGDAL.initialize(*m_pGDALDataSource->get(), nOverview);

		// if SRS has not been set in GDALFileProcess
		if (!m_bHasSRS)
		{
			std::string strWKT;

			// first look for a .prj file to read the SRS from with the same base name as the input.
			// This is treated as an override for any other SRS that may have been specified 
			// in the data file itself.
			size_t nSeparatorPosition = strFileName.rfind('.', strFileName.length());
			std::string strPRJFileName = strFileName.substr(0, nSeparatorPosition + 1) + "prj";

			if (FileUtils::exists(strPRJFileName))
			{
				std::ifstream file(strPRJFileName.c_str());
				if (file)
				{
					std::stringstream strSRS;
					strSRS << file.rdbuf();
					strWKT = strSRS.str();
				}

				// if we got a new SRS from the .prj file, try and use it.
				if (!strWKT.empty())
				{
					try
					{
						m_coordConverter = new CoordConverterImpl();
						m_coordConverter->initialize(strWKT);

						m_coordConverterExternal = m_coordConverter;
						m_bHasSRS = true;
					}
					catch (PYXException& e)
					{
						PYXRETHROW(
							e, 
							MissingSRSException, 
							"Unable to open '" << strFileName << "' because it has a bad SRS in the " << strPRJFileName << " file.");
					}
				}
			}

			// if we did not find an SRS in the .prj file or if the .prj file does not exist
			// then we try to get the SRS from the data set itself.
			if (!m_bHasSRS)
			{
				strWKT = m_pGDALDataSource->get()->GetProjectionRef();
				if (strWKT.empty())
				{
					strWKT = m_pGDALDataSource->get()->GetGCPProjection();
				}
				if (!strWKT.empty())
				{
					try
					{
						m_coordConverter = new CoordConverterImpl();
						m_coordConverter->initialize(strWKT);

						m_coordConverterExternal = m_coordConverter;
						m_bHasSRS = true;
					}
					catch (PYXException& e)
					{
						PYXRETHROW(
							e, 
							MissingSRSException, 
							"Unable to open '" << strFileName << "' because it has a bad SRS.");
					}
				}
			}
		}
		readFile();
	}
	catch (MissingGeometryException& e)
	{
		PYXRETHROW(
			e, 
			MissingGeometryException, 
			"Unable to open '" << strFileName << "' because it has a missing or empty geometry.");
	}
	catch (MissingWorldFileException& e)
	{
		PYXRETHROW(
			e, 
			MissingWorldFileException, 
			"Unable to open '" << strFileName << "' because it has a missing geotransform.");
	}
	catch (MissingSRSException& e)
	{
		PYXRETHROW(
			e, 
			MissingSRSException, 
			"Unable to open '" << strFileName << "' because it is missing a spatial reference system.");
	}
	catch (PYXException& e)
	{
		PYXRETHROW(e, GDALProcessException, "Unable to open '" << strFileName << "'.");
	}

	return true;
}

/*!
Continue reading file after open and SRS has been set.
*/
void GDALXYCoverage::readFile()
{
	// set the bounds
	m_bounds.setEmpty();
	m_bounds.expand(m_metaDataGDAL.getSouthEast());
	m_bounds.expand(m_metaDataGDAL.getSouthWest());
	m_bounds.expand(m_metaDataGDAL.getNorthEast());
	m_bounds.expand(m_metaDataGDAL.getNorthWest());

	if (m_bounds.degenerate())
	{
		PYXTHROW(MissingGeometryException, "The data set has a missing or empty geometry.");
	}

	// set the step size (may be negative if data source is addressed from max to min)
	m_stepSize = PYXCoord2DDouble(	m_metaDataGDAL.getPixelWidth(), m_metaDataGDAL.getPixelHeight() );

	// tell the coordinate converter about the bounds
	if (m_coordConverter != NULL)
	{
		m_coordConverter->setNativeBounds(m_bounds);
	}

	// initialize the geometry
	double fPrecision = getSpatialPrecision();
	if (0 <= fPrecision)
	{
		// convert spatial precision in metres to a PYXIS resolution
		double fArcRadians = fPrecision / ReferenceSphere::kfRadius;
		int nResolution = SnyderProjection::getInstance()->precisionToResolution(fArcRadians);

		assert(getCoordConverter());

		// create the geometry at that resolution
		m_spGeom = PYXXYBoundsGeometry::create(getBounds(), *(getCoordConverter()), nResolution);
	}
	else
	{
		if (!m_bHasSRS)
		{
			PYXTHROW(MissingSRSException, "The data set is missing a spatial reference system.");
		}

		/*  This shouldn't ever be hit, as if there is no SRS, it is caught above when
		open(const std::string& strFileName, boost::intrusive_ptr<ISRS> spSRS) is
		executed. */
		PYXTHROW(GDALProcessException, "The data set is missing a spatial reference system. Unable to create geometry.");
	}
}

/*!
Initializes the meta data.
*/
void GDALXYCoverage::initMetaData()
{
	// Add the data set metadata
	addDataSetMetadata(this, m_pGDALDataSource, getRasterBand(knMinBandOffset), getContext());

	// The field name must be "RGB" for backward compatibility.
	addContentDefinition(getCoverageDefinition(), m_vecSelectedBands, "RGB", getContext(), m_nGDALDataType);

	// the selected bands must map to a single PYXValue or there will be memory overwrites
	m_coverageValueSpec = getCoverageDefinition()->getFieldDefinition(0).getTypeCompatibleValue();
}

/*!
Add the metadata for the data set.

\param pRecord		The record associated with the data set
\param pGDALDataSet	The GDAL data set
\param pBand		The raster band (ownership retained by caller)
\param nContext		The context of the band
*/
void GDALXYCoverage::addDataSetMetadata(
	IRecord* pRecord,
	PYXPointer<PYXSharedGDALDataSet> pGDALDataSet,
	GDALRasterBand* pBand,
	PYXFieldDefinition::eContextType nContext)
{
	char** papszMetadata;
	bool bIsGrib = pGDALDataSet->isGRIB();
	bool bIsNetCDF = pGDALDataSet->isNetCDF();
	std::string strNetCDFTimeReference;

	// add data set metadata
	papszMetadata = pGDALDataSet->get()->GetMetadata(NULL);
	if (CSLCount(papszMetadata) > 0)
	{
		for (auto i = 0; papszMetadata[i] != NULL; i++)
		{
			char* pszKey;
			const char* pszValue = CPLParseNameValue(papszMetadata[i], &pszKey);

			// add name value pair to data set
			pRecord->addField(
				pszKey,
				PYXFieldDefinition::knContextNone,
				PYXValue::knString,
				1,
				PYXValue(pszValue));

			if (bIsNetCDF && boost::iequals(pszKey, "time#units"))
			{
				strNetCDFTimeReference = pszValue;
			}
		}
	}

	// add band metadata
	papszMetadata = pBand->GetMetadata(NULL);
	if (CSLCount(papszMetadata) > 0)
	{
		for (auto i = 0; papszMetadata[i] != NULL; i++)
		{
			char* pszKey;
			const char* pszValue = CPLParseNameValue(papszMetadata[i], &pszKey);

			// add name value pair to data set
			pRecord->addField(
				pszKey,
				PYXFieldDefinition::knContextNone,
				PYXValue::knString,
				1,
				PYXValue(pszValue));

			// handle special fields
			if (bIsGrib)
			{
				handleSpecialGRIBFields(pRecord, pszKey, pszValue);
			}
			else if (bIsNetCDF)
			{
				handleSpecialNetCDFFields(pRecord, pszKey, pszValue, strNetCDFTimeReference);
			}

			CPLFree(pszKey);
		}
	}

	// calculate the number of pixels in the data set to give an estimate of its size.
	double fPixels = ((double) pGDALDataSet->get()->GetRasterXSize()) *
		pGDALDataSet->get()->GetRasterYSize();
	pRecord->addField(
		PYXDataSet::s_strPyxisPixelCount,
		PYXFieldDefinition::knContextNone,
		PYXValue::knDouble,
		1,
		PYXValue(fPixels));

	if (PYXFieldDefinition::knContextElevation == nContext)
	{
		pRecord->addField(
			"elev hint",
			PYXFieldDefinition::knContextNone,
			PYXValue::knBool,
			1,
			PYXValue(true));
	}
}

/*!
Handle the special GRIB metadata fields and record them with PYXIS keys.

\param pRecord	The record associated with the data set
\param pszKey	The GRIB key
\param pszValue	The GRIB value
*/
void GDALXYCoverage::handleSpecialGRIBFields(
	IRecord* pRecord,
	const char* pszKey,
	const char* pszValue)
{
	if (boost::iequals(pszKey, "GRIB_VALID_TIME"))
	{
		// time when the measurement was taken, should be of the format #### sec UTC
		std::cmatch results;
		std::regex regexTime("^\\s*(\\d+)\\s*sec\\s*UTC\\s*$", std::regex::icase);
		if (std::regex_match(pszValue, results, regexTime))
		{
			long nTime = atol(results[1].str().c_str());
			pRecord->addField(
				PYXDataSet::s_strPyxisDimensionTime,
				PYXFieldDefinition::knContextNone,
				PYXValue::knInt32,
				1,
				PYXValue(nTime));
		}
	}
	else if (boost::iequals(pszKey, "GRIB_COMMENT"))
	{
		// record as the long name
		pRecord->addField(
			PYXDataSet::s_strPyxisLongName,
			PYXFieldDefinition::knContextNone,
			PYXValue::knString,
			1,
			PYXValue(pszValue));

	}
	else if (boost::iequals(pszKey, "GRIB_SHORT_NAME"))
	{
		// record as the short name
		pRecord->addField(
			PYXDataSet::s_strPyxisShortName,
			PYXFieldDefinition::knContextNone,
			PYXValue::knString,
			1,
			PYXValue(pszValue));

		// height above ground level where the measurement was taken, should be of the format ####-HTGL
		std::cmatch results;
		std::regex regexHeight("^(\\d+)\\-HTGL\\s*$", std::regex::icase);
		if (std::regex_match(pszValue, results, regexHeight))
		{
			int nHeight = atoi(results[1].str().c_str());
			pRecord->addField(
				PYXDataSet::s_strPyxisDimensionHeight,
				PYXFieldDefinition::knContextNone,
				PYXValue::knInt32,
				1,
				PYXValue(nHeight));
		}
	}
	else if (boost::iequals(pszKey, "GRIB_PDS_TEMPLATE_NUMBERS"))
	{
		// model number, should be the 27th number in a space separated list of 28 numbers
		std::cmatch results;
		std::regex regexModel("^\\s*(?:\\d+\\s*){26}\\s*(\\d+)\\s*\\d+\\s*$");
		if (std::regex_match(pszValue, results, regexModel))
		{
			int nModel = atoi(results[1].str().c_str());
			pRecord->addField(
				PYXDataSet::s_strPyxisDimensionGRIBModel,
				PYXFieldDefinition::knContextNone,
				PYXValue::knInt32,
				1,
				PYXValue(nModel));
		}
	}
	else if (boost::iequals(pszKey, "GRIB_UNIT"))
	{
		// units, may be enclosed in square braces
		std::cmatch results;
		std::regex regexUnits("^\\s*\\[?([^\\]\\s]+)\\]?\\s*$");
		if (std::regex_match(pszValue, results, regexUnits))
		{
			auto strUnits = results[1].str();
			pRecord->addField(
				PYXDataSet::s_strPyxisUnits,
				PYXFieldDefinition::knContextNone,
				PYXValue::knString,
				1,
				PYXValue(strUnits));
		}
	}
}

/*!
Handle the special netCDF metadata fields and record them with PYXIS keys.

\param pRecord			The record associated with the data set
\param pszKey			The netCDF key
\param pszValue			The netCDF value
\param strTimeReference	The time reference from the netCDF metadata
*/
void GDALXYCoverage::handleSpecialNetCDFFields(
	IRecord* pRecord,
	const char* pszKey,
	const char* pszValue,
	const std::string& strTimeReference)
{
	if (boost::iequals(pszKey, "NETCDF_DIM_time") && !strTimeReference.empty())
	{
		// look for interval and reference for the time dimension
		//	should look like: interval since datetime timezone
		std::cmatch results;
		std::string strOffset("((year)|(month)|(day)|(hour)|(minute)|(second))s?");
		std::string strBase("(\\d{1,4})-(\\d{1,2})-(\\d{1,2})[T ]((\\d{2}):(\\d{2}):(\\d+(\\.\\d+)?)\\s*(Z|(UTC))?)?");
		std::regex regexIntervalAndBase("^\\s*" + strOffset + "\\s*since\\s*" + strBase + "\\s*$", std::regex::icase);
		if (std::regex_match(strTimeReference.c_str(), results, regexIntervalAndBase))
		{
			std::string strInterval(results[1].str());

			struct tm tm;
			tm.tm_isdst = 0;
			tm.tm_year = std::max(atoi(results[8].str().c_str()) - 1900, 70);
			tm.tm_mon = std::max(atoi(results[9].str().c_str()) - 1, 0);
			tm.tm_mday = std::max(atoi(results[10].str().c_str()), 1);

			if (results.length() > 14)
			{
				tm.tm_hour = atoi(results[12].str().c_str());
				tm.tm_min = atoi(results[13].str().c_str());

				// we only resolve to the nearest second for now
				tm.tm_sec = MathUtils::round(atof(results[14].str().c_str()));
			}

			// time offset when the measurement was taken, should be of the format ####
			std::cmatch results;
			std::regex regexTime("^\\s*(\\d+)\\s*$", std::regex::icase);
			if (std::regex_match(pszValue, results, regexTime))
			{
				int nOffset = atoi(results[1].str().c_str());

				// apply the time offset to the base time
				if (boost::iequals(strInterval, "year"))
				{
					tm.tm_year += nOffset;
				}
				else if (boost::iequals(strInterval, "month"))
				{
					tm.tm_mon += nOffset;
				}
				else if (boost::iequals(strInterval, "day"))
				{
					tm.tm_mday += nOffset;
				}
				else if (boost::iequals(strInterval, "hour"))
				{
					tm.tm_hour += nOffset;
				}
				else if (boost::iequals(strInterval, "minute"))
				{
					tm.tm_min += nOffset;
				}
				else if (boost::iequals(strInterval, "second"))
				{
					tm.tm_sec += nOffset;
				}

				// store the time in seconds since Jan. 1, 1970 UTC
				long nTime = static_cast<long>(_mkgmtime(&tm));
				if (nTime >= 0)
				{
					pRecord->addField(
						PYXDataSet::s_strPyxisDimensionTime,
						PYXFieldDefinition::knContextNone,
						PYXValue::knInt32,
						1,
						PYXValue(nTime));
				}
			}
		}
	}
	else if (boost::iequals(pszKey, "NETCDF_DIM_level"))
	{
		// height above ground level where the measurement was taken, should be of the format ####
		std::cmatch results;
		std::regex regexHeight("^\\s*(\\d+)\\s*$", std::regex::icase);
		if (std::regex_match(pszValue, results, regexHeight))
		{
			int nHeight = atoi(results[1].str().c_str());
			pRecord->addField(
				PYXDataSet::s_strPyxisDimensionHeight,
				PYXFieldDefinition::knContextNone,
				PYXValue::knInt32,
				1,
				PYXValue(nHeight));
		}
	}
	else if (boost::iequals(pszKey, "long_name"))
	{
		// record as the long name
		pRecord->addField(
			PYXDataSet::s_strPyxisLongName,
			PYXFieldDefinition::knContextNone,
			PYXValue::knString,
			1,
			PYXValue(pszValue));

	}
	else if (boost::iequals(pszKey, "abbreviation"))
	{
		// record as the short name
		pRecord->addField(
			PYXDataSet::s_strPyxisShortName,
			PYXFieldDefinition::knContextNone,
			PYXValue::knString,
			1,
			PYXValue(pszValue));
	}
	else if (boost::iequals(pszKey, "units"))
	{
		// units, may be enclosed in square braces
		std::cmatch results;
		std::regex regexUnits("^\\s*\\[?([^\\]\\s]+)\\]?\\s*$");
		if (std::regex_match(pszValue, results, regexUnits))
		{
			auto strUnits = results[1].str();
			pRecord->addField(
				PYXDataSet::s_strPyxisUnits,
				PYXFieldDefinition::knContextNone,
				PYXValue::knString,
				1,
				PYXValue(strUnits));
		}
	}
}

/*!
Add the definition for the data set's content

\param pTableDefn		The table definition for the content
\param vecBands			Vector of band numbers 1..n
\param strFieldName		The field name
\param nContext			The context
\param nGDALDataType	The GDAL data type of the band
*/
void GDALXYCoverage::addContentDefinition(
	PYXPointer<PYXTableDefinition> pTableDefn,
	const std::vector<int>& vecBands,
	const std::string& strFieldName,
	PYXFieldDefinition::eContextType nContext,
	GDALDataType nGDALDataType)
{
	auto nBandCount = vecBands.size();
	auto nFieldType = mapType(nGDALDataType);

	if (PYXFieldDefinition::knContextCLUT == nContext)
	{
		assert(PYXValue::knUInt8);
		assert(1 == nBandCount);

		pTableDefn->addFieldDefinition(
			strFieldName,
			PYXFieldDefinition::knContextRGB,
			nFieldType,
			3);
	}
	else if (3 == nBandCount || 4 == nBandCount)
	{
		pTableDefn->addFieldDefinition(
			strFieldName,
			nContext,
			nFieldType,
			nBandCount);
	}
	else if (1 == nBandCount)
	{
		pTableDefn->addFieldDefinition(
			strFieldName,
			PYXFieldDefinition::knContextGreyScale,
			nFieldType,
			1);
	}
	else
	{
		PYXTHROW(PYXProcsException, "Invalid number of bands.");
	}
}

/*
GDALXYCoverage::PixelRect::PixelRect(const GDALMetaData & metaData,const PYXRect2DInt & pixelRect, const PYXRect2DDouble & bounds,const PYXPointer<CoordConverterImpl> & coordConverter)
{
	m_pixelRect = pixelRect;
	m_region = PYXXYBoundsRegion::create(bounds,*coordConverter);
	m_subRectX = 0;
	m_subRectY = 0;

	if (isBigRegion())
	{
		int size = MinBlockSize;
		while( size*SubBlockCount < std::max(pixelRect.width(),pixelRect.height()) )
		{
			size *= SubBlockCount;
		}

		m_subRectX = ((pixelRect.width()-1) / size) + 1;
		m_subRectY = ((pixelRect.height()-1) / size) + 1;

		m_subRects.resize(m_subRectX * m_subRectY);

		for(int x=0; x<m_subRectX; x++)
		{
			for(int y=0; y<m_subRectY; y++)
			{
				PYXRect2DInt subRect(std::min(x*size+pixelRect.xMin(),pixelRect.xMax()),std::min(y*size+pixelRect.yMin(),pixelRect.yMax()),
					std::min((x+1)*size+pixelRect.xMin(),pixelRect.xMax()),std::min((y+1)*size+pixelRect.yMin(),pixelRect.yMax()));

				PYXCoord2DDouble pt00;
				PYXCoord2DDouble pt01;
				PYXCoord2DDouble pt10;
				PYXCoord2DDouble pt11;

				metaData.rasterToNative(PYXCoord2DInt(subRect.xMin(),subRect.yMin()),&pt00);
				metaData.rasterToNative(PYXCoord2DInt(subRect.xMax(),subRect.yMin()),&pt01);
				metaData.rasterToNative(PYXCoord2DInt(subRect.xMin(),subRect.yMax()),&pt10);
				metaData.rasterToNative(PYXCoord2DInt(subRect.xMax(),subRect.yMax()),&pt11);

				PYXRect2DDouble subBounds;
				subBounds.expand(pt00);
				subBounds.expand(pt01);
				subBounds.expand(pt10);
				subBounds.expand(pt10);

				m_subRects[x*m_subRectY+y] = PixelRect::create(
					metaData,
					subRect,
					subBounds,
					coordConverter);

			}
		}
	}
}

PYXRegion::CellIntersectionState GDALXYCoverage::PixelRect::intersectTile(const PYXIcosIndex & index) const
{
	return m_region->intersects(index,true);
}

bool GDALXYCoverage::PixelRect::isBigRegion() const 
{
	return m_pixelRect.width() > MinBlockSize || m_pixelRect.height() > MinBlockSize;
}

double distanceBetween(PYXRect2DInt & rect1,PYXRect2DInt & rect2)
{
	if (rect1.touch(rect2))
		return 0;

	PYXCoord2DInt pt2 = rect2.pin(PYXCoord2DInt(rect1.xMin(),rect1.yMin()));
	PYXCoord2DInt pt1 = rect1.pin(pt2);

	return pt1.distance(pt2);
}

void mergeRects(PYXRect2DInt & rect1,PYXRect2DInt & rect2,PYXRect2DInt & subRect)
{
	if (!subRect.empty())
	{
		if (rect1.empty())
		{
			rect1 = subRect;
		} 
		else if (rect1.touch(subRect))
		{
			rect1.expand(subRect);
		}
		else if (rect2.empty())
		{
			rect2 = subRect;
		}
		else if (rect2.touch(subRect))
		{			
			rect2.expand(subRect);
		}
		else {
			//attach to the nearest rect.
			if (distanceBetween(rect1,subRect) <= distanceBetween(rect2,subRect))
				rect1.expand(subRect);
			else 
				rect2.expand(subRect);
		}

		if (!rect1.empty() && !rect2.empty() && rect1.touch(rect2))
		{
			rect1.expand(rect2);
			rect2.setEmpty();
		}
	}
}

void GDALXYCoverage::PixelRect::getTileBounds(const PYXIcosIndex & index,PYXRect2DInt & rect1,PYXRect2DInt & rect2) const
{
	rect1.setEmpty();
	rect2.setEmpty();

	switch(intersectTile(index))
	{
	case PYXRegion::knNone:
		return;
	case PYXRegion::knComplete:
	case PYXRegion::knPartial:
		//contine...
		break;
	}

	//small enoguh region
	if (!isBigRegion())
	{
		rect1 = m_pixelRect;
	}
	//the region is big - but it very small comapring to the tile size...
	else if (m_region->getBoundingCircle().getRadius() < PYXIcosMath::UnitSphere::calcTileCircumRadius(index)/10)
	{
		rect1 = m_pixelRect;
	}
	//lets dig into the smaller regions...
	else
	{
		PYXRect2DInt subRect1;
		PYXRect2DInt subRect2;

		for(std::vector<PYXPointer<PixelRect>>::const_iterator it = m_subRects.begin();it!=m_subRects.end();++it)
		{
			const PYXPointer<PixelRect> & subRect = *it;

			subRect->getTileBounds(index,subRect1,subRect2);

			mergeRects(rect1,rect2,subRect1);
			mergeRects(rect1,rect2,subRect2);
		}
	}
}


void GDALXYCoverage::PixelRect::collectRects(const PYXIcosIndex & index,std::list<PYXRect2DInt> & rects) const
{
	switch(intersectTile(index))
	{
	case PYXRegion::knNone:
		return;
	case PYXRegion::knComplete:
	case PYXRegion::knPartial:
		//contine...
		break;
	}

	if (!isBigRegion())
	{
		rects.push_back(m_pixelRect);
	}
	//the region is big - but it very small comapring to the tile size...
	else if (m_region->getBoundingCircle().getRadius() < PYXIcosMath::UnitSphere::calcTileCircumRadius(index)/10)
	{
		rects.push_back(m_pixelRect);
	}
	//lets dig into the smaller regions...
	else 
	{
		for(std::vector<PYXPointer<PixelRect>>::const_iterator it = m_subRects.begin();it!=m_subRects.end();++it)
		{
			const PYXPointer<PixelRect> & subRect = *it;

			subRect->collectRects(index,rects);
		}
	}
}

void GDALXYCoverage::PixelRect::getTileBounds2(const PYXIcosIndex & index,PYXRect2DInt & rect1,PYXRect2DInt & rect2) const
{
	std::list<PYXRect2DInt> rects;

	collectRects(index,rects);

	rect1.setEmpty();
	rect2.setEmpty();

	for(std::list<PYXRect2DInt>::iterator it = rects.begin();it != rects.end();++it)
	{
		mergeRects(rect1,rect2,*it);
	}
}
*/


GDALXYCoverage::Buffer::Buffer() : m_pReadBuffer(0),m_nBands(0),m_nPixelBytes(0),m_nLineBytes(0),m_nBandBytes(0)
{
}

GDALXYCoverage::Buffer::~Buffer()
{
	if (m_pReadBuffer != 0)
	{
		this->consumeMemory(-getBufferSize());
		delete[] m_pReadBuffer;
	}
}

//allocate the buffer
void GDALXYCoverage::Buffer::alloc(const PYXRect2DInt & bounds,GDALDataType type,int nBands)
{
	if (m_pReadBuffer != 0)
	{
		delete[] m_pReadBuffer;
		this->consumeMemory(-getBufferSize());
		m_pReadBuffer = 0;
	}

	m_readBufferBounds = bounds;
	m_nGDALDataType = type;
	m_nBands = nBands;
	m_nPixelBytes = GDALGetDataTypeSize( m_nGDALDataType ) / 8;
	m_nLineBytes = m_nPixelBytes * (m_readBufferBounds.width() + 1);
	m_nBandBytes = m_nLineBytes * (m_readBufferBounds.height() + 1);

	if (m_nBandBytes > knBufferLimiter)
	{
		return;
	}

	m_pReadBuffer = new byte[getBufferSize()];
	this->consumeMemory(getBufferSize());	
}

//return true if read was successful
bool GDALXYCoverage::Buffer::read(const PYXCoord2DInt& raster,byte * dest)
{
	if (!m_readBufferBounds.inside(raster))
		return false;

	byte* pMoveFrom = m_pReadBuffer;

	// the x offset
	pMoveFrom += (raster.x() - m_readBufferBounds.xMin()) * m_nPixelBytes;

	// the Y offset
	pMoveFrom += (raster.y() - m_readBufferBounds.yMin()) * m_nLineBytes;

	for (int nBand = 1; nBand <= m_nBands; ++nBand)
	{
		memcpy(dest, pMoveFrom, m_nPixelBytes);
		dest += m_nPixelBytes;
		pMoveFrom += m_nBandBytes;
	}
	return true;
}

boost::mutex GDALXYCoverage::BufferCache::s_mutex;
CacheMap<GDALXYCoverage::BufferCache*,std::list<PYXPointer<GDALXYCoverage::Buffer>>> GDALXYCoverage::BufferCache::s_allBufferCache(10);

GDALXYCoverage::BufferCache::BufferCache()
{
}

GDALXYCoverage::BufferCache::~BufferCache()
{
	boost::mutex::scoped_lock lock(s_mutex);
	s_allBufferCache.erase(this);
}

void GDALXYCoverage::BufferCache::add( PYXPointer<Buffer> buffer)
{
	boost::mutex::scoped_lock lock(s_mutex);
	auto & list = s_allBufferCache[this];
	list.push_front(buffer);
	if (list.size()>20)
	{
		list.pop_back();
	}
}

void GDALXYCoverage::BufferCache::freeLargeBuffers()
{
	boost::mutex::scoped_lock lock(s_mutex);
	auto & list = s_allBufferCache[this];
	while (list.size()>3)
	{
		list.pop_back();
	}
}

PYXPointer<GDALXYCoverage::Buffer> GDALXYCoverage::BufferCache::findContainingBuffer( const PYXRect2DInt& bufferBounds)
{
	boost::mutex::scoped_lock lock(s_mutex);

	auto & list = s_allBufferCache[this];
	for(auto item = list.begin();item != list.end(); ++item)
	{
		if ((*item)->getReadBounds().contains(bufferBounds))
		{
			auto result = *item;
			if (list.begin() != item)
			{				
				swap(list.front(),*item);
			}
			return result;
		}
	}
	return 0;
}

PYXPointer<GDALXYCoverage::Buffer> GDALXYCoverage::BufferCache::findContainingBuffer( const PYXCoord2DInt & raster)
{
	boost::mutex::scoped_lock lock(s_mutex);

	auto & list = s_allBufferCache[this];
	for(auto item = list.begin();item != list.end(); ++item)
	{
		if ((*item)->getReadBounds().inside(raster))
		{
			auto result = *item;
			if (list.begin() != item)
			{				
				swap(list.front(),*item);
			}
			return result;
		}
	}
	return 0;
}


