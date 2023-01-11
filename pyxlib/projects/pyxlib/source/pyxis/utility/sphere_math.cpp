/******************************************************************************
sphere_math.cpp

begin		: 2004-02-17
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/sphere_math.h"

// local includes
#include "pyxis/utility/math_utils.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <cassert>
#include <cmath>


const double SphereMath::knNumericEpsilon = 1e-12; //which is 100th of a Milimeter on earth.

const double SphereMath::knDotNumericEpsilon = 1e-7; //distance smaller then this will return acos = 0

//! Tester class
Tester<SphereMath> gTester;

///! Test method
void SphereMath::test()
{
	// test conversions
	CoordLatLon ll;
	PYXCoord3DDouble xyz;

	ll.setLatInDegrees(-90.0);
	ll.setLonInDegrees(0.0);

	llxyz(ll, &xyz);
	TEST_ASSERT(xyz.equal(PYXCoord3DDouble(0.0, 0.0, -1.0)));
	TEST_ASSERT(ll.equal(xyzll(xyz)));

	ll.setLatInDegrees(0.0);
	ll.setLonInDegrees(0.0);

	llxyz(ll, &xyz);
	TEST_ASSERT(xyz.equal(PYXCoord3DDouble(1.0, 0.0, 0.0)));
	TEST_ASSERT(ll.equal(xyzll(xyz)));

	ll.setLatInDegrees(0.0);
	ll.setLonInDegrees(90.0);

	llxyz(ll, &xyz);
	TEST_ASSERT(xyz.equal(PYXCoord3DDouble(0.0, 1.0, 0.0)));
	TEST_ASSERT(ll.equal(xyzll(xyz)));

	ll.setLatInDegrees(0.0);
	ll.setLonInDegrees(180.0);

	xyz = llxyz(ll);
	TEST_ASSERT(xyz.equal(PYXCoord3DDouble(-1.0, 0.0, 0.0)));
	TEST_ASSERT(ll.equal(xyzll(xyz)));

	ll.setLatInDegrees(0.0);
	ll.setLonInDegrees(-90.0);

	xyz = llxyz(ll);
	TEST_ASSERT(xyz.equal(PYXCoord3DDouble(0.0, -1.0, 0.0)));
	TEST_ASSERT(ll.equal(xyzll(xyz)));

	ll.setLatInDegrees(90.0);
	ll.setLonInDegrees(0.0);

	xyz = llxyz(ll);
	TEST_ASSERT(xyz.equal(PYXCoord3DDouble(0.0, 0.0, 1.0)));
	TEST_ASSERT(ll.equal(xyzll(xyz)));

	// test polygon surface area calculations
	std::vector<CoordLatLon> vecVertices;
	vecVertices.push_back(CoordLatLon(0.0, 0.0));
	vecVertices.push_back(CoordLatLon(0.0, MathUtils::kf90Rad));
	vecVertices.push_back(CoordLatLon(MathUtils::kf90Rad, 0.0));

	TEST_ASSERT(MathUtils::equal(	MathUtils::kf90Rad,
									calcPolygonArea(vecVertices)	));
}

/*!
Transform a point on a unit sphere from xyz coordinates to a latitude and
longitude. The xyz point is assumed to be normalized.

\param	xyz	The point in xyz coordinates

\return	The point in lat/lon coordinates.
*/
CoordLatLon SphereMath::xyzll(const PYXCoord3DDouble& xyz)
{
	CoordLatLon	ll;

	double fZ = xyz.z();
	if ((fabs(fZ) - 1) < MathUtils::kfDefaultDoublePrecision)
	{
		fZ = MathUtils::constrain(fZ, -1.0, 1.0);

		ll.setLat(asin(fZ));

		if (ll.isNorthPole() || ll.isSouthPole())
		{
			ll.setLon(0.0);
		}
		else
		{
			ll.setLon(atan2(xyz.y(), xyz.x()));
		}
	}
	else 
	{
		// sphere radius is not 1 unit
		assert(false);

		ll.set(0.0, 0.0);
	}

	return ll;
}

