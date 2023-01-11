/******************************************************************************
child_iterator.cpp

begin		: 2006-09-08
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/child_iterator.h"

// pyxlib includes
#include "pyxis/derm/index_math.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <cassert>

//! Tester class
Tester<PYXChildIterator> gTester;

//! Test method
void PYXChildIterator::test()
{
	std::string strIndex;

	// Centroid cell children.
	// Should be either 5 or 6 children, which is handled by the valid direction iterator.
	{
		PYXIcosIndex index("1");
		PYXChildIterator it(index);

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "1-0");
		it.next();

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "1-2");
		++it;

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "1-3");
		++it;

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "1-4");
		it.next();

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "1-5");
		it.next();

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "1-6");
		it.next();

		TEST_ASSERT(it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "1-0");
	}

	// 1-00
	{
		PYXIcosIndex index("1-00");
		PYXChildIterator it(index);

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "1-000");
		it.next();

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "1-002");
		++it;

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "1-003");
		++it;

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "1-004");
		it.next();

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "1-005");
		it.next();

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "1-006");
		it.next();

		TEST_ASSERT(it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "1-000");
	}

	// 4-00
	{
		PYXIcosIndex index("12-00");
		PYXChildIterator it(index);

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "12-000");
		it.next();

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "12-001");
		it.next();

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "12-002");
		++it;

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "12-003");
		++it;

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "12-005");
		it.next();

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "12-006");
		it.next();

		TEST_ASSERT(it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "12-000");
	}

	// Resolution 1 hexagon children.
	{
		PYXIcosIndex index("B");
		PYXChildIterator it(index);

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "B-0");
		it.next();

		TEST_ASSERT(it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "B-0");
	}

	// B-00
	{
		PYXIcosIndex index("B-00");
		PYXChildIterator it(index);

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "B-000");
		it.next();

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "B-001");
		it.next();

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "B-002");
		++it;

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "B-003");
		++it;

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "B-004");
		it.next();

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "B-005");
		it.next();

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "B-006");
		it.next();

		TEST_ASSERT(it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "B-000");
	}

	// Vertex cell children.
	// Should only be 1.
	{
		PYXIcosIndex index("B-01");
		PYXChildIterator it(index);

		TEST_ASSERT(!it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "B-010");
		it.next();

		TEST_ASSERT(it.end());
		strIndex = it.getIndex().toString();
		TEST_ASSERT(strIndex == "B-010");
	}
}

/*!
Constructor

\param	indexRoot	The index of the cell whose children to iterate over. 
					This cannot be a null index.
*/
PYXChildIterator::PYXChildIterator(const PYXIcosIndex& indexRoot) :
	m_itValidDirection(indexRoot),
	m_index(indexRoot),
	m_indexRoot(indexRoot)
{
	assert(!(indexRoot.isNull()) && "Invalid argument.");
	m_index.incrementResolution();
}

/*!
Advance to the next child cell.
*/
void PYXChildIterator::next()
{
	m_index = m_indexRoot;
	do
	{
		m_itValidDirection.next();
	}
	while (!PYXIcosMath::zoomIntoChildren(&m_index, m_itValidDirection.getDirection()));
}

/*! 
Is the iterator complete.

\return	true if the iterator is complete, otherwise false.
*/
bool PYXChildIterator::end() const
{
	return m_itValidDirection.end();
}

/*!
Get the PYXIS icos index for the current cell.

\return	The PYXIS icos index for the current cell.
*/
const PYXIcosIndex& PYXChildIterator::getIndex() const
{
	return m_index;
}





/////////////////////////////////////////////
//			Inner Child Iterator
/////////////////////////////////////////////




//! Tester class
Tester<PYXInnerChildIterator> gTester2;

//! Test method
void PYXInnerChildIterator::test()
{
	
}

/*!
Constructor

\param	indexRoot	The index of the cell whose children to iterate over. 
					This cannot be a null index.
*/
PYXInnerChildIterator::PYXInnerChildIterator(const PYXIcosIndex& indexRoot) :
	m_itValidDirection(indexRoot),
	m_index(indexRoot),
	m_indexRoot(indexRoot)
{
	assert(!(indexRoot.isNull()) && "Invalid argument.");
	m_index.incrementResolution();
}

/*!
Advance to the next child cell.
*/
void PYXInnerChildIterator::next()
{
	m_index = m_indexRoot;
	m_index.incrementResolution();
	do
	{
		m_itValidDirection.next();
	}
	while (!PYXIcosMath::zoomIntoChildren(&m_index, m_itValidDirection.getDirection()));

}

/*! 
Is the iterator complete.

\return	true if the iterator is complete, otherwise false.
*/
bool PYXInnerChildIterator::end() const
{
	return m_itValidDirection.end();
}

/*!
Get the PYXIS icos index for the current cell.

\return	The PYXIS icos index for the current cell.
*/
const PYXIcosIndex& PYXInnerChildIterator::getIndex() const
{
	return m_index;
}
