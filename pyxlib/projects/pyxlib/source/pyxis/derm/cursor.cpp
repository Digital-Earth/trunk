/******************************************************************************
cursor.cpp

begin		: 2006-12-14
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/cursor.h"

// pxylib includes
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/utility/trace.h"

// standard includes
#include <iostream>
#include <sstream>

// Tester class
Tester<PYXCursor> gTester;

// Test method
void PYXCursor::test()
{
	{
		PYXCursor c(PYXIcosIndex("A-0"), PYXMath::knDirectionOne);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-3") && c.getInitialDir() == PYXMath::knDirectionOne);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("A-0") && c.getInitialDir() == PYXMath::knDirectionOne);
	}
	{
		PYXCursor c(PYXIcosIndex("A-0"), PYXMath::knDirectionTwo);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-2") && c.getInitialDir() == PYXMath::knDirectionTwo);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("A-0") && c.getInitialDir() == PYXMath::knDirectionTwo);
	}
	{
		PYXCursor c(PYXIcosIndex("A-0"), PYXMath::knDirectionThree);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("2-2") && c.getInitialDir() == PYXMath::knDirectionThree);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("A-0") && c.getInitialDir() == PYXMath::knDirectionThree);
	}
	{
		PYXCursor c(PYXIcosIndex("A-0"), PYXMath::knDirectionFour);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("2-6") && c.getInitialDir() == PYXMath::knDirectionFour);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("A-0") && c.getInitialDir() == PYXMath::knDirectionFour);
	}
	{
		PYXCursor c(PYXIcosIndex("A-0"), PYXMath::knDirectionFive);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("3-3") && c.getInitialDir() == PYXMath::knDirectionFive);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("A-0") && c.getInitialDir() == PYXMath::knDirectionFive);
	}
	{
		PYXCursor c(PYXIcosIndex("A-0"), PYXMath::knDirectionSix);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("3-2") && c.getInitialDir() == PYXMath::knDirectionSix);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("A-0") && c.getInitialDir() == PYXMath::knDirectionSix);
	}

	{
		PYXCursor c(PYXIcosIndex("A-00"), PYXMath::knDirectionOne);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("A-01") && c.getInitialDir() == PYXMath::knDirectionOne);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("A-00") && c.getInitialDir() == PYXMath::knDirectionOne);
	}
	{
		PYXCursor c(PYXIcosIndex("A-00"), PYXMath::knDirectionTwo);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("A-02") && c.getInitialDir() == PYXMath::knDirectionTwo);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("A-00") && c.getInitialDir() == PYXMath::knDirectionTwo);
	}
	{
		PYXCursor c(PYXIcosIndex("A-00"), PYXMath::knDirectionThree);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("A-03") && c.getInitialDir() == PYXMath::knDirectionThree);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("A-00") && c.getInitialDir() == PYXMath::knDirectionThree);
	}
	{
		PYXCursor c(PYXIcosIndex("A-00"), PYXMath::knDirectionFour);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("A-04") && c.getInitialDir() == PYXMath::knDirectionFour);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("A-00") && c.getInitialDir() == PYXMath::knDirectionFour);
	}
	{
		PYXCursor c(PYXIcosIndex("A-00"), PYXMath::knDirectionFive);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("A-05") && c.getInitialDir() == PYXMath::knDirectionFive);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("A-00") && c.getInitialDir() == PYXMath::knDirectionFive);
	}
	{
		PYXCursor c(PYXIcosIndex("A-00"), PYXMath::knDirectionSix);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("A-06") && c.getInitialDir() == PYXMath::knDirectionSix);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("A-00") && c.getInitialDir() == PYXMath::knDirectionSix);
	}

	{
		PYXCursor c(PYXIcosIndex("1-0"), PYXMath::knDirectionOne);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionTwo && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("1-0"), PYXMath::knDirectionTwo);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionTwo && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("1-0"), PYXMath::knDirectionThree);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionThree && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("1-0"), PYXMath::knDirectionFour);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionFour && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("1-0"), PYXMath::knDirectionFive);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionFive && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("1-0"), PYXMath::knDirectionSix);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionSix && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("12-0"), PYXMath::knDirectionOne);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionOne && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("12-0"), PYXMath::knDirectionTwo);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionTwo && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("12-0"), PYXMath::knDirectionThree);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionThree && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("12-0"), PYXMath::knDirectionFour);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionFive && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("12-0"), PYXMath::knDirectionFive);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionFive && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("12-0"), PYXMath::knDirectionSix);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionSix && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("1-00"), PYXMath::knDirectionOne);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionSix && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("1-00"), PYXMath::knDirectionTwo);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionTwo && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("1-00"), PYXMath::knDirectionThree);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionThree && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("1-00"), PYXMath::knDirectionFour);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionFour && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("1-00"), PYXMath::knDirectionFive);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionFive && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("1-00"), PYXMath::knDirectionSix);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionSix && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("12-00"), PYXMath::knDirectionOne);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionOne && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("12-00"), PYXMath::knDirectionTwo);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionTwo && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("12-00"), PYXMath::knDirectionThree);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionThree && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("12-00"), PYXMath::knDirectionFour);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionThree && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("12-00"), PYXMath::knDirectionFive);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionFive && c.getRotation() == 0);
	}
	{
		PYXCursor c(PYXIcosIndex("12-00"), PYXMath::knDirectionSix);
		TEST_ASSERT(c.getDir() == PYXMath::knDirectionSix && c.getRotation() == 0);
	}

	{
		PYXMath::eHexDirection nDir = PYXMath::knDirectionFive;
		PYXCursor c(PYXIcosIndex("1-2"), nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-0") && c.getInitialDir() == nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-5") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-0") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-2") && c.getInitialDir() == nDir);
	}
	{
		PYXMath::eHexDirection nDir = PYXMath::knDirectionSix;
		PYXCursor c(PYXIcosIndex("1-3"), nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-0") && c.getInitialDir() == nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-6") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-0") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-3") && c.getInitialDir() == nDir);
	}
	{
		PYXMath::eHexDirection nDir = PYXMath::knDirectionOne;
		PYXCursor c(PYXIcosIndex("1-4"), nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-0") && c.getInitialDir() == nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-2") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-0") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-4") && c.getInitialDir() == nDir);
	}
	{
		PYXMath::eHexDirection nDir = PYXMath::knDirectionTwo;
		PYXCursor c(PYXIcosIndex("1-5"), nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-0") && c.getInitialDir() == nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-3") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-0") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-5") && c.getInitialDir() == nDir);
	}
	{
		PYXMath::eHexDirection nDir = PYXMath::knDirectionThree;
		PYXCursor c(PYXIcosIndex("1-6"), nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-0") && c.getInitialDir() == nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-4") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-0") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-6") && c.getInitialDir() == nDir);
	}

	{
		PYXMath::eHexDirection nDir = PYXMath::knDirectionFour;
		PYXCursor c(PYXIcosIndex("12-1"), nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-0") && c.getInitialDir() == nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-5") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-0") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-1") && c.getInitialDir() == nDir);
	}
	{
		PYXMath::eHexDirection nDir = PYXMath::knDirectionFive;
		PYXCursor c(PYXIcosIndex("12-2"), nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-0") && c.getInitialDir() == nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-6") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-0") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-2") && c.getInitialDir() == nDir);
	}
	{
		PYXMath::eHexDirection nDir = PYXMath::knDirectionSix;
		PYXCursor c(PYXIcosIndex("12-3"), nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-0") && c.getInitialDir() == nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-1") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-0") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-3") && c.getInitialDir() == nDir);
	}
	{
		PYXMath::eHexDirection nDir = PYXMath::knDirectionTwo;
		PYXCursor c(PYXIcosIndex("12-5"), nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-0") && c.getInitialDir() == nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-2") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-0") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-5") && c.getInitialDir() == nDir);
	}
	{
		PYXMath::eHexDirection nDir = PYXMath::knDirectionThree;
		PYXCursor c(PYXIcosIndex("12-6"), nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-0") && c.getInitialDir() == nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-3") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-0") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-6") && c.getInitialDir() == nDir);
	}

	{
		PYXMath::eHexDirection nDir = PYXMath::knDirectionFive;
		PYXCursor c(PYXIcosIndex("1-02"), nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-00") && c.getInitialDir() == nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-04") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-00") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-02") && c.getInitialDir() == nDir);
	}
	{
		PYXMath::eHexDirection nDir = PYXMath::knDirectionSix;
		PYXCursor c(PYXIcosIndex("1-03"), nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-00") && c.getInitialDir() == nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-05") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-00") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-03") && c.getInitialDir() == nDir);
	}
	{
		PYXMath::eHexDirection nDir = PYXMath::knDirectionOne;
		PYXCursor c(PYXIcosIndex("1-04"), nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-00") && c.getInitialDir() == nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-06") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-00") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-04") && c.getInitialDir() == nDir);
	}
	{
		PYXMath::eHexDirection nDir = PYXMath::knDirectionTwo;
		PYXCursor c(PYXIcosIndex("1-05"), nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-00") && c.getInitialDir() == nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-02") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-00") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-05") && c.getInitialDir() == nDir);
	}
	{
		PYXMath::eHexDirection nDir = PYXMath::knDirectionThree;
		PYXCursor c(PYXIcosIndex("1-06"), nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-00") && c.getInitialDir() == nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-03") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-00") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("1-06") && c.getInitialDir() == nDir);
	}

	{
		PYXMath::eHexDirection nDir = PYXMath::knDirectionFour;
		PYXCursor c(PYXIcosIndex("12-01"), nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-00") && c.getInitialDir() == nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-03") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-00") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-01") && c.getInitialDir() == nDir);
	}
	{
		PYXMath::eHexDirection nDir = PYXMath::knDirectionFive;
		PYXCursor c(PYXIcosIndex("12-02"), nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-00") && c.getInitialDir() == nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-05") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-00") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-02") && c.getInitialDir() == nDir);
	}
	{
		PYXMath::eHexDirection nDir = PYXMath::knDirectionSix;
		PYXCursor c(PYXIcosIndex("12-03"), nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-00") && c.getInitialDir() == nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-06") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-00") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-03") && c.getInitialDir() == nDir);
	}
	{
		PYXMath::eHexDirection nDir = PYXMath::knDirectionTwo;
		PYXCursor c(PYXIcosIndex("12-05"), nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-00") && c.getInitialDir() == nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-01") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-00") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-05") && c.getInitialDir() == nDir);
	}
	{
		PYXMath::eHexDirection nDir = PYXMath::knDirectionThree;
		PYXCursor c(PYXIcosIndex("12-06"), nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-00") && c.getInitialDir() == nDir);
		c.forward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-02") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-00") && c.getInitialDir() == nDir);
		c.backward();
		TEST_ASSERT(c.getIndex() == PYXIcosIndex("12-06") && c.getInitialDir() == nDir);
	}

#if NDEBUG
	// Timing tests.
	{
		unsigned int iuPanCount = 20000000;
		PYXIcosIndex index("1-00");
		PYXCursor c(index);

		TRACE_TEST("Timing " << iuPanCount << " random pans at resolution " << index.getResolution() << "..."); 
		const clock_t tClock = clock();
		srand((int unsigned const)time(0));
		for (; iuPanCount > 0; --iuPanCount) {
			/* Get a random direction, between 1 and 6 inclusive. */
			PYXMath::eHexDirection nDirection = (PYXMath::eHexDirection)((rand() % 5) + 1);

			/* Pan in that direction. */
			c.setDir(nDirection);
			c.forward();
		}
		TRACE_TEST(" " << ((double)(clock() - tClock) / CLOCKS_PER_SEC) << "seconds.");
	}
