/******************************************************************************
grd98_process.cpp

begin		: 2005-04-01
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_GRD98_SOURCE
#include "grd98_process.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/derm/wgs84.h"
#include "pyxis/procs/path.h"
#include "pyxis/sampling/xy_utils.h"

// {751D51B6-4B30-4bdc-8DCD-AFBBF056B96F}
PYXCOM_DEFINE_CLSID(GRD98Process, 
0x751d51b6, 0x4b30, 0x4bdc, 0x8d, 0xcd, 0xaf, 0xbb, 0xf0, 0x56, 0xb9, 0x6f);
PYXCOM_CLASS_INTERFACES(GRD98Process, IProcess::iid, IXYCoverage::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(GRD98Process, "GRD98 File Reader", "Geodas Gridded Data Format", "Reader",
					IXYCoverage::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IPath::iid, 1, 1, "Data File", "The GRD98 data file.");
IPROCESS_SPEC_END

/*
Note: The following values are specific to ETOPO2 GRD98 formated data.  These values 
are in no way representative of the dataset as a whole.  The ETOP02 dataset is 
made up of roughly 20 different data sets.  Each dataset having varying levels of 
accuracy and precision.  Roughly three quarters of ETOPO2 is made up of DTED0.  
The values below reflect the accuracy and precision of this dataset primarily.
*/

// Maximum amount of memory to allocate in each block in bytes.
static const unsigned int knMaxMemoryBlock = 100000;

// The spatial accuracy for GRD98 files in metres.
static const double kfSpatialAccuracy = 50.0;

// The data accuracy for GRD98 files in metres.
static const double kfDataAccuracy = 30.0;

// The data precision for GRD98 files in metres.
static const double kfDataPrecision = 20.0;

//! GRD98 file extension.
const std::string GRD98Process::kstrFileExt = "g98";

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

IProcess::eInitStatus GRD98Process::initImpl()
{
	boost::intrusive_ptr<IPath> spPath;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(IPath::iid, (void**) &spPath);
	assert(spPath);

	// open the file
	if (!open(spPath->getLocallyResolvedPath()))
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Unable to open GRD98 process at: " + spPath->getLocallyResolvedPath());
		return knFailedToInit;
	}
	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// IXYCoverage
////////////////////////////////////////////////////////////////////////////////

