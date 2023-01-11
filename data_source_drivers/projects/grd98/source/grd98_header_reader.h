#ifndef GRD98_HEADER_READER_H
#define GRD98_HEADER_READER_H
/******************************************************************************
grd98_header_reader.h

begin		: 2004-06-08
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_grd98.h"

// pyxlib includes
#include "pyxis/utility/coord_lat_lon.h"

// standard includes
#include <iosfwd>

/*!
The GRD98HeaderReader class contains logic for reading the header
GEODAS GRID DATA FORMAT - "GRD98" data file. The header is 128
bytes long. It is made up of 32 4byte signed integers.  

\verbatim

GRD98 Header format

Offset	Length	Name	Description
(bytes)	(bytes)			
-------------------------------------------------------------------------------
0       4       Version
                        1,000,000,001 is version 1.

4       4       Length	
                        Length of header (128).

8       4       Data Type
                        Describes what the cell values represent..
                            1 = Data, e.g. interpolated	depths
                            2 = Data Density
                            3 = Grid-Radius 
													
12      4       Latitude Degrees	
                        Degrees portion of uppermost cell’s latitude.

16      4       Latitude Minutes	
                        Minutes portion of uppermost cell’s latitude.

20      4       Latitude Seconds	
                        Seconds portion of uppermost cell’s latitude.

24      4       Latitude Cell Size	
                        Latitudinal size (height) of each cell in seconds. 
                    
28      4       Latitude Number of Cells
                        Number of rows in grid.

32      4       Longitude Degrees
                        Degrees portion of leftmost cell’s longitude.

36      4       Longitude Minutes
                        Minutes portion of leftmost cell’s longitude.

40      4       Longitude Seconds
                        Seconds portion of leftmost cell’s longitude.

44      4       Longitude Cell Size
                        Longitudinal size (width) of each cell
                        in seconds.

48      4       Longitude Number of Cells
                        Number columns in grid.

52      4       Minimum Value
                        Minimum value of all cells in grid, excluding empty 
                        grid cells. Per precision, i.e. based on actual numbers
                        found in cells.  E.g.  if the lowest cell value found
                        is -123 and	precision is 10ths of meters (-12.3 meters)
                        the Minimum Value is -123.

56      4       Maximum Value
                        Maximum value of all cells in grid, excluding empty 
                        grid cells. Per precision (see above).

60      4       Grid Radius
                        The Grid-Radius which was applied to the grid.  When a 
                        Grid-Radius of n is applied this means that only cells
                        which are within n cells of actual data will be filled
                        with data values. Cells for which real data is more
                        than n cells away will be given the Empty Grid Cell
                        Value. If the Grid-Radius equals -1 then Grid-Radius 
                        was not applied.

64      4       Precision
                        Precision of the cell data values.
                            1 = whole units
                            10 = tenths of units
	
68      4       Empty Grid Cell Value
                        Value placed in a cell with no data (e.g. grid-radius
                        was applied to a Data Grid). For Density Grids or Grid-
                        Radius Grids, land (as opposed to water) cells would 
                        contain this value if density and grid-radius were not
                        calculated for land cells.

72      4       Number Type
                        Byte size of cell data values, positive = integers, 
                        negative = floats (e.g. +2 = 2-byte integers).

76      4       Water Datum
                        The vertical datum used for non-land cell depths. Local
                        datums could be used for inland lakes, with land values
                        tied to mean sea level.  Using local water datums means
                        that water shore values will be zero. Using MSL for water
                        means that water shore values will match up with land shore 
                        values.
                            0 = Mean Sea Level
                            1 = Local Vertical Datum used for depths.

80      4       Data Value Limit
                        This is the maximum possible value for cell data. This
                        is used when there is a limit to the calculated values,
                        e.g. to keep values within The range of Number Type.
                        Cell values containing this number mean that the value
                        is this large or larger.  
                            0 = not applied

84      4       Unused

88      4       Unused

92      4       Unused

96      4       Unused

100     4       Unused

104     4       Unused

108     4       Unused

112     4       Unused

116     4       Unused

120     4       Unused

124     4       Unused
-------------------------------------------------------------------------------
\endverbatim

\sa See the file grd98.txt for more detail.

*/
//! Reads the GRD98 Header structure from the data file.
class GRD98HeaderReader 
{
private:

	//! Possible data formats of the value.
	enum eValueType
	{
		knValueTypeInt = 0,
		knValueTypeFloat
	};

