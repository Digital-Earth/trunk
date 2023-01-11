/******************************************************************************
grd98_header_reader.cpp

begin		: 2004-06-08
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_GRD98_SOURCE
#include "grd98_header_reader.h"

// local includes
#include "pyxis/data/exceptions.h"
#include "pyxis/utility/math_utils.h"

// standard includes
#include <cassert>

//! Offset of user header label in data file.
static const unsigned int knHeaderOffset = 0;

//! Size of the header.
static const int knHeaderSize = 128;

//! Maximum possible Longitudinal degree.
static const int knMaxLongDeg = 180;

//! Minimum possible Longitudinal degree.
static const int knMinLongDeg = -knMaxLongDeg;

//! Maximum possible Longitudinal degree.
static const int knMaxLatDeg = 90;

//! Minimum possible Longitudinal degree.
static const int knMinLatDeg = -knMaxLatDeg;

//! Maximum possible second.
static const int knMaxSecond = 60;

//! Minimum possible second.
static const int knMinSecond = -knMaxSecond;

//! Maximum possible minute.
static const int knMaxMinute = 60;

//! Minumum possible minute.
static const int knMinMinute = -knMaxMinute;

//! Default value size.
static const int knDefaultValueSize = 4;

//! Size of the individual values.
static const int knFieldCount = 32;

//! Header version.
static const int knHeaderVersion = 1000000001;

//! Valid Precision values.
static const int knHeaderPrecisionOne = 1;
static const int knHeaderPrecisionTen = 10;

// Pre-calculated constants for performance.
static const int knSecondsPerMinute = 60;
static const int knMinutesPerDegree = 60;
static const int knSecondsPerDegree = knSecondsPerMinute * knMinutesPerDegree;

// Constants for null value handling
static const int knNullIntValue = -32767;
static const double kfNullDoubleValue = -99999.0;

/*!
Constructor initializes member variables.
*/
GRD98HeaderReader::GRD98HeaderReader() :
	m_nOriginLongitude(0),
	m_nOriginLatitude(0),
	m_nLongitudeDataInterval(0),
	m_nLatitudeDataInterval(0),
	m_nLongitudePoints(0),
	m_nLatitudeLines(0),
	m_nValueSize(knDefaultValueSize),
	m_nValueType(knValueTypeInt),
	m_nValueConverter(knHeaderPrecisionOne),
	m_nWaterDatum(knWaterDatumMeanSeaLevel)
{
}

/*!
Destructor does nothing.
*/
GRD98HeaderReader::~GRD98HeaderReader()
{
}

/*!
Get the size of the header in bytes.

\return	The size of the header in bytes.
*/
int GRD98HeaderReader::getHeaderSize() const
{
	return knHeaderSize;
}