#endif

	/*
	Exhaustively test the connectivity of PYXIS indices. Move in a simple sequence
	that should always bring the cursor back to the starting cell. Repeat over
	the entire grid at a resolution
	*/
	{		
		for (int nRes = 2; nRes < 3; ++nRes)
		{
			TRACE_TIME("Performing Exhaustive cursor iteration at res: " << nRes);
			for (PYXIcosIterator it(nRes); !it.end(); ++it)
			{
				// Pentagons in the cursor tend on direction or the other depending on class	
				int nPentagonModifier = 0;
				if (PYXMath::getHexClass(nRes) == PYXMath::knClassII)
				{
					nPentagonModifier = 1;
				}

				// walk in a circle around the current cell
				PYXCursor c(it.getIndex(), PYXMath::knDirectionTwo);

				// go around each of the neighbours
				for (int nNeighbour = 0; nNeighbour < it.getIndex().determineNumSides(); ++nNeighbour)
				{
					c.forward();
					(c.getIndex().isPentagon()) ? c.left(nPentagonModifier) : c.left();
					if (nNeighbour == 0)
					{
						// the first neighbour came out from the centre, needs an extra turn.
						c.left();
					}
				}

				// move back to the original centre cell
				c.left();
				c.forward();
				TEST_ASSERT(c.getIndex() == it.getIndex());
			}
			TRACE_TIME("Finised Exhaustive cursor iteration at res: " << nRes);
		}
	}
}