/*!
Get the coverage value at the specified native coordinates.
This call MUST be thread-safe.

\param	native		The native coordinates.
\param  pValue      an pointer to the PYXValue to be filled in

\return	True if a data value was retrieved, false if there was no value to return.
*/
bool GRD98Process::getCoverageValue(const PYXCoord2DDouble& native,
									PYXValue* pValue) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	assert(!getID().empty() && "Data set not open.");

	try
	{
		if (m_bounds.inside(native))
		{
			// calculate the coordinate file offset
			PYXCoord2DInt raster;
			XYUtils::nativeToLowerRaster(getBounds(), getStepSize(), native, &raster);
			unsigned int nOffset = calcFileOffset(raster);
			
			// read the value from the file
			if (m_header.getValueType() == GRD98HeaderReader::knValueTypeInt)
			{
				int nValue = readValueInt(nOffset);
				if (m_header.getEmptyValue() != nValue)
				{
					double fValue = static_cast<double>(nValue) / m_header.getValueConverter();
					pValue->setDouble(fValue);
					return true;
				}
			}
			else if (m_header.getValueType() == GRD98HeaderReader::knValueTypeFloat)
			{
				double fValue = readValueDouble(nOffset);
				// TODO need to check for empty value?
				// Currently header only returns empty value as int, but it could be float?
				assert(false && "Need to check for null data value.");
				pValue->setDouble(fValue);
				return true;
			}
			else
			{
				// unknown data type
				PYXTHROW(	PYXDataSourceException,
							"Unsupported data type: '" << m_header.getValueType() << 
							"'."	);
			}
		}
	}
	catch (PYXException& e)
	{
		PYXRETHROW(e, PYXDataSourceException, "Unable to read file.");
	}
	catch (std::ios_base::failure& f)
	{
		// a read error has occurred
		PYXTHROW(PYXDataSourceException, "Unable to read file: " << f.what());
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
void GRD98Process::getMatrixOfValues(	const PYXCoord2DDouble& nativeCentre,
											PYXValue* pValues,
											int sizeX,
											int sizeY) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	assert(!getID().empty() && "Data set not open.");

	try
	{
		if (m_bounds.inside(nativeCentre))
		{
			// calculate the lower left corner in raster coordinates.
			PYXCoord2DInt rasterBase;
			XYUtils::nativeToLowerRaster(getBounds(), getStepSize(), nativeCentre, &rasterBase);
			rasterBase.setX(rasterBase.x()-static_cast<int>((sizeX - 1)/2.0));
			rasterBase.setY(rasterBase.y()-static_cast<int>((sizeY - 1)/2.0));

			// find the maximum bounds for the data source in native coordinatees.
			PYXCoord2DDouble nativeMax;
			nativeMax.setX(getBounds().xMax());
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
			XYUtils::nativeToLowerRaster(getBounds(), getStepSize(), nativeMax, &rasterMax);
			rasterMax.setX(rasterMax.x()-1);
			rasterMax.setY(rasterMax.y()-1);

			PYXCoord2DInt raster;  // the position of the current point we are retrieving.
			for (int x = 0; x < sizeX; x++)
			{
				// set the x coordinate of the raster and limit it to the available data.
				raster.setX(rasterBase.x()+x);
				if (raster.x() < 0)
				{
					raster.setX(0);
				}
				if (raster.x() > rasterMax.x())
				{
					raster.setX(rasterMax.x());
				}

				for (int y = 0; y < sizeY; y++)
				{
					// set the y coordinate of the raster and limit it to the available data.
					raster.setY(rasterBase.y()+y);
					if (raster.y() < 0)
					{
						raster.setY(0);
					}
					if (raster.y() > rasterMax.y())
					{
						raster.setY(rasterMax.y());
					}

					// get the offset of the value
					unsigned int nOffset = calcFileOffset(raster);
					// read the value from the file
					double fValue;
					if (m_header.getValueType() == GRD98HeaderReader::knValueTypeInt)
					{
						int nValue = readValueInt(nOffset);
						if (m_header.getEmptyValue() != nValue)
						{
							fValue = static_cast<double>(nValue) / m_header.getValueConverter();
						}
					}
					else if (m_header.getValueType() == GRD98HeaderReader::knValueTypeFloat)
					{
						fValue = readValueDouble(nOffset);
						// TODO need to check for empty value?
						// Currently header only returns empty value as int, but it could be float?
						assert(false && "Need to check for null data value.");
					}
					else
					{
						// unknown data type
						PYXTHROW(	PYXDataSourceException,
									"Unsupported data type: '" << m_header.getValueType() << 
									"'."	);
					}

					pValues[x*sizeX+y] = PYXValue(fValue);
				}
			}
		}
			
	}
	catch (PYXException& e)
	{
		PYXRETHROW(e, PYXDataSourceException, "Unable to read file.");
	}
	catch (std::ios_base::failure& f)
	{
		// a read error has occurred
		PYXTHROW(PYXDataSourceException, "Unable to read file: " << f.what());
	}
}