/*!
Read the user header label from a data file. Assumes caller has set up stream
so an exception will be thrown if a read error occurs.

\param	in	The input file stream.
*/
void GRD98HeaderReader::read(std::istream& in)
{

	// check assumptions about integer size
	assert(sizeof(int) == 4);

	// save file position
	std::istream::pos_type nOldPos = in.tellg();

	// move to header
	in.seekg(knHeaderOffset);
	
	// read header data into the array
	int nFieldArray[knFieldCount];

	memset(nFieldArray, 0, sizeof(nFieldArray));

	in.read(reinterpret_cast<char*>(&nFieldArray), sizeof(nFieldArray));

	// Load the version
	int nVersion = nFieldArray[knFieldVersion];
	if (knHeaderVersion != nVersion)
	{
		PYXTHROW(	PYXDataSourceException,
					"Invalid header version: '" << nVersion << "'."	);
	}

	// Load the length
	int nLength = nFieldArray[knFieldLength];
	if (knHeaderSize != nLength)
	{
		PYXTHROW(PYXDataSourceException, "Invalid header size.");
	}

	// Load the DataType
	int nDataType = nFieldArray[knFieldDataType];

	// Load the latitude
	loadLatitude(nFieldArray);

	// Load the longitude
	loadLongitude(nFieldArray);

	// Load the latitude cell size
	m_nLatitudeDataInterval = nFieldArray[knFieldLatitudeCellSize];
	if (0 >= m_nLatitudeDataInterval)
	{
		PYXTHROW(PYXDataSourceException, "Invalid header size.");
	}

	// Load the longitude cell size
	m_nLongitudeDataInterval = nFieldArray[knFieldLongitudeCellSize];
	if (0 >= m_nLongitudeDataInterval)
	{
		PYXTHROW(PYXDataSourceException, "Invalid header size.");
	}

	// Load the latitude cell count
	m_nLatitudeLines = nFieldArray[knFieldLatitudeCellCount];
	if (0 >= m_nLatitudeLines)
	{
		PYXTHROW(PYXDataSourceException, "Invalid header size.");
	}

	// Load the longitude cell count
	m_nLongitudePoints = nFieldArray[knFieldLongitudeCellCount];
	if (0 >= m_nLongitudePoints)
	{
		PYXTHROW(PYXDataSourceException, "Invalid header size.");
	}

	// Load the Value Type and Value Size
	int nValueType = nFieldArray[knFieldNumberType];
	if (0 < nValueType)
	{
		m_nValueType = knValueTypeInt;
	}
	else
	{
		m_nValueType = knValueTypeFloat;
	}

	m_nValueSize = abs(nValueType);

	// Load the Empty Value.
	m_nEmptyValue = nFieldArray[knFieldEmptyGridCellValue];

	// Load the Value Converter.
	m_nValueConverter = nFieldArray[knFieldPrecision];
	if ( (knHeaderPrecisionOne != m_nValueConverter) &&
		 (knHeaderPrecisionTen != m_nValueConverter) )
	{
		PYXTHROW(	PYXDataSourceException,
				"Invalid value converter: '" << m_nValueConverter << "'."	);
	}

	// Load the Water Datum Value
	int nWaterDatum = nFieldArray[knFieldWaterDatum];
	if ( (-1 > nWaterDatum) ||
	     ( 1 < nWaterDatum) )
	{
		PYXTHROW(	PYXDataSourceException,
					"Invalid water datum: '" << nWaterDatum << "'."	);
	}

	m_nWaterDatum = static_cast<eWaterDatum>(nWaterDatum);

	// restore file position
	in.seekg(nOldPos);
}

/*!
Loads the Origins Latitude from the header

\param	nFieldArray	The FieldArray pointer
*/
void GRD98HeaderReader::loadLatitude(int* nFieldArray)
{

	// get the degrees
	int nLatitudeDegree = nFieldArray[knFieldLatitudeDegree];
	if ( (knMinLatDeg > nLatitudeDegree) ||
		 (knMaxLatDeg < nLatitudeDegree) )
	{
		PYXTHROW(PYXDataSourceException, "Invalid origin.");
	}

	// get the minutes
	int nLatitudeMinute = nFieldArray[knFieldLatitudeMinute];
	if ( (knMinMinute > nLatitudeMinute) ||
		 (knMaxMinute < nLatitudeMinute) )
	{
		PYXTHROW(PYXDataSourceException, "Invalid origin.");
	}

	// get the seconds
	int nLatitudeSecond = nFieldArray[knFieldLatitudeSecond];
	if ( (knMinSecond > nLatitudeSecond) ||
		 (knMaxSecond < nLatitudeSecond) )
	{
		PYXTHROW(PYXDataSourceException, "Invalid origin.");
	}

	// calculate total seconds
	m_nOriginLatitude = nLatitudeSecond;
	m_nOriginLatitude += nLatitudeMinute * knSecondsPerMinute;
	m_nOriginLatitude += nLatitudeDegree * knSecondsPerDegree;

}

