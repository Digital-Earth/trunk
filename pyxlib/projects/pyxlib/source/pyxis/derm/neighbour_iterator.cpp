/******************************************************************************
neighbour_iterator.cpp

begin		: 2006-05-05
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/neighbour_iterator.h"

// pyxlib includes
#include "pyxis/derm/index_math.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <cassert>

//! Tester class
Tester<PYXNeighbourIterator> gTester;

//! Test method
void PYXNeighbourIterator::test()
{
	std::string strIndex;

	// Hexagon centroid.
	{
		PYXIcosIndex index("A-00");
		PYXNeighbourIterator itNeighbour(index);

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "A-00");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "A-01");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "A-02");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "A-03");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "A-04");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "A-05");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "A-06");
		itNeighbour.next();

		TEST_ASSERT(itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "A-00");
	}

	// Hexagon vertex (neighbour = stay on the tessellation).
	{
		PYXIcosIndex index("B-05");
		PYXNeighbourIterator itNeighbour(index);

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "B-05");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "B-06");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "B-00");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "B-04");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "4-30");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "4-02");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "4-20");
		itNeighbour.next();

		TEST_ASSERT(itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "B-05");
	}

	// Hexagon vertex (neighbour = fall off the tessellation).
	{
		PYXIcosIndex index("D-01");
		PYXNeighbourIterator itNeighbour(index);

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "D-01");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "1-05");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "1-50");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "D-02");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "D-00");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "D-06");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "1-60");
		itNeighbour.next();

		TEST_ASSERT(itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "D-01");
	}

	// Pentagon (stays on the tesselation).
	{
		PYXIcosIndex index("1-0");
		PYXNeighbourIterator itNeighbour(index);

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "1-0");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "1-2");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "1-3");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "1-4");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "1-5");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "1-6");
		itNeighbour.next();

		TEST_ASSERT(itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "1-0");
	}

	// Pentagon (falls off the tesselation).
	{
		PYXIcosIndex index("1-2");
		PYXNeighbourIterator itNeighbour(index);

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "1-2");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "E-0");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "2-2");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "A-0");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "1-3");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "1-0");
		itNeighbour.next();

		TEST_ASSERT(!itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "1-6");
		itNeighbour.next();

		TEST_ASSERT(itNeighbour.end());
		strIndex = itNeighbour.getIndex().toString();
		TEST_ASSERT(strIndex == "1-2");
	}
}

/*!
Constructor

\param	indexRoot	The centre index of cell in the centre of the neighbourhood to iterate over. 
					This cannot be a null index.
*/
PYXNeighbourIterator::PYXNeighbourIterator(const PYXIcosIndex& indexRoot) :
	m_indexRoot(indexRoot),
	m_index(indexRoot), 
	m_itValidDirection(indexRoot)
{
	assert(!(indexRoot.isNull()) && "Invalid argument.");
}

/*! 
Is the iterator complete.

\return	true if the iterator is complete, otherwise false.
*/
bool PYXNeighbourIterator::end() const
{
	return m_itValidDirection.end();
}

/*!
Advance to the next cell.
*/
void PYXNeighbourIterator::next()
{
	m_itValidDirection.next();
	m_index = PYXIcosMath::move(m_indexRoot, m_itValidDirection.getDirection());
}
