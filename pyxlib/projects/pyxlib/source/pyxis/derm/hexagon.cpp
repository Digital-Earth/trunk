/******************************************************************************
hexagon.cpp

begin		: 2004-04-05
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/hexagon.h"

// pyxlib includes
#include "pyxis/utility/math_utils.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <cassert>
#include <cmath>

Tester<Hexagon> gTester;
template <class T>
static bool AreEqual( const T &value, const T &expectedValue, const T &tolerance)
{
	if (value > expectedValue)
		return (value - expectedValue) < tolerance;
	else
		return (expectedValue - value) < tolerance;
}

///! Test method
void Hexagon::test()
{
	TEST_ASSERT( AreEqual( Hexagon::circumRadiusToInRadius( 0.0), 0.0, 0.0005));
	TEST_ASSERT( AreEqual( Hexagon::circumRadiusToInRadius( 1.0), 0.86602540378, 0.000000001));
	TEST_ASSERT( AreEqual( Hexagon::inRadiusToCircumRadius( 0.0), 0.0, 0.0005));
	TEST_ASSERT( AreEqual( Hexagon::inRadiusToCircumRadius( 1.0), 1.15470053838, 0.000000001));
}


//! The number of vertices for the hexagon.
const int Hexagon::knNumVertices = 6;

//! The number of sides for the hexagon.
const int Hexagon::knNumSides = 6;


/*!
Convert a circumradius to an inradius.

\param	fValue	The circumradius

\return	The inradius
*/
double Hexagon::circumRadiusToInRadius(double fValue)
{
	assert(MathUtils::kfSqrt3);
	return fValue * MathUtils::kfSqrt3 / 2.0;
}

/*!
Convert an inradius to a circumradius.

\param	fValue	The inradius.

\return	The circumradius.
*/
double Hexagon::inRadiusToCircumRadius(double fValue)
{
	assert(MathUtils::kfSqrt3);
	return fValue * 2.0 / MathUtils::kfSqrt3;
}
