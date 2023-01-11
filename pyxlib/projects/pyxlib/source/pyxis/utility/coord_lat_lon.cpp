/******************************************************************************
coord_lat_lon.cpp

begin		: 2004-01-13
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/coord_lat_lon.h"

// local includes
#include "pyxis/utility/math_utils.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <cassert>
#include <cmath>
#include <cstdlib>

//! Maximum absolute latitude value in degrees.
const double CoordLatLon::kfLatitudeAbsMax = 90.0;
	
//! Maximum absolute longitude value in degrees.
const double CoordLatLon::kfLongitudeAbsMax = 180.0;

//! Tester class
Tester<CoordLatLon> gCoordLatLonTester;

//! Test method
void CoordLatLon::test()
{
	// test default constructor
	CoordLatLon latLon;
	TEST_ASSERT(MathUtils::equal(0.0, latLon.lat()));
	TEST_ASSERT(MathUtils::equal(0.0, latLon.lon()));

	// test normalization

	latLon.setLatInDegrees(45.0);
	TEST_ASSERT(MathUtils::equal(45.0, latLon.latInDegrees()));

	latLon.setLatInDegrees(135.0);
	TEST_ASSERT(MathUtils::equal(45.0, latLon.latInDegrees()));

	latLon.setLatInDegrees(225.0);
	TEST_ASSERT(MathUtils::equal(-45.0, latLon.latInDegrees()));

	latLon.setLatInDegrees(315.0);
	TEST_ASSERT(MathUtils::equal(-45.0, latLon.latInDegrees()));

	latLon.setLatInDegrees(405.0);
	TEST_ASSERT(MathUtils::equal(45.0, latLon.latInDegrees()));

	latLon.setLatInDegrees(-45.0);
	TEST_ASSERT(MathUtils::equal(-45.0, latLon.latInDegrees()));

	latLon.setLatInDegrees(-135.0);
	TEST_ASSERT(MathUtils::equal(-45.0, latLon.latInDegrees()));

	latLon.setLatInDegrees(-225.0);
	TEST_ASSERT(MathUtils::equal(45.0, latLon.latInDegrees()));

	latLon.setLatInDegrees(-315.0);
	TEST_ASSERT(MathUtils::equal(45.0, latLon.latInDegrees()));

	latLon.setLatInDegrees(-405.0);
	TEST_ASSERT(MathUtils::equal(-45.0, latLon.latInDegrees()));

	latLon.setLonInDegrees(45.0);
	TEST_ASSERT(MathUtils::equal(45.0, latLon.lonInDegrees()));

	latLon.setLonInDegrees(135.0);
	TEST_ASSERT(MathUtils::equal(135.0, latLon.lonInDegrees()));

	latLon.setLonInDegrees(225.0);
	TEST_ASSERT(MathUtils::equal(-135.0, latLon.lonInDegrees()));

	latLon.setLonInDegrees(315.0);
	TEST_ASSERT(MathUtils::equal(-45.0, latLon.lonInDegrees()));

	latLon.setLonInDegrees(405.0);
	TEST_ASSERT(MathUtils::equal(45.0, latLon.lonInDegrees()));

	latLon.setLonInDegrees(-45.0);
	TEST_ASSERT(MathUtils::equal(-45.0, latLon.lonInDegrees()));

	latLon.setLonInDegrees(-135.0);
	TEST_ASSERT(MathUtils::equal(-135.0, latLon.lonInDegrees()));

	latLon.setLonInDegrees(-225.0);
	TEST_ASSERT(MathUtils::equal(135.0, latLon.lonInDegrees()));

	latLon.setLonInDegrees(-315.0);
	TEST_ASSERT(MathUtils::equal(45.0, latLon.lonInDegrees()));

	latLon.setLonInDegrees(-405.0);
	TEST_ASSERT(MathUtils::equal(-45.0, latLon.lonInDegrees()));

	// test inside method

	CoordLatLon southWest;
	southWest.setInDegrees(-45.0, 90.0);

	CoordLatLon northEast;
	northEast.setInDegrees(45.0, -90.0);

	latLon.setInDegrees(0.0, 180.0);
	TEST_ASSERT(latLon.insideSWNE(southWest, northEast));
	TEST_ASSERT(!latLon.insideSWNE(northEast, southWest));

	latLon.setInDegrees(75.0, 180.0);
	TEST_ASSERT(!latLon.insideSWNE(southWest, northEast));
	TEST_ASSERT(!latLon.insideSWNE(northEast, southWest));

	latLon.setInDegrees(0.0, 0.0);
	TEST_ASSERT(!latLon.insideSWNE(southWest, northEast));
	TEST_ASSERT(!latLon.insideSWNE(northEast, southWest));

	// test north pole, south pole, equator, prime meridian and int'l date line
	latLon.setInDegrees(90.0, 0.0);
	TEST_ASSERT(latLon.isNorthPole());
	TEST_ASSERT(!latLon.isSouthPole());
	TEST_ASSERT(!latLon.isOnEquator());
	TEST_ASSERT(latLon.isOnPrimeMeridian());
	TEST_ASSERT(!latLon.isOnIntlDateLine());

	latLon.setInDegrees(-90.0, 180.0);
	TEST_ASSERT(!latLon.isNorthPole());
	TEST_ASSERT(latLon.isSouthPole());
	TEST_ASSERT(!latLon.isOnEquator());
	TEST_ASSERT(!latLon.isOnPrimeMeridian());
	TEST_ASSERT(latLon.isOnIntlDateLine());

	latLon.setInDegrees(0.0, 180.0);
	TEST_ASSERT(!latLon.isNorthPole());
	TEST_ASSERT(!latLon.isSouthPole());
	TEST_ASSERT(latLon.isOnEquator());

	// test equal

	CoordLatLon test;
	latLon.setInDegrees(45.0, 90.0);

	test.setInDegrees(45.0, 90.0);
	TEST_ASSERT(latLon.equal(test));

	test.setInDegrees(-45.0, -90.0);
	TEST_ASSERT(!latLon.equal(test));

	// test crossesIntlDateLine
	CoordLatLon ll1;
	CoordLatLon ll2;

	ll1.setInDegrees(0.0, 170.0);
	ll2.setInDegrees(0.0, -170.0);
	TEST_ASSERT(crossesIntlDateLine(ll1, ll2));

	ll1.setInDegrees(0.0, 10.0);
	ll2.setInDegrees(0.0, -10.0);
	TEST_ASSERT(!crossesIntlDateLine(ll1, ll2));
}



/*!
Constructor initializes member variables.

\param	fLat	The latitude in radians.
\param	fLon	The longitude in radians.
*/
CoordLatLon::CoordLatLon(double fLat, double fLon) :
	m_fLat(fLat),
	m_fLon(fLon)
{
	normalizeLatitude();
	normalizeLongitude();
}

