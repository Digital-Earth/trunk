/******************************************************************************
point_location.cpp

begin		: Saturday, December 22, 2012 12:45:10 AM
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"

// pyxlib includes
#include "pyxis/derm/point_location.h"
#include "pyxis/utility/tester.h"
#include "pyxis/derm/wgs84_coord_converter.h"
#include "pyxis/utility/sphere_math.h"

// standard includes
#include <cassert>


//! Tester class
Tester<PointLocation> gTester();

//! Test method
void PointLocation::test()
{
	PYXCoord2DDouble latlon (1, 2);
	PointLocation location1(latlon);
	PointLocation location2(location1.asXYZ());
	PointLocation location3(location1.asPYXIcosIndex(40));

	TEST_ASSERT(location1.distance(location2) < 1 && 
				location1.distance(location2) < 1 &&
				location1.distance(location3) < 1 );


}
PointLocation::PointLocation( const PYXIcosIndex& index ) 
{
	WGS84CoordConverter converter;
	converter.pyxisToNative(index,&m_wgs84LatLon);
}

PointLocation::PointLocation( const PYXCoord3DDouble& xyz )
{
	WGS84CoordConverter converter;
	converter.latLonToNative(SphereMath::xyzll(xyz),&m_wgs84LatLon);
}

PointLocation::PointLocation( const PYXCoord2DDouble& wgs84LatLon ) :m_wgs84LatLon(wgs84LatLon)
{

}
PointLocation::PointLocation(double lat, double lon) : m_wgs84LatLon(lon, lat)
{

}

PYXIcosIndex PointLocation::asPYXIcosIndex(int resolution) const
{
	WGS84CoordConverter conveter;
	PYXIcosIndex index;
	conveter.nativeToPYXIS(m_wgs84LatLon,&index,resolution);
	return index;
}

PYXCoord3DDouble PointLocation::asXYZ() const
{
	WGS84CoordConverter converter;
	CoordLatLon ll;
	converter.nativeToLatLon(m_wgs84LatLon,&ll);
	return SphereMath::llxyz(ll);
}



PYXCoord2DDouble PointLocation::asWGS84() const
{
	return m_wgs84LatLon;
}

double PointLocation::distance( const PointLocation & p2 ) const
{
	return SphereMath::distanceBetween(asXYZ(),p2.asXYZ())*SphereMath::knEarthRadius;
}

CoordLatLon PointLocation::asGeocentric() const
{
	WGS84CoordConverter converter;
	CoordLatLon ll;
	converter.nativeToLatLon(m_wgs84LatLon,&ll);
	return ll;
}


PointLocation PointLocation::fromGeocenteric( double lat, double lon )
{
	WGS84CoordConverter converter;
	CoordLatLon ll(lat,lon);
	PYXCoord2DDouble wgs84LatLon;
	converter.latLonToNative(ll,&wgs84LatLon);
	return PointLocation(wgs84LatLon);
}

PointLocation PointLocation::fromGeocenteric( const CoordLatLon& geocenteric )
{
	WGS84CoordConverter converter;
	PYXCoord2DDouble wgs84LatLon;
	converter.latLonToNative(geocenteric,&wgs84LatLon);
	return PointLocation(wgs84LatLon);
}

