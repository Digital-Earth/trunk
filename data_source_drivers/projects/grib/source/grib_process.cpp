/******************************************************************************
grib_process.cpp

begin		: 2006-03-20
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define GRIB_SOURCE
#include "grib_process.h"

// local includes
#include "grib_record.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/derm/wgs84.h"
#include "pyxis/pipe/pipe_utils.h"
#include "pyxis/sampling/spatial_reference_system.h"
#include "pyxis/sampling/xy_utils.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <algorithm>
#include <cassert>

// {66794B66-6449-4946-AC21-71FDAA7D5FB4}
PYXCOM_DEFINE_CLSID(GRIBProcess, 
0x66794b66, 0x6449, 0x4946, 0xac, 0x21, 0x71, 0xfd, 0xaa, 0x7d, 0x5f, 0xb4);
PYXCOM_CLASS_INTERFACES(GRIBProcess, IProcess::iid, IXYCoverage::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(GRIBProcess, "GRIB File Reader", "GRIdded Binary weather data", "Reader",
					IXYCoverage::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_PARAMETER(IPath::iid, 1, 1, "Grib File Path", "A Process containing the path of the file to open");
IPROCESS_SPEC_END

// The spatial accuracy for GRIB files in metres.
static const double kfSpatialAccuracy = 50.0;

// The data accuracy for GRIB files in metres.
static const double kfDataAccuracy = 30.0;

// The data precision for GRIB files in metres.
static const double kfDataPrecision = 20.0;

//! The name of the description field
static const std::string kstrImageDescription = "image description";

// Tester class
Tester<GRIBProcess> gTester;

// Test method
void GRIBProcess::test()
{
	//Tested in PipeBuilderManager
}

/*!
Set the record number within the grib file to use for the process. The
file is not necessarily open or even know at the time this variable is
set so no checking is done to see if the value is within range.

\param nRecord	The image number within the GRIB file.
*/
void GRIBProcess::setRecord(int nRecord)
{
	m_nRecord = nRecord;
}

//! Serialize the process to a map of strings.
std::map<std::string, std::string> STDMETHODCALLTYPE GRIBProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	mapAttr["image_index"] = StringUtils::toString(m_nRecord);

	return mapAttr;
}

std::string STDMETHODCALLTYPE GRIBProcess::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"GRIBProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"image_index\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Image Index</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"			  
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

//! Deserialize the process from a map of strings.
void STDMETHODCALLTYPE GRIBProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::map<std::string, std::string>::const_iterator it;

	it = mapAttr.find("image_index");
	if (it == mapAttr.end())
	{
		PYXTHROW(PYXDataException, "Unable to find 'record' in GRIB attributes.");
	}
	m_nRecord = atoi(it->second.c_str());
}

IProcess::eInitStatus GRIBProcess::initImpl()
{
	m_spPath = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IPath>();
	try
	{
		readFile(FileUtils::stringToPath(m_spPath->getLocallyResolvedPath().c_str()), m_nRecord);
	}
	catch(PYXException&)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Error during grib file read");
		return knFailedToInit;
	}
	return knInitialized;
}

/*!
Open the GRIB data file. Opens Version 1 files only. Throws an exception if the file
can not be read.

\param path		The boost filesystem path to open.
\param nRecord	The grib record to open within the file.
*/
void GRIBProcess::readFile(const boost::filesystem::path& path, int nRecord)
{
	// If we do not have any images in the file we were unable to open it.  Exit.
	if (!m_gribReader.open(path, nRecord))
	{
		TRACE_INFO("No valid images found in grib file '" << FileUtils::pathToString(path) << "'.");
		PYXTHROW(PYXException, "Grib reader process cannot open: " << FileUtils::pathToString(path));
	}
	else
	{
		m_nRecord = nRecord;
		m_strID = FileUtils::pathToString(path);

		// initialize the metadata for the coverage
		initMetaData();
		
		/*
		NOTE: Native coordinates for this data source start at zero and extend
		for the width of data.
		*/
		m_bounds.setEmpty();
		m_bounds.setXMin(0.0);
		m_bounds.setXMax(m_gribReader.getRecord().getBandXSizeDouble());
		m_bounds.setYMin(0.0);
		m_bounds.setYMax(m_gribReader.getRecord().getBandYSizeDouble());

		// Set left edge of data source in coord converter.
		m_coordConverter.initialize(
			m_gribReader.getRecord().getLonMin(), 
			m_gribReader.getRecord().getLatMin(),
			m_gribReader.getRecord().getLonStep(), 
			m_gribReader.getRecord().getLatStep()	);
	}
}