/*!
Set the latitude in radians. This method normalizes the latitude to a value in
the range [-pi/2, pi/2].

\param fLat	The latitude in radians.
*/
void CoordLatLon::setLat(double fLat)
{
	m_fLat = fLat;
	normalizeLatitude();
}

/*!
Get the latitude in degrees.

\return	The latitude in degrees.
*/
double CoordLatLon::latInDegrees() const
{
	return MathUtils::radiansToDegrees(m_fLat);
}

/*!
Set the latitude in degrees.

\param	fLatInDegrees	The latitude in degrees.
*/
void CoordLatLon::setLatInDegrees(double fLatInDegrees)
{
	setLat(MathUtils::degreesToRadians(fLatInDegrees));
}

/*!
Set the longitude in radians. This method normalizes the longitude to a value
in the range [-pi, pi)

\param	fLon	The longitude in radians.
*/
void CoordLatLon::setLon(double fLon)
{
	m_fLon = fLon;
	normalizeLongitude();
}

/*!
Get the longitude in degrees.

\return	The longitude in degrees.
*/
double CoordLatLon::lonInDegrees() const
{
	return MathUtils::radiansToDegrees(m_fLon);
}

/*!
Set the longitude in degrees.

\param	fLonInDegrees	The longitude in degrees.
*/
void CoordLatLon::setLonInDegrees(double fLonInDegrees)
{
	setLon(MathUtils::degreesToRadians(fLonInDegrees));
}

