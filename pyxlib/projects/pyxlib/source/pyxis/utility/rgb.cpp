/******************************************************************************
pyx_rgb.cpp

begin		: 2004-11-11
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/rgb.h"

// local includes

// standard includes

/*!
Format a PYXRGB value and write it out to a space delimited string in the
order:  red green blue alpha

\param out	The output stream.
\param rgb	The RGB from which to write.
*/
std::ostream& operator<<(std::ostream& out, const PYXRGB& rgb)
{
	return out <<
		rgb.red() << ' ' <<
		rgb.green() << ' ' <<
		rgb.blue() << ' ' <<
		rgb.alpha();
}

/*!
Read the text for the RGB value from the stream.

\param in		The input stream.
\param rgb		The RGB to which to write.
\return	The input stream.
*/
std::istream& operator>> (std::istream& in, PYXRGB& rgb)
{
	int nR;
	int nG;
	int nB;
	int nA;

	in >> nR >> nG >> nB >> nA;
	rgb.set(nR, nG, nB, nA);
	return in;
}
