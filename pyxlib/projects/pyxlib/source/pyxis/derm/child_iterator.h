#ifndef PYXIS__DERM__CHILD_ITERATOR_H
#define PYXIS__DERM__CHILD_ITERATOR_H
/******************************************************************************
child_iterator.h

begin		: 2006-09-06
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/iterator.h"
#include "pyxis/derm/valid_direction_iterator.h"

// standard includes

/*!
PYXChildIterator iterates over the children of a PYXIS cell, starting
with the centroid child, followed by the vertex children in counter-
clockwise order.
*/
//! Iterates over the children of a PYXIS cell.
class PYXLIB_DECL PYXChildIterator : public PYXIterator
{
public:

	//! Test method
	static void test();

	//! Constructor
	explicit PYXChildIterator(const PYXIcosIndex& indexRoot);

	//! Destructor
	virtual ~PYXChildIterator() {}

	//! Move to the next cell.
	virtual void next();

	/*!
	See if we have covered all the cells.

	\return	true if all cells have been covered, otherwise false.
	*/
	virtual bool end() const;

	/*!
	Get the PYXIS icos index for the current cell.

	\return	The PYXIS icos index for the current cell.
	*/
	virtual const PYXIcosIndex& getIndex() const;

	//! Simple operator of the form "++myIterator".
	virtual PYXChildIterator& operator ++() 
	{
		next();
		return *this;
	}

private:

	//! Disable default constructor.
	PYXChildIterator();

	//! Disable copy constructor.
	PYXChildIterator(const PYXChildIterator&);

	//! Disable copy assignment.
	void operator =(const PYXChildIterator&);

private:

	//! An iterator over the valid directions (i.e. excluding pentagon gaps).
	PYXValidDirectionIterator m_itValidDirection;

	//!	The current index.
	PYXIcosIndex m_index;

	//! A cache for the root index, for optimization reasons.
	const PYXIcosIndex m_indexRoot;
};



/*!
PYXInnerChildIterator iterates over the Inner children of a PYXIS Cell.
*/
//! Iterates over the Inner children of a PYXIS cell.
class PYXLIB_DECL PYXInnerChildIterator : public PYXIterator
{
public:

	//! Test method
	static void test();

	//! Constructor
	explicit PYXInnerChildIterator(const PYXIcosIndex& indexRoot);

	//! Destructor
	virtual ~PYXInnerChildIterator() {}

	//! Move to the next cell.
	virtual void next();

	/*!
	See if we have covered all the cells.

	\return	true if all cells have been covered, otherwise false.
	*/
	virtual bool end() const;

	/*!
	Get the PYXIS icos index for the current cell.

	\return	The PYXIS icos index for the current cell.
	*/
	virtual const PYXIcosIndex& getIndex() const;

	//! Simple operator of the form "++myIterator".
	virtual PYXInnerChildIterator& operator ++() 
	{
		next();
		return *this;
	}

private:

	//! Disable default constructor.
	PYXInnerChildIterator();

	//! Disable copy constructor.
	PYXInnerChildIterator(const PYXInnerChildIterator&);

	//! Disable copy assignment.
	void operator =(const PYXInnerChildIterator&);

private:

	//! An iterator over the valid directions (i.e. excluding pentagon gaps).
	PYXValidDirectionIterator m_itValidDirection;

	//!	The current index.
	PYXIcosIndex m_index;

	//! A cache for the root index, for optimization reasons.
	const PYXIcosIndex m_indexRoot;
};




#endif // guard