/*!
Transform a lat/lon to a point on a unit sphere in xyz coordinate.

\param	ll	The point in lat/lon coordinates

\return	The point in xyz coordinates.
*/
void SphereMath::llxyz(const CoordLatLon& ll, PYXCoord3DDouble* result)
{
	double fX = cos(ll.lat()) * cos(ll.lon());
	double fY = cos(ll.lat()) * sin(ll.lon());
	double fZ = sin(ll.lat());

	if (MathUtils::equal(fX, 0.0))
	{
		fX = 0.0;
	}

	if (MathUtils::equal(fY, 0.0))
	{
		fY = 0.0;
	}

	if (MathUtils::equal(fZ, 0.0))
	{
		fZ = 0.0;
	}

	result->set(fX, fY, fZ);
}

// NOTE: duplicate function as above, but returns the value instead of taking a
// pointer to value to fill in a result.

/*!
Transform a lat/lon to a point on a unit sphere in xyz coordinate.

\param	ll	The point in lat/lon coordinates

\return	The point in xyz coordinates.
*/
PYXCoord3DDouble SphereMath::llxyz(const CoordLatLon& ll)
{
	double fX = cos(ll.lat()) * cos(ll.lon());
	double fY = cos(ll.lat()) * sin(ll.lon());
	double fZ = sin(ll.lat());

	if (MathUtils::equal(fX, 0.0))
	{
		fX = 0.0;
	}

	if (MathUtils::equal(fY, 0.0))
	{
		fY = 0.0;
	}

	if (MathUtils::equal(fZ, 0.0))
	{
		fZ = 0.0;
	}

	return PYXCoord3DDouble(fX, fY, fZ);
}

/*!
Calculate the area of a closed spherical polygon on the surface of a sphere.
The vertices form the ends of great circle arcs and are assumed to be in order.
See "Computing the Area of a Spherical Polygon" by Robert D. Miller in
"Graphics Gems IV", Academic Press, 1994

\param	vecVertices	Vector of vertices in lat/lon coordinates.
\param	fRadius		The radius of the sphere.

\return	The area.
*/
double SphereMath::calcPolygonArea(	const std::vector<CoordLatLon>& vecVertices,
									double fRadius	)
{
	size_t knCount = vecVertices.size();
	double fSum = 0.0;

    for (unsigned int nIndex = 0; nIndex < knCount; ++nIndex)
    {
		PYXCoord3DDouble pt1 = llxyz(vecVertices[(nIndex + knCount - 1) % knCount]);
		PYXCoord3DDouble pt2 = llxyz(vecVertices[nIndex]);
		PYXCoord3DDouble pt3 = llxyz(vecVertices[(nIndex + 1) % knCount]);

		PYXCoord3DDouble cross12 = pt1.cross(pt2);
		cross12.normalize();

		PYXCoord3DDouble cross32 = pt3.cross(pt2);
		cross32.normalize();

		double fAngle = acos(cross12.dot(cross32));
		fSum += fAngle;
	}

	return ((fSum - MathUtils::kfPI * (knCount - 2)) * fRadius * fRadius);
}

/*
Calcualte the distance in (radians) between two points on the globe
*/
double SphereMath::distanceBetween(const PYXCoord3DDouble & pointA,const PYXCoord3DDouble & pointB)
{
	//distance betwee doesnt work with non-normalized points.
	assert( MathUtils::equal(pointA.squareLength(),1.0,MathUtils::kfDefaultDoublePrecision) && "pointA is not a normalized vector");
	assert( MathUtils::equal(pointB.squareLength(),1.0,MathUtils::kfDefaultDoublePrecision) && "pointB is not a normalized vector");

	//calculate euclid distance, which is numeric safe (for our needs right now)
	double euclidDistance = pointA.distance(pointB);
	if (euclidDistance <= DBL_EPSILON)
	{
		// this consider to be the same point
		return 0;
	}
	else if (euclidDistance < knDotNumericEpsilon)
	{
		//cross operation is more stable when points are close to each other
		double cross = pointA.cross(pointB).length();
		assert(cross>=0);
		
		double distance = asin(cross);		
		assert(distance>=0);
		return distance;
	}
	else
	{
		//it is safe to do dot operation which is faster then cross
		double dot = pointA.dot(pointB);

		//make sure we acos could work - dot sometimes return values larger then 1.0 - and then acos return #NAN.
		if (dot > 1.0) { dot = 1.0; }
		else if (dot < -1.0) { dot = -1.0; }

		double distance = acos(dot);
		assert(distance > 0);
		return distance;
	}
}

