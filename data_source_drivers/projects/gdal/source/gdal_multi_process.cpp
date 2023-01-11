/******************************************************************************
gdal_multi_data_source.cpp

begin		: 2004-10-15
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_GDAL_SOURCE
#include "gdal_multi_process.h"

// local includes
#include "exceptions.h"
#include "pyxis/utility/file_utils.h"
#include "gdal_xy_coverage.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/utility/value.h"
#include "pyxis/sampling/xy_utils.h"
#include "pyxis/procs/path.h"
#include "pyxis/geometry/cell.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <cassert>
#include <io.h>

// {49288859-ED22-413b-B892-441D99FBAA6B}
PYXCOM_DEFINE_CLSID(GDALMultiProcess, 
0x49288859, 0xed22, 0x413b, 0xb8, 0x92, 0x44, 0x1d, 0x99, 0xfb, 0xaa, 0x6b);

PYXCOM_CLASS_INTERFACES(
	GDALMultiProcess, IProcess::iid, IXYCoverage::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(GDALMultiProcess, "GDAL Multi Coverage", "A collection of Geospatial files opened by the GDAL library", "Drop",
					IXYCoverage::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IPath::iid, 1, -1, "Path to open", "Path to a Gdal Datasource to open")
	IPROCESS_SPEC_PARAMETER(ISRS::iid, 0, 1, "An optional SRS", "Overrides SRS in wkt")
IPROCESS_SPEC_END

//! Tester class
Tester<GDALMultiProcess> gMultiReaderTester;
//! Test method
void GDALMultiProcess::test()
{

#if 0

	PYXSpatialReferenceSystem spatialReference;

	spatialReference.setSystem(PYXSpatialReferenceSystem::knSystemProjected);
	spatialReference.setDatum(PYXSpatialReferenceSystem::knDatumWGS84);
	spatialReference.setProjection(PYXSpatialReferenceSystem::knProjectionUTM );
	spatialReference.setZone(19);
	
	GDALMultiProcess reader;

	std::string strDirName = "c:\\data\\ortho\\quebec_32_colour";

	try
	{
		reader.open(strDirName, "tif", spatialReference);
	}
	catch (PYXException& e)
	{
		return;
	}

						
	PYXIcosIndex index;

	index = "3-20406050060010100000300204";

/*	const void* pValue = reader.getValue(index);

	int nCount = 0;
	
	for (PYXPointer<PYXIterator> spIt(reader.getIterator()); !spIt->end(); spIt->next())
	{
		pValue = spIt->getFieldValue();
		if (0 != pValue)
		{
			int nValue = *((int*) pValue);
		}

		nCount++;

		if (nCount > 100)
		{
			break;
		}
	}
*/
#endif

}

/*!
Constructor initializes member variables.
*/
GDALMultiProcess::GDALMultiProcess() :
	m_bounds(),
	m_bDataInit(false)
{
	m_bHasSRS = false;
	m_fSpatialPrecision = -1.0;
	m_spLastAccessed = boost::intrusive_ptr<IXYCoverage>();
	m_spSRS = boost::intrusive_ptr<ISRS>();
}

/*!
Destructor cleans up memory.
*/
GDALMultiProcess::~GDALMultiProcess()
{
}

IProcess::eInitStatus GDALMultiProcess::initImpl()
{
	boost::intrusive_ptr<ISRS> spSRS = 0;
	if (getParameter(1)->getValueCount() > 0)
	{
		getParameter(1)->getValue(0)->QueryInterface(ISRS::iid, (void**) &spSRS);
		m_spSRS = spSRS;
	}

	std::vector<boost::filesystem::path> vecInptPaths;
	std::vector<boost::intrusive_ptr<IProcess> > vecParaValues = getParameter(0)->getValues();
	
	for (std::vector<boost::intrusive_ptr<IProcess> >::const_iterator paramValuesIt = 
		vecParaValues.begin(); paramValuesIt != vecParaValues.end(); ++paramValuesIt)
	{
		boost::intrusive_ptr<IPath> spPath;
		(*paramValuesIt)->QueryInterface(IPath::iid, (void**) &spPath);
		if (spPath)
		{
			vecInptPaths.push_back(spPath->getLocallyResolvedPath());
		}
	}

	m_bDataInit = false;
	open(vecInptPaths);
	return knInitialized;
}

