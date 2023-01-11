/******************************************************************************
wgs84.cpp

begin		: 2004-04-01
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "wgs84.h"

// local includes
#include "pyxis/utility/ellipsoid_math.h"
#include "pyxis/utility/trace.h"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"

//! Singleton instance
WGS84* WGS84::m_pInstance = 0;

//! The radius of the semi-major axis (at equator) in metres
static const double kfSemiMajorAxis = 6378137.0;

//! The flattening constant = (semi-major - semi-minor) / semi-major
static const double kfFlattening = 1.0 / 298.257223563;
static const double kfOneMinusFSqr = (1.0 - kfFlattening) * (1.0 - kfFlattening);

//! The radius of the semi-minor axis (at poles in metres
static const double kfSemiMinorAxis = kfSemiMajorAxis * (1.0 - kfFlattening);

/*!
Get the instance of the datum.

\return	The instance of the datum (ownership retained)
*/
WGS84 const * WGS84::getInstance()
{
	assert(m_pInstance);
	return m_pInstance;	
}

/*!
Initialize static data when application starts
*/
void WGS84::initStaticData()
{
	m_pInstance = new WGS84();
	TRACE_INFO("Instance of 'WGS84' created");
}

/*!
Free static data when application exits
*/
void WGS84::freeStaticData()
{
	delete m_pInstance;
	m_pInstance = 0;
	TRACE_DEBUG("Instance of 'WGS84' destroyed");
}

/*!
Calculate the distance in metres between two points on the earth's
surface.

\param	pt1	The first point in datum lat/lon coordinates.
\param	pt2	The second point in datum lat/lon coordinates.

\return	The distance in metres.
*/
double WGS84::calcDistance(	const CoordLatLon& pt1,
							const CoordLatLon& pt2	) const
{
	return EllipsoidMath::calcDistance(	pt1,
										pt2,
										kfSemiMajorAxis,
										kfFlattening	);
}

/*!
Convert a point in datum lat/lon coordinates to geocentric lat/lon
coordinates.

\param	pt	The point in datum lat/lon coordinates.

\return	The point in geocentric lat/lon coordinates.
*/
CoordLatLon WGS84::toGeocentric(const CoordLatLon& pt) const
{
	double fLat = atan(tan(pt.lat()) * kfOneMinusFSqr);

	return CoordLatLon(fLat, pt.lon());
}

/*!
Convert a point in geocentric lat/lon coordinates to datum lat/lon
coordinates.

\param	pt	The point in geocentric lat/lon coordinates.

\return	The point in datum lat/lon coordinates.
*/
CoordLatLon WGS84::toDatum(const CoordLatLon& pt) const
{
	double fLat = atan(tan(pt.lat()) / kfOneMinusFSqr);

	return CoordLatLon(fLat, pt.lon());
}

bool WGS84::tryParseFromString( std::string expression , PYXCoord2DDouble & wgs84)
{
	std::vector <std::string> fields;
	boost::algorithm::split(fields,
		expression,
		boost::algorithm::is_any_of(","),
		boost::algorithm::token_compress_on);

	if (fields.size() != 2 )
	{
		return false;
	}
	if (fields[0].empty() || fields[1].empty())
	{
		return false;
	}
	//TODO: add regex match to make sure its a double.

	double lat = 0;
	double lon = 0;
	StringUtils::fromString<double>(fields[0],&lat);
	StringUtils::fromString<double>(fields[1],&lon);


	if (lat < -90 || lat > 90 || lon > 180 || lon < -180)
	{
		return false;
	}
	wgs84.setX(lat);
	wgs84.setY(lon);
	return true;
}