/*!
Calculate the spatial precision by taking half the worst case distance between
elevation posts.

\return	The spatial precision in metres or -1.0 if not known.
*/
double GRD98Process::getSpatialPrecision() const
{
	CoordLatLon pt1;
	CoordLatLon pt2;

	HorizontalDatum const * pDatum = WGS84::getInstance();

	// convert bounds in seconds to degrees
	PYXRect2DDouble bounds = m_bounds;
	bounds.scale(1.0 / WGS84CoordConverter::knSecondsPerDegree);

	// convert latitude and longitude resolutions to degrees
	double fLatResolution = static_cast<double>(getLatResolution()) / WGS84CoordConverter::knSecondsPerDegree;
	double fLonResolution = static_cast<double>(getLonResolution()) / WGS84CoordConverter::knSecondsPerDegree;

	// the worst case latitude distance occurs at the most extreme latitude
	if (fabs(bounds.yMin()) > fabs(bounds.yMax()))
	{
		// data set is mostly in southern hemisphere
		pt1.setInDegrees(bounds.yMin(), bounds.xMin());
		pt2.setInDegrees(bounds.yMin() + fLatResolution, bounds.xMin());
	}
	else
	{
		// data set is mostly in northern hemisphere
		pt1.setInDegrees(bounds.yMax(), bounds.xMax());
		pt2.setInDegrees(bounds.yMax() - fLatResolution, bounds.xMax());
	}

	double fLatDistance = pDatum->calcDistance(pt1, pt2);

	// the worst case longitude distance occurs at the smallest latitude
	if (bounds.yMin() >= 0.0)
	{
		// data set is in northern hemisphere
		pt1.setInDegrees(bounds.yMin(), bounds.xMin());
		pt2.setInDegrees(bounds.yMin(), bounds.xMin() + fLonResolution);
	}
	else if (bounds.yMax() <= 0.0)
	{
		// data set is in southern hemisphere
		pt1.setInDegrees(bounds.yMax(), bounds.xMax());
		pt2.setInDegrees(bounds.yMax(), bounds.xMax() - fLonResolution);
	}
	else
	{
		// data set straddles the equator
		pt1.setInDegrees(0.0, bounds.xMin());
		pt2.setInDegrees(0.0, bounds.xMin() + fLonResolution);
	}

	double fLonDistance = pDatum->calcDistance(pt1, pt2);

	return std::max(fLatDistance, fLonDistance) / 2.0;
}

PYXCoord2DDouble GRD98Process::nativeToRasterSubPixel(const PYXCoord2DDouble & native) const
{
	PYXCoord2DDouble raster;
	XYUtils::nativeToLowerRasterSubPixel(getBounds(),getStepSize(),native,&raster);
	return raster;
}

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

/*!
Constructor.
*/
GRD98Process::GRD98Process() :
	m_spDefn(PYXTableDefinition::create()),
	m_coordConverter(WGS84CoordConverter::knSecondsPerDegree),
	m_nFileSize(0),
	m_nNumBlocks(0),
	m_nLastBlock(-1),
    m_nAccessCounter(0)
{
}

/*!
Destructor closes file.
*/
GRD98Process::~GRD98Process()
{
	// Close the file.
	m_in.close();

	// delete the memory resource if any
	std::vector<MemoryResource*>::iterator it = m_vecMemoryResource.begin();
	while (it != m_vecMemoryResource.end())
	{
		delete (*it);
		++it;
	}
}