/*!
Open a GDAL multi data set.  

There are two ways that failure can occur:
1. It is not a valid GDAL multi data set, and
2. It is a valid GDAL multi data set but it had some problem opening it.

If it is not a valid GDAL multi data set, then the method will return false.
If it is valid but failed to open it, then an exception will be thrown.
If it is valid and was successfully opened, then the method will return true.

\param	strFileName			The file to open.

\returns false if it is not a valid GDAL multi data set, 
			true if it opens successfully.
*/
bool GDALMultiProcess::open(const std::vector<boost::filesystem::path>& filePaths)
{
	boost::recursive_mutex::scoped_lock lock(m_Mutex);

	// initialize the rTree
	m_rTree.initialize();

	for (std::vector<boost::filesystem::path>::const_iterator inptFilePathIt = filePaths.begin();
		inptFilePathIt != filePaths.end(); ++inptFilePathIt)
	{
		std::vector<boost::filesystem::path> vecPaths;
		getFiles(*inptFilePathIt, vecPaths);
		 
		for (std::vector<boost::filesystem::path>::const_iterator fileOpenIterator = vecPaths.begin();
			fileOpenIterator != vecPaths.end(); ++fileOpenIterator)
		{
			try
			{
				openFile(FileUtils::pathToString(*fileOpenIterator));
			}
			catch(...)
			{

			}
		}
	}

	// flush all of the items to file for the rtree.
	m_rTree.flush();

	return true;
}

/*!
Get the spatial precision of the data.

\return	The spatial precision in metres or -1.0 if not known.
*/
double GDALMultiProcess::getSpatialPrecision() const
{
	return m_fSpatialPrecision;
}
	
/*!
Call this method after the data source is opened to determine if the data
source contains a spatial reference. If not, a spatial reference must be
supplied by calling setSpatialReference() before getFeatureIterator() is
called. Native PYXIS data sources always return true.

\return	true if the data source has a spatial reference, otherwise false
*/
bool GDALMultiProcess::hasSpatialReferenceSystem() const
{
	return m_bHasSRS;
}

/*!
Specify the spatial reference for the data source. Call this method to set the
spatial reference if after the data source is opened hasSpatialReference()
returns false.

\param	spSRS	The spatial reference system.
*/
void GDALMultiProcess::setSpatialReferenceSystem(boost::intrusive_ptr<ISRS> spSRS)
{
	assert(spSRS && "Invalid argument.");
	if (spSRS)
	{
		m_spSRS = spSRS;
		m_bHasSRS = true;
	}
	else
	{
		m_bHasSRS = false;
		m_spSRS = boost::intrusive_ptr<ISRS>();
	}
}


