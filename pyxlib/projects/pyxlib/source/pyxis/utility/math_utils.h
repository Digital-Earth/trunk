#ifndef PYXIS__UTILITY__MATH_UTILS_H
#define PYXIS__UTILITY__MATH_UTILS_H
/******************************************************************************
math_utils.h

begin		: 2004-01-13
copyright	: derived from math_utils.h (C) 2000 by iGO Technologies Inc.
web			: www.igotechnologies.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

// standard includes
#include <cassert>
#include <cmath>

/*!
Various math utilities and pre calculated values

min() and max() are defined here because they are missing from the default
VC++6.0 STL. Normally, you would include "<algorithm>" to get them. They are
available on Windows in minmax.h, but an ifdeffing WIN32 seemed a bit extreme
just for min and max.
*/
//! Various mathematical calculations and values
class PYXLIB_DECL MathUtils
{
public:

	// Pre-calculated radian values.
	static const double kf30Rad;
	static const double kf60Rad;
	static const double kf90Rad;
	static const double kf120Rad;
	static const double kf150Rad;
	static const double kf180Rad;
	static const double kf240Rad;
	static const double kf300Rad;
	static const double kf360Rad;

	// Pre-calculated trig values.
	static const double kfSin30;
	static const double kfCos30;
	static const double kfSin60;
	static const double kfCos60;
	static const double kfTan60;

	// The default float precision
	static const float kfDefaultFloatPrecision;
	
	//! The default double precision
	static const double kfDefaultDoublePrecision;

	//! An approximation of PI
	static const double kfPI;

	//! Conversion factor from degrees to radians
	static const double kfDegreesToRadians;

	//! the golden ratio
	static const double kfPHI;

	//! The square root of three
	static const double kfSqrt3;

	/*!
	The number is rounded up if the decimal portion is 0.5 or greater otherwise
	it is rounded down. Templated so it can be used with floats or doubles.

	\param	value	The value to be rounded.

	\return	The rounded value.
	*/
	//! Round a floating point numbers of 0.5 or greater up.
	template <class T>
	static int round(T value)
	{
		if (0 < value)
		{
			value += 0.5;
		}
		else if (0 > value)
		{
			value -= 0.5;
		}
		
		return static_cast<int>(value);
	}

	/*!
	Check to see if two floating point numbers are equal to each other within
	the limits of floating point precision. Implementations are provided for
	floats and doubles.

	\param fLHS		The left hand side.	
	\param fRHS		The right hand side.
	\param fEpsilon	The precision.

	\return	true if the numbers are equal, false if not
	*/
	//! Determine equality within a floating point precision.
	static inline bool equal(
		float fLHS,
		float fRHS,
		float fEpsilon = kfDefaultFloatPrecision	)
	{
		return (fabs(fLHS - fRHS) < fEpsilon);
	}

	/*!
	Check to see if two floating point numbers are equal to each other within
	the limits of floating point precision. Implementations are provided for
	floats and doubles.

	\param fLHS		The left hand side.	
	\param fRHS		The right hand side.
	\param fEpsilon	The precision.

	\return	true if the numbers are equal, false if not
	*/
	//! Determine equality within a floating point precision.
	static inline bool equal(
		double fLHS,
		double fRHS,
		double fEpsilon = kfDefaultDoublePrecision	)
	{
		return (fabs(fLHS - fRHS) < fEpsilon);
	}

	/*!
	Determine if a value is between two others (inclusive)

	\param	a	The value to be tested for containment.
	\param	b	An end point.
	\param	c	The other end point.

	\return	true if the value is between, otherwise false.
	*/
	//! Determine if a value is within a range.
	template <class T>
	static bool between(const T& a, const T& b, const T& c)
	{
		return (((a <= b) && (a >= c)) || ((a >= b) && (a <= c)));
	}

	/*!
	Constrain a value to be between a minimum and maximum (inclusive)

	\param	a	The value
	\param	b	The minimum value
	\param	c	The maximum value

	\return	The constrained value.
	*/
	//! Limit a value to be within a range.
	template <class T>
	static T constrain(const T& a, const T& b, const T& c)
	{
		T d = std::max(a, b);
		d = std::min(d, c);

		return d;
	}

	/*!
	Convert an angle from radians to degrees.

	\param fAngle	The angle in radians.

	\return	The angle in degrees.
	*/
	//! Convert radians to degrees.
	static inline double radiansToDegrees(double fAngle)
	{
		assert(0 != kfDegreesToRadians);
		return (fAngle / kfDegreesToRadians); 
	}

	/*!
	Convert an angle from degrees to radians.

	\param fAngle	The angle in degrees.

	\return	The angle in radians.
	*/
	//! Convert degrees to radians.
	static inline double degreesToRadians(double fAngle)
	{
		assert(0 != kfDegreesToRadians);
		return (fAngle * kfDegreesToRadians); 
	}

	/*!
	Calculate the haversine function.

	\param	fAngle	The angle in radians.

	\return	The haversine of the angle.
	*/
	//! Calculate the haversine function of an angle.
	static inline double hav(double fAngle)
	{
		return (1.0 - cos(fAngle)) / 2.0;
	}
};

#endif // guard