//! Calcualte the distance in (radians) between two points on the globe
double SphereMath::distanceBetween(const CoordLatLon & pointA,const CoordLatLon & pointB)
{
	double fA = sin((pointA.lat() - pointB.lat()) / 2.0);
	double fB = sin((pointA.lon() - pointB.lon()) / 2.0);

	double fRadians =	2 * asin(	sqrt(fA * fA + 
									cos(pointA.lat()) *
									cos(pointB.lat()) *
									fB * fB)	);
	return fRadians;
}

//! Calculate the heading (0...360) from a location to a different location
double SphereMath::headingInDegrees( const CoordLatLon & from,const CoordLatLon & to )
{
	//taken from: http://www.yourhomenow.com/house/haversine.html
	//var y = Math.sin(dLon) * Math.cos(lat2);
	//var x = Math.cos(lat1)*Math.sin(lat2) -
	//	Math.sin(lat1)*Math.cos(lat2)*Math.cos(dLon);
	//var brng = Math.atan2(y, x).toBrng();

	double y = sin(to.lon()-from.lon()) * cos(from.lat());
	double x = cos(to.lat())*sin(from.lat()) - sin(to.lat())*cos(to.lat())*cos(to.lon()-from.lon());
	double brng = atan2(y, x)/MathUtils::kfDegreesToRadians;

	if (brng<0) 
	{
		brng+=360;
	}
	return brng;
}

////////////////////////////////////////////////////////////////////
// SphereMath::GreatCircleArc
////////////////////////////////////////////////////////////////////

SphereMath::GreatCircleArc::GreatCircleArc(const CoordLatLon & pointA,const CoordLatLon & pointB, bool includePointA)
	: m_pointA(SphereMath::llxyz(pointA)),
	  m_pointB(SphereMath::llxyz(pointB)),
	  m_normal(m_pointA.cross(m_pointB)),
	  m_pointANormal(m_normal.cross(m_pointA)),
	  m_pointBNormal(m_pointB.cross(m_normal)),
	  m_includePointA(includePointA)
{
	assert( ! m_pointA.equal(m_pointB) && "pointA is equal to pointB - this is a point not a curve");
	m_normal.normalize();
	m_pointANormal.normalize();
	m_pointBNormal.normalize();
}

SphereMath::GreatCircleArc::GreatCircleArc(const PYXCoord3DDouble & pointA,const PYXCoord3DDouble & pointB, bool includePointA)
	: m_pointA(pointA),
	  m_pointB(pointB),
	  m_normal(pointA.cross(pointB)),
	  m_pointANormal(m_normal.cross(pointA)),
	  m_pointBNormal(pointB.cross(m_normal)),
	  m_includePointA(includePointA)
{
	assert( MathUtils::equal(pointA.squareLength(),1.0,MathUtils::kfDefaultDoublePrecision) && "pointA is not a normalized vector");
	assert( MathUtils::equal(pointB.squareLength(),1.0,MathUtils::kfDefaultDoublePrecision) && "pointB is not a normalized vector");
	assert( ! m_pointA.equal(m_pointB) && "pointA is equal to pointB - this is a point not a curve");

	m_normal.normalize();
	m_pointANormal.normalize();
	m_pointBNormal.normalize();
}

SphereMath::GreatCircleArc::GreatCircleArc(const SphereMath::GreatCircleArc & arc)
	: m_pointA(arc.m_pointA),
	  m_pointB(arc.m_pointB),
	  m_normal(arc.m_normal),
	  m_pointANormal(arc.m_pointANormal),
	  m_pointBNormal(arc.m_pointBNormal),
	  m_includePointA(arc.m_includePointA)
{
}