/*!
Open a GDAL data source.

There are two ways that failure can occur.  It is not a valid GDAL data set,
and it is a valid GDAL data set but it had some problem opening it.
If it is not a valid GDAL data set, then the method will return 0.
If it is valid but failed to open it, then an exception will be thrown.
If it is valid and was successfully opened, then the data source will be returned.

\param	strFileName			The name of the data source.

\return	true if successfull, false otherwise.
*/
bool GDALMultiProcess::openFile(const std::string& strFileName)
{
	boost::recursive_mutex::scoped_lock lock(m_Mutex);

	// create the data source
	boost::intrusive_ptr<GDALXYCoverage> spDataSource(new GDALXYCoverage());

	// open the data source.  
	if (!spDataSource->openAsRGB(strFileName, m_spSRS))
	{
		// if the data set is not a GDAL data set then false.
		return false;
	}
	TRACE_DEBUG("Opening Multi GDAL data source.");

	if (!m_bDataInit)
	{
		initDriverData(spDataSource);
	}

	// get the bounds of the reader
	const PYXRect2DDouble dataSourceBounds = spDataSource->getBounds();
	PYXCoord2DDouble pt(dataSourceBounds.xMin(), dataSourceBounds.yMin());

	if (m_bounds.empty())
	{
		m_bounds = dataSourceBounds;
	}
	else
	{
		m_bounds.expand(dataSourceBounds);
	}

	PYXCoord2DDouble ll = spDataSource->getMetaDataGDAL()->getSouthWest();
	PYXCoord2DDouble ur = spDataSource->getMetaDataGDAL()->getNorthEast();

	PYXRect2DDouble area(ll.x(), ll.y(), ur.x(), ur.y());
	
	IXYCoverage* pXYCoverage = spDataSource.get();
	
	PYXCOM_IUnknown* pUnknown;
	pXYCoverage->QueryInterface(PYXCOM_IUnknown::iid, (void**) &pUnknown);
	
	/*
	We need to add an additional reference to this process when adding it into the
	R-Tree to ensure that the object doesn't disapear before it comes out of the 
	R-Tree. This reference is released when the R-Tree closes.
	*/
	pUnknown->AddRef();
	
	// insert above two coordinates & labels in the rtree
	m_rTree.insert(area, reinterpret_cast<void*>(pUnknown));

	return true;
}

/*!
Initialize the members that are needed by this process. Members are initialized bassed
on the datasource that is successfully opened by the GDal driver.

\param spXYCov The datasource to initalize the members from.
*/
void GDALMultiProcess::initDriverData(boost::intrusive_ptr<IXYCoverage> spXYCov)
{
	if (spXYCov)
	{
		m_fSpatialPrecision = spXYCov->getSpatialPrecision();
		m_bHasSRS = m_bHasSRS ? m_bHasSRS : spXYCov->hasSpatialReferenceSystem();
		m_stepSize = spXYCov->getStepSize();
		m_spCoordConv = spXYCov->getCoordConverter()->clone();

		// setup the coverage definition from the first file
		if (getCoverageDefinition()->getFieldCount() <= 0)
		{
			m_spCovDefn = spXYCov->getCoverageDefinition()->clone();
		}

		m_bDataInit = true;
		return;
	}
	m_bDataInit = false;
}

/*!
Get the data source which contains information for a particular native
coordinate.

\param	native	The native coordinate.

\return	the GDALDataSource.
*/
boost::intrusive_ptr<IXYCoverage> GDALMultiProcess::getDataSource(const PYXCoord2DDouble& native) const 
{
	boost::recursive_mutex::scoped_lock lock(m_Mutex);
	void * ptr = const_cast<void*>(m_rTree.containsFirst(native));
	PYXCOM_IUnknown* pIUnkown = reinterpret_cast<PYXCOM_IUnknown*> (ptr);
	if (pIUnkown != 0)
	{
		IXYCoverage* pXYCov;
		pIUnkown->QueryInterface(IXYCoverage::iid, (void**) &pXYCov);
		return boost::intrusive_ptr<IXYCoverage>(pXYCov);
	}
	return boost::intrusive_ptr<IXYCoverage>();
}

/*!
\return The coordinate converter.
*/
const ICoordConverter* GDALMultiProcess::getCoordConverter() const
{
	return m_spCoordConv ? m_spCoordConv.get() : 0;
}

PYXCoord2DDouble GDALMultiProcess::nativeToRasterSubPixel(const PYXCoord2DDouble & native) const
{
	auto spDataSrc = getDataSource(native);
	return spDataSrc->nativeToRasterSubPixel(native);
}

