#ifndef PYXIS__DERM__HEXAGON_H
#define PYXIS__DERM__HEXAGON_H
/******************************************************************************
hexagon.h

begin		: 2004-04-05
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

/*!
Hexagon contains methods relating to the geometry of a hexagon.
*/
//! Represents the geometric properties of a hexagon.
class PYXLIB_DECL Hexagon
{
public:

	//! Test method
	static void test();

	//! The number of vertices for the hexagon.
	static const int knNumVertices;

	//! The number of sides for the hexagon.
	static const int knNumSides;
	
	//! Convert a circumradius to an inradius.
	static double circumRadiusToInRadius(double fValue);

	//! Convert an inradius to a circumradius
	static double inRadiusToCircumRadius(double fValue);

private:

	//! Disable constructor
	Hexagon();

	//! Disable copy constructor
	Hexagon(const Hexagon&);

	//! Disable copy assignment
	void operator=(const Hexagon&);
};

#endif // guard