//! Move the cursor to the next cell, in the direction the cursor is moving.
void PYXCursor::forward()
{
	int nRot = m_nRot;
	PYXIcosMath::move(&m_index, m_nDir, &m_nRot);
	if (m_nRot != nRot) // speed optimization
	{
		m_nDir = PYXMath::rotateDirection(m_nDir, m_nRot - nRot);
	}
	resolveDirection();
	checkInvariant();
}

void PYXCursor::forward(int nCount)
{
	if (nCount>0)
	{
		for(int i=0;i<nCount;i++)
		{
			forward();
		}
	}
	else if (nCount<0)
	{
		backward(-nCount);
	}
}

//! Move the cursor to the next cell, in the opposite direction the cursor is moving.
void PYXCursor::backward()
{
	PYXMath::eHexDirection nDir = PYXMath::negateDir(m_nDir);

	if (m_index.isPentagon())
	{
		int nGap = m_index.getPrimaryResolution() <= 6 ? 1 : 4;
		int nTest = nDir - nGap;
		int nD = 1;
		if (PYXMath::getHexClass(m_index.getResolution()) == PYXMath::knClassI)
		{
			nD = -1;
			(nTest += 2) %= 6; // tricky math
		}

		if (0 <= nTest && nTest <= 2)
		{
			m_nDir = PYXMath::rotateDirection(m_nDir, nD);
			PYXMath::rotateDelta(&m_nRot, nD);
			nDir = PYXMath::negateDir(m_nDir);
		}
	}

	int nRot = m_nRot;
	PYXIcosMath::move(&m_index, nDir, &m_nRot);
	if (m_nRot != nRot) // speed optimization
	{
		m_nDir = PYXMath::rotateDirection(m_nDir, m_nRot - nRot);
	}
	checkInvariant();
}

