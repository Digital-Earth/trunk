/******************************************************************************
iterator.cpp

begin		: 2006-03-30
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/iterator.h"

// pyxlib includes
#include "pyxis/utility/value.h"

// standard includes
#include <cassert>

/*!
Get the value of the current cell.

\param	nFieldIndex	The field index (default = 0)

\return	The value.
*/
PYXValue PYXIterator::getFieldValue(int nFieldIndex) const
{
	assert(false && "Method not implemented.");
	return PYXValue();
}

/*!
Set the value of the current cell.

\param	value		The new value.
\param	nFieldIndex	The field index (default = 0)
*/
void PYXIterator::setFieldValue(const PYXValue& value, int nFieldIndex)
{
	assert(false && "Method not implemented.");
}