/*!
Set the latitude and longitude in radians.

\param	fLat	The latitude in radians.
\param	fLon	The longitude in radians.
*/
void CoordLatLon::set(double fLat, double fLon)
{
	setLat(fLat);
	setLon(fLon);
}

/*!
Set the latitude and longitude in degrees.

\param	fLatInDegrees	The latitude in degrees.
\param	fLonInDegrees	The longitude in degrees.
*/
void CoordLatLon::setInDegrees(	double fLatInDegrees,
								double fLonInDegrees	)
{
	setLatInDegrees(fLatInDegrees);
	setLonInDegrees(fLonInDegrees);
}

/*!
Randomizes (uniformly) the latitude in the range [-pi/2, pi/2] and the longitude
in the range [-pi, pi).
*/
void CoordLatLon::randomize()
{
	// No need to normalize as we manage the range manually.
	m_fLat = static_cast<double>(rand()) / RAND_MAX *
		MathUtils::kfPI - MathUtils::kfPI / 2.0;
	m_fLon = static_cast<double>(rand()) / RAND_MAX *
		MathUtils::kfPI * 2.0 - MathUtils::kfPI;
	if (m_fLon == MathUtils::kfPI)
	{
		m_fLon = -MathUtils::kfPI;
	}
}

/*!
Perturbs (uniformly) the latitude in the range [-fMaxDegLat, fMaxDegLat] and
the longitude in the range [-fMaxDegLon, fMaxDegLon].

\param fMaxDegLat The maximum (non-negative) degrees to perturb the latitude.
\param fMaxDegLon The maximum (non-negative) degrees to perturb the longitude.
*/
void CoordLatLon::perturb(double fMaxDegLat, double fMaxDegLon)
{
	assert(0 <= fMaxDegLat);
	assert(0 <= fMaxDegLon);
	setLatInDegrees(
		latInDegrees() +
			static_cast<double>(rand()) / RAND_MAX *
				(2 * fMaxDegLat) - fMaxDegLat);
	setLonInDegrees(
		lonInDegrees() +
			static_cast<double>(rand()) / RAND_MAX *
				(2 * fMaxDegLon) - fMaxDegLon);
}

/*!
Perturbs (uniformly) the latitude in the range [-fMaxDeg, fMaxDeg] and the
longitude in the range [-fMaxDeg, fMaxDeg].

\param fMaxDeg The maximum (non-negative) degrees to perturb this coordinate.
*/
//! Perturbs this coordinate.
inline void CoordLatLon::perturb(double fMaxDeg)
{
	assert(0 <= fMaxDeg);
	perturb(fMaxDeg, fMaxDeg);
}

/*!
Does the latitude represent the north pole.

\param fEpsilon	The precision.

\return true if the latitude is the north pole, otherwise false.
*/
bool CoordLatLon::isNorthPole(double fEpsilon) const
{
	return MathUtils::equal(m_fLat, MathUtils::kf90Rad, fEpsilon);
}

/*!
Does the latitude represent the south pole.

\param fEpsilon	The precision.

\return true if the latitude is the south pole, otherwise false.
*/
bool CoordLatLon::isSouthPole(double fEpsilon) const
{
	return MathUtils::equal(m_fLat, -MathUtils::kf90Rad, fEpsilon);
}

/*!
Is the latitude on the equator.

\param fEpsilon	The precision.

\return true if the latitude is on the equator, otherwise false.
*/
bool CoordLatLon::isOnEquator(double fEpsilon) const
{
	return MathUtils::equal(m_fLat, 0.0, fEpsilon);
}

/*!
Is the longitude on the the prime meridian.

\param	fEpsilon	The precision.

\return	true if the longitude is on the prime meridian, otherwise false
*/
bool CoordLatLon::isOnPrimeMeridian(double fEpsilon) const
{
	return MathUtils::equal(m_fLon, 0.0, fEpsilon);
}

