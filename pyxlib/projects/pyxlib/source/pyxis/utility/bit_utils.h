#ifndef PYXIS__UTILITY__BIT_UTILS_H
#define PYXIS__UTILITY__BIT_UTILS_H
/******************************************************************************
bit_utils.h

begin		: 2007-08-07
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

// standard includes
#include <climits>

/*!
A class containing static bit manipulation convenience functions.
Each function takes 'int' for efficiency, although only the low byte is used.
*/
struct PYXLIB_DECL BitUtils
{
	static const unsigned int knBit1 = 0x01;
	static const unsigned int knBit2 = 0x02;
	static const unsigned int knBit3 = 0x04;
	static const unsigned int knBit4 = 0x08;
	static const unsigned int knBit5 = 0x10;
	static const unsigned int knBit6 = 0x20;
	static const unsigned int knBit7 = 0x40;
	static const unsigned int knBit8 = 0x80;

	//! Test method.
	static void test();

	//! The offset of the sign bit.
	static const unsigned int knSignBitOffset = (CHAR_BIT - 1);

	//! The mask for the sign bit.
	static const unsigned int knSignBitMask = (1U << knSignBitOffset);

	//! The number of bits that can be portably guaranteed to be in a byte.
	static const unsigned int knUsedBitCountPerByte = 8;

	//! Return the number of 1 bits in the byte.
	static unsigned int getSetBitCount(unsigned int nBits);

	//! Return the number of 1 bits prior to offset 'nOffset' in the byte.
	static unsigned int getLowerSetBitCount(unsigned int nBits, unsigned int nOffset);

	//! Return the offset of the next higher 1 bit, or 'knUsedBitCountPerByte' if none.
	static unsigned int getSetBitOffset(unsigned int nBits, unsigned int nLowerSetBitCount);

	//! Change the lowest 1 bit to 0.
	static unsigned int clearLowestSetBit(const unsigned int nBits);

	//! Return true if the sign bit is 1.
	static bool isSignBitSet(const unsigned int nBits);

	//! Set the sign bit to 0.
	static unsigned int clearSignBit(const unsigned int nBits);

	//! Return true if the bit at the offset is 1.
	static bool isOffsetBitSet(const unsigned int nBits, const unsigned int nOffset);

	//! Return true if the bit at the offset is 1.
	static bool isOffsetBitSet(const unsigned char nBits, const unsigned char nOffset);

	//! Set the bit at the offset to 1.
	static unsigned int setOffsetBit(const unsigned int nBits, const unsigned int nOffset);

	//! Set the bit at the offset to 0.
	static unsigned int clearOffsetBit(const unsigned int nBits, const unsigned int nOffset);
};

#endif // guard