SphereMath::GreatCircleArc & SphereMath::GreatCircleArc::operator=(const SphereMath::GreatCircleArc & arc)
{
	m_pointA = arc.m_pointA;
	m_pointB = arc.m_pointB;
	m_normal = arc.m_normal;
	m_pointANormal = arc.m_pointANormal;
	m_pointBNormal = arc.m_pointBNormal;
	m_includePointA = arc.m_includePointA;

	return *this;
}

bool SphereMath::GreatCircleArc::operator==(const SphereMath::GreatCircleArc & arc)
{
	return m_pointA.equal(arc.m_pointA) && m_pointB.equal(arc.m_pointB) && m_includePointA == arc.m_includePointA;
}

bool SphereMath::GreatCircleArc::operator!=(const SphereMath::GreatCircleArc & arc)
{
	return ! m_pointA.equal(arc.m_pointA) || !m_pointB.equal(arc.m_pointB) || m_includePointA != arc.m_includePointA;
}

bool SphereMath::GreatCircleArc::intersects(const SphereMath::GreatCircleArc & arc,PYXCoord3DDouble & intersection) const
{
	intersection = m_normal.cross(arc.m_normal);
	intersection.normalize();

	if (MathUtils::equal(abs(m_normal.dot(arc.m_normal)),1.0,MathUtils::kfDefaultDoublePrecision))
	{
		//Line more or less the same. numeric error may occur
		//So, we try to do a binary search instead - which is slower
		//TODO: this code need more refined examinations

		PYXCoord3DDouble a(m_pointA);
		PYXCoord3DDouble b(m_pointB);
		double aDistance = arc.distanceTo(a)+DBL_EPSILON;
		double bDistance = arc.distanceTo(b)+DBL_EPSILON;

		double aFactor = bDistance/(aDistance+bDistance);
		double bFactor = aDistance/(aDistance+bDistance);

		intersection.setX(a.x()*aFactor+b.x()*bFactor);
		intersection.setY(a.y()*aFactor+b.y()*bFactor);
		intersection.setZ(a.z()*aFactor+b.z()*bFactor);
		intersection.normalize();
		intersection = closestPointOnArc(intersection); //make sure we are on the same line
		
		double intersectionDistance = arc.distanceTo(intersection);
		
		//if the intersection point is closer then A & B - we will have intersection
		while(intersectionDistance > DBL_EPSILON && aDistance > intersectionDistance && bDistance > intersectionDistance)
		{
			if (aDistance < bDistance)
			{
				b = intersection;
				bDistance = intersectionDistance;
			}
			else
			{
				a = intersection;
				aDistance = intersectionDistance;
			}

			aFactor = bDistance/(aDistance+bDistance);
			bFactor = aDistance/(aDistance+bDistance);

			intersection.setX(a.x()*aFactor+b.x()*bFactor);
			intersection.setY(a.y()*aFactor+b.y()*bFactor);
			intersection.setZ(a.z()*aFactor+b.z()*bFactor);
			intersection.normalize();
			intersection = closestPointOnArc(intersection);

			intersectionDistance = arc.distanceTo(intersection);
		}

		return intersectionDistance <= DBL_EPSILON;
	}

	//check that the intersection point is inside two Lunes
	if (insideArcLune(intersection) && arc.insideArcLune(intersection))
	{
		return true;
	}

	//normals can result the intersection point on the other side of the sphere - try again
	intersection.negate();

	if (insideArcLune(intersection) && arc.insideArcLune(intersection))
	{
		return true;
	}

	return false;
}

