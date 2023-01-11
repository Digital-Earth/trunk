#ifndef PYXIS__UTILITY__ELLIPSOID_MATH_H
#define PYXIS__UTILITY__ELLIPSOID_MATH_H
/******************************************************************************
ellipsoid_math.h

begin		: 2004-04-01
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyxlib.h"
#include "coord_lat_lon.h"

//! Various ellipsoid math methods
/*!
This class contains various methods for dealing with the geometry of
ellipsoids.
*/
class EllipsoidMath
{
public:

	//! Test method
	static void test();

	//! Calculate the distance between two points
	static double calcDistance(	const CoordLatLon& pt1,
								const CoordLatLon& pt2,
								const double fA,
								const double fFlattening	);

	//! Calculate the average radius for an ellipsoid at a given latitude
	static double calcAverageRadius(	const double fA,
										const double fB,
										const double fLat	);

	//! Convert a geodetic latitude to a geocentric latitude
	static double geodeticToGeocentric(	const double fLat,
										const double fFlattening	);

	//! Convert a geocentric latitude to a geodetic latitude
	static double geocentricToGeodetic(	const double fLat,
										const double fFlattening	);

protected:

private:

};

#endif // guard
