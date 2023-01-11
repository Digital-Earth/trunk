/******************************************************************************
bit_utils.cpp

begin		: 2006-08-07
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h" 
#include "pyxis/utility/bit_utils.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <cassert>

//! The unit test class
Tester<BitUtils> gTester;

//! Test method
void BitUtils::test()
{
	// TO DO
	TEST_ASSERT(1);
}

unsigned int BitUtils::getSetBitCount(unsigned int nBits)
{
	unsigned int nCount = 0;
	for (; 0 != nBits; ++nCount)
	{
		nBits = clearLowestSetBit(nBits);
	}
	return nCount;
}

unsigned int BitUtils::getLowerSetBitCount(unsigned int nBits, unsigned int nOffset)
{
	unsigned int nLowerSetBitCount = 0;
	for (; nOffset > 0; nBits >>= 1, --nOffset)
	{
		assert(nBits != 0);
		if (0 != (nBits & 1))
		{
			++nLowerSetBitCount;
		}
	}
	return nLowerSetBitCount;
}

unsigned int BitUtils::getSetBitOffset(unsigned int nBits, unsigned int nLowerSetBitCount)
{
	for (unsigned int nOffset = 0; 0 < nBits; nBits >>= 1, ++nOffset)
	{
		if (0 != (nBits & 1))
		{
			if (0 == nLowerSetBitCount)
			{
				return nOffset;
			}
			--nLowerSetBitCount;
		}
	}

	// Not found.
	return knUsedBitCountPerByte;
}

unsigned int BitUtils::clearLowestSetBit(const unsigned int nBits)
{
	return (nBits & (nBits - 1));
}

bool BitUtils::isSignBitSet(const unsigned int nBits)
{
	return 0 != (knSignBitMask & nBits);
}

unsigned int BitUtils::clearSignBit(const unsigned int nBits)
{
	return nBits & ~knSignBitMask;
}


bool BitUtils::isOffsetBitSet(const unsigned char nBits, const unsigned char nOffset)
{
	return (0 != ((nBits >> nOffset) & 1));
}

bool BitUtils::isOffsetBitSet(const unsigned int nBits, const unsigned int nOffset)
{
	return (0 != ((nBits >> nOffset) & 1));
}

unsigned int BitUtils::setOffsetBit(const unsigned int nBits, const unsigned int nOffset)
{
	return nBits | (1U << nOffset);
}

unsigned int BitUtils::clearOffsetBit(const unsigned int nBits, const unsigned int nOffset)
{
	return nBits & ~(1U << nOffset);
}