bool SphereMath::GreatCircleArc::intersects(const SphereMath::GreatCircleArc & arc) const
{
	double aDot = arc.m_normal.dot(m_pointA);
	double bDot = arc.m_normal.dot(m_pointB);

	if (abs(aDot) <= DBL_EPSILON || abs(bDot) <= DBL_EPSILON)
	{
		//if one of the end points is on the other arc - they intersects
		return (m_includePointA && arc.distanceTo(m_pointA) <= DBL_EPSILON)|| arc.distanceTo(m_pointB) <= DBL_EPSILON ||
			  (arc.m_includePointA && distanceTo(arc.m_pointA) <= DBL_EPSILON) || distanceTo(arc.m_pointB) <= DBL_EPSILON;
	}

	if (aDot < -DBL_EPSILON && bDot >= DBL_EPSILON || aDot > DBL_EPSILON && bDot <= -DBL_EPSILON)
	{
		aDot = m_normal.dot(arc.m_pointA);
		bDot = m_normal.dot(arc.m_pointB);

		if (abs(aDot) <= DBL_EPSILON || abs(bDot) <= DBL_EPSILON)
		{
			//if one of the end points is on the other arc - they intersects
			return (m_includePointA && arc.distanceTo(m_pointA) <= DBL_EPSILON)|| arc.distanceTo(m_pointB) <= DBL_EPSILON ||
				   (arc.m_includePointA && distanceTo(arc.m_pointA) <= DBL_EPSILON) || distanceTo(arc.m_pointB) <= DBL_EPSILON;
		}

		if (aDot < -DBL_EPSILON && bDot >= DBL_EPSILON || aDot > DBL_EPSILON && bDot <= -DBL_EPSILON)
		{
			PYXCoord3DDouble arcDirection(arc.m_pointA);
			arcDirection.translate(arc.m_pointB);
			
			PYXCoord3DDouble thisDirection(m_pointA);
			thisDirection.translate(m_pointB);

			return thisDirection.dot(arcDirection) > 0;
		}
	}

	return false;
}

double SphereMath::GreatCircleArc::distanceTo(const PYXCoord3DDouble & point) const
{
	assert( MathUtils::equal(point.squareLength(),1.0,MathUtils::kfDefaultDoublePrecision) && "pointA is not a normalized vector");

	if (insideArcLune(point))
	{
		//! if the point is on the line - the agnle would be 90 degrees (in radians). so the distance is the difference from 90 degrees
		return abs( acos(m_normal.dot(point)) - MathUtils::kf90Rad );
	}
	else
	{
		return std::min(SphereMath::distanceBetween(point,m_pointA),SphereMath::distanceBetween(point,m_pointB));
	}
}

//! return the min and max distance (in radians) inside a given circle on the unit sphere surface
void SphereMath::GreatCircleArc::minAndMaxDistanceInsideCircle(const PYXCoord3DDouble & center,double radius, double * minDistance, double * maxDistance ) const
{
	assert(minDistance != 0);
	assert(maxDistance != 0);

	double distanceToArc = distanceTo(center);

	*minDistance = std::max(distanceToArc-radius,0.0);
	*maxDistance = distanceToArc+radius;
}

PYXCoord3DDouble SphereMath::GreatCircleArc::closestPointOnArc(const PYXCoord3DDouble & point) const
{
	assert( MathUtils::equal(point.squareLength(),1.0,MathUtils::kfDefaultDoublePrecision) && "pointA is not a normalized vector");

	if (insideArcLune(point))
	{
		PYXCoord3DDouble result = m_normal.cross(point).cross(m_normal);
		result.normalize();
		if (result.squareLength() > 0)
		{
			return result;
		}
		else
		{
			return m_pointA; //every other point on the curve will be good as well
		}
	}
	else if (SphereMath::distanceBetween(point,m_pointA) < SphereMath::distanceBetween(point,m_pointB))
	{
		return m_pointA;
	}
	else
	{
		return m_pointB;
	}
}

bool SphereMath::GreatCircleArc::insideArcLune(const PYXCoord3DDouble & point) const
{
	//check if both point is inside both 2 hemi-sphere of defined point pointA,B normals.
	if (m_includePointA)
	{
		return !(m_pointANormal.dot(point)<0) && !(m_pointBNormal.dot(point)<0);
	}
	else
	{
		return m_pointANormal.dot(point) > 0 && !(m_pointBNormal.dot(point)<0);
	}
}

const PYXCoord3DDouble & SphereMath::GreatCircleArc::getPointA() const
{
	return m_pointA;
}

const PYXCoord3DDouble & SphereMath::GreatCircleArc::getPointB() const
{
	return m_pointB;
}

