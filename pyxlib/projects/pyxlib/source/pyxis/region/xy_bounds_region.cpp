/******************************************************************************
xy_bounds_region.cpp

begin		: 2011-02-05
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "xy_bounds_region.h"

#include "pyxis/derm/index.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/snyder_projection.h"

#include "pyxis/utility/math_utils.h"

/*!
Make a copy of the region.

\return A copy of the region.
*/
PYXPointer<IRegion> PYXXYBoundsRegion::clone() const
{
	return PYXXYBoundsRegion::create(*this);
}

/*!
Get the distance from a point to the border of the bounding region.

\param location			A point in xyz coordinates on the unit sphere.
\param fErrorThreshold	Unused.

\return	The distance in radians.
*/
double PYXXYBoundsRegion::getDistanceToBorder(const PYXCoord3DDouble& location, double fErrorThreshold) const
{
	//clip the location to a t "lat/lon" area so we can perform latLonToNative.
	CoordLatLon ll = SphereMath::xyzll(location);
	ll = m_latLonBounds.pin(ll);
	
	PYXCoord2DDouble native;	
	m_coordConverter->latLonToNative(ll, &native);

	if (!m_nativeBounds.inside(native))
	{
		native = m_nativeBounds.pin(native);
	}

	CoordLatLon point;
	double distance;

	PYXCoord2DDouble native2(m_nativeBounds.xMin(), native.y());
	m_coordConverter->nativeToLatLon(native2, &point);
	distance = SphereMath::distanceBetween(location, SphereMath::llxyz(point));

	native2.setX(m_nativeBounds.xMax());
	m_coordConverter->nativeToLatLon(native2, &point);
	distance = std::min(distance,SphereMath::distanceBetween(location, SphereMath::llxyz(point)));

	native2.setX(native.x());
	native2.setY(m_nativeBounds.yMin());
	m_coordConverter->nativeToLatLon(native2, &point);
	distance = std::min(distance,SphereMath::distanceBetween(location, SphereMath::llxyz(point)));

	native2.setY(m_nativeBounds.yMax());
	m_coordConverter->nativeToLatLon(native2, &point);
	distance = std::min(distance,SphereMath::distanceBetween(location, SphereMath::llxyz(point)));

	return distance;
}

/*!
Determine if a point is contained within the bounding region.

\param location			The point in xyz coordinates on the unit sphere.
\param fErrorThreshold	Unused.

\return	true if the point is contained within the region, otherwise false.
*/
bool PYXXYBoundsRegion::isPointContained(const PYXCoord3DDouble& location, double fErrorThreshold) const
{
	// if the coverage is covering only half the sphere - check we are on the right sphere
	// note: the bounding circle is not working well for very large data sources
	if (	m_boundingCircle.getRadius() < MathUtils::kfPI/2 &&
			!m_boundingCircle.contains(location)	)
	{
		return false;
	}

	CoordLatLon ll = SphereMath::xyzll(location);

	// make sure we are inside the safe "lat/lon" area.
	if (!m_latLonBounds.inside(ll))
	{
		return false;
	}

	PYXCoord2DDouble native;
	m_coordConverter->latLonToNative(ll, &native);
	return m_nativeBounds.inside(native);
}

/*!
Get the bounding circle of the region.

\return	The bounding circle of the region.
*/
PYXBoundingCircle PYXXYBoundsRegion::getBoundingCircle() const
{
	return m_boundingCircle;
}

/*!
Constructor.

\param nativeBounds		The bounding rectangle in native coordinates.
\param coordConverter	Used to convert between native and geocentric lat lon coordinates.
*/
PYXXYBoundsRegion::PYXXYBoundsRegion(	const PYXRect2DDouble& nativeBounds,
										const ICoordConverter& coordConverter ) :
	m_nativeBounds(nativeBounds),
	m_coordConverter(coordConverter.clone())
{
	assert(!m_nativeBounds.degenerate() && "Native bounding box cannot be empty.");

	/*
	Check for global bounds. Bounds are considered global if, when converted to
	geocentric lat/lon coordinates, the min and max latitudes range from at least
	-89 to 89 degrees and the min and max longitudes are equal within a degree or
	are separated by 360 degrees within a degree.
	*/
	CoordLatLon minPoint;
	PYXCoord2DDouble minNative(m_nativeBounds.xMin(), m_nativeBounds.yMin());
	m_coordConverter->nativeToLatLon(minNative, &minPoint);

	CoordLatLon maxPoint;
	PYXCoord2DDouble maxNative(m_nativeBounds.xMax(), m_nativeBounds.yMax());
	m_coordConverter->nativeToLatLon(maxNative, &maxPoint);

	if (	(minPoint.latInDegrees() <= -89.0) &&
			(maxPoint.latInDegrees() >= 89.0) &&
			(	MathUtils::equal(minPoint.lonInDegrees(), maxPoint.lonInDegrees(), 1.0) ||
				MathUtils::equal(abs(maxPoint.lonInDegrees() - minPoint.lonInDegrees()), 360.0, 1.0)	)	)
	{
		m_boundingCircle = PYXBoundingCircle::global();
		m_latLonBounds.setToGlobal();
	}
	else
	{
		// calculate bounding circle and box the old way
		createBoundingCircle();
		createLatLonBoundingBox();
	}
}