PYXCoord2DDouble GRIBProcess::nativeToRasterSubPixel(const PYXCoord2DDouble & native) const
{
	PYXCoord2DDouble raster;
	XYUtils::nativeToLowerRasterSubPixel(getBounds(),getStepSize(),native,&raster);
	return raster;
}

/*!
Calculate the spatial precision. Determine the coordinate system of the 
datasource and call the appropriate method.

\return	The spatial precision in metres or -1.0 if not known.
*/
double GRIBProcess::getSpatialPrecision() const
{
	CoordLatLon pt1;
	CoordLatLon pt2;

	// the worst case latitude distance occurs at the most extreme latitude
	double fPixelHeight = m_gribReader.getRecord().getLatStep();
	double fDelta = MathUtils::degreesToRadians(fPixelHeight);

	pt1 = CoordLatLon(
		m_gribReader.getRecord().getLatStep(), 
		m_gribReader.getRecord().getLonStep());
	pt2 = CoordLatLon(pt1.lat() + fDelta, pt1.lon());

	HorizontalDatum const * pDatum = WGS84::getInstance();
	double fLatDistance = pDatum->calcDistance(pt1, pt2);

	// the largest longitude distance occurs at the smallest latitude
	double fPixelWidth = m_gribReader.getRecord().getLonStep();
	fDelta = MathUtils::degreesToRadians(fPixelWidth);

	pt1 = CoordLatLon(
		m_gribReader.getRecord().getLatStep(), 
		m_gribReader.getRecord().getLonStep());
	pt2 = CoordLatLon(pt1.lat(), pt1.lon() + fDelta);
	double fLonDistance = pDatum->calcDistance(pt1, pt2);
	return std::max(fLatDistance, fLonDistance) / 2.0;
}

/*!
Get the value at the specified native integer raster coordinate.

\param	raster			The native raster coordinate.

\return	The value as a PYXValue.
*/
bool GRIBProcess::getValue(const PYXCoord2DInt& raster, PYXValue* pValue) const
{
	// get the value from the reader
	return m_gribReader.getValue(raster, pValue);
}

/*!
Get the coverage value at the specified native coordinates.
This call MUST be thread-safe.

\param	native		The native coordinates.
\param  pValue      an pointer to the PYXValue to be filled in

\return	True if a data value was retrieved, false if there was no value to return.
*/
bool GRIBProcess::getCoverageValue(
	const PYXCoord2DDouble& native,
	PYXValue* pValue) const
{
	// this routine is a Critical Section in terms of thread safety
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	// convert to raster coordinates
	PYXCoord2DInt raster;
	raster.setX(MathUtils::round(native.x()));
	raster.setY(MathUtils::round(native.y()));

	return getValue(raster, pValue);
}

/*!
Get the field values around the specified native coordinate.
The origin of the matrix of returned values will be at the grid point 
in the mesh that is lower or equal to the nativeCentre point and adjusted 
left (or down) by trunc((size - 1)/2).  

If the values to be returned fall outside of the current data set, then 
the edges of the data set will be duplicated and returned.

\param	nativeCentre	The native coordinate.
\param  pValues         an array of PYXValue to be filled in with field values
						that are centered on the point requested.
\param  sizeX           width of PYXValue array
\param  sizeY           height of PYXValue array
*/
void GRIBProcess::getMatrixOfValues(
	const PYXCoord2DDouble& nativeCentre,
	PYXValue* pValues,
	int sizeX,
	int sizeY) const
{
	// this routine is a Critical Section in terms of thread safety
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	PYXValue nullValue;

	assert(pValues != 0 && "validate the pointer to the array.");
	assert(sizeX > 0 && "we want an array size that is positive");
	assert(sizeY > 0 && "we want an array size that is positive");

	if (m_bounds.inside(nativeCentre))
	{
		// calculate the lower left corner in raster coordinates.
		PYXCoord2DInt rasterBaseLL;
		XYUtils::nativeToLowerRaster(getBounds(), getStepSize(), nativeCentre, &rasterBaseLL);
		rasterBaseLL.setX(rasterBaseLL.x() - static_cast<int>((sizeX - 1) / 2.0));
		rasterBaseLL.setY(rasterBaseLL.y() - static_cast<int>((sizeY - 1) / 2.0));

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
		rasterMax.setX(rasterMax.x() - 1);
		rasterMax.setY(rasterMax.y() - 1);

		PYXValue pv((double)0.0);
		PYXCoord2DInt raster;  // the position of the current point we are retrieving.
		for (int x = 0; x < sizeX; x++)
		{
			// set the x coordinate of the raster and limit it to the available data.
			raster.setX(rasterBaseLL.x() + x);
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
				raster.setY(rasterBaseLL.y() + y);
				if (raster.y() < 0)
				{
					raster.setY(0);
				}
				if (raster.y() > rasterMax.y())
				{
					raster.setY(rasterMax.y());
				}

				// fill in the value
				if (getValue(raster, &pv))
				{
					pValues[x*sizeX+y] = pv;
				}
				else 
				{
					//fill as null value
					pValues[x*sizeX+y] = nullValue;
				}
			}
		}
	}
}

