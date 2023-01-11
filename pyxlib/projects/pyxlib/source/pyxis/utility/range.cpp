/******************************************************************************
range.cpp

begin		: Dec 15, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "range.h"
#include "tester.h"
#include "exceptions.h"


#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"

// standard includes
#include <cassert>

typedef Range<PYXValue> RangePYXValue;

class RangeTester
{
public:
	static void test();
};


//! The unit test class
Tester<RangeTester> gTester;

void RangeTester::test()
{
	{
		RangePYXValue range(PYXValue(0.0),PYXValue(1.0),knClosed,knOpen);

		RangePYXValue lower = range.lowerHalf();

		TEST_ASSERT (lower.min == range.min);
		TEST_ASSERT (lower.max == range.middle());

		TEST_ASSERT (lower.minType == knClosed);
		TEST_ASSERT (lower.maxType == knOpen);

		TEST_ASSERT (range.contains(lower));
	}

	{
		RangeInt range(0,10,knClosed,knClosed);

		TEST_ASSERT(!range.contains(-1));
		TEST_ASSERT(range.contains(0));
		TEST_ASSERT(range.contains(10));
		TEST_ASSERT(!range.contains(11));

		range.maxType = knOpen;

		TEST_ASSERT(range.contains(0));
		TEST_ASSERT(!range.contains(10));

		range.maxType = knInfinite;
		range.minType = knOpen;

		TEST_ASSERT(!range.contains(0));
		TEST_ASSERT(range.contains(11));

		range.maxType = knClosed;
		range.minType = knInfinite;

		TEST_ASSERT(range.contains(-1));
		TEST_ASSERT(range.contains(0));
		TEST_ASSERT(range.contains(10));
		TEST_ASSERT(!range.contains(11));
	}

	{
		RangeInt range(0,10,knClosed,knClosed);
		RangeInt range2(10,11,knClosed,knClosed);

		TEST_ASSERT(range.intersects(range2));
	}

	{
		RangeInt range(0,10,knClosed,knOpen);
		RangeInt range2(10,11,knClosed,knClosed);

		TEST_ASSERT(!range.intersects(range2));
	}

	{
		RangeInt range(0,10,knClosed,knClosed);
		RangeInt range2(10,11,knOpen,knClosed);

		TEST_ASSERT(!range.intersects(range2));
	}

	{
		RangeInt range(11,12,knClosed,knClosed);
		RangeInt range2(10,11,knClosed,knClosed);

		TEST_ASSERT(range.intersects(range2));
	}

	{
		RangeInt range(11,12,knOpen,knClosed);
		RangeInt range2(10,11,knClosed,knClosed);

		TEST_ASSERT(!range.intersects(range2));
	}

	{
		RangeInt range(11,12,knClosed,knClosed);
		RangeInt range2(10,11,knClosed,knOpen);

		TEST_ASSERT(!range.intersects(range2));
	}

	{
		RangeInt range(0,10,knOpen,knOpen);
				
		TEST_ASSERT(!range.intersects(RangeInt(0)));
		TEST_ASSERT(range.intersects(RangeInt(1)));
		TEST_ASSERT(range.intersects(RangeInt(9)));
		TEST_ASSERT(!range.intersects(RangeInt(10)));
	}

	{
		RangeInt range(0,10,knClosed,knClosed);

		TEST_ASSERT(range.intersects(RangeInt(0)));
		TEST_ASSERT(range.intersects(RangeInt(1)));
		TEST_ASSERT(range.intersects(RangeInt(9)));
		TEST_ASSERT(range.intersects(RangeInt(10)));
	}
}