/*!
Is the longitude on the international date line.

\param	fEpsilon	The precision.

\return	true if the latitude is on the international date line, otherwise false
*/
bool CoordLatLon::isOnIntlDateLine(double fEpsilon) const
{
	return (	MathUtils::equal(m_fLon, MathUtils::kf180Rad, fEpsilon) ||
				MathUtils::equal(m_fLon, -MathUtils::kf180Rad, fEpsilon)	);
}

/*!
Are the coordinates equal within a given precision. This method handles the
special conditions at the poles and at the international date line. The
coordinates must be normalized before entry to this method.

\param latLon		The latitude / longitude to compare.
\param fEpsilon	The precision to use for comparison.

\return true if the coordinates are equal otherwise false.
*/
bool CoordLatLon::equal(const CoordLatLon& latLon, double fEpsilon) const
{
	bool bEqual = false;

	// handle special cases at the poles, ignore longitude
	if (isNorthPole(fEpsilon))
	{
		bEqual = latLon.isNorthPole(fEpsilon);
	}
	else if (isSouthPole(fEpsilon))
	{
		bEqual = latLon.isSouthPole(fEpsilon);
	}
	else if (MathUtils::equal(m_fLat, latLon.m_fLat, fEpsilon))
	{
		// latitudes are equal and not at poles, check longitude

		// handle special case on the international date line
		if (isOnIntlDateLine(fEpsilon))
		{
			bEqual = latLon.isOnIntlDateLine(fEpsilon);
		}
		else
		{
			bEqual = MathUtils::equal(m_fLon, latLon.m_fLon, fEpsilon);
		}
	}

	return bEqual;
}

/*!
Determine if a coordinate falls inside or on the border of the rectangle
defined by a southwest and northeast coordinate.

\return	true if the coordinate is inside the rectangle, otherwise false.
*/
bool CoordLatLon::insideSWNE(	const CoordLatLon& southWest,
								const CoordLatLon& northEast	) const
{
	bool bInside = false;;

	// check latitudes first
	if ((southWest.lat() <= m_fLat) && (northEast.lat() >= m_fLat))
	{
		/*
		Check for special case where the longitudes straddle the international
		date line.
		*/
		if (southWest.lon() > northEast.lon())
		{
			bInside = (	(southWest.lon() <= m_fLon) ||
						(northEast.lon() >= m_fLon)	);
		}
		else
		{
			// normal case
			bInside = (	(southWest.lon() <= m_fLon) &&
						(northEast.lon() >= m_fLon)	);
		}
	}

	return bInside;
}

/*!
Determine if a coordinate falls inside or on the border of the rectangle
defined by a northwest and southeast coordinate.

\return	true if the coordinate is inside the rectangle, otherwise false.
*/
bool CoordLatLon::insideNWSE(	const CoordLatLon& northWest,
								const CoordLatLon& southEast) const
{
	bool bInside = false;;

	// check latitudes first
	if ((southEast.lat() <= m_fLat) && (northWest.lat() >= m_fLat))
	{
		/*
		Check for special case where the longitudes straddle the international
		date line.
		*/
		if (northWest.lon() > southEast.lon())
		{
			bInside = (	(northWest.lon() <= m_fLon) ||
						(southEast.lon() >= m_fLon)	);
		}
		else
		{
			// normal case
			bInside = (	(northWest.lon() <= m_fLon) &&
						(southEast.lon() >= m_fLon)	);
		}
	}

	return bInside;
}

/*!
Normalize the latitude. Convert to a value in the range [-pi/2, pi/2].
*/
void CoordLatLon::normalizeLatitude()
{
	// convert latitude to value in the range [0, 2pi)
	int nMultiple = static_cast<int>(m_fLat / (2.0 * MathUtils::kfPI));
	m_fLat -= 2.0 * MathUtils::kfPI * nMultiple;

	if (0.0 > m_fLat)
	{
		m_fLat = 2.0 * MathUtils::kfPI + m_fLat;
	}

	// convert latitude to a value in the range [-pi/2, pi/2]
	if ((3.0 * MathUtils::kfPI / 2.0) < m_fLat)
	{
		m_fLat = m_fLat - 2.0 * MathUtils::kfPI;
	}
	else if (MathUtils::kfPI < m_fLat)
	{
		m_fLat = MathUtils::kfPI - m_fLat;
	}
	else if ((MathUtils::kfPI / 2.0) < m_fLat)
	{
		m_fLat = MathUtils::kfPI - m_fLat;
	}
}

