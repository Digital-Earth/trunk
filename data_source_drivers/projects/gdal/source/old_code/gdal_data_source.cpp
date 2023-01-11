/******************************************************************************
gdal_data_source.cpp

begin		: 2004-06-08
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define MODULE_GDAL_SOURCE
#include "gdal_data_source.h"

#include "pyxis/data/exceptions.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/reference_sphere.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/derm/wgs84.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/sampling/spatial_reference_system.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"

// Windows includes
#include <direct.h>

// GDAL includes
#include "cpl_string.h"
#include "gdal_priv.h"

// standard includes
#include <algorithm>
#include <cassert>

// The spatial accuracy for GDAL files in metres.
static const double kfSpatialAccuracy = 50.0;

// The data accuracy for GDAL files in metres.
static const double kfDataAccuracy = 30.0;

// The data precision for GDAL files in metres.
static const double kfDataPrecision = 20.0;

//! Tester class
TesterUnit<GDALDataSource> gTester;

//! Test method.
void GDALDataSource::test()
{
	std::string strFolderName;

	// set up the default data path
	char* pstrCWD = _getcwd(0, 0);
	strFolderName = pstrCWD;
	strFolderName += "\\";

	free(pstrCWD);

#if 0 

// First test color.
	{
		PYXSpatialReferenceSystem srs;

		srs.setSystem(PYXSpatialReferenceSystem::knSystemProjected);
		srs.setDatum(PYXSpatialReferenceSystem::knDatumWGS84);
		srs.setProjection(PYXSpatialReferenceSystem::knProjectionUTM );
		srs.setZone(19);

		GDALDataSource reader;

		std::string strFileName = strFolderName;
		strFileName += "e-4_o-32.tif";
//		strFileName += "94813_64.TIF";

		reader.open(strFileName, srs);
	
		assert(DataSetInfo::knRGB == reader.getInfo().getDataType());

		double fHigh = 0;
		double fLow = 0;
		char* pcValue = 0;

		bool bFirst = true;

		for (PYXPointer<PYXIterator> spIt(reader.getNativeIterator()); !spIt->end(); spIt->next())
		{
			const void* pValue = spIt->getFieldValue();
			if (0 != pValue)
			{
//				PYXRGB* pRGB = static_cast<PYXRGB*>(pValue);
			}
		}
	}

#endif

// Now Test Grey Scale.

#if 0
	{
		PYXSpatialReferenceSystem srs;

		srs.setSystem(PYXSpatialReferenceSystem::knSystemProjected);
		srs.setDatum(PYXSpatialReferenceSystem::knDatumWGS84);
		srs.setProjection(PYXSpatialReferenceSystem::knProjectionUTM );
		srs.setZone(19);

		GDALDataSource reader;

		std::string strFileName = strFolderName;
		strFileName += "e-4_o-32G.tif";

		reader.open(strFileName, srs);

		assert(DataSetInfo::knGreyScale == reader.getInfo().getDataType());

		unsigned char cHigh = 0;
		unsigned char cLow = 0;
		const unsigned char* pcValue = 0;

		bool bFirst = true;

		PYXPointer<PYXIterator> spIt = reader.getNativeIterator();
		for (; !spIt->end(); spIt->next())
		{
			const void* pValue = spIt->getFieldValue();
			if (0 != pValue)
			{
				pcValue = static_cast<const unsigned char*>(pValue);
				if(bFirst)
				{
					cLow = cHigh = *pcValue;
					bFirst = false;
				}
				else
				{
					if (cLow > *pcValue)
					{
						cLow = *pcValue;
					}
					else if (cHigh < *pcValue)
					{
						cHigh = *pcValue;
					}
				}
			}
		}
	}
#endif

}

/*!
Constructor.
*/
GDALDataSource::GDALDataSource() :
	m_bounds(),
	m_fSpatialAccuracy(-1.0),
	m_fSpatialPrecision(-1.0),
	m_bHasSRS(false)
{
}

/*!
Destructor deletes the GDALBandIterator and the GDALDataset.
This effectively closes the file.
*/
GDALDataSource::~GDALDataSource()
{
}