/*!
Create a new coverage definition for the the the metadata for the data source.
*/
void GRIBProcess::initMetaData()
{
	// Set the coverage definition. Always a single double precision value
	m_spCovDefn = PYXTableDefinition::create();
	m_spCovDefn->addFieldDefinition(
		"Unknown",
		PYXFieldDefinition::knContextNone,
		PYXValue::knDouble,
		1	);

	// create a new definition
	m_spDefn = PYXTableDefinition::create();

	setProcDescription(m_gribReader.getRecord().getDescription());
	
	addField(
		"record description",
		PYXFieldDefinition::knContextNone,
		PYXValue::knString,
		1,
		PYXValue(m_gribReader.getRecord().getDescription())	);

	addField(	
		"longitude min",
		PYXFieldDefinition::knContextNone,
		PYXValue::knDouble,
		1,
		PYXValue(m_gribReader.getRecord().getLonMin())	);

	addField(
		"longitude max",
		PYXFieldDefinition::knContextNone,
		PYXValue::knDouble,
		1,
		PYXValue(m_gribReader.getRecord().getLonMax())	);

	addField(
		"latitude min",
		PYXFieldDefinition::knContextNone,
		PYXValue::knDouble,
		1,
		PYXValue(m_gribReader.getRecord().getLatMin())	);

	addField(
		"latitude max",
		PYXFieldDefinition::knContextNone,
		PYXValue::knDouble,
		1,
		PYXValue(m_gribReader.getRecord().getLatMax())	);

	addField(
		"longitude step",
		PYXFieldDefinition::knContextNone,
		PYXValue::knDouble,
		1,
		PYXValue(m_gribReader.getRecord().getLonStep())	);

	addField(
		"latitude step",
		PYXFieldDefinition::knContextNone,
		PYXValue::knDouble,
		1,
		PYXValue(m_gribReader.getRecord().getLatStep())	);

	addField(
		"native bounds x min",
		PYXFieldDefinition::knContextNone,
		PYXValue::knDouble,
		1,
		PYXValue(0.0)	);

	addField(
		"native bounds x max",
		PYXFieldDefinition::knContextNone,
		PYXValue::knDouble,
		1,
		PYXValue(m_gribReader.getRecord().getBandXSizeDouble())	);

	addField(
		"native bounds y min",
		PYXFieldDefinition::knContextNone,
		PYXValue::knDouble,
		1,
		PYXValue(0.0)	);

	addField(
		"native bounds y max",
		PYXFieldDefinition::knContextNone,
		PYXValue::knDouble,
		1,
		PYXValue(m_gribReader.getRecord().getBandYSizeDouble())	);
}

//! The equality operator.
bool operator ==(const GRIBProcess& lhs, const GRIBProcess& rhs)
{
	if (!lhs.m_spPath && !rhs.m_spPath)
	{
		return true;
	}
	
	if (!lhs.m_spPath || !rhs.m_spPath)
	{
		return false;
	}
	if (	lhs.m_spPath->getPath() == 
			rhs.m_spPath->getPath() &&
			lhs.m_nRecord == rhs.m_nRecord )
	{
		return true;
	}
	return false;
}

//! The inequality operator.
bool operator !=(const GRIBProcess& lhs, const GRIBProcess& rhs)
{
	if (lhs == rhs)
	{
		return false;
	}
	return true;
}
