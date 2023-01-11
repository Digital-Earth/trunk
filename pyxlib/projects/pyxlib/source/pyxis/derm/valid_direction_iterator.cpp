/******************************************************************************
valid_direction_iterator.cpp

begin		: 2006-09-07
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/valid_direction_iterator.h"

// pyxlib includes
#include "pyxis/derm/index_math.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <cassert>

//! Tester class
Tester<PYXValidDirectionIterator> gTester;

//! Test method
void PYXValidDirectionIterator::test()
{
	// A pentagon (direction "1" missing).
	{
		PYXIcosIndex index("1");
		PYXValidDirectionIterator itDirection(index);

		TEST_ASSERT(itDirection.getDirection() == PYXMath::knDirectionZero);
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

	// A pentagon (direction "4" missing).
	{
		PYXIcosIndex index("7");
		PYXValidDirectionIterator itDirection(index);

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

	// A hexagon.
	// Note that directions for vertex children are still valid.
	{
		PYXIcosIndex index("B");
		PYXValidDirectionIterator itDirection(index);

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
}

/*!
Constructor

\param	indexRoot	The centre index of cell in the centre of the neighbourhood to iterate over. 
					This cannot be a null index.
*/
PYXValidDirectionIterator::PYXValidDirectionIterator(const PYXIcosIndex& indexRoot) :
	m_indexRoot(indexRoot),
	m_bIsPentagon(indexRoot.isPentagon())
{
	assert(!(indexRoot.isNull()) && "Invalid argument.");
}

//! Move to the next direction.
void PYXValidDirectionIterator::next()
{
	// If this is not a pentagon, it will just go to the next direction,
	// or zero if the end is reached.
	// If this is a pentagon, it will keep going until it hits a valid direction.
	do
	{
		m_itHexDirection.next();
	}
	while (	m_bIsPentagon &&
			!PYXIcosMath::isValidDirection(m_indexRoot, getDirection())	);
}

/*!
See if we have covered all the directions.

\return	true if all directions have been covered, otherwise false.
*/
bool PYXValidDirectionIterator::end() const
{
	return m_itHexDirection.end();
}

//! Get the current direction.
PYXMath::eHexDirection PYXValidDirectionIterator::getDirection() const
{
	return m_itHexDirection.getDirection();
}