/*!
Leads the Origins Longitude from the header

\param nFieldArray The Field Array pointer
*/
void GRD98HeaderReader::loadLongitude(int* nFieldArray)
{

	int nLongitudeDegree = nFieldArray[knFieldLongitudeDegree];
	if ( (knMinLongDeg > nLongitudeDegree) ||
		 (knMaxLongDeg < nLongitudeDegree) )
	{
		PYXTHROW(PYXDataSourceException, "Invalid origin.");
	}

	int nLongitudeMinute = nFieldArray[knFieldLongitudeMinute];
	if ( (knMinMinute > nLongitudeMinute) ||
		 (knMaxMinute < nLongitudeMinute) )
	{
		PYXTHROW(PYXDataSourceException, "Invalid origin.");
	}

	int nLongitudeSecond = nFieldArray[knFieldLongitudeSecond];
	if ( (knMinSecond > nLongitudeSecond) ||
		 (knMaxSecond < nLongitudeSecond) )
	{
		PYXTHROW(PYXDataSourceException, "Invalid origin.");
	}

	m_nOriginLongitude = nLongitudeSecond;
	m_nOriginLongitude += nLongitudeMinute * knSecondsPerMinute;
	m_nOriginLongitude += nLongitudeDegree * knSecondsPerDegree;
}

#if 0
/*!
Open file and return a FileInfo structure about it.

\param	strURI	Name and path of file to open.
\param	pFileInfo	Pointer to FileInfo structure to fill with information about file.
\param  pbExit	Pointer to optional bool set to true if the function should exit immediately.

\return	true if able to process file.  false otherwise.
*/
bool GRD98HeaderReader::getFileInfo(std::string& strURI, IDataDriver::FileInfo* pFileInfo, bool* pbExit)
{
	// open the file
	std::ifstream in;

	try
	{
		// open the file
		in.open(strURI.c_str(), std::ios::binary);

		if (!in.good())
		{
			return false;
		}

		// setup for exceptions on input file
		in.exceptions(	std::ios::badbit |
						std::ios::failbit |
						std::ios::eofbit	);

		// read the header
		read(in);
	}
	catch(...)
	{
		// Any errors - return false.  Unable to process file.
		return false;
	}

	// Place data values in FileInfo structure.
	pFileInfo->m_strURI = strURI;
	
	// All DTED files have one band.
	pFileInfo->m_nNumBands = 1;

	pFileInfo->m_nRasterSize.setX(getLongitudePoints());
	pFileInfo->m_nRasterSize.setY(getLatitudeLines());

	pFileInfo->m_nFileType = IDataDriver::GRD98;
	pFileInfo->m_nLayerType = IDataDriver::knCoverage;

	pFileInfo->m_strDriverName = "Pyxis GRD98 Reader";

	int nWidth = getLongitudeDataInterval() * (getLongitudePoints() - 1);
	int nHeight = getLatitudeDataInterval() * (getLatitudeLines() - 1);

	double fMinX = getOriginLongitude();
	double fMinY = getOriginLatitude() - nHeight;
	double fMaxX = getOriginLongitude() + nWidth;
	double fMaxY = getOriginLatitude();

	// NOTE: GRD98 coordinates are in seconds.  Convert to degrees (60*60).
	fMinX /= 3600.0;
	fMinY /= 3600.0;
	fMaxX /= 3600.0;
	fMaxY /= 3600.0;

	// store the extents of the data set
	pFileInfo->m_bounds.setXMin(fMinX);
	pFileInfo->m_bounds.setYMin(fMinY);
	pFileInfo->m_bounds.setXMax(fMaxX);
	pFileInfo->m_bounds.setYMax(fMaxY);

	IDataDriver::FileInfo::BandInfo bandInfo;

	bandInfo.m_bHaveBounds = false;
	bandInfo.m_bHasNoDataValue = true;
	bandInfo.m_nNumColorTableEntries = 0;
	bandInfo.m_strBandDesc = "Elevation";
	bandInfo.m_strColorInterpretation = "Undefined";
	bandInfo.m_strColorTableInterpretation = "";
	bandInfo.m_nNumOverviews = 0;

	pFileInfo->m_nFileInterpretation = IDataDriver::knElevation;

	if (GRD98HeaderReader::knValueTypeInt == getValueType())
	{
		bandInfo.m_strBandType = "Int32";
		bandInfo.m_fNoDataValue = knNullIntValue;
	}
	else
	{
		bandInfo.m_strBandType = "Float64";
		bandInfo.m_fNoDataValue = kfNullDoubleValue;
	}

	pFileInfo->m_vecBandInfo.push_back(bandInfo);

	in.close();

	return true;
}
#endif