	//! The format of the header.
	enum eHeaderFields
	{
		knFieldVersion = 0,
		knFieldLength,
		knFieldDataType,
		knFieldLatitudeDegree,
		knFieldLatitudeMinute,
		knFieldLatitudeSecond,
		knFieldLatitudeCellSize,
		knFieldLatitudeCellCount,
		knFieldLongitudeDegree,
		knFieldLongitudeMinute,
		knFieldLongitudeSecond,
		knFieldLongitudeCellSize,
		knFieldLongitudeCellCount,
		knFieldMinimumValue,
		knFieldMaximumValue,
		knFieldGridRadius,
		knFieldPrecision,
		knFieldEmptyGridCellValue,
		knFieldNumberType,
		knFieldWaterDatum,
		knFieldDataValueLimit
	};

	//! The Water Datum of the data.
	enum eWaterDatum
	{
		knWaterDatumMeanSeaLevel= 0,
		knWaterDatumLocalVerticalDatum
	};

private:

	//! Constructor.
	GRD98HeaderReader();

	//! Destructor.
	~GRD98HeaderReader();

	//! Disable copy constructor.
	GRD98HeaderReader(const GRD98HeaderReader&);

	//! Disable copy assignment.
	void operator=(const GRD98HeaderReader&);

private:

	//! Get the size of the file header in bytes.
	int getHeaderSize() const;

	//! Read a file header from a data file.
	void read(std::istream& in);

	//! Loads the uppermost Latitude.
	void loadLatitude(int* pnFieldArray);

	//! Loads the leftmost Longitude.
	void loadLongitude(int* pnFieldArray);

	/*!
	Get the longitude at the origin.

	\return	The longitude at the northwest corner of the cell in seconds.
	*/
	const int getOriginLongitude() const {return m_nOriginLongitude;}

	/*!
	Get the latitude at the origin.

	\return	The latitude at the northwest corner of the cell in seconds.
	*/
	const int getOriginLatitude() const {return m_nOriginLatitude;}

	/*!
	Get the longitude data interval in seconds.

	\return	The longitude data interval in seconds.
	*/
	int getLongitudeDataInterval() const {return m_nLongitudeDataInterval;}

	/*!
	Get the latitude data interval in seconds.
	
	\return	The latitude data interval in seconds.
	*/
	int getLatitudeDataInterval() const {return m_nLatitudeDataInterval;}

	/*!
	Get the number of longitude Points for a latitude line.

	\return	The number of longitude Points.
	*/
	int getLongitudePoints() const {return m_nLongitudePoints;}

	/*!
	Get the number of latitude lines in the dataset.

	\return	The number of latitude lines.
	*/
	int getLatitudeLines() const {return m_nLatitudeLines;}

	/*!
	Get the size in bytes of the value.

	\return	The size in bytes of the value.
	*/
	int getValueSize() const {return m_nValueSize;}

	/*!
	Get the value converter.

	\return	the factor required to convert the value to meters.
	*/
	int getValueConverter() const {return m_nValueConverter;}

	/*!
	Get the value data type.  It could be integer or float.

	\return	The data type of the value.
	*/
	eValueType getValueType() const {return m_nValueType;}

	/*!
	Get the the value that indicates an empty cell.

	\return	The empty cell value.
	*/
	int getEmptyValue() const {return m_nEmptyValue;}

#if 0
	//! Open file and return a FileInfo structure about it.
	bool getFileInfo(std::string& strURI, IDataDriver::FileInfo* pFileInfo, bool* pbExit);
#endif

private:

	//! Longitude at origin in seconds.
	int m_nOriginLongitude;

	//! Latitude at origin in seconds.
	int m_nOriginLatitude;

	//! Longitude data interval in seconds.
	int m_nLongitudeDataInterval;

	//! Latitude data interval in seconds.
	int m_nLatitudeDataInterval;

	//! Number of longitude Points in a latitude line.
	int m_nLongitudePoints;

	//! Number of latitude lines in the dataset.
	int m_nLatitudeLines;

	//! Value to represent an empty cell.
	int m_nEmptyValue;

	//! The Value size in bytes.
	int m_nValueSize;

	//! The Value Data Type.
	eValueType m_nValueType;

	//! The Value Converter to convert the value to metres.
	int m_nValueConverter;

	//! The Water Datum.
	eWaterDatum m_nWaterDatum;

private:
	
	friend class GRD98Process;
};

#endif