const PYXCoord3DDouble & SphereMath::GreatCircleArc::getArcNormal() const
{
	return m_normal;
}

PYXCoord3DDouble SphereMath::GreatCircleArc::pointAlongArc(double fraction) const
{
	assert(fraction >= 0 && fraction <= 1 && "fraction is out of range");

	//if the distance is less the 1 metter
	auto distance = SphereMath::distanceBetween(m_pointA,m_pointB);
	if (distance < knDotNumericEpsilon)
	{
		//assume earth is a plane... and avoid normierc issues
		PYXCoord3DDouble point(m_pointA.x()*fraction + m_pointB.x()*(1-fraction),
							   m_pointA.y()*fraction + m_pointB.y()*(1-fraction),
			                   m_pointA.z()*fraction + m_pointB.z()*(1-fraction));

		point.normalize();
		return point;
	}
	else 
	{
		//assume earth isn't a plane, its a sphere :D
		double angle = distance * fraction;
		assert(angle == angle); //make sure angle is not no-a-number/#IND.
		double sinAngle = sin(angle);
		double cosAngle = cos(angle);
		PYXCoord3DDouble point(	m_pointA.x()*cosAngle + m_pointANormal.x()*sinAngle,
								m_pointA.y()*cosAngle + m_pointANormal.y()*sinAngle,
								m_pointA.z()*cosAngle + m_pointANormal.z()*sinAngle);
		point.normalize();
		return point;
	}
}

//! Tester class
Tester<SphereMath::GreatCircleArc> gGreatCircleArcTester;

