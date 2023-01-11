/******************************************************************************
pyx_stdint.cpp

begin		: 2006-02-24
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/stdint.h"

// local includes
#include "pyxis/utility/tester.h"

class StdInt
{
public:

	//! Test method
	static void test();
};

//! Tester class
Tester<StdInt> gTester;

//! Test method
void StdInt::test()
{
	TEST_ASSERT(sizeof(int8_t) == 1);
	TEST_ASSERT(sizeof(uint8_t) == 1);
	TEST_ASSERT(sizeof(int16_t) == 2);
	TEST_ASSERT(sizeof(uint16_t) == 2);
	TEST_ASSERT(sizeof(int32_t) == 4);
	TEST_ASSERT(sizeof(uint32_t) == 4);
#if 0
	TEST_ASSERT(sizeof(int64_t) == 8);
	TEST_ASSERT(sizeof(uint64_t) == 8);
#endif
}