/*
Open the GRD98 data file for read only.  

There are two ways that failure can occur:
1. It is not a valid GRD98 data set,
2. It is a valid GRD98 data set but it had some problem opening it.

If it is not a valid GRD98 data set, then the method will return false.
If it is valid but failed to open it, then an exception will be thrown.
If it is valid and was successfully opened, then the method will return true.

\param	strFileName			The file to open.

\returns false if it is not a valid GRD98 data set, true if it opens successfully.
*/
bool GRD98Process::open(const std::string& strFileName)
{
	assert((!strFileName.empty()) && "Invalid agrument.");

	// make sure it has the right extension
	std::string::size_type nIndex = strFileName.find(kstrFileExt);
	std::string::size_type nPos = -1;
	if (nIndex == nPos)
	{
		TRACE_INFO("Invalid file extension.");
		return false;
	}
	
	TRACE_INFO("Opening GRD98 dataset '" << strFileName << "'.");

	// confirm assumptions about integer size
	assert(4 == sizeof(int));

	assert(!strFileName.empty() && "Invalid argument.");

	m_strID = strFileName;

	// open the file to determine its length
	openFileForRead(m_in);
	m_in.seekg(0, std::ios_base::end);
	
	m_nFileSize = unsigned int(m_in.tellg()) - unsigned int(m_header.getHeaderSize());

	m_nNumBlocks = (m_nFileSize/knMaxMemoryBlock)+1;

	// Setup memory block vector.
	m_vecMemoryResource.resize(m_nNumBlocks, 0);

	// Setup memory block access last accessed time.
	m_vecLastAccessedTime.resize(m_nNumBlocks, 0);

	// Set vector size to number of items.
	m_vecTimes.resize(m_nNumBlocks);

	// read the header
	try
	{
		m_header.read(m_in);
	}
	catch (PYXException& e)
	{
		TRACE_ERROR("Unable to read header in file: '" + strFileName + "': \n" + e.getFullErrorString());
		return false;
	}

	// set the bounds
	m_bounds.setXMin(m_header.getOriginLongitude());
	m_bounds.setYMin(m_header.getOriginLatitude() - getHeight());
	m_bounds.setXMax(m_header.getOriginLongitude() + getWidth());
	m_bounds.setYMax(m_header.getOriginLatitude());

	// set the step size
	m_stepSize = PYXCoord2DDouble(getLonResolution(), -1.0 * getLatResolution());

	initMetaData();

	return true;
}

/*!
Get the number of points in the data source.

\return	The number of points in the data source.
*/
double GRD98Process::getPointCount() const
{
	return ((double) m_header.getLongitudePoints()) * m_header.getLatitudeLines();
}

/*!
Get the resolution of the data source.

\return	The resolution of the data source in geodetic seconds of longitude.
*/
int GRD98Process::getLonResolution() const
{
	assert(!getID().empty() && "Data set not open.");

	// get the long resolution in seconds
	return m_header.getLongitudeDataInterval();
}

/*!
Get the resolution of the data source.

\return	The resolution of the data source in geodetic seconds of latitude.
*/
int GRD98Process::getLatResolution() const
{
	assert(!getID().empty() && "Data set not open.");

	// get the latitude resolution in seconds
	return m_header.getLatitudeDataInterval();
}

/*!
Comparison function for sort.

\param	arg1	First item to compare.
\param	arg2	Second item to compare.

\return	true if arg1 is greater than arg2.
*/
bool GRD98Process::compareGRD98(GRD98Process::BLOCKTIMESTRUCT& arg1, GRD98Process::BLOCKTIMESTRUCT& arg2)
{
	return arg1.nTime < arg2.nTime;
}

/*!
Called by the memory manager when memory is low.
*/
void GRD98Process::freeMemory()
{
	unsigned int nNumItems = 0;
	for (unsigned int nIndex = 0; nIndex < m_nNumBlocks; ++nIndex)
	{
		// Only add blocks that have memory allocated.
		if (m_vecMemoryResource[nIndex] != 0)
		{
			m_vecTimes[nNumItems].nTime = m_vecLastAccessedTime[nIndex];
			m_vecTimes[nNumItems].nBlock = nIndex;
			++nNumItems;
		}
	}

	std::sort(m_vecTimes.begin(), m_vecTimes.begin()+nNumItems, compareGRD98);

	// Do we have items to delete?
	unsigned int nCount = 0;

	// Delete oldest 100 blocks.
	while (nCount < nNumItems && nCount < 100)
	{
		// Get index of block to remove.
		int nBlock = m_vecTimes[nCount].nBlock;

		// Delete oldest block of memory.
		delete m_vecMemoryResource[nBlock];

		// Set pointer to zero.
		m_vecMemoryResource[nBlock] = 0;

		++nCount;
	}
}

