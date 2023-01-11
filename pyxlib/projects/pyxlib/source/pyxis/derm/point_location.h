#ifndef PYXIS__DERM__POINT_LOCATION_H
#define PYXIS__DERM__POINT_LOCATION_H
/******************************************************************************
point_location.h

begin		: Friday, December 21, 2012 11:44:30 PM
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "index.h"
#include "pyxis\utility\coord_lat_lon.h"
#include "pyxis\utility\coord_2d.h"
#include "pyxis\utility\coord_3d.h"


class PYXLIB_DECL PointLocation 
{
public:

	//! Test method
	static void test();

	//! Constructors

	static PointLocation fromWGS84(double lat, double lon)
	{
		return PointLocation(lat,lon);
	}

	static PointLocation fromWGS84(const PYXCoord2DDouble& wgs84LatLon)
	{
		return PointLocation(wgs84LatLon);
	}

	static PointLocation fromGeocenteric(double lat, double lon);

	static PointLocation fromGeocenteric(const CoordLatLon& geocenteric);

	static PointLocation fromPYXIndex(const PYXIcosIndex& index)
	{
		return PointLocation(index);
	}

	static PointLocation fromXYZ(const PYXCoord3DDouble& xyz)
	{
		return PointLocation(xyz);
	}


	//! Methods.
	PYXCoord2DDouble asWGS84() const;
	CoordLatLon asGeocentric() const;
	PYXCoord3DDouble asXYZ() const;
	PYXIcosIndex asPYXIcosIndex(int resolution) const;
	//! return distance to point in meters
	double distance(const PointLocation & p2) const;

protected:
	PointLocation(const PYXIcosIndex& index);
	PointLocation(const PYXCoord3DDouble& xyz);
	PointLocation(const PYXCoord2DDouble& wgs84LatLon);
	PointLocation(double lat, double lon);

private:
	//! The Point of location
	PYXCoord2DDouble m_wgs84LatLon;
};
#endif // guard