void PYXCursor::backward(int nCount)
{
	if (nCount>0)
	{
		for(int i=0;i<nCount;i++)
		{
			backward();
		}
	}
	else if (nCount<0)
	{
		forward(-nCount);
	}
}


//! Rotate the cursor's current direction by one, to the left
void PYXCursor::left(int nCount)
{
	int nDir = m_nDir;
	m_nDir = PYXIcosMath::rotateDirection(m_index, m_nDir, nCount);
	if (m_nDir != nDir) // speed optimization
	{
		PYXMath::rotateDelta(&m_nRot, m_nDir - nDir);
	}
	checkInvariant();
}

//! Rotate the cursor's current direction by one, to the right
void PYXCursor::right(int nCount)
{
	int nDir = m_nDir;
	m_nDir = PYXIcosMath::rotateDirection(m_index, m_nDir, -nCount);
	if (m_nDir != nDir) // speed optimization
	{
		PYXMath::rotateDelta(&m_nRot, m_nDir - nDir);
	}
	checkInvariant();
}

//! Decrement, by one, the resolution of the cell the cursor is on.
void PYXCursor::zoomOut()
{
	m_index.decrementResolution();
	resolveDirection();
	checkInvariant();
}

//! Increment, by one, the resolution of the cell the cursor is on.
void PYXCursor::zoomIn()
{
	m_index.incrementResolution();
	//resolveDirection(); MLEPAGE cursor zoom in should never change direction
	checkInvariant();
}

