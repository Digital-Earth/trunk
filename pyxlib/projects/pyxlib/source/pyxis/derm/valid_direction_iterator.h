#ifndef PYXIS__DERM__VALID_DIRECTION_ITERATOR_H
#define PYXIS__DERM__VALID_DIRECTION_ITERATOR_H
/******************************************************************************
valid_direction_iterator.h

begin		: 2006-09-07
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/direction_iterator.h"
#include "pyxis/derm/hex_direction_iterator.h"
#include "pyxis/derm/index.h"

//! Iterates over the valid directions of a PYXIS cell.
class PYXLIB_DECL PYXValidDirectionIterator : public PYXDirectionIterator
{
public:

	//! Test method
	static void test();

	/*!
	Constructor

	\param	indexRoot	The index of the cell whose valid directions to iterate over. 
						This cannot be a null index.
	*/
	explicit PYXValidDirectionIterator(const PYXIcosIndex& indexRoot);

	//! Destructor
	virtual ~PYXValidDirectionIterator() {}

	//! Simple operator of the form "++myIterator".
	virtual PYXValidDirectionIterator& operator ++() 
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

	//! Get the root index.
	const PYXIcosIndex& getRootIndex() const {return m_indexRoot;}

private:

	//! Disable default constructor.
	PYXValidDirectionIterator();

	//! Disable copy constructor.
	PYXValidDirectionIterator(const PYXValidDirectionIterator&);

	//! Disable copy assignment.
	void operator =(const PYXValidDirectionIterator&);

private:

	//! The hex direction iterator.
	PYXHexDirectionIterator m_itHexDirection;

	//! The index of the cell whose directions will be iterated.
	PYXIcosIndex m_indexRoot;

	//! An optimization indicating if the cell is a pentagon or a hexagon.
	bool m_bIsPentagon;
};

#endif // guard
