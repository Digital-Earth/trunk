#ifndef PYXIS__DERM__DIRECTION_ITERATOR_H
#define PYXIS__DERM__DIRECTION_ITERATOR_H
/******************************************************************************
direction_iterator.h

begin		: 2006-09-07
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/utility/abstract_iterator.h"

//! Iterates over the directions of a PYXIS cell.
class PYXLIB_DECL PYXDirectionIterator : public PYXAbstractIterator
{
public:

	//! Get the current direction.
	PYXMath::eHexDirection operator *() const
	{
		return getDirection();
	}

public:

	//! Get the current direction.
	virtual PYXMath::eHexDirection getDirection() const = 0;

protected:

	//! Default constructor.
	PYXDirectionIterator() {}

	//! Destructor
	virtual ~PYXDirectionIterator() {}

private:

	//! Disable copy constructor.
	PYXDirectionIterator(const PYXDirectionIterator&);

	//! Disable copy assignment.
	void operator =(const PYXDirectionIterator&);
};

#endif // guard