/*!
Open the GDAL data file.  

There are two ways that failure can occur:
1. It is not a valid GDAL data set, and
2. It is a valid GDAL data set but it had some problem opening it.

If it is not a valid GDAL data set, then the method will return false.
If it is valid but failed to open it, then an exception will be thrown.
If it is valid and was successfully opened, then the method will return true.

\param	strFileName			The file to open.
\param	bCreateGeometry		true to create the geometry, otherwise false

\returns false if it is not a valid GDAL data set, true if it opens successfully.
*/
bool GDALDataSource::open(	
	const std::string& strFileName,
	bool bCreateGeometry	)
{

	std::string strTheFileName(strFileName);

	// If we specify an overview - get the number and remove from the string name for opening.
	size_t nPos = strTheFileName.find("<O=");
	
	int nOverview = -1;

	if (nPos != std::string::npos)
	{
		strTheFileName = strFileName.substr(0, nPos); 

		// Get the overview from the URI and set it in the dataset reader.
		nOverview = atoi(strFileName.substr(nPos+3).c_str());
		m_datasetReader.setOverview(nOverview);
	}

	// Open the file and get its GDALDataset object handle.
	void* hDataset = GDALOpen(strTheFileName.c_str(), GA_ReadOnly);
    if (hDataset == 0)
    {
		TRACE_INFO("Invalid data set.");
		return false;
 	}

	TRACE_INFO("Opening GDAL data set '" << strFileName << "'.");

	try
	{
		setName(strFileName);

		/*
		Convert the handle to a GDALDataset object pointer. This pointer must be
		deleted in order to close the file. This object will be passed into the
		GDALDatasetReader object and will be deleted by that object.
		*/
		m_pDataset = static_cast<GDALDataset*>(hDataset);

		// initialize the MetaData object.
		m_metaDataGDAL.initialize(*m_pDataset, nOverview);

		// initialize the Conversion object
		std::string strWKT = m_pDataset->GetGCPProjection();
		std::string strWKTP = m_pDataset->GetProjectionRef();
		if (!strWKTP.empty())
		{
			strWKT = strWKTP;
		}

		if (!strWKT.empty())
		{
			getCoordConverter()->initialize(strWKT);
			m_bHasSRS = true;
		}

		readFile();
	}
	catch (PYXDataSourceException& e)
	{
		PYXRETHROW(
			e,
			PYXDataSourceException,
			" Unable to open '" << strFileName << "'."	);
	}

	return true;
}

/*! 
Set spatial reference system for the reader.

\param spSRS	The new spatial reference system for the object.
*/
void GDALDataSource::setSpatialReferenceSystem(PYXPointer<PYXSpatialReferenceSystem> spSRS)
{
	assert(m_bHasSRS == false);

	getCoordConverter()->initialize(*spSRS);
	m_bHasSRS = true;
}

/*!
Continue reading file after open and SRS has been set.
*/
void GDALDataSource::readFile()
{
	try
	{
		// open the DatasetReader.  Ownership is retained by GDALDataSource
		m_datasetReader.open(m_pDataset);
	}
	catch (PYXException& e)
	{
		PYXRETHROW(	e, PYXDataSourceException, 
					"Unable to open GDALDataset reader.");
	}

	// get the Data Reader Iterator.  (Ownership is transferred).
	m_spDataIterator = m_datasetReader.getIterator();
	if (m_spDataIterator.get() == 0)
	{
		PYXTHROW(PYXDataSourceException, "Invalid iterator.");
	}

	// set the bounds of the coverage in native coordinates.
	m_bounds.setEmpty();
	m_bounds.expand(m_metaDataGDAL.getSouthWest());
	m_bounds.expand(m_metaDataGDAL.getNorthEast());

	initMetaData();

	// tell the coordinate converter about the bounds
	getCoordConverter()->setNativeBounds(getBounds());
}

