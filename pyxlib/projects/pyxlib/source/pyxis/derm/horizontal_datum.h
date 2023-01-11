#ifndef PYXIS__DERM__HORIZONTAL_DATUM_H
#define PYXIS__DERM__HORIZONTAL_DATUM_H
/******************************************************************************
horizontal_datum.h

begin		: 2004-02-17
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/coord_lat_lon.h"

// standard includes
#include <string>

/*!
HorizontalDatum is the abstract base for all horizontal datums used in spatial
reference systems. It provides calculations based on the datums.
*/
//! Represents a horizontal datum for a spatial reference system.
class PYXLIB_DECL HorizontalDatum
{
public:

	/*!
	Get the name of this datum.

	\return	The name of this datum.
	*/
	virtual std::string getName() const = 0;

	/*!
	Calculate the distance in metres between two points on the earth's
	surface.

	\param	pt1	The first point in datum lat/lon coordinates.
	\param	pt2	The second point in datum lat/lon coordinates.

	\return	The distance in metres.
	*/
	virtual double calcDistance(	const CoordLatLon& pt1,
									const CoordLatLon& pt2	) const = 0;

	/*!
	Convert a point in datum lat/lon coordinates to geocentric lat/lon
	coordinates.

	\param	pt	The point in datum lat/lon coordinates.

	\return	The point in geocentric lat/lon coordinates.
	*/
	virtual CoordLatLon toGeocentric(const CoordLatLon& pt) const = 0;

	/*!
	Convert a point in geocentric lat/lon coordinates to datum lat/lon
	coordinates.

	\param	pt	The point in geocentric lat/lon coordinates.

	\return	The point in datum lat/lon coordinates.
	*/
	virtual CoordLatLon toDatum(const CoordLatLon& pt) const = 0;

protected:

	//! Destructor
	virtual ~HorizontalDatum() {}
};

#endif // guard