/*!
Create the bounding circle for the region.
*/
void PYXXYBoundsRegion::createBoundingCircle()
{
	PYXCoord3DDouble center = getBoundsCenter();

	double radius = 0;

	CoordLatLon point;

	PYXCoord2DDouble native(m_nativeBounds.xMin(), m_nativeBounds.yMin());
	m_coordConverter->nativeToLatLon(native, &point);
	radius = SphereMath::distanceBetween(center, SphereMath::llxyz(point));

	native.setX((m_nativeBounds.xMin() + m_nativeBounds.xMax()) / 2);
	m_coordConverter->nativeToLatLon(native, &point);
	radius = std::max(radius, SphereMath::distanceBetween(center, SphereMath::llxyz(point)));

	native.setX(m_nativeBounds.xMax());
	m_coordConverter->nativeToLatLon(native, &point);
	radius = std::max(radius, SphereMath::distanceBetween(center, SphereMath::llxyz(point)));

	native.setY((m_nativeBounds.yMin() + m_nativeBounds.yMax()) / 2);
	m_coordConverter->nativeToLatLon(native, &point);
	radius = std::max(radius, SphereMath::distanceBetween(center, SphereMath::llxyz(point)));

	native.setY(m_nativeBounds.yMax());
	m_coordConverter->nativeToLatLon(native, &point);
	radius = std::max(radius, SphereMath::distanceBetween(center, SphereMath::llxyz(point)));

	native.setX((m_nativeBounds.xMin() + m_nativeBounds.xMax()) / 2);
	m_coordConverter->nativeToLatLon(native, &point);
	radius = std::max(radius, SphereMath::distanceBetween(center, SphereMath::llxyz(point)));

	native.setX(m_nativeBounds.xMin());
	m_coordConverter->nativeToLatLon(native, &point);
	radius = std::max(radius, SphereMath::distanceBetween(center, SphereMath::llxyz(point)));

	native.setY((m_nativeBounds.yMin() + m_nativeBounds.yMax()) / 2);
	m_coordConverter->nativeToLatLon(native, &point);
	radius = std::max(radius, SphereMath::distanceBetween(center, SphereMath::llxyz(point)));

	m_boundingCircle = PYXBoundingCircle(center,radius);
}

/*!
Create the bounding box for the region in geocentric lat lon coordinates/
*/
void PYXXYBoundsRegion::createLatLonBoundingBox()
{
	CoordLatLon point;

	PYXRect2DDouble nativeBounds = m_nativeBounds;

	const int size = 10;
	//offset sample come to handle with -180 == 180 issues. 
	//having a small sample offset will ensure we -180 will get -180+0.005 and 180 will be 180-0.005 while keeping all samples inside a the original bbox
	const double sampleOffset = 0.005;
	auto xDelta = nativeBounds.width() * (1-sampleOffset) / size;
	auto yDelta = nativeBounds.height() * (1-sampleOffset) / size;

	//sample the native bounds (11x11 samples) to figure out the estimated lat/lon bbox.
	for (auto x = 0; x <= size; x++) 
	{
		for (auto y = 0; y <= size; y++) 
		{
			PYXCoord2DDouble native(nativeBounds.xMin() + xDelta * x, nativeBounds.yMin() + yDelta * y);
			m_coordConverter->nativeToLatLon(native, &point);	
			m_latLonBounds.expand(point);

			//offset sample.
			native.setX(nativeBounds.xMin() + xDelta * x + nativeBounds.width()*sampleOffset);
			native.setY(nativeBounds.yMin() + yDelta * y + nativeBounds.height()*sampleOffset);
			m_coordConverter->nativeToLatLon(native, &point);	
			m_latLonBounds.expand(point);
		}
	}

	//we only do 11x11 samples in the region. in order to make it to work correctly we need to expand region by 1 sample size.
	auto samplingError = 0.5 / size;
	m_latLonBounds.expandInRadians(m_latLonBounds.heightInRadians()*samplingError,m_latLonBounds.widthInRadians()*samplingError);

	//sanitize with a size of sampling error in mind
	m_latLonBounds.sanitize(m_latLonBounds.heightInRadians()*samplingError,m_latLonBounds.widthInRadians()*samplingError);

	//TRACE_INFO("BBOX safe latlon box" << m_latLonBounds.toReadableString());
}

/*!
Get the centre of the bounding region.

\return The centre of the bounding region as an xyz coordinate on the unit sphere.
*/
PYXCoord3DDouble PYXXYBoundsRegion::getBoundsCenter() const
{
	PYXCoord2DDouble native(	(m_nativeBounds.xMin() + m_nativeBounds.xMax()) / 2.0,
								(m_nativeBounds.yMin() + m_nativeBounds.yMax()) / 2.0	);
	CoordLatLon center;
	m_coordConverter->nativeToLatLon(native, &center);
	return SphereMath::llxyz(center);
}

PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXXYBoundsRegion & region)
{
	assert(0 && "not implemented");
	return buffer;
}

PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXXYBoundsRegion> & region)
{
	assert(0 && "not implemented");
	return buffer;
}