//! Test method
void SphereMath::GreatCircleArc::test()
{
	//test SphereMath::GreateCircleArc

	PYXCoord3DDouble px(1,0,0);
	PYXCoord3DDouble py(0,1,0);
	PYXCoord3DDouble pz(0,0,1);
	PYXCoord3DDouble pxyz(1,1,1);
	PYXCoord3DDouble pxy(1,1,0);
	PYXCoord3DDouble pxz(1,0,1);
	pxyz.normalize();
	pxy.normalize();
	pxz.normalize();


	{
		SphereMath::GreatCircleArc arc(px,py);

		//check if pointA,pointB of the arc are on the arc.
		TEST_ASSERT(MathUtils::equal(arc.distanceTo(px),0));
		TEST_ASSERT(MathUtils::equal(arc.distanceTo(py),0));

		//test insideArcLune - inside range
		{
			PYXCoord3DDouble ptInside1(1,1,1);
			ptInside1.normalize();
			TEST_ASSERT(arc.insideArcLune(ptInside1));
		}

		{
			PYXCoord3DDouble ptInside1(1,1,-1);
			ptInside1.normalize();
			TEST_ASSERT(arc.insideArcLune(ptInside1));
		}

		{
			PYXCoord3DDouble ptInside1(1,1,0);
			ptInside1.normalize();
			TEST_ASSERT(arc.insideArcLune(ptInside1));
		}

		//boundry case - inisde of Lune!!!!
		{
			TEST_ASSERT(arc.insideArcLune(px));
			TEST_ASSERT(arc.insideArcLune(py));
			TEST_ASSERT(arc.insideArcLune(pz));
		}

		//outside cases
		{
			PYXCoord3DDouble ptOutside1(-1,1,1);
			ptOutside1.normalize();
			TEST_ASSERT(arc.insideArcLune(ptOutside1) == false);
		}

		{
			PYXCoord3DDouble ptOutside1(1,-1,-1);
			ptOutside1.normalize();
			TEST_ASSERT(arc.insideArcLune(ptOutside1) == false);
		}

		{
			PYXCoord3DDouble ptOutside1(-1,-1,0);
			ptOutside1.normalize();
			TEST_ASSERT(arc.insideArcLune(ptOutside1) == false);
		}

		//check if the distance is calulated right
		TEST_ASSERT(MathUtils::equal(arc.distanceTo(pz),MathUtils::kf90Rad));
	}

	{
		//test with px not include on the arc
		SphereMath::GreatCircleArc arc(px,py,false);

		//boundry case - pointB is inisde of Lune and pointA is outside the Lune.
		{
			TEST_ASSERT(arc.insideArcLune(px) == false);
			TEST_ASSERT(arc.insideArcLune(py));
			TEST_ASSERT(arc.insideArcLune(pz) == false); // because it hit the px line then it's out
			TEST_ASSERT(arc.insideArcLune(pxyz));
		}
	}

	{
		//operators =, ==, !=
		SphereMath::GreatCircleArc arc1(px,py);
		SphereMath::GreatCircleArc arc2(px,py,false);

		SphereMath::GreatCircleArc arc3(arc1);

		TEST_ASSERT(arc3==arc1 && ! (arc3!=arc1));
		TEST_ASSERT(arc3!=arc2 && ! (arc3==arc2));

		arc3 = arc2;

		TEST_ASSERT(arc3!=arc1);
		TEST_ASSERT(arc3==arc2);

		arc3 = SphereMath::GreatCircleArc(px,pz);

		TEST_ASSERT(arc3!=arc1);
		TEST_ASSERT(arc3!=arc2);
	}

	{
		//intersection
		SphereMath::GreatCircleArc arc1(px,py);
		SphereMath::GreatCircleArc arc2(px,pz);
		SphereMath::GreatCircleArc arc3(py,pz);

		PYXCoord3DDouble intersection;

		TEST_ASSERT(arc1.intersects(arc2,intersection));
		TEST_ASSERT(px.equal(intersection));
		TEST_ASSERT(arc2.intersects(arc3,intersection));
		TEST_ASSERT(pz.equal(intersection));

		SphereMath::GreatCircleArc arc4(pz,pxyz);

		TEST_ASSERT(arc1.intersects(arc4) == false);
		TEST_ASSERT(arc2.intersects(arc4));

		//now without pz.
		SphereMath::GreatCircleArc arc5(pz,pxyz);

		TEST_ASSERT(arc1.intersects(arc5) == false);
		TEST_ASSERT(arc2.intersects(arc5));
		TEST_ASSERT(arc5.intersects(arc5));
	}

	{
		//distance between points
		SphereMath::GreatCircleArc arc(px,py);

		TEST_ASSERT(MathUtils::equal(arc.distanceTo(pz),MathUtils::kf90Rad));
		TEST_ASSERT(MathUtils::equal(arc.distanceTo(pxy),0));
		TEST_ASSERT(MathUtils::equal(arc.distanceTo(pxyz),SphereMath::distanceBetween(pxy,pxyz)));
		TEST_ASSERT(MathUtils::equal(arc.distanceTo(pxz),SphereMath::distanceBetween(px,pxz)));
	}

	{
		//closest point test
		SphereMath::GreatCircleArc arc(px,py);

		TEST_ASSERT(MathUtils::equal(arc.distanceTo(pz),SphereMath::distanceBetween(arc.closestPointOnArc(pz),pz)));
		TEST_ASSERT(MathUtils::equal(arc.distanceTo(pxy),SphereMath::distanceBetween(arc.closestPointOnArc(pxy),pxy)));
		TEST_ASSERT(MathUtils::equal(arc.distanceTo(pxyz),SphereMath::distanceBetween(arc.closestPointOnArc(pxyz),pxyz)));
		TEST_ASSERT(MathUtils::equal(arc.distanceTo(pxz),SphereMath::distanceBetween(arc.closestPointOnArc(pxz),pxz)));
	}

	{
		SphereMath::GreatCircleArc arc(px,py);

		TEST_ASSERT(px.equal(arc.pointAlongArc(0)));
		TEST_ASSERT(py.equal(arc.pointAlongArc(1)));
		TEST_ASSERT(pxy.equal(arc.pointAlongArc(0.5)));
		TEST_ASSERT(MathUtils::equal(SphereMath::distanceBetween(arc.pointAlongArc(0.25),px)*2,
									 SphereMath::distanceBetween(arc.pointAlongArc(0.5),px)));
		TEST_ASSERT(MathUtils::equal(SphereMath::distanceBetween(arc.pointAlongArc(0.75),py)*2,
									 SphereMath::distanceBetween(arc.pointAlongArc(0.5),px)));
	}
}