/*!
Calculate spatial precision for projection coordinates.

\return	The spatial precision in metres.
*/
double GDALDataSource::calcSpatialPrecisionProjected() const
{	
	double fPixelWidth = fabs(m_metaDataGDAL.getGeoTransform()[GDALMetaData::knGTPixelWidth]);
	double fPixelHeight = fabs(m_metaDataGDAL.getGeoTransform()[GDALMetaData::knGTPixelHeight]);
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
posts. Needs to be completed

\param	pDatum	The horizontal datum used to generate the data.

\return	The spatial precision in metres.
*/
double GDALDataSource::calcSpatialPrecisionGeographical(HorizontalDatum* pDatum) const
{
	double fDelta;
	CoordLatLon pt1;
	CoordLatLon pt2;

	// the worst case latitude distance occurs at the most extreme latitude
	double fPixelHeight = fabs(m_metaDataGDAL.getGeoTransform()[GDALMetaData::knGTPixelHeight]);
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
	double fPixelWidth = fabs(m_metaDataGDAL.getGeoTransform()[GDALMetaData::knGTPixelWidth]);
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
Calculate the spatial precision. Determine the coordinate system of the 
datasource and call the appropriate method.

\return	The spatial precision in metres or -1.0 if not known.
*/
double GDALDataSource::getSpatialPrecision() const
{
	if (m_bHasSRS)
	{
		// determine if it is a projected system or a geographic system.
		if (getCoordConverter()->isProjected())
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

/*!
Get the value at the specified native integer raster coordinate.

\param	raster			The native raster coordinate.

\return	The value as a PYXValue.
*/
PYXValue GDALDataSource::getValue(const PYXCoord2DInt& raster) const
{
	try
	{
		PYXValue value = m_datasetReader.getValue(raster);

		if (!value.isNull())
		{
			if (m_datasetReader.getContext() ==
				PYXFieldDefinition::knContextCLUT)
			{
				// Return value looked up in table.
				return lookupValue(value.getInt());
			}

			// Return value as is.
			return value;
		}
	}
	catch (PYXException& e)
	{
		PYXRETHROW(e, PYXDataSourceException, "Unable to read file.");
	}

	return PYXValue();
}

/*!
Get the coverage value at the specified raster coordinate.

\param	native		The native coordinate.
\param	nFieldIndex	The field index (ignored).

\return	The value.
*/
PYXValue GDALDataSource::getCoverageValue(
	const PYXCoord2DDouble& native,
	int nFieldIndex	) const
{
	// this routine is a Critical Section in terms of thread safety
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	assert(nFieldIndex == 0 && "Ignored, so should be 0.");

	// get the value at raster coordinates
	PYXCoord2DInt raster;
	nativeToNearestRaster(native, &raster);
	return getValue(raster);
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
void GDALDataSource::getMatrixOfAttrValues(	const PYXCoord2DDouble& nativeCentre,
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

	// calculate the lower left corner in raster coordinates.
	PYXCoord2DInt rasterBaseLL;
	nativeToLowerRaster(nativeCentre, &rasterBaseLL);
	rasterBaseLL.setX(rasterBaseLL.x()-static_cast<int>((sizeX - 1)/2.0));
	rasterBaseLL.setY(rasterBaseLL.y()-static_cast<int>((sizeY - 1)/2.0));

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
	nativeToLowerRaster(nativeMax, &rasterMax);
	rasterMax.setX(rasterMax.x()-1);
	rasterMax.setY(rasterMax.y()-1);

	PYXCoord2DInt raster;  // the position of the current point we are retrieving.
	for (int x = 0; x < sizeX; x++)
	{
		// set the x coordinate of the raster and limit it to the available data.
		raster.setX(rasterBaseLL.x()+x);
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
			raster.setY(rasterBaseLL.y()+y);
			if (raster.y() < 0)
			{
				raster.setY(0);
			}
			if (raster.y() > rasterMax.y())
			{
				raster.setY(rasterMax.y());
			}
			// fill in the value
			pValues[x*sizeX+y] = getValue(raster);
		}
	}
}

//! Get the distance between data points in this coverage
const PYXCoord2DDouble GDALDataSource::getStepSize() const
{
	// note: step size may be negative if data source is addressed from max to min
	double fXStepSize = m_metaDataGDAL.getGeoTransform()[GDALMetaData::knGTPixelWidth];
	double fYStepSize = m_metaDataGDAL.getGeoTransform()[GDALMetaData::knGTPixelHeight];

	return PYXCoord2DDouble(fXStepSize, fYStepSize);
}

/*! 
This method uses the GDAL reader associated with the data source to convert
a value into an array of 3 characters that can be interpreted as an RGB.

\param nIndex	The table index.
\return The looked up value.
*/
PYXValue GDALDataSource::lookupValue(int nIndex) const
{
	assert(	m_datasetReader.getContext() == PYXFieldDefinition::knContextCLUT && 
			"Invalid field count for lookup"	);

	// get the colour table
	GDALColorTable* pColourTable = m_datasetReader.getColourTable();
	if (pColourTable == 0)
	{
		PYXTHROW(	PYXDataSourceException,
					"Colour table not available for source '" << 
					getName() << "'."	);
	}

	const GDALColorEntry* pColourEntry = pColourTable->GetColorEntry(nIndex);
	if (pColourEntry != 0)
	{
		uint8_t data[3];

		// interpret the colour table result
		switch (pColourTable->GetPaletteInterpretation())
		{
			// Interpret the lookup as rgb
			case GPI_RGB:
			{
				assert(0 <= pColourEntry->c1 && pColourEntry->c1 <= 255);
				assert(0 <= pColourEntry->c2 && pColourEntry->c2 <= 255);
				assert(0 <= pColourEntry->c3 && pColourEntry->c3 <= 255);
				data[0] = static_cast<uint8_t>(pColourEntry->c1);
				data[1] = static_cast<uint8_t>(pColourEntry->c2);
				data[2] = static_cast<uint8_t>(pColourEntry->c3);
				break;
			}

			// Interpret the lookup as greyscale
			case GPI_Gray:
			{
				assert(0 <= pColourEntry->c1 && pColourEntry->c1 <= 255);
				data[0] = static_cast<uint8_t>(pColourEntry->c1);
				data[1] = static_cast<uint8_t>(pColourEntry->c1);
				data[2] = static_cast<uint8_t>(pColourEntry->c1);
				break;
			}

			// Interpretation is not supported
			default:
			{
				assert(false && "Invalid colour palette interpretaion.");
				PYXTHROW(	PYXDataSourceException,
							"Unsupported colour palette interpretation '"
								<< pColourTable->GetPaletteInterpretation()
								<< "'."	);
			}
		}

		// return with the lookup value intact
		return PYXValue(data, 3);
	}

	// the value could not be resolved
    return PYXValue();
}

/*!
Initializes the meta data.
*/
void GDALDataSource::initMetaData()
{
	std::string strTemp;

	char** papszMetadata = ((GDALMajorObject*) m_pDataset)->GetMetadata(NULL);
	int nCount = CSLCount(papszMetadata);
	for (int i = 0; i < nCount; i++ )
	{
		strTemp = papszMetadata[i];
		size_t nThePosn = strTemp.find_first_of('=');

		addField(
			strTemp.substr(0, nThePosn),
			PYXFieldDefinition::knContextNone,
			PYXValue::knString,
			1,
			PYXValue(strTemp.substr(nThePosn + 1))	);
	}

	PYXValue::eType nFieldType = m_datasetReader.getDataType();

	// set the data source type and coverage definition
	if (m_datasetReader.getDataSetType() == PYXDataSource::knDEM)
	{
		// elevation
		setType(knDEM);
	
		getCoverageDefinition()->addFieldDefinition(
			"1",
			m_datasetReader.getContext(),
			nFieldType,
			m_datasetReader.getBandCount()	);

		// Set to true as we do not specify a SRS for DEM data types.
		m_bHasSRS = true;
	}
	else
	{
		// raster
		setType(knRaster);

		getCoverageDefinition()->addFieldDefinition(
			"1",
			m_datasetReader.getContext(),
			nFieldType,
			m_datasetReader.getBandCount()	);
	}
}