/*!
Get the coverage value at the specified native coordinate.

\param	native		The native coordinate.
\param	pValue		The value being retrieved.

\return	The value.
*/
bool GDALMultiProcess::getCoverageValue(
								  const PYXCoord2DDouble& native,
								  PYXValue* pValue) const
{
	// this routine is a Critical Section in terms of thread safety
	boost::recursive_mutex::scoped_lock lock(m_Mutex);
	
	boost::intrusive_ptr<IXYCoverage> spDataSrc = boost::intrusive_ptr<IXYCoverage>();
	
	if (!m_spLastAccessed)
	{
		spDataSrc = getDataSource(native);
	
		if (spDataSrc)
		{	
			m_spLastAccessed = spDataSrc;
			return m_spLastAccessed->getCoverageValue(native, pValue);
		}
	}
	else
	{
		bool bHasData = m_spLastAccessed->getCoverageValue(native, pValue);
		if (bHasData)
		{
			//Return that we have data.
			return bHasData;
		}
		else
		{
			/*
			If we don't have data then set the ptr to the last accessed to zero.
			Then call ourself, to find another datasource we can get data from.
			*/
			m_spLastAccessed = 0;
			return this->getCoverageValue(native, pValue);
		}
	}

	return false;
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
*/
void STDMETHODCALLTYPE GDALMultiProcess::getMatrixOfValues(	const PYXCoord2DDouble& nativeCentre,
											PYXValue* pValues,
											int sizeX,
											int sizeY) const

{
	// this routine is a Critical Section in terms of thread safety
	boost::recursive_mutex::scoped_lock lock(m_Mutex);

	assert(pValues != 0 && "validate the pointer to the array.");
	assert(sizeX > 0 && "we want an array size that is positive");
	assert(sizeY > 0 && "we want an array size that is positive");

	// calculate the lower left corner in raster coordinates.
	PYXCoord2DInt rasterBaseLL;
	XYUtils::nativeToLowerRaster(getBounds(), getStepSize(), nativeCentre, &rasterBaseLL);
	rasterBaseLL.setX(rasterBaseLL.x()-static_cast<int>((sizeX - 1)/2.0));
	rasterBaseLL.setY(rasterBaseLL.y()-static_cast<int>((sizeY - 1)/2.0));

	// find the data source for the lower left corner
	PYXCoord2DDouble nativeLL;
	XYUtils::rasterToNative(getBounds(), getStepSize(), rasterBaseLL, &nativeLL);
	boost::intrusive_ptr<IXYCoverage> spDataSourceLL = getDataSource(nativeLL);

	// find the data source for the upper right corner
	PYXCoord2DInt rasterUR;
	rasterUR.setX(rasterBaseLL.x()+sizeX-1);
	rasterUR.setY(rasterBaseLL.y()+sizeY-1);
	PYXCoord2DDouble nativeUR;
	XYUtils::rasterToNative(getBounds(), getStepSize(), rasterUR, &nativeUR);
	
	boost::intrusive_ptr<IXYCoverage> spDataSourceUR = getDataSource(nativeUR);

	// if the matrix is covered completely by one data sorce, or one
	// of the corners falls outside the data.
	if (spDataSourceLL == spDataSourceUR &&  !spDataSourceLL)
	{
		spDataSourceLL->getMatrixOfValues(nativeCentre, pValues, sizeX, sizeY);
	}
	else if (!spDataSourceLL && spDataSourceUR)
	{
		spDataSourceLL->getMatrixOfValues(nativeCentre, pValues, sizeX, sizeY);
	}
	else if (!spDataSourceUR && spDataSourceLL)
	{
		spDataSourceUR->getMatrixOfValues(nativeCentre, pValues, sizeX, sizeY);
	}
	else
	{
		// default to get values one at a time if they span data sources.
	    for (int x = 0; x < sizeX; x++)
		{
			PYXCoord2DInt rasterGetAt;
			rasterGetAt.setX(rasterBaseLL.x()+x);
			for (int y = 0; y < sizeY; y++)
			{
				rasterGetAt.setY(rasterBaseLL.y()+y);
				PYXCoord2DDouble nativeGetAt;
				XYUtils::rasterToNative(getBounds(), getStepSize(), rasterGetAt, &nativeGetAt);
				getValue(nativeGetAt, &pValues[x*sizeX+y]);
			}
		}
	}
}

/*!
Helper method to get a value based on based on the last accesse datasource.

\param nativeGetAt The native coordinates to retrieve a value for.
\param pValue	   A pointer to a PYXValue to fill with data.
*/
void GDALMultiProcess::getValue(PYXCoord2DDouble nativeGetAt, PYXValue* pValue) const
{
	// TODO: write this so it makes sense!

	if (!m_spLastAccessed)
	{
		m_spLastAccessed = getDataSource(nativeGetAt);
		if (m_spLastAccessed)
		{
			if (m_spLastAccessed->getCoverageValue(nativeGetAt, pValue))
			{
				return;
			}
			else
			{
				m_spLastAccessed = 0;
				getValue(nativeGetAt, pValue);
			}
		}
	}
	else
	{
		if (m_spLastAccessed->getCoverageValue(nativeGetAt, pValue))
		{
			return;
		}
		else
		{
			m_spLastAccessed = 0;
			getValue(nativeGetAt, pValue);
		}
	}
}

//! Get the distance between data points in this coverage
PYXCoord2DDouble GDALMultiProcess::getStepSize() const
{
	return m_stepSize;
}

/*!
Recursively iterate through sub directories of a root directory extracting file 
paths when encountered. Each file path is put into a vector and the vector is 
returned by reference.

\param directory	The root level directory to traverse through.
\param filePaths	A vector passed in by reference to fill with all the file paths found.

*/
void GDALMultiProcess::getFiles(	const boost::filesystem::path& directory,
									std::vector<boost::filesystem::path>& filePaths	) const
{
	
	if (FileUtils::isDirectory(directory))
	{
		boost::filesystem::directory_iterator endIt;

		for (boost::filesystem::directory_iterator dirIt(directory); dirIt != endIt; ++dirIt)
		{
			if (FileUtils::isDirectory(*dirIt))
			{
				getFiles(*dirIt, filePaths);
			}
			else
			{
				filePaths.push_back((*dirIt));
			}
		}
	}
	else
	{
		filePaths.push_back(directory);
	}
}

void GDALMultiProcess::tileLoadHint(const PYXTile& tile) const
{
	PYXrTree::KeyList keyList;
	PYXRect2DDouble pRect1; 
	PYXRect2DDouble pRect2; 
	tile.getBoundingRects(m_spCoordConv.get(), &pRect1, &pRect2);
	m_rTree.overlaps(pRect1, pRect2, keyList);

	for (PYXrTree::KeyList::const_iterator keyListIt = keyList.begin(); keyListIt != keyList.end(); ++keyListIt)
	{
		boost::intrusive_ptr<IXYCoverage> spCovPtr = boost::intrusive_ptr<IXYCoverage>((IXYCoverage*) *keyListIt);
		spCovPtr->tileLoadHint(tile);
	}
}

void GDALMultiProcess::tileLoadDoneHint(const PYXTile& tile) const
{
	PYXrTree::KeyList keyList;
	PYXRect2DDouble pRect1; 
	PYXRect2DDouble pRect2; 
	tile.getBoundingRects(m_spCoordConv.get(), &pRect1, &pRect2);
	m_rTree.overlaps(pRect1, pRect2, keyList);

	for (PYXrTree::KeyList::const_iterator keyListIt = keyList.begin(); keyListIt != keyList.end(); ++keyListIt)
	{
		boost::intrusive_ptr<IXYCoverage> spCovPtr = boost::intrusive_ptr<IXYCoverage>((IXYCoverage*) *keyListIt);
		spCovPtr->tileLoadDoneHint(tile);
	}
}

