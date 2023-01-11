/******************************************************************************
great_circle_arc.cpp

begin		: 2005-04-04
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/great_circle_arc.h"

// local includes
#include "tester.h"

// standard includes
#include <cassert>
#include <cmath>

//! Tester class
Tester<GreatCircleArc> gGreatCircleArcTester;

//! Test method
void GreatCircleArc::test()
{
	// test great circle distance calculation
	CoordLatLon pt1;
	CoordLatLon pt2;

	pt1.setInDegrees(-90.0, 0.0);
	pt2.setInDegrees(90.0, 0.0);
	TEST_ASSERT(MathUtils::equal(MathUtils::kfPI, calcDistance(pt1, pt2)));

	pt1.setInDegrees(0.0, 0.0);
	pt2.setInDegrees(0.0, 180.0);
	TEST_ASSERT(MathUtils::equal(MathUtils::kfPI, calcDistance(pt1, pt2)));

	// test start and end points
	GreatCircleArc gca1(pt1, pt2);
	TEST_ASSERT(gca1.getPoint(0.0).equal(pt1));
	TEST_ASSERT(gca1.getPoint(1.0).equal(pt2));

	// test point along arc at equator
	pt1.setInDegrees(0.0, 170.0);
	pt2.setInDegrees(0.0, -170.0);
	GreatCircleArc gca2(pt1, pt2);
	TEST_ASSERT(gca2.getPoint(0.5).isOnIntlDateLine());

	pt1.setInDegrees(0.0, 10.0);
	pt2.setInDegrees(0.0, -10.0);
	GreatCircleArc gca3(pt1, pt2);
	TEST_ASSERT(gca3.getPoint(0.5).isOnPrimeMeridian());

	// if not at equator, great circle arcs should bow towards poles
	CoordLatLon result;

	pt1.setInDegrees(50.0, -25.0);
	pt2.setInDegrees(50.0, 25.0);
	GreatCircleArc gca4(pt1, pt2);
	result = gca4.getPoint(0.5);
	TEST_ASSERT(result.isOnPrimeMeridian());
	TEST_ASSERT(result.lat() > pt1.lat());

	pt1.setInDegrees(-50.0, 155.0);
	pt2.setInDegrees(-50.0, -155.0);
	GreatCircleArc gca5(pt1, pt2);
	result = gca5.getPoint(0.5);
	TEST_ASSERT(result.isOnIntlDateLine());
	TEST_ASSERT(result.lat() < pt1.lat());
}

/*!
Constructor initializes member variables.

\param	pt1	The start location
\param	pt2	The end location
*/
GreatCircleArc::GreatCircleArc(	const CoordLatLon& pt1,
								const CoordLatLon& pt2	) :
	m_pt1(pt1),
	m_pt2(pt2),
	m_fDistance(calcDistance(pt1, pt2)),
	m_fSinDistance(sin(m_fDistance))
{
}

/*!
Calculate the distance between two points along a great circle arc. If the
radius is omitted or 1.0, the returned distance is in radians. Otherwise it is
in the same units as the radius.

See the  "Aviation Formulary V1.42" page by By Ed Williams at
"http://williams.best.vwh.net/avform.htm"

\param	pt1		The start point.
\param	pt2		The end point.
\param	fRadius	The radius of the sphere (default = 1.0)

\return	The distance.
*/
double GreatCircleArc::calcDistance(	const CoordLatLon& pt1,
										const CoordLatLon& pt2,
										double fRadius	)
{
	double fA = sin((pt1.lat() - pt2.lat()) / 2.0);
	double fB = sin((pt1.lon() - pt2.lon()) / 2.0);

	double fRadians =	2 * asin(	sqrt(fA * fA + 
									cos(pt1.lat()) *
									cos(pt2.lat()) *
									fB * fB)	);
	return (fRadians * fRadius);
}

/*!
Determine the lat lon coordinate on a great circle arc as fraction of the
distance from between the points. 

See the  "Aviation Formulary V1.42" page by By Ed Williams at
"http://williams.best.vwh.net/avform.htm"

\param	fFraction	The fraction of distance along the great circle arc.

\return	The point.
*/
CoordLatLon GreatCircleArc::getPoint(double fFraction) const
{
	assert(	0.0 < fFraction || MathUtils::equal(fFraction, 0.0) && 
			fFraction < 1.0 || MathUtils::equal(fFraction, 1.0)	);
	
	double fA = sin((1.0 - fFraction) * m_fDistance) / m_fSinDistance;
 	double fB = sin(fFraction * m_fDistance) / m_fSinDistance;

	double fC = fA * cos(m_pt1.lat());
	double fD = fB * cos(m_pt2.lat());

	double fX = fC * cos(m_pt1.lon()) + 
				fD * cos(m_pt2.lon());

    double fY = fC * sin(m_pt1.lon()) + 
				fD * sin(m_pt2.lon());

    double fZ = fA * sin(m_pt1.lat()) + fB * sin(m_pt2.lat());
  
	double fLat = atan2(fZ, sqrt(fX * fX + fY * fY));
	double fLon = atan2(fY, fX);

	return CoordLatLon(fLat, fLon);
}
