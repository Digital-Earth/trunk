#ifndef PYXIS__UTILITY__SPHERE_MATH_H
#define PYXIS__UTILITY__SPHERE_MATH_H
/******************************************************************************
sphere_math.h

begin		: 2004-02-17
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/coord_3d.h"
#include "pyxis/utility/coord_lat_lon.h"

// standard includes
#include <vector>

/*!
This class contains various methods for dealing with the geometry of spheres.
*/
//! Various spherical math methods
class PYXLIB_DECL SphereMath
{
public:
	class GreatCircleArc;

public:

	static const int knEarthRadius = 6371007; //in Meters

	static const double knNumericEpsilon; //is 100th of a Milimeter on earth.

	static const double knDotNumericEpsilon; //error of a dot operation

	//! Test method
	static void test();

	//! Transform a point on a unit sphere from xyz to a lat/lon.
	static CoordLatLon xyzll(const PYXCoord3DDouble& xyz);

	//! Transform a point on a unit sphere from lat/lon to xyz.
	static void llxyz(const CoordLatLon& ll, PYXCoord3DDouble* result);

	//! Transform a point on a unit sphere from lat/lon to xyz.
	static PYXCoord3DDouble llxyz(const CoordLatLon& ll);

	//! Calculate the area of a closed spherical polygon.
	static double calcPolygonArea(	const std::vector<CoordLatLon>& vecVertices,
									double fRadius = 1.0	);

	//! Calculate the distance in (radians) between two points on the globe
	static double distanceBetween(const PYXCoord3DDouble & pointA,const PYXCoord3DDouble & pointB);

	//! Calculate the distance in (radians) between two points on the globe
	static double distanceBetween(const CoordLatLon & pointA,const CoordLatLon & pointB);

	//! Calculate the heading (0...360) from a location to a different location
	static double headingInDegrees(const CoordLatLon & from,const CoordLatLon & to);
};

/*!
SphereMath::GreatCircleArc - is a greate circle arc based on XYZ coodinates.

GreatCircleArc is very efficent in calcualting distance from the arc, and intersection with onther arcs.

For current ploygon boundy intersection count (for inside/outside test), GreatCircleArc the origin point can exluded from the arc.
so, the GreatCircleArc is from pointA(exluded) to pointB(included). 

This allow to create a boundry constructed with list of GreatCircleArc that intersect only once at the boundry joint vertices

Note: this class SphereMath::GreatCircleArc but is simllar to the class ::GreatCircleArc - that work on LatLon coordinates.
*/
class PYXLIB_DECL SphereMath::GreatCircleArc
{
public:
	static void test();

protected:

	//! origin point
	PYXCoord3DDouble m_pointA;

	//! destination point
	PYXCoord3DDouble m_pointB;

	//! normal for to speed up the calcualtions
	PYXCoord3DDouble m_normal;
	PYXCoord3DDouble m_pointANormal;
	PYXCoord3DDouble m_pointBNormal;

	//! include origin point
	bool m_includePointA;

public:
	GreatCircleArc(const CoordLatLon & pointA,const CoordLatLon & pointB,bool includePointA = true);
	GreatCircleArc(const PYXCoord3DDouble & pointA,const PYXCoord3DDouble & pointB, bool includePointA = true);
	GreatCircleArc(const GreatCircleArc & arc);

	GreatCircleArc & operator=(const GreatCircleArc & arc);
	
	bool operator==(const GreatCircleArc & arc);
	bool operator!=(const GreatCircleArc & arc);

public:
	//! return true if the two Arcs intersects
	bool intersects(const GreatCircleArc & arc) const;

	//! return true if the two Arcs intersects and update the intersection point 
	bool intersects(const GreatCircleArc & arc,PYXCoord3DDouble & intersection) const;
	
	//! return the distance (in radians) from the point to the arc
	double distanceTo(const PYXCoord3DDouble & point) const;

	//! return the min and max distance (in radians) inside a given circle on the unit sphere surface
	void minAndMaxDistanceInsideCircle(const PYXCoord3DDouble & center,double radius, double * minDistance, double * maxDistance ) const;

	//! return a point on the Arc that closest to the given point
	PYXCoord3DDouble closestPointOnArc(const PYXCoord3DDouble & point) const;

	//! return true if the the point is insde the ArcLune
	bool insideArcLune(const PYXCoord3DDouble & point) const;

	//! return a point along the GreateCircleArc. fraction = 0 is pointA and fraction = 1 is pointB.
	PYXCoord3DDouble pointAlongArc(double fraction) const;

public:
	const PYXCoord3DDouble & getPointA() const;
	const PYXCoord3DDouble & getPointB() const;
	const PYXCoord3DDouble & getArcNormal() const;
};

#endif // guard
