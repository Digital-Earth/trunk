/******************************************************************************
hex_direction_iterator.cpp

begin		: 2006-09-07
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/hex_direction_iterator.h"

// pyxlib includes
#include "pyxis/derm/hexagon.h"
#include "pyxis/utility/tester.h"

//! Tester class
Tester<PYXHexDirectionIterator> gTester;

//! Test method
void PYXHexDirectionIterator::test()
{
	PYXHexDirectionIterator itDirection;

	TEST_ASSERT(itDirection.getDirection() == PYXMath::knDirectionZero);
	TEST_ASSERT(*itDirection == itDirection.getDirection());
	TEST_ASSERT(!(itDirection.end()));

	itDirection.next();
	TEST_ASSERT(itDirection.getDirection() == PYXMath::knDirectionOne);
	TEST_ASSERT(*itDirection == itDirection.getDirection());
	TEST_ASSERT(!(itDirection.end()));

	++itDirection;
	TEST_ASSERT(itDirection.getDirection() == PYXMath::knDirectionTwo);
	TEST_ASSERT(*itDirection == itDirection.getDirection());
	TEST_ASSERT(!(itDirection.end()));

	PYXDirectionIterator& itDirectionRef = ++itDirection;
	TEST_ASSERT(itDirection.getDirection() == PYXMath::knDirectionThree);
	TEST_ASSERT(*itDirection == itDirection.getDirection());
	TEST_ASSERT(itDirectionRef.getDirection() == itDirection.getDirection());
	TEST_ASSERT(!(itDirection.end()));

	++itDirection;
	TEST_ASSERT(itDirection.getDirection() == PYXMath::knDirectionFour);
	TEST_ASSERT(*itDirection == itDirection.getDirection());
	TEST_ASSERT(!(itDirection.end()));

	itDirection.next();
	TEST_ASSERT(itDirection.getDirection() == PYXMath::knDirectionFive);
	TEST_ASSERT(*itDirection == itDirection.getDirection());
	TEST_ASSERT(!(itDirection.end()));

	itDirection.next();
	TEST_ASSERT(itDirection.getDirection() == PYXMath::knDirectionSix);
	TEST_ASSERT(*itDirection == itDirection.getDirection());
	TEST_ASSERT(!(itDirection.end()));

	itDirection.next();
	TEST_ASSERT(itDirection.getDirection() == PYXMath::knDirectionZero);
	TEST_ASSERT(*itDirection == itDirection.getDirection());
	TEST_ASSERT(itDirection.end());
}

/*!
Constructor
*/
PYXHexDirectionIterator::PYXHexDirectionIterator() :
	m_nDirection(PYXMath::knDirectionZero),
	m_bEnd(false)
{
}

/*!
Advance to the next direction.
*/
void PYXHexDirectionIterator::next()
{
	if (m_nDirection == Hexagon::knNumSides)
	{
		m_nDirection = PYXMath::knDirectionZero;
		m_bEnd = true;

		return;
	}

	m_nDirection = static_cast<PYXMath::eHexDirection>(m_nDirection + 1);
}

/*! 
Is the iterator complete.

\return	true if the iterator is complete, otherwise false.
*/
bool PYXHexDirectionIterator::end() const
{
	return m_bEnd;
}

/*!
Get the current direction.
*/
PYXMath::eHexDirection PYXHexDirectionIterator::getDirection() const
{
	return m_nDirection;
}
