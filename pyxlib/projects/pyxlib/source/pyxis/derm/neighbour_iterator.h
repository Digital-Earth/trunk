#ifndef PYXIS__DERM__NEIGHBOR_ITERATOR_H
#define PYXIS__DERM__NEIGHBOR_ITERATOR_H
/******************************************************************************
neighbour_iterator.h

begin		: 2006-05-05
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/iterator.h"	
#include "pyxis/derm/valid_direction_iterator.h"

/*!
PYXNeighbourIterater iterates over all the cells in a neighbourhood. That is a 
centroid cell and the six adjoining cells. 
*/
//! Iterates over the cells of a Neighbourhood.
class PYXLIB_DECL PYXNeighbourIterator : public PYXIterator
{
public:
	static PYXPointer<PYXNeighbourIterator> create(const PYXIcosIndex& indexRoot)
	{
		return PYXNEW(PYXNeighbourIterator,indexRoot);
	}

public:

	//! Test method
	static void test();

	//! Constructor
	explicit PYXNeighbourIterator(const PYXIcosIndex& indexRoot);

	//! Destructor
	virtual ~PYXNeighbourIterator() {;}

	//! Determine if all of the cells have been iterated over.
	virtual bool end() const;

	//! Advance to the next cell
	virtual void next();

	//! Access the index at the current iterator position.
	virtual const PYXIcosIndex& getIndex() const
	{
		return m_index;
	}

	//! Return the current direction of the iterator.
	virtual PYXMath::eHexDirection getDirection() 
	{
		return m_itValidDirection.getDirection();
	}

	/*!
	Get the root index.

	\return	The root index.
	*/
	const PYXIcosIndex& getRootIndex() const
	{
		return m_indexRoot;
	}

protected:

private:

	//! Disable default constructor
	PYXNeighbourIterator();

	//! Disable copy constructor
	PYXNeighbourIterator(const PYXNeighbourIterator&);

	//! Disable copy assignment
	void operator =(const PYXNeighbourIterator&);

private:

	//! The index whose neighbourhood is being iterated.
	const PYXIcosIndex m_indexRoot;

	//! Index for temporary use.
	PYXIcosIndex m_index;

	//! The valid direction iterator.
	PYXValidDirectionIterator m_itValidDirection;
};

#endif // guard