/*!
Get the width of the data file.

\return	The width of the data file in geodetic seconds of longitude.
*/
int GRD98Process::getWidth() const
{
	assert(!getID().empty() && "Data set not open.");

	int nSeconds = m_header.getLongitudeDataInterval();
	nSeconds *= (m_header.getLongitudePoints());

	return nSeconds;
}

/*!
Get the height of the data file.

\return	The height of the data file in geodetic seconds of latitude.
*/
int GRD98Process::getHeight() const
{
	assert(!getID().empty() && "Data set not open.");

	int nSeconds = m_header.getLatitudeDataInterval();
	nSeconds *= (m_header.getLatitudeLines());

	return nSeconds;
}

/*!
Calculate the file offset for the given native coordinates. Data are stored by
longitude first, then by increasing latitude within each longitude.The
following code calculates the file offset given the longitude and latitude of
the requested position relative to the file origin.

\param	nativeRaster	The native coordinates in geodetic seconds.

\return	The file offset
*/
unsigned int GRD98Process::calcFileOffset(const PYXCoord2DInt& nativeRaster) const
{
	// calculate number of data points
	unsigned int nOffset = nativeRaster.y() * m_header.getLongitudePoints() + nativeRaster.x();

	// multiply by the data size
	nOffset *= m_header.getValueSize();

	return nOffset;
}

/*!
Read a value from the file

\param nOffset	The offset where the value can be found.

\return	The value
*/
int GRD98Process::readValueInt(unsigned int nOffset) const
{
	// this routine is a Critical Section in terms of thread safety
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	unsigned int nBlock = nOffset / knMaxMemoryBlock;
	unsigned int nBlockOffset = nOffset - (nBlock*knMaxMemoryBlock);

	// load data buffer if required
	if (m_vecMemoryResource[nBlock] == 0)
	{
		loadDataBuffer(nBlock);
	}

	char* ptr = m_vecMemoryResource[nBlock]->getPtr() + nBlockOffset;

	if (m_nLastBlock != nBlock)
	{
		m_nLastBlock = nBlock;
		m_vecLastAccessedTime[nBlock] = ++m_nAccessCounter;
	}

	int nValueInt = -1;
	int nValueSize = m_header.getValueSize();
	if (2 == nValueSize)
	{
		assert(2 == sizeof(short));

		short nValue = *(reinterpret_cast<short*>(ptr));
		nValueInt = nValue;
	}
	else if (4 == nValueSize)
	{
		assert(4 == sizeof(int));

		nValueInt = *(reinterpret_cast<int*>(ptr));
	}
	else
	{
		assert(false);
	}
	
	return nValueInt;
}

/*!
Read a floating point value from the file 

\param nOffset	The offset where the value can be found.

\return	The value
*/
double GRD98Process::readValueDouble(unsigned int nOffset) const
{
	// this routine is a Critical Section in terms of thread safety
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	unsigned int nBlock = nOffset / knMaxMemoryBlock;
	unsigned int nBlockOffset = nOffset - (nBlock*knMaxMemoryBlock);

	// load data buffer if required
	if (m_vecMemoryResource[nBlock] == 0)
	{
		loadDataBuffer(nBlock);
	}

	char* ptr = m_vecMemoryResource[nBlock]->getPtr() + nBlockOffset;
	double fValueDouble = -1.0;

	int nValueSize = m_header.getValueSize();
	if (4 == nValueSize)
	{
		assert(4 == sizeof(float));

		float fValue = *(reinterpret_cast<float*>(ptr));
		fValueDouble = fValue;
	}
	else if (8 == nValueSize)
	{
		assert(8 == sizeof(double));

		fValueDouble = *(reinterpret_cast<double*>(ptr));
	}
	else
	{
		assert(false);
	}
	
	return fValueDouble;
}

