#ifndef PYXIS__UTILITY__COORD_POLAR_H
#define PYXIS__UTILITY__COORD_POLAR_H
/******************************************************************************
coord_polar.h

begin		: 2004-03-10
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/math_utils.h"

//! Represents a polar coordinate.
class PYXLIB_DECL PYXCoordPolar
{
public:

	//! Test method
	static void test();
	
	/*!
	Constructor.

	\param	fRadius	The radius.
	\param	fAngle	The angle in radians.
	*/
	PYXCoordPolar(double fRadius = 0.0, double fAngle = 0.0) :
		m_fRadius(fRadius),
		m_fAngle(fAngle)
	{
		normalize();
	}

	//! Get the radius
	inline double radius() const {return m_fRadius;}

	//! Set the radius
	inline void setRadius(double fRadius) {m_fRadius = fRadius;}

	//! Get the angle in radians
	inline double angle() const {return m_fAngle;}

	//! Set the angle in radians
	inline void setAngle(double fAngle)
	{
		m_fAngle = fAngle;
		normalize();
	}

	//! Get the angle in degrees
	inline double angleInDegrees() const
	{
		return MathUtils::radiansToDegrees(m_fAngle);
	}

	//! Set the angle in degrees
	inline void setAngleInDegrees(double fAngle)
	{
		m_fAngle = MathUtils::degreesToRadians(fAngle);
		normalize();
	}

	//! Are the coordinates equal within a given precision
	bool equal(	const PYXCoordPolar& pt,
				double fPrecision = MathUtils::kfDefaultDoublePrecision	) const;

	PYXCoordPolar& operator +=(const PYXCoordPolar& rhs)
	{
		if (rhs.m_fRadius) // TODO maybe should compare with tolerance
		{
#if 0
			// TODO this code should be faster but has issues with angle calculation
			// in some cases
			double fTheta = m_fAngle - rhs.m_fAngle;
			if (fTheta < -MathUtils::kf180Rad || MathUtils::kf180Rad <= fTheta)
			{
				fTheta = m_fAngle + rhs.m_fAngle;
			}
			m_fRadius = sqrt(
				(m_fRadius * m_fRadius)
				+ (rhs.m_fRadius * rhs.m_fRadius)
				- (2 * m_fRadius * rhs.m_fRadius * cos(fTheta)));
			assert(m_fRadius);
			m_fAngle += asin(rhs.m_fRadius * sin(-fTheta) / m_fRadius);
#else
			// adapted from formulas at http://www.iancgbell.clara.net/maths/vectors.htm
			double x1 = m_fRadius * cos(m_fAngle);
			double y1 = m_fRadius * sin(m_fAngle);
			double x2 = rhs.m_fRadius * cos(rhs.m_fAngle);
			double y2 = rhs.m_fRadius * sin(rhs.m_fAngle);
			double x = x1 + x2;
			double y = y1 + y2;
			m_fRadius = sqrt(x*x + y*y);
			if (x < 0)
			{
				m_fAngle = MathUtils::kf180Rad - atan(-y/x);
			}
			else if (0 < x)
			{
				m_fAngle = atan(y/x);
			}
			else if (y < 0)
			{
				m_fAngle = -MathUtils::kf90Rad;
			}
			else if (0 < y)
			{
				m_fAngle = MathUtils::kf90Rad;
			}
			else
			{
				m_fAngle = 0; // arbitrary
			}
#endif
			normalize();
		}
		return *this;
	}

	PYXCoordPolar& operator -=(const PYXCoordPolar& rhs)
	{
		PYXCoordPolar tmp(rhs);
		tmp.negate();
		return (*this) += tmp;
	}

private:

	//! Normalize the angle. Convert to a value in the range [-pi, pi).
	void normalize();

	//! Restrict angle to range, assuming it's only out by at most two pi.
	double restrictSingle(double fAngle)
	{
		double fRestrict = fAngle;
		if (fRestrict < -MathUtils::kf180Rad)
		{
			fRestrict += MathUtils::kf360Rad;
		}
		else if (MathUtils::kf180Rad <= fRestrict)
		{
			fRestrict -= MathUtils::kf360Rad;
		}
		return fRestrict;
	}

	void negate()
	{
		m_fAngle = restrictSingle(m_fAngle + MathUtils::kf180Rad);
	}

private:

	//! The radius
	double m_fRadius;

	//! The angle in radians
	double m_fAngle;
};

#endif // guard
