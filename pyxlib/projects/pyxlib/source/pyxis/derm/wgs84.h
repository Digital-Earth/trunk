#ifndef WGS84_H
#define WGS84_H
/******************************************************************************
wgs84.h

begin		: 2004-04-01
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyxlib.h"
#include "horizontal_datum.h"
#include "..\utility\coord_2d.h"

//! Represents the World Geodetic System Datum of 1984
/*!
WGS84 provides calculations based on the World Geodetic System datum of 1984.
*/
class PYXLIB_DECL WGS84 : public HorizontalDatum
{
public:

	//! Get the instance of the datum.
	static WGS84 const * getInstance();

	/*!
	Get the name of this datum.

	\return	The name of this datum.
	*/
	virtual std::string getName() const {return "WGS84";}

	//! Calculate the distance in metres between two points.
	virtual double calcDistance(	const CoordLatLon& pt1,
									const CoordLatLon& pt2	) const;

	//! Convert a point to geocentric lat/lon coordinates.
	virtual CoordLatLon toGeocentric(const CoordLatLon& pt) const;

	//! Convert a point in to datum lat/lon coordinates.
	virtual CoordLatLon toDatum(const CoordLatLon& pt) const;

	//! Parsing Lat Lon from a string
	static bool tryParseFromString( std::string expression, PYXCoord2DDouble & wgs84 );

private:

	//! Hide Constructor
	WGS84() {}

	//! Disable copy constructor
	WGS84(const WGS84&);

	//! Disable copy assignment
	void operator=(const WGS84&);

	//! Initialize static data
	static void initStaticData();

	//! Free static data
	static void freeStaticData();

	//! Singleton instance
	static WGS84* m_pInstance;

	//! Allows PYXLibInstance to initialize the static data.
	friend class PYXLibInstance;
};

#endif	// WGS84_H