/*!
Allocate the data buffer and read data from the file.
*/
void GRD98Process::loadDataBuffer(unsigned int nBlock) const
{
	int nOffset = m_header.getHeaderSize() + (nBlock*knMaxMemoryBlock);
	
	int nNumBytes = std::min(knMaxMemoryBlock, m_nFileSize - nOffset);

	if (0 >= nNumBytes)
	{
		m_in.close();
		PYXTHROW(	PYXDataSourceException,
					"Invalid file name: '" << getID() << "'."	);
	}

	// Position outselves at read point of file.
	m_in.seekg(nOffset);

	// Allocate memory and read file.

	MemoryResource* pMemoryResource = MemoryManager::getInstance()->requestMemory(nNumBytes);

	if (0 == pMemoryResource)
	{
		m_in.close();
		PYXTHROW(	PYXDataSourceException,
					"Unable to allocate data buffer: '" << nNumBytes << "' bytes.");
	}

	try
	{
		m_in.read(pMemoryResource->getPtr(), nNumBytes);
		m_vecMemoryResource[nBlock]=pMemoryResource;
	}
	catch (std::ios_base::failure& f)
	{
		// cleanup memory
		std::vector<MemoryResource*>::iterator it = m_vecMemoryResource.begin();
		while (it != m_vecMemoryResource.end())
		{
			delete (*it);
		}

		// a read error has occurred
		PYXTHROW(PYXDataSourceException, "Unable to read file: " << f.what());
	}
	catch (...)
	{
		// cleanup memory
		std::vector<MemoryResource*>::iterator it = m_vecMemoryResource.begin();
		while (it != m_vecMemoryResource.end())
		{
			delete (*it);
		}

		throw;
	}
}

/*!
Open the data file for reading.

\return	The input stream
*/
void GRD98Process::openFileForRead(std::ifstream& in) const
{
	assert(!getID().empty());

	// open the file
	if (!in.is_open())
	{
		in.open(getID().c_str(), std::ios::binary);
		if (!in.good())
		{
			PYXTHROW(	PYXDataSourceException,
						"Unable to open file: '" << getID() << "'."	);
		}
	}
	// setup for exceptions on input file
	in.exceptions(	std::ios::badbit |
					std::ios::failbit |
					std::ios::eofbit	);
}

/*!
Initializes the metadata.
*/
void GRD98Process::initMetaData()
{
	m_spDefn = PYXTableDefinition::create();
	addField("elev hint", PYXFieldDefinition::knContextNone, PYXValue::knBool, 1, PYXValue(true));

	addField(
		"origin longitude",
		PYXFieldDefinition::knContextNone,
		PYXValue::knInt32,
		1,
		PYXValue(m_header.getOriginLongitude())	);

	addField(
		"origin latitude",
		PYXFieldDefinition::knContextNone,
		PYXValue::knInt32,
		1,
		PYXValue(m_header.getOriginLatitude())	);

	addField(
		"longitude resolution",
		PYXFieldDefinition::knContextNone,
		PYXValue::knInt32,
		1,
		PYXValue(m_header.getLongitudeDataInterval())	);

	addField(
		"latitude resolution",
		PYXFieldDefinition::knContextNone,
		PYXValue::knInt32,
		1,
		PYXValue(m_header.getLongitudeDataInterval())	);

	addField(
		"longitude points",
		PYXFieldDefinition::knContextNone,
		PYXValue::knInt32,
		1,
		PYXValue(m_header.getLongitudePoints())	);

	addField(
		"latitude lines",
		PYXFieldDefinition::knContextNone,
		PYXValue::knInt32,
		1,
		PYXValue(m_header.getLatitudeLines())	);

	m_spCovDefn = PYXTableDefinition::create();
	addContentDefinition(m_spCovDefn);
}

/*!
Add the definition for the data set's content

\param pTableDefn	The table definition.
*/
void GRD98Process::addContentDefinition(PYXPointer<PYXTableDefinition> pTableDefn)
{
	pTableDefn->addFieldDefinition(
		"Elevation",
		PYXFieldDefinition::knContextElevation,
		PYXValue::knDouble,
		1	);
}