//! Resolves the direction.
void PYXCursor::resolveDirection()
{
	if (m_index.isPentagon())
	{
		int nGap = m_index.getPrimaryResolution() <= 6 ? 1 : 4;
		int nTest = m_nDir - nGap;
		int nD = 1;
		if (PYXMath::getHexClass(m_index.getResolution()) == PYXMath::knClassII)
		{
			nD = -1;
			(nTest += 2) %= 6; // tricky math
		}

		if (0 <= nTest && nTest <= 2)
		{
			m_nDir = PYXIcosMath::rotateDirection(m_index, m_nDir, nD);
			PYXMath::rotateDelta(&m_nRot, nD);
		}
	}
}

//! Get the current backward direction.
PYXMath::eHexDirection PYXCursor::getBackDir() const
{
	PYXMath::eHexDirection nDir = PYXMath::negateDir(m_nDir);

#if 1
	if (m_index.isPentagon())
	{
		int nGap = m_index.getPrimaryResolution() <= 6 ? 1 : 4;
		int nTest = nDir - nGap;
		int nD = 1;
		if (PYXMath::getHexClass(m_index.getResolution()) == PYXMath::knClassII)
		{
			nD = -1;
			(nTest += 2) %= 6; // tricky math
		}

		if (0 <= nTest && nTest <= 2)
		{
			nDir = PYXMath::rotateDirection(nDir, nD);
		}
	}
#else
	if (!PYXIcosMath::isValidDirection(m_index, nDir))
	{
		int nD = PYXMath::getHexClass(m_index.getResolution()) == PYXMath::knClassI ? 1 : -1;
		nDir = PYXIcosMath::rotateDirection(m_index, nDir, nD);
	}
#endif

	return nDir;
}

//! Reset the current state.
void PYXCursor::reset(const PYXIcosIndex& index, PYXMath::eHexDirection nDir)
{
	m_index = index;
	m_nDir = nDir;
	if (!PYXIcosMath::isValidDirection(m_index, m_nDir))
	{
		resolveDirection();
	}
	setInitialDir();
	checkInvariant();
}

//! Set the current index the cursor is on.
void PYXCursor::setIndex(const PYXIcosIndex& index)
{
	m_index = index;
	if (!PYXIcosMath::isValidDirection(m_index, m_nDir))
	{
		resolveDirection();
	}
	setInitialDir();
	checkInvariant();
}

//! Set the direction the cursor is moving in.
void PYXCursor::setDir(PYXMath::eHexDirection nDir)
{
	m_nDir = nDir;
	if (!PYXIcosMath::isValidDirection(m_index, m_nDir))
	{
		resolveDirection();
	}
	setInitialDir();
	checkInvariant();
}

//! Returns a string representation.
std::string PYXCursor::toString() const
{
	std::ostringstream out;
	out << *this;
	return out.str();
}

std::ostream& operator <<(std::ostream& out, const PYXCursor& cursor)
{
	return out << '{' << cursor.getIndex().toString() << ' ' << cursor.getDir() << ' ' << cursor.getRotation() << '}';
}
