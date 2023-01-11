/******************************************************************************
coord_polar.cpp

begin		: 2004-05-06
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/coord_polar.h"

// local includes
#include "pyxis/utility/tester.h"

// standard includes
#include <cmath>

//! Tester class
Tester<PYXCoordPolar> gTester;

//! Test method
void PYXCoordPolar::test()
{
	PYXCoordPolar pt1;
	PYXCoordPolar pt2;

	// test equal method

	// test normal cases
	pt1.setRadius(10.0);
	pt1.setAngleInDegrees(30.0);
	pt2 = pt1;
	TEST_ASSERT(pt1.equal(pt2));

	pt2.setRadius(15.0);
	TEST_ASSERT(!pt1.equal(pt2));

	pt2 = pt1;
	pt2.setAngleInDegrees(45.0);
	TEST_ASSERT(!pt1.equal(pt2));

	// test special case - radius = 0
	pt1.setRadius(0.0);
	pt1.setAngleInDegrees(0.0);
	pt2.setRadius(0.0);
	pt2.setAngleInDegrees(30.0);
	TEST_ASSERT(pt1.equal(pt2));

	// test special case - angles around +/- 180
	pt1.setRadius(10.0);
	pt1.setAngleInDegrees(179.9);
	pt2.setRadius(10.0);
	pt2.setAngleInDegrees(-179.9);
	TEST_ASSERT(pt1.equal(pt2, 0.01));

	// test addition and subtraction
	for (double fDeg = 0; fDeg < 360 + MathUtils::kfPI; fDeg += MathUtils::kfPI)
	{
		double fRad = MathUtils::degreesToRadians(fDeg);
		PYXCoordPolar a(1, MathUtils::kf30Rad + fRad);
		PYXCoordPolar b(sqrt(3.0), MathUtils::kf120Rad + fRad);
		PYXCoordPolar c(2, MathUtils::kf90Rad + fRad);
		{
			PYXCoordPolar d(a);
			d += b;
			TEST_ASSERT(d.equal(c));
		}
		{
			PYXCoordPolar d(b);
			d += a;
			TEST_ASSERT(d.equal(c));
		}
		{
			PYXCoordPolar d(c);
			d -= a;
			TEST_ASSERT(d.equal(b));
		}
		{
			PYXCoordPolar d(c);
			d -= b;
			TEST_ASSERT(d.equal(a));
		}
	}
}

/*!
Normalize the angle. Convert to a value in the range [-pi, pi).
*/
void PYXCoordPolar::normalize()
{
	while (m_fAngle > MathUtils::kf180Rad)
	{
		m_fAngle -= MathUtils::kf360Rad;
	}

	while (m_fAngle < -MathUtils::kf180Rad)
	{
		m_fAngle += MathUtils::kf360Rad;
	}
};

/*!
Are the coordinates equal within a given precision. This method assumes the
angle has been normalized.

\param	pt			The coordinate to compare with this one.
\param	fPrecision	The precision with which to compare.

\return	true if the coordinates are equal within the given precision, otherwise
		false.
*/
bool PYXCoordPolar::equal(	const PYXCoordPolar& pt,
							double fPrecision) const
{
	bool bEqual = false;

	// check for equal radius
	if (MathUtils::equal(m_fRadius, pt.m_fRadius, fPrecision))
	{
		// if the radius is zero, angle doesn't matter
		if (MathUtils::equal(m_fRadius, 0.0, fPrecision))
		{
			bEqual = true;
		}
		else	// check angle
		{
			// if angle is near 180 degrees, must check +/- case
			if (MathUtils::equal(fabs(m_fAngle), MathUtils::kf180Rad, fPrecision))
			{
				if (MathUtils::equal(fabs(m_fAngle), fabs(pt.m_fAngle), fPrecision))
				{
					bEqual = true;
				}
			}
			else if (MathUtils::equal(m_fAngle, pt.m_fAngle, fPrecision))
			{
				bEqual = true;
			}
		}
	}
	
	return bEqual;
}
