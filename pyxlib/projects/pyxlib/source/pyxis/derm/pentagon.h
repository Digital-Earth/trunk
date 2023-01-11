#ifndef PYXIS__DERM__PENTAGON_H
#define PYXIS__DERM__PENTAGON_H
/******************************************************************************
pentagon.h

begin		: 2004-10-08
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

/*!
The icosahedron will have 12 pentagons on each resolution.  The pentagons will
be centred on the vertices of the icosahedron.  The area of each of the 
pentagons will be 5/6 of the area of a hexagon on the same resolution.
*/
//! Represents the geometric properties of a pentagon.
class PYXLIB_DECL Pentagon
{
public:

	// PYXIS errors

	//! The number of vertices for the pentagon.
	static const int knNumVertices;

	//! The number of sides for the pentagon.
	static const int knNumSides;
	
private:

	//! Disable constructor
	Pentagon();

	//! Disable copy constructor
	Pentagon(const Pentagon&);

	//! Disable copy assignment
	void operator=(const Pentagon&);
};

#endif // guard