/*!
Normalize the longitude. Convert to a value in the range [-pi, pi).
*/
void CoordLatLon::normalizeLongitude()
{
	// convert longitude to value in the range [0, 2pi)
	int nMultiple = static_cast<int>(m_fLon / (2.0 * MathUtils::kfPI));
	m_fLon -= 2.0 * MathUtils::kfPI * nMultiple;

	// convert longitude to value in the range [-pi, pi)
	if (MathUtils::kfPI <= m_fLon)
	{
		m_fLon -= 2.0 * MathUtils::kfPI;
	}
	else if (-MathUtils::kfPI > m_fLon)
	{
		m_fLon += 2.0 * MathUtils::kfPI;
	}
}

/*!
Allows latitude/longitude coordinates to be written to streams.

\param	out			The output stream.
\param	latLon		The latitude and longitude.

\return	A reference to the output stream.
*/
std::ostream& operator<< (std::ostream& out, const CoordLatLon& latLon)
{
	double fDegrees;
	char cHemi;

	fDegrees = latLon.latInDegrees();
	cHemi = (0 <= fDegrees) ? 'N' : 'S';
	out << fabs(fDegrees) << cHemi;
	
	out << ' ';

	fDegrees = latLon.lonInDegrees();
	cHemi = (0 <= fDegrees) ? 'E' : 'W';
	out << fabs(fDegrees) << cHemi;

	return out;
}

/*!
Set the point and calculate the precomputed values.

\param	point	The point in lat/lon coordinates.
*/
void PreCompLatLon::setPoint(const CoordLatLon& point)
{
	m_point = point;
	m_fSinLat = sin(m_point.lat());
	m_fSinLon = sin(m_point.lon());
	m_fCosLat = cos(m_point.lat());
	m_fCosLon = cos(m_point.lon());
}

/*!
Determine if the shortest great circle arc between two points crosses the
international date line.

\param	pt1	The first point.
\param	pt2	The second coordinate.

\return	true if the coordinates straddle the date line otherwise false.
*/
bool CoordLatLon::crossesIntlDateLine(	const CoordLatLon& pt1,
										const CoordLatLon& pt2	)
{
	return (	!pt1.isOnIntlDateLine() &&
				!pt2.isOnIntlDateLine() &&
				(MathUtils::kfPI < fabs(pt2.lon() - pt1.lon()))	);
}

/*!
Equality than operator provided to allow ordering in containers.

\param	pt	The coordinates to be compared with this one.

\return	true if this coordinate is equal to the one passed in otherwise false.
*/
bool CoordLatLon::operator==(const CoordLatLon& pt) const
{
	return (	MathUtils::equal(lon(), pt.lon()) &&
				MathUtils::equal(lat(), pt.lat())	);
}

/*!
Less than operator provided to allow ordering in containers. Sorting is done
first by longitude, then by latitude.

\param	pt	The coordinates to be compared with this one.

\return	true if this coordinate is less than the one passed in otherwise false.
*/
bool CoordLatLon::operator<(const CoordLatLon& pt) const
{
	bool bLess = (lon() < pt.lon());

	if (MathUtils::equal(lon(), pt.lon()))
	{
		bLess = (lat() < pt.lat());
	}

	return bLess;
}


/* 
Based on the precision of the entries, calculates the resolution
in Radians.

\param strLatitude The latitude.
\param strLongitude The longitude.

return The resolution in radians.
*/
double CoordLatLon::calculateResolutionRadians(	const std::string& strLatitude,
												const std::string& strLongitude	)
{
	// get maximum number of decimal places
	int nLatDecimals = StringUtils::countDecimalPlaces(strLatitude);
	int nLonDecimals = StringUtils::countDecimalPlaces(strLongitude);

	int nDecimals = (nLatDecimals > nLonDecimals) ? nLatDecimals : nLonDecimals;

	// convert to a precision on the ground
	double fDegrees = 0.5;
	for (int nCount = nDecimals; nCount > 0; --nCount)
	{
		fDegrees /= 10.0;
	}

	double fRadians = MathUtils::degreesToRadians(fDegrees);
	
	return fRadians;
}

