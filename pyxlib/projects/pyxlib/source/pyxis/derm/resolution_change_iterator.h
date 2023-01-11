#ifndef PYXIS__DERM__RESOLUTION_CHANGE_ITERATOR_H
#define PYXIS__DERM__RESOLUTION_CHANGE_ITERATOR_H
/******************************************************************************
resolution_change_iterator.h

begin		: 2005-12-09
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/iterator.h"

//! Iterator that changes resolution.
class PYXLIB_DECL PYXResolutionChangeIterator : public PYXIterator
{
public:

	//! Test method
	static void test();

	//! Constructor
	PYXResolutionChangeIterator(PYXPointer<PYXIterator> spIt, int nResolution);

	//! Destructor
	virtual ~PYXResolutionChangeIterator() {;}

	//! Move to the next cell
	virtual void next();

	/*!
	See if we have covered all the cells.

	\return	true if all cells have been covered, otherwise false.
	*/
	virtual bool end() const
	{
		return m_spItExhaustive == 0 && m_spIt->end();
	}

	/*!
	Get the PYXIS icos index for the current cell.

	\return	The PYXIS icos index for the current cell.
	*/
	virtual const PYXIcosIndex& getIndex() const
	{
		return m_spItExhaustive != 0 ? m_spItExhaustive->getIndex() : m_index;
	}

private:

	//! The contained iterator.
	PYXPointer<PYXIterator> m_spIt;

	//! The exhaustive iterator. If not null, it isn't at its end.
	PYXPointer<PYXIterator> m_spItExhaustive;

	//! The changed resolution.
	int m_nResolution;

	//! The index. Used only if m_spItExhaustive is null.
	PYXIcosIndex m_index;
};

#endif	//guard