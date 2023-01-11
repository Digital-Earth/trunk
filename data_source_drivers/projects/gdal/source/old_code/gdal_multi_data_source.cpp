/******************************************************************************
gdal_multi_data_source.cpp

begin		: 2004-10-15
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define MODULE_GDAL_SOURCE
#include "gdal_multi_data_source.h"

// local includes
#include "gdal_data_source.h"
#include "pyxis/data/exceptions.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/derm/reference_sphere.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/wgs84.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"

// standard includes
#include <cassert>
#include <io.h>

//! Tester class
TesterUnit<GDALMultiDataSource> gMultiReaderTester;

//! Test method
void GDALMultiDataSource::test()
{

#if 0

	PYXSpatialReferenceSystem spatialReference;

	spatialReference.setSystem(PYXSpatialReferenceSystem::knSystemProjected);
	spatialReference.setDatum(PYXSpatialReferenceSystem::knDatumWGS84);
	spatialReference.setProjection(PYXSpatialReferenceSystem::knProjectionUTM );
	spatialReference.setZone(19);
	
	GDALMultiDataSource reader;

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
GDALMultiDataSource::GDALMultiDataSource() :
	m_bounds()
{
}

/*!
Destructor cleans up memory.
*/
GDALMultiDataSource::~GDALMultiDataSource()
{
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
\param	bCreateGeometry		true to create the geometry, otherwise false

\returns false if it is not a valid GDAL multi data set, 
			true if it opens successfully.
*/
bool GDALMultiDataSource::open(	const std::string& strDir,
								const std::string& strFileExt)
{
	// If the data source name is not a directory, exit.
	if (!FileUtils::isDir(strDir))
	{
		TRACE_INFO("Invalid directory name '" << strDir << "'");
		return false;
	}

	// save the name of the data source
	setName(strDir);

	// initialize the rTree
	m_rTree.initialize();

	int nFoundFiles = 0;

	// Note: Windows specific code
	std::string strSearchDir(strDir);
	std::string strFileSpec(strSearchDir);
	strFileSpec.append("\\*.*");

	// find all directories in the current directory
	struct _finddata_t dir;
	intptr_t nDir;

	nDir = _findfirst(strFileSpec.c_str(), &dir);
	if (-1L != nDir)
	{
		do
		{
			if (dir.attrib & _A_SUBDIR)
			{
				// ignore current and parent directories
				if ('.' != dir.name[0])
				{
					// open the files in this directory
					std::string strSubDir(strSearchDir);

					strSubDir += '\\';
					strSubDir += dir.name;
					
					nFoundFiles += openFiles(strSubDir, strFileExt);
				}
			}

		} while (0 == _findnext(nDir, &dir));

		_findclose(nDir);
	}

	// We do not have a directory of directories.
	// Search specified directory for files.
	if (nFoundFiles == 0)
	{
		nFoundFiles = openFiles(strDir, strFileExt);
	}

	if (nFoundFiles == 0)
	{
		TRACE_INFO("Invalid data set.");
		return false;
	}
	
	// There should be items in the map
	assert(!m_mapDataSource.empty() && "Map should not be empty.");

	// flush all of the items to file for the rtree.
	m_rTree.flush();

	return true;
}

/*!
Get the spatial precision of the data.

\return	The spatial precision in metres or -1.0 if not known.
*/
double GDALMultiDataSource::getSpatialPrecision() const
{
	GDALDataSourceMap::iterator it = m_mapDataSource.begin();
	if (it->second->hasSpatialReferenceSystem())
	{
		return it->second->getSpatialPrecision();
	}

	return -1.0;
}
	


/*!
Call this method after the data source is opened to determine if the data
source contains a spatial reference. If not, a spatial reference must be
supplied by calling setSpatialReference() before getFeatureIterator() is
called. Native PYXIS data sources always return true.

\return	true if the data source has a spatial reference, otherwise false
*/
bool GDALMultiDataSource::hasSpatialReferenceSystem() const
{
	if (!m_mapDataSource.empty())
	{
		GDALDataSourceMap::iterator it = m_mapDataSource.begin();
		return it->second->hasSpatialReferenceSystem();
	}

	return false;
}

/*!
Specify the spatial reference for the data source. Call this method to set the
spatial reference if after the data source is opened hasSpatialReference()
returns false.

\param	spSRS	The spatial reference system.
*/
void GDALMultiDataSource::setSpatialReferenceSystem(PYXPointer<PYXSpatialReferenceSystem> spSRS)
{
	assert(spSRS && "Invalid argument.");
	assert(!hasSpatialReferenceSystem() && "Spatial reference system already set.");

	if (m_mapDataSource.empty())
	{
		PYXTHROW(PYXDataSourceException, "Multi data source has no children.");
	}

	GDALDataSourceMap::iterator it = m_mapDataSource.begin();
	for (; it != m_mapDataSource.end(); it++)
	{
		it->second->setSpatialReferenceSystem(spSRS);
	}
}


/*!
Open the files located in the directory specified.  

\param	strDir		Directory containing GDAL files.
\param	strFileExt	File extension of GDAL files (handles multiple file types)

\return	The number of files opened.
*/
int GDALMultiDataSource::openFiles(	const std::string& strDir,
								const std::string& strFileExt)
{
	void* pDataSource = 0;

	// Note: Windows specific code

	std::string strExt = "*.";
	strExt += strFileExt;

	int nOpenedFiles = 0;

	boost::shared_ptr<char> spOldCWD;
	try
	{
		std::string strSearchDir(strDir.c_str());

		std::string strFileSpec(strSearchDir);
		strFileSpec.append("\\");
		strFileSpec.append(strExt);

		// find all files in the current directory
		struct _finddata_t file;
		intptr_t nDir;

		nDir = _findfirst(strFileSpec.c_str(), &file);
		if (-1L != nDir)
		{
			do
			{
				// open the files in this directory
				std::string strFile(strSearchDir);

				strFile += '\\';
				strFile += file.name;
				
				// this method can throw
				if (!openFile(strFile))
				{
					return 0;
				}
				
				++nOpenedFiles;

			} while (0 == _findnext(nDir, &file));

			_findclose(nDir);
		}
	}
	catch (PYXException& e)
	{
		throw(e);
	}

	return nOpenedFiles;
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
bool GDALMultiDataSource::openFile(const std::string& strFileName)
{
	// create the data source
	PYXPointer<GDALDataSource> spDataSource(GDALDataSource::create());

	// open the data source.  
	if (!spDataSource->open(strFileName))
	{
		// if the data set is not a GDAL data set then false.
		return false;
	}

	TRACE_INFO("Opening GDAL data source.");

	// set the data source type from the first file
	if (m_mapDataSource.empty())
	{
		setType(spDataSource->getType());
	}
	else if (getType() != spDataSource->getType())
	{
		PYXTHROW(	
			PYXDataSourceException,
			"Type conflict in file: '" << strFileName << "'."	);
	}

	// update fields from the input data source
	updateFields(*spDataSource);

	// setup the coverage definition from the first file
	if (getCoverageDefinition()->getFieldCount() <= 0)
	{
		setCoverageDefinition(spDataSource->getCoverageDefinition()->clone());
	}

	// get the bounds of the reader
	const PYXRect2DDouble dataSourceBounds = spDataSource->getBounds();

	PYXCoord2DDouble pt(dataSourceBounds.xMin(), dataSourceBounds.yMin());
	if (m_mapDataSource.find(pt) != m_mapDataSource.end())
	{
		PYXTHROW(	
			PYXDataSourceException,
			"Spatial conflict in file: '" << strFileName << "'."	);
	}

	// keep track of data extents
	if (m_mapDataSource.empty())
	{
		m_bounds = dataSourceBounds;
	}
	else
	{
		m_bounds.expand(dataSourceBounds);
	}

	// Store the reader based on its geodetic southwest coordinate.
	m_mapDataSource[pt] = spDataSource;

	PYXCoord2DDouble ll = spDataSource->getMetaDataGDAL()->getSouthWest();
	PYXCoord2DDouble ur = spDataSource->getMetaDataGDAL()->getNorthEast();

	PYXRect2DDouble area(ll.x(), ll.y(), ur.x(), ur.y());

	// insert above two coordinates & labels in the rtree
	m_rTree.insert(area, (void *) spDataSource.get());

	return true;
}

/*!
Get the data source which contains information for a particular native
coordinate.

\param	native	The native coordinate.

\return	the GDALDataSource.
*/
const GDALDataSource* GDALMultiDataSource::getDataSource(const PYXCoord2DDouble& native) const
{
	return static_cast<const GDALDataSource*>(m_rTree.containsFirst(native));
}

/*!
\return The coordinate converter.
*/
const CoordConverterImpl* GDALMultiDataSource::getCoordConverter() const
{
	assert(!m_mapDataSource.empty() && "Data source not open.");

	return m_mapDataSource.begin()->second->getCoordConverter();
}

/*!
Get the coverage value at the specified native coordinate.

\param	native		The native coordinate.
\param	nFieldIndex	The field index (ignored).

\return	The value.
*/
PYXValue GDALMultiDataSource::getCoverageValue(
	const PYXCoord2DDouble& native,
	int nFieldIndex	) const
{
	// this routine is a Critical Section in terms of thread safety
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	assert(nFieldIndex == 0 && "Ignored, so should be 0.");
	assert(!m_mapDataSource.empty() && "Data source not open.");

	const GDALDataSource* pDataSource = getDataSource(native);
	if (0 != pDataSource)
	{
		return pDataSource->getCoverageValue(native);
	}

	return PYXValue();
}

/*!
Get the field values around the specified native coordinate.
The origin of the matrix of returned values will be at the grid point 
in the mesh that is lower or equal to the nativeCentre point and adjusted 
left (or down) by trunc((size - 1)/2).  

If the values to be returned fall outside of the current data set, then 
the edges of the data set will be duplicated and returned.

\param	nativeCentre	The native coordinate.
\param	nFieldIndex		The field index.
\param  pValues         an array of PYXValue to be filled in with field values
						that are centered on the point requested.
\param  sizeX           width of PYXValue array
\param  sizeY           height of PYXValue array
*/
void GDALMultiDataSource::getMatrixOfAttrValues(	const PYXCoord2DDouble& nativeCentre,
										PYXValue* pValues,
										int sizeX,
										int sizeY,
										int nFieldIndex) const
{
	// this routine is a Critical Section in terms of thread safety
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	assert(pValues != 0 && "validate the pointer to the array.");
	assert(sizeX > 0 && "we want an array size that is positive");
	assert(sizeY > 0 && "we want an array size that is positive");
	assert(nFieldIndex == 0 && "Ignored, so should be 0.");
	assert(!m_mapDataSource.empty() && "Data source not open.");

	// calculate the lower left corner in raster coordinates.
	PYXCoord2DInt rasterBaseLL;
	nativeToLowerRaster(nativeCentre, &rasterBaseLL);
	rasterBaseLL.setX(rasterBaseLL.x()-static_cast<int>((sizeX - 1)/2.0));
	rasterBaseLL.setY(rasterBaseLL.y()-static_cast<int>((sizeY - 1)/2.0));

	// find the data source for the lower left corner
	PYXCoord2DDouble nativeLL;
	rasterToNative(rasterBaseLL, &nativeLL);
	const GDALDataSource* pDataSourceLL = getDataSource(nativeLL);

	// find the data source for the upper right corner
	PYXCoord2DInt rasterUR;
	rasterUR.setX(rasterBaseLL.x()+sizeX-1);
	rasterUR.setY(rasterBaseLL.y()+sizeY-1);
	PYXCoord2DDouble nativeUR;
	rasterToNative(rasterUR, &nativeUR);
	const GDALDataSource* pDataSourceUR = getDataSource(nativeUR);

	// if the matrix is covered completely by one data sorce, or one
	// of the corners falls outside the data.
	if (pDataSourceLL == pDataSourceUR && 0 != pDataSourceLL)
	{
		pDataSourceLL->getMatrixOfAttrValues(nativeCentre, pValues, sizeX, sizeY, nFieldIndex);
	}
	else if (0 != pDataSourceLL && 0 == pDataSourceUR)
	{
		pDataSourceLL->getMatrixOfAttrValues(nativeCentre, pValues, sizeX, sizeY, nFieldIndex);
	}
	else if (0 != pDataSourceUR && 0 == pDataSourceLL)
	{
		pDataSourceUR->getMatrixOfAttrValues(nativeCentre, pValues, sizeX, sizeY, nFieldIndex);
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
				rasterToNative(rasterGetAt, &nativeGetAt);
				const GDALDataSource* pDataSource = getDataSource(nativeGetAt);
				if (0 != pDataSource)
				{
					pValues[x*sizeX+y] = pDataSource->getCoverageValue(nativeGetAt, nFieldIndex);
				}
			}
		}
	}
}

//! Get the distance between data points in this coverage
const PYXCoord2DDouble GDALMultiDataSource::getStepSize() const
{
	return m_mapDataSource.begin()->second->getStepSize();
}