/* 
Check to see if the latitude in degrees is valid.

\param fLatitude The latitude in degrees.

\return true if valid, false otherwise.
*/
bool CoordLatLon::isLatitudeValid(double fLatitude)
{
	if (abs(fLatitude) > kfLatitudeAbsMax)
	{
		return false;
	}
	return true;
}

/* 
Check to see if the longitude in degrees is valid.

\param fLatitude The longitude in degrees.

\return true if valid, false otherwise.
*/
bool CoordLatLon::isLongitudeValid(double fLongitude)
{
	if (abs(fLongitude) > kfLongitudeAbsMax)
	{
		return false;
	}
	return true;
}

/*!
Get the north pole.

\return	The north pole.
*/
const CoordLatLon& CoordLatLon::northPole()
{
	static CoordLatLon np(MathUtils::kf90Rad, 0.0);

	return np;
}

/*!
Get the south pole.

\return	The south pole.
*/
const CoordLatLon& CoordLatLon::southPole()
{
	static CoordLatLon sp(-MathUtils::kf90Rad, 0.0);

	return sp;
}

/*!
Calculate the shortest distance from 'this' point to the line specified by L1, L2

\param L1	Starting point of line to measure distance to.
\param L2	Ending point of line to measure distance to.

\return	Distance from point to line in degrees.
*/
double CoordLatLon::distanceToLine(CoordLatLon& L1, CoordLatLon& L2)
{
 double y0 = L1.latInDegrees();
 double x0 = L1.lonInDegrees();

 double y1 = L2.latInDegrees();
 double x1 = L2.lonInDegrees();

 double y = this->latInDegrees();
 double x = this->lonInDegrees();

 double x1x0 = x1-x0;
 double y1y0 = y1-y0;

 double fTop = y1y0*x + x1x0*y + (x0*y1-x1*y0);

 double fBottom = sqrt( x1x0 * x1x0 + y1y0 * y1y0);

 return fabs(fTop/fBottom);
}

double CoordLatLon::sideN(double resolution) 
{ 
	return sqrt(pow(3, -(resolution-1))); 
}

double CoordLatLon::hexagonCircumradius(double resolution) 
{
	return (earthRadius/icosahedronRadius)*sideN(resolution);
}

double CoordLatLon::hexagonInradius(double resolution) 
{
	return (earthRadius/icosahedronRadius)*sideN(resolution) * sqrt(3.0)/2;
}

double CoordLatLon::pentagonCircumradius(double resolution) 
{
	return (earthRadius/icosahedronRadius)*sideN(resolution)*1/10*sqrt(50+10*sqrt(5.0));
}

double CoordLatLon::pentagonInradius(double resolution) 
{
	return (earthRadius/icosahedronRadius)*sideN(resolution)*1/10*sqrt(25+10*sqrt(5.0));
}

CoordLatLon::CoordLatLon(const CoordLatLon from, double brng, double d) 
{
	double m_fLat = asin( sin(from.m_fLat)*cos(d/earthRadius) + 
		cos(from.m_fLat)*sin(d/earthRadius)*cos(brng) );
	double m_fLon = from.m_fLon + atan2(sin(brng)*sin(d/earthRadius)*cos(from.m_fLat), 
		cos(d/earthRadius)-sin(from.m_fLat)*sin(m_fLat));
	normalizeLatitude();
	normalizeLongitude();
}

double CoordLatLon::operator-( const CoordLatLon& pt ) const
{
	// Calculate the distance between points using the haversine formula.
	double dLat = pt.m_fLat - m_fLat;
	double dLon = pt.m_fLon - m_fLon;
	double a = pow(sin(dLat/2), 2) +  cos(m_fLat) * cos(pt.m_fLat) * pow(sin(dLon/2), 2);			
	double c = 2 * atan2(sqrt(a), sqrt(1-a));
	return earthRadius * c;
}
