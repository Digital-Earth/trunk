#ifndef PYXIS__DERM__HEX_DIRECTION_ITERATOR_H
#define PYXIS__DERM__HEX_DIRECTION_ITERATOR_H
/******************************************************************************
hex_direction_iterator.h

begin		: 2006-09-07
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/direction_iterator.h"

//! Iterates over the directions of a PYXIS cell.
class PYXLIB_DECL PYXHexDirectionIterator : public PYXDirectionIterator
{
public:

	//! Test method
	static void test();

	//! Constructor
	PYXHexDirectionIterator();

	//! Destructor
	virtual ~PYXHexDirectionIterator() {}

	//! Simple operator of the form "++myIterator".
	virtual PYXHexDirectionIterator& operator ++() 
	{
		next();
		return *this;
	}

	//! Move to the next direction.
	virtual void next();

	/*!
	See if we have covered all the directions.

	\return	true if all directions have been covered, otherwise false.
	*/
	virtual bool end() const;

	//! Get the current direction.
	virtual PYXMath::eHexDirection getDirection() const;

private:

	//! Disable copy constructor.
	PYXHexDirectionIterator(const PYXHexDirectionIterator&);

	//! Disable copy assignment.
	void operator =(const PYXHexDirectionIterator&);

private:

	//! The current direction.
	PYXMath::eHexDirection m_nDirection;

	//! Whether or not one pass has been completed.
	bool m_bEnd;
};

#endif // guard
