/******************************************************************************
pyx_index.cpp

begin		: 2003-12-03
copyright	: (C) 2003, 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/sub_index.h"

// pyxlib includes
#include "pyxis/derm/exceptions.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <algorithm>
#include <cassert>
#include <cstdlib>

//! Bit mask for the digit
const unsigned int PYXIndex::knDigitMask = 0xF;

//! Number of bits to shift for a digit
const int PYXIndex::knBitsPerDigit = 4;

//! Tester class
Tester<PYXIndex> gTester;

//! Test method
void PYXIndex::test()
{					
	const unsigned int knTestValue = 1020304;
	const std::string kstrTestValueDigits("1020304");
	const std::string kstrTestDigits("10203040506010");									  
	const std::string kstrNegatedTestDigits("40506010203040");
											 
	// test that constructor creates null index
	PYXIndex index1;
	TEST_ASSERT(index1.isNull());

	// test copy constructor
	PYXIndex index2(index1);
	TEST_ASSERT(index1 == index2);
	TEST_ASSERT(index2.isNull());

	// test construction from string
	PYXIndex index3;
	index3 = kstrTestDigits;
	TEST_ASSERT(index3.toString() == kstrTestDigits);

	// test copy assignment
	index2 = index3;
	TEST_ASSERT(index2 == index3);
	TEST_ASSERT(index2.toString() == kstrTestDigits);

	// test resolution
	int nResolution = index3.getResolution();
	TEST_ASSERT(kstrTestDigits.length() == (nResolution + 1));

	nResolution -= 2;
	const std::string kstrNewDigits(kstrTestDigits.substr(0, kstrTestDigits.length() - 2));
	index3.setResolution(nResolution);
	TEST_ASSERT(index3.getResolution() == nResolution);
	TEST_ASSERT(kstrNewDigits == index3.toString());

	// test equality and less than operators
	index1.reset();
	index2.reset();
	TEST_ASSERT(index1 == index2);
	TEST_ASSERT(!(index1 < index2));

	index1 = "1";
	TEST_ASSERT(index1 != index2);
	TEST_ASSERT(!(index1 < index2));

	index2 = "1";
	TEST_ASSERT(index1 == index2);
	TEST_ASSERT(!(index1 < index2));

	index2 = "2";
	TEST_ASSERT(index1 != index2);
	TEST_ASSERT(index1 < index2);

	// test reset()
	index3.reset();
	TEST_ASSERT(index3.isNull());
	TEST_ASSERT(0 > index3.getResolution());

	// test set()
	index1.set(knTestValue);
	TEST_ASSERT(kstrTestValueDigits == index1.toString());

	// test setDigit()
	index1.setDigit(2, 6);
	TEST_ASSERT(index1.getDigit(2) == 6);

	// test isAtOrigin()
	index1.reset();
	TEST_ASSERT(!index1.isAtOrigin());
	index1 = kstrTestDigits;
	TEST_ASSERT(!index1.isAtOrigin());
	index1 = "000000000000";
	TEST_ASSERT(index1.isAtOrigin());

	// test isAtOrigin(unsigned int)
	TEST_ASSERT(!index1.isAtOrigin(0));
	TEST_ASSERT(index1.isAtOrigin(1));
	TEST_ASSERT(index1.isAtOrigin(index1.getDigitCount() + 1));

	// test isPYXDigit()
	TEST_ASSERT(!isPYXDigit(-1));
	TEST_ASSERT(!isPYXDigit(8));
	TEST_ASSERT(!isPYXDigit('A'));
	for (unsigned int nIndex = 0; nIndex <= 6; ++nIndex)
	{
		TEST_ASSERT(isPYXDigit(nIndex));
	}

	// test appendDigit()
	index1 = "40";
	index1.appendDigit(1);
	index1.appendDigit(0);
	TEST_ASSERT("4010" == index1.toString());

	// test appendPair()
	index1.reset();
	index1.appendPair(0x10);
	index1.appendPair(0x40);
	TEST_ASSERT("1040" == index1.toString());

	// test append()
	index1 = "01";
	index1.append(PYXIndex("020304"));
	TEST_ASSERT("01020304" == index1.toString());

	// test prependDigit()
	index1 = "01";
	index1.prependDigit(0);
	index1.prependDigit(4);
	TEST_ASSERT("4001" == index1.toString());

	// test prependPair()
	index1.reset();
	index1.prependPair(0x10);
	index1.prependPair(0x40);
	TEST_ASSERT("4010" == index1.toString());

	// test prepend()
	index1 = "04";
	index1.prepend(PYXIndex("010203"));
	TEST_ASSERT("01020304" == index1.toString());

	// test negate()
	index1 = kstrTestDigits;
	index1.negate();
	TEST_ASSERT(kstrNegatedTestDigits == index1.toString());

	// test stripRight()
	index1 = kstrTestDigits;
	unsigned int nDigit = index1.stripRight();
	TEST_ASSERT(kstrTestDigits[kstrTestDigits.length() - 1] == nDigit + '0');
	TEST_ASSERT(kstrTestDigits.substr(0, kstrTestDigits.length() - 1) == index1.toString());

	// test stripRightPair()
	index1 = kstrTestDigits;
	unsigned int nPair = index1.stripRightPair();
	TEST_ASSERT(kstrTestDigits[kstrTestDigits.length() - 1] == (nPair & knDigitMask) + '0');
	TEST_ASSERT(kstrTestDigits[kstrTestDigits.length() - 2] == ((nPair >> knBitsPerDigit) & knDigitMask) + '0');
	TEST_ASSERT(kstrTestDigits.substr(0, kstrTestDigits.length() - 2) == index1.toString());

	// test stripLeft()
	index1 = kstrTestDigits;
	index1.stripLeft();
	TEST_ASSERT(kstrTestDigits.substr(1, kstrTestDigits.length()) == index1.toString());

	// test mostSignificant()
	index1 = "000103";
	int nPosition;
	TEST_ASSERT(1 == index1.mostSignificant(&nPosition));
	TEST_ASSERT(2 == nPosition);

	// test isCentroidChild()
	index1 = PYXIndex();
	TEST_ASSERT(!index1.hasVertexChildren());

	index1 = "01";
	TEST_ASSERT(!index1.hasVertexChildren());

	index1 = "10";
	TEST_ASSERT(index1.hasVertexChildren());

	// test adjustResolutionLeft()
	index1 = "10203";
	index1.adjustResolutionLeft(index1.getResolution() + 2);
	TEST_ASSERT(index1.getResolution() == 6);
	TEST_ASSERT("0010203" == index1.toString());

	index1.adjustResolutionLeft(index1.getResolution() - 4);
	TEST_ASSERT(index1.getResolution() == 4);
	TEST_ASSERT("10203" == index1.toString());

	index1 = "00010203";
	index1.adjustResolutionLeft(5);
	TEST_ASSERT(index1.getResolution() == 5);
	TEST_ASSERT("010203" == index1.toString());

	// test add and return operators, don't grow
	index1 = "50601";
	index2 = "10203";
	TEST_ASSERT(index1 + index2 == PYXIndex("60102"));
	index1 += index2;
    TEST_ASSERT(index1 == PYXIndex("60102"));

    // test add and return operators, grow case
	index1 = "402";
	index2 = "403";
	TEST_ASSERT(index1 + index2 == PYXIndex("3040"));
	index1 += index2;
    TEST_ASSERT(index1 == PYXIndex("3040"));

	// test substract and return operators, don't grow
	index1 = "404";
	index2 = "403";
	TEST_ASSERT(index1 - index2 == PYXIndex("005"));
	index1 -= index2;
    TEST_ASSERT(index1 == PYXIndex("005"));

	// test randomization
	for (int nResolution = 0; nResolution <= PYXMath::knMaxAbsResolution; ++nResolution)
	{
		PYXIndex index;
		index.randomize(nResolution);
		TEST_ASSERT(index.getResolution() == nResolution);
	}

	// test subseq
	for (int n = 0; n != 10; ++n)
	{
		PYXIndex index;
		index.randomize(rand() % 10);
		std::string str = index.toString();
		for (int nPos = 0; nPos != index.getDigitCount(); ++nPos)
		{
			TEST_ASSERT(index.subseq(nPos).toString() == str.substr(nPos));
			for (int nLen = 0; nLen != index.getDigitCount() - nPos; ++nLen)
			{
				TEST_ASSERT(index.subseq(nPos, nLen).toString() == str.substr(nPos, nLen));
			}
		}
	}
}

/*!
Constructor zeroes out the index and sets the resolution to zero
*/
PYXIndex::PYXIndex()
{
	reset();
}

/*!
Copy constructor.

\param	rhs	The PYXIS index to copy.
*/
PYXIndex::PYXIndex(const PYXIndex& rhs)
{
	// call copy assignment operator
	*this = rhs;
}


/*!
Construct the index from a string.

\param strIndex	The string.
*/
PYXIndex::PYXIndex(const std::string& strIndex)
{
	// call string assignment operator
	*this = strIndex;
}

/*!
Destructor does nothing
*/
PYXIndex::~PYXIndex()
{
}

/*!
Copy assignment.

\param	rhs	The PYXIS index to copy.
*/
PYXIndex& PYXIndex::operator =(const PYXIndex &rhs)
{
	m_nDigitCount = rhs.m_nDigitCount;
	memcpy(m_pcDigits, rhs.m_pcDigits, m_nDigitCount + 1);

	return *this;
}

/*!
String assignment. The string must consist of digits in the range {0...7}
and the number of digits must be less than or equal to the maximum allowed.

\param	strIndex	The index.
*/
PYXIndex& PYXIndex::operator =(const std::string& strIndex)
{
	// check for position in range
	if (sizeof(m_pcDigits) <= strIndex.length())
	{
		PYXTHROW(	PYXIndexException,
					"Invalid digit position: '" << static_cast<int>(strIndex.length()) << "'."	);
	}

	// check for valid characters
	bool bLastIsZero = true;
	std::string::const_iterator it = strIndex.begin();
	for (; it != strIndex.end(); ++it)
	{
		// verify the digit is in range
		if (!isPYXDigit(*it - '0'))
		{
			PYXTHROW(	PYXIndexException,
						"Invalid digit '" << *it <<
						"' in string '" << strIndex << "'."	);
		}

		// verify 2 non-zeros are never consecutive
		if ('0' == *it)
		{
			bLastIsZero = true;
		}
		else
		{
			if (!bLastIsZero)
			{
				PYXTHROW(	PYXIndexException,
							"Invalid consecutive non-zero digits in index: '" << strIndex << "'."	);
			}
			bLastIsZero = false;
		}
	}

	// copy the string
	m_nDigitCount = static_cast<int>(strIndex.length());
	memcpy(m_pcDigits, strIndex.c_str(), m_nDigitCount + 1);

	return *this;
}

/*!
Equality operator. Two indices are equal if their digits are the same.
\return	true if the indices are equal, otherwise false.
*/
bool PYXIndex::operator ==(const PYXIndex& rhs) const
{
	bool bEqual = false;

	if (m_nDigitCount == rhs.m_nDigitCount)
	{
		if (0 == m_nDigitCount)
		{
			bEqual = true;
		}
		else
		{
			bEqual = (0 == memcmp(m_pcDigits, rhs.m_pcDigits, m_nDigitCount));
		}
	}

	return bEqual;
}

/*!
Less than operator. Returns true if this index occurs before the specified
index in an alphabetical sort.

\param	rhs	The index to compare.

\return true if this index is less than the index passed in.
*/
bool PYXIndex::operator <(const PYXIndex& rhs) const
{
	int nMinDigits = std::min(m_nDigitCount, rhs.m_nDigitCount);

	int nResult = memcmp(m_pcDigits, rhs.m_pcDigits, nMinDigits);
	if (nResult < 0)
	{
		return true;
	}
	else if (nResult > 0)
	{
		return false;
	}

	// else first part of indices are the same, look at resolution
	return (m_nDigitCount < rhs.m_nDigitCount);
}

/*!
Add and return operator. Returns the sum after adding another index.

Will use the max resolution of both operands, and grow the resolution if necessary.

\param	rhs	The index to be added.

\return The sum index after add.
*/
PYXIndex& PYXIndex::operator +=(const PYXIndex& rhs)
{
	PYXMath::add(*this, rhs, std::max(getResolution(), rhs.getResolution()), this, true);

	return *this;
}

/*!
Subtract and return operator. Returns the difference after subtracting another index.

Will use the resolution of the index where the subtraction is performed.

\param	rhs	The subtrahend.

\return The difference after subtract.
*/
PYXIndex& PYXIndex::operator -=(const PYXIndex& rhs)
{
	PYXMath::subtract(*this, rhs, getResolution(), this);

	return *this;
}

/*!
Reset the PYXIndex to the null index.
*/
void PYXIndex::reset()
{
	m_nDigitCount = 0;
	m_pcDigits[0] = 0;
}

/*!
Set the number of digits in the PYXIS index. Unused digits are set to zero.

\param nDigitCount	The new number of digits
*/
void PYXIndex::setDigitCount(int nDigitCount)
{
	// check for position in range
	if ((0 > nDigitCount) || (sizeof(m_pcDigits) <= nDigitCount))
	{
		PYXTHROW(	PYXIndexException,
					"Invalid digit position: '" << nDigitCount << "'."	);
	}

	if (m_nDigitCount < nDigitCount)
	{
		// number zero
		memset(&(m_pcDigits[m_nDigitCount]), '0', nDigitCount - m_nDigitCount);
	}

	// null terminator
	m_nDigitCount = nDigitCount;
	m_pcDigits[m_nDigitCount] = 0;
}

/*!
Set the value of the index.

\param	nValue	The value.
\param	nBase	Base in which value is defined (default = 10)
*/
void PYXIndex::set(unsigned int nValue, int nBase)
{
	_itoa_s(nValue, m_pcDigits, nBase);
	m_nDigitCount = static_cast<int>(strlen(m_pcDigits));
}

/*!
Get the digit at a given position. Digits are numbered starting with zero at
the most significant digit.

\param	nPosition	The position.

\return	The digit at the position.
*/
unsigned int PYXIndex::getDigit(int nPosition) const
{
	// check for position in range
	if ((0 > nPosition) || (m_nDigitCount <= nPosition))
	{
		PYXTHROW(	PYXIndexException,
					"Invalid digit position: '" << nPosition << "'."	);
	}

	return (m_pcDigits[nPosition] - '0');
}

/*!
Get the digit at a the end of the index.

\return	The digit at the position.
*/

unsigned int PYXIndex::getLastDigit() const
{
	// check for position in range
	if (m_nDigitCount==0)
	{
		PYXTHROW(PYXIndexException, "Index have no digits");
	}
	return (m_pcDigits[m_nDigitCount-1] - '0');
}


/*!
Get a subsequence of an index. If the index is too short, the remaining digits are used.

\param	nPos	The position at which to begin the subsequence (must be 0 to digit count).
\param	nLen	The length of the subsequence (must be non-negative).
\return The subsequence as a PYXIndex.
*/
PYXIndex PYXIndex::subseq(int nPos, int nLen) const
{
	assert(0 <= nPos && nPos <= m_nDigitCount && 0 <= nLen);

	PYXIndex r;

	char* pDst = r.m_pcDigits;
	const char* pSrc = m_pcDigits + nPos;
	const char* pEnd = pSrc + ((nLen != INT_MAX && nPos + nLen <= m_nDigitCount) ? nLen : m_nDigitCount - nPos);

	while (pSrc != pEnd)
	{
		*pDst++ = *pSrc++;
	}

	*pDst = 0; // terminate
	r.m_nDigitCount = static_cast<int>(pDst - r.m_pcDigits);

	return r;
}

/*!
Set the digit at a given position. Digits are numbered starting with zero at
the most significant digit.

\param	nPosition	The position.
\param	nDigit		The digit.
*/
void PYXIndex::setDigit(int nPosition, unsigned int nDigit)
{
	// check for position in range
	if ((0 > nPosition) || (m_nDigitCount <= nPosition))
	{
		PYXTHROW(	PYXIndexException,
					"Invalid digit position: '" << nPosition << "'."	);
	}

	// check for valid digit value
	if (!isPYXDigit(nDigit))
	{
		PYXTHROW(	PYXIndexException,
					"Invalid digit: '" << nDigit << "'."	);
	}

	m_pcDigits[nPosition] = nDigit + '0';
}

/*!
Determine if this hexagon is centred on the origin.

\return	true if the hexagon is centred on the origin, otherwise false.
*/
bool PYXIndex::isAtOrigin() const
{
	bool bIsAtOrigin;
	
	if (isNull())
	{
		bIsAtOrigin = false;
	}
	else
	{
		bIsAtOrigin = (atoi(m_pcDigits) == 0);
	}

	return bIsAtOrigin;
}

/*!
Determine if this hexagon is centred on the origin up to a particular
digit count.

\param nDigitCount	The number of digits to check to see if the index is 
					at the origin.

\return	true if the hexagon is centred on the origin up to the specified
		resolution, otherwise false.
*/
bool PYXIndex::isAtOrigin(unsigned int nDigitCount) const
{
	assert(0 <= getDigitCount());

	// Adjust digit count if past end.
	const unsigned int nActualDigitCount = getDigitCount();
	if (nDigitCount > nActualDigitCount)
	{
		nDigitCount = nActualDigitCount;
	}

	if (isNull() || nDigitCount == 0)
	{
		return false;
	}

	// Iterate through digits, up to and including offset,
	// and return true if they are all zero.
	while (nDigitCount > 0)
	{
		if (m_pcDigits[--nDigitCount] != '0')
		{
			return false;
		}
	}

	return true;
}

/*!
Determine if this PYXIS index is a centroid child of its parent.

\return	true if this index is its parent's centroid child otherwise false.
*/
bool PYXIndex::hasVertexChildren() const
{
	bool bIsCentroidChild = false;

	if (0 < m_nDigitCount)
	{
		bIsCentroidChild = ('0' == m_pcDigits[m_nDigitCount - 1]);
	}

	return bIsCentroidChild;
}

/*!
Determine if the index is an ancestor of the specified index. A is an ancestor
of B if A and B are the same index or if B is generated by a tesselation of A
at some given resolution. If either A or B or both are the null index then
there is no ancestor relationship.

\param	index	The specified index.

\return	true if this index is an ancestor of the specified index, otherwise
		false.
*/
bool PYXIndex::isAncestorOf(const PYXIndex& index) const
{
	bool bIsAncestor = false;

	// the null index is a parent of everything
	if (!isNull() && !index.isNull())
	{
		if (m_nDigitCount <= index.m_nDigitCount)
		{
			bIsAncestor = true;
			for (int nPos = 0; nPos < m_nDigitCount; ++nPos)
			{
				if (m_pcDigits[nPos] != index.m_pcDigits[nPos])
				{
					bIsAncestor = false;
					break;
				}
			}
		}
	}

	return bIsAncestor;
}

/*!
Determine if this index is a descendant of the specified index. A is a
descendant of B if B is an ancestor of A.

\param	index	The specified index.

\return	true if this index is a descendant of the specified index, otherwise
		false.
*/
bool PYXIndex::isDescendantOf(const PYXIndex& index) const
{
	// call the descenant method with the parameters switched
	return index.isAncestorOf(*this);
}

/*!
Append a single digit to the right hand side of an index.

\param nDigit	The digit.
*/
void PYXIndex::appendDigit(unsigned int nDigit)
{
	if (!isPYXDigit(nDigit))
	{
		PYXTHROW(	PYXIndexException,
					"Invalid digit: '" << nDigit << "'."	);
	}

	// check for position in range
	if (sizeof(m_pcDigits) <= m_nDigitCount)
	{
		PYXTHROW(	PYXIndexException,
					"Invalid digit position: '" << m_nDigitCount << "'."	);
	}

	m_pcDigits[m_nDigitCount++] = nDigit + '0';
	m_pcDigits[m_nDigitCount] = 0;
}

/*!
Append a pair of digits to the right hand side of the index.

\param	nHexDigits	The digits as a hex number.
*/
void PYXIndex::appendPair(unsigned int nHexDigits)
{
	// check for a valid digit
	unsigned int nDigit1 = nHexDigits & knDigitMask;
	if (!isPYXDigit(nDigit1))
	{
		PYXTHROW(	PYXIndexException,
					"Invalid digit: '" << nDigit1 << "'."	);
	}

	// check for a valid digit
	nHexDigits >>= knBitsPerDigit;
	unsigned int nDigit2 = nHexDigits & knDigitMask;
	if (!isPYXDigit(nDigit2))
	{
		PYXTHROW(	PYXIndexException,
					"Invalid digit: '" << nDigit2 << "'."	);
	}

	m_pcDigits[m_nDigitCount++] = nDigit2 + '0';
	m_pcDigits[m_nDigitCount++] = nDigit1 + '0';
	m_pcDigits[m_nDigitCount] = 0;
}

/*!
Append a PYXIndex to the right side of an index.

\param index	The index.
*/
void PYXIndex::append(const PYXIndex& index)
{
	if (m_nDigitCount + index.m_nDigitCount >= sizeof(m_pcDigits))
	{
		int nResolution = getResolution() + index.getResolution();
		PYXTHROW(	PYXIndexException,
					"Invalid resolution: '" << nResolution << "'."	);
	}

	memcpy(m_pcDigits + m_nDigitCount, index.m_pcDigits, index.m_nDigitCount + 1);
	m_nDigitCount += index.m_nDigitCount;
}

/*!
Prepend a single digit to the left hand side of the index.

\param	nDigit	The digit.
*/
void PYXIndex::prependDigit(unsigned int nDigit)
{
	// check for a valid digit
	if (!isPYXDigit(nDigit))
	{
		PYXTHROW(	PYXIndexException,
					"Invalid digit: '" << nDigit << "'."	);
	}

	// check for position in range
	if (sizeof(m_pcDigits) <= m_nDigitCount)
	{
		PYXTHROW(	PYXIndexException,
					"Invalid digit position: '" << m_nDigitCount << "'."	);
	}

	memmove(&(m_pcDigits[1]), m_pcDigits, m_nDigitCount + 1);
	m_nDigitCount++;

	m_pcDigits[0] = nDigit + '0';
}

/*!
Prepend a pair of digits to the left hand side of the index.

\param	nHexDigits	The digits as a hex number.
*/
void PYXIndex::prependPair(unsigned int nHexDigits)
{
	// check for a valid digit
	unsigned int nDigit1 = nHexDigits & knDigitMask;
	if (!isPYXDigit(nDigit1))
	{
		PYXTHROW(	PYXIndexException,
					"Invalid digit: '" << nDigit1 << "'."	);
	}

	// check for a valid digit
	nHexDigits >>= knBitsPerDigit;
	unsigned int nDigit2 = nHexDigits & knDigitMask;
	if (!isPYXDigit(nDigit2))
	{
		PYXTHROW(	PYXIndexException,
					"Invalid digit: '" << nDigit2 << "'."	);
	}

	memmove(&(m_pcDigits[2]), m_pcDigits, m_nDigitCount + 1);
	m_nDigitCount += 2;

	m_pcDigits[0] = nDigit2 + '0';
	m_pcDigits[1] = nDigit1 + '0';
}

/*!
Prepend a PYXIndex to the left side of an index. Note that this method is more
expensive than append.

\sa append()

\param index	The index to add.
*/
void PYXIndex::prepend(const PYXIndex& index)
{
	if (m_nDigitCount + index.m_nDigitCount >= sizeof(m_pcDigits))
	{
		int nResolution = getResolution() + index.getResolution();
		PYXTHROW(	PYXIndexException,
					"Invalid resolution: '" << nResolution << "'."	);
	}

	memmove(m_pcDigits + index.m_nDigitCount, m_pcDigits, m_nDigitCount + 1);
	memcpy(m_pcDigits, index.m_pcDigits, index.m_nDigitCount);
	m_nDigitCount += index.m_nDigitCount;
}

/*!
Negate a PYXIS index.
*/
void PYXIndex::negate()
{
	static char pcNegate[7] = { '0', '4', '5', '6', '1', '2', '3' };

	/*
	Negate the PYXIS index by negating each digit according to the preceeding
	array.
	*/
	char* ptr = m_pcDigits;
	char* ptrEnd = ptr + m_nDigitCount;
	for (; ptr < ptrEnd; ++ptr)
	{
		if ('0' != *ptr)
		{
			*ptr = pcNegate[*ptr - '0'];
		}
	}
}

/*!
Strip the least significant digit from the index.

\return	The least significant digit.
*/
unsigned int PYXIndex::stripRight()
{
	unsigned int nDigit = 0;

	if (0 < m_nDigitCount)
	{
		m_nDigitCount--;
		nDigit = m_pcDigits[m_nDigitCount] - '0';
		m_pcDigits[m_nDigitCount] = 0;
	}

	return nDigit;
}

/*!
Strip the two least significant digits from the index.

\return	The two least significant digits as a hex number.
*/
unsigned int PYXIndex::stripRightPair()
{
	unsigned int nDigits = 0;

	if (0 < m_nDigitCount)
	{
		m_nDigitCount--;
		nDigits = m_pcDigits[m_nDigitCount] - '0';
		m_pcDigits[m_nDigitCount] = 0;
	}

	if (0 < m_nDigitCount)
	{
		m_nDigitCount--;
		nDigits += ((m_pcDigits[m_nDigitCount] - '0') << 4);
		m_pcDigits[m_nDigitCount] = 0;
	}

	return nDigits;
}

/*!
Strip the most significant digit from the index.

\return	The most significant digit.
*/
unsigned int PYXIndex::stripLeft()
{
	unsigned int nDigit = 0;

	if (0 < m_nDigitCount)
	{
		nDigit = m_pcDigits[0] - '0';

		memmove(m_pcDigits, &(m_pcDigits[1]), m_nDigitCount);
		m_nDigitCount--;
	}

	return nDigit;
}


unsigned int PYXIndex::stripLeftPair()
{
	int leftPair = 0; 
	std::stringstream ss; 
	ss << stripLeft();
	ss << stripLeft(); 
	ss >> leftPair; 

	return leftPair; 
}

/*!
Strip the specified number of most significant digits from the index.

\param nCount	The number of digits to strip from the index
*/
void PYXIndex::stripLeftCount(int nCount)
{
	if (0 <= nCount && nCount < m_nDigitCount)
	{
		m_nDigitCount -= nCount;
		memmove(m_pcDigits, m_pcDigits + nCount, m_nDigitCount);
		m_pcDigits[m_nDigitCount] = 0;
	}
}

/*!
Get the most significant non-zero digit in the index and the position of that
digit where zero is the position of the least significant digit. Note that this
is a different numbering scheme than the one used for getDigit() and setDigit().

\sa getDigit()
\sa setDigit()

\param	pnPosition	The position of the digit or -1 if at the origin (out)

\return	The digit.
*/
unsigned int PYXIndex::mostSignificant(int* pnPosition) const
{
	unsigned int nDigit = 0;

	int nPosition = m_nDigitCount - 1;
	int nIndex = 0;
	for (; nIndex < m_nDigitCount; ++nIndex)
	{
		if ('0' != m_pcDigits[nIndex])
		{
			break;
		}

		nPosition--;
	}

	if (0 != pnPosition)
	{
		*pnPosition = nPosition;
	}

	if (nPosition != -1)
	{
		nDigit = m_pcDigits[nIndex] - '0';
	}

	return nDigit;
}

/*!
This method attempts to make the index match the target resolution by adding or
removing digits from the left side.

If the index resolution is less than the target resolution, zeros are added to
the front of the index until the target resolution is reached.

If the index resolution is greater than the target resolution, zeros are
removed until the first non-zero digit is detected or the target resolution is
reached.

\param	nTargetResolution	The target resolution
*/
void PYXIndex::adjustResolutionLeft(int nTargetResolution)
{
	if ((nTargetResolution < 0) || (nTargetResolution >= sizeof(m_pcDigits)))
	{
		PYXTHROW(	PYXIndexException,
					"Invalid resolution: '" << nTargetResolution << "'."	);
	}

	int nResolution = getResolution();
	if (nResolution < nTargetResolution)
	{
		// add zeros to left
		int nCount = nTargetResolution - nResolution;
		memmove(m_pcDigits + nCount, m_pcDigits, m_nDigitCount + 1);
		m_nDigitCount += nCount;

		char* ptr = m_pcDigits;
		for (; nCount > 0; --nCount)
		{
			*ptr = '0';
			ptr++;
		}

	}
	else if (nResolution > nTargetResolution)
	{
		// remove zeros from left
		char* ptr = m_pcDigits;
		while (nResolution > nTargetResolution)
		{
			if (*ptr != '0')
			{
				break;
			}
			
			ptr++;
			--nResolution;
		}

		int nCount = static_cast<int>(ptr - m_pcDigits);
		if (nCount > 0)
		{
			memmove(m_pcDigits, m_pcDigits + nCount, m_nDigitCount - nCount + 1);
			m_nDigitCount -= nCount;
		}		
	}
}

/*!
Split a PYXindex into two parts at the designated point.

\param nCount	The number of digits counting from the front to split off

return An index comprised of the first nCount digits. Null index on error
*/
PYXIndex PYXIndex::split(int nCount)
{
	PYXIndex first;

	if (nCount < getResolution())
	{
		while (0 < nCount)
		{
			first.appendDigit(stripLeft());
			nCount--;
		}
	}
	
	return first;
}


/*!
Split this index into two indices where the "00" digit pattern is found.  The
method will start at the right side of the index and move left until the "00"
patern is found or the beginning of the string is encountered.  The least 
significant portion of the index will be assigned to the passed parameter and
the more significant portion will remain with this index.

\param pTail	Pointer to the index of least significant digits
*/
void PYXIndex::split(PYXIndex* pTail)
{
	//see if the "00" sequence exists in the string
	const char* pszSubString_c = 0;
	const char* pszSubString_b = 0;
	
	char* pszSubString = strstr(&m_pcDigits[0],"00");
	while (0 != pszSubString)
	{
		pszSubString_c = pszSubString;
		pszSubString ++;
		pszSubString ++;
		pszSubString = strstr(pszSubString,"00");
	}

	if (0 == pszSubString_c)
	{
		*pTail = m_pcDigits;
	}
	else
	{
		*pTail = pszSubString_c;
	}

	int nRes = pTail->getResolution();

	for (int nIndex = 0; nIndex <= nRes; nIndex++)
	{
		stripRight();
	}
}

/*!
Randomizes this index at the specified resolution.

\param nResolution	The resolution.
*/
void PYXIndex::randomize(int nResolution)
{
	assert(0 <= nResolution && nResolution <= PYXMath::knMaxAbsResolution);

	for (int n = 0; n != nResolution + 1; ++n)
	{
		if (n == 0 || m_pcDigits[n - 1] == '0')
		{
			// Choose digits 0 through 6.
			// The probabilities here are subtle...
			int nRand = static_cast<int>(static_cast<double>(rand()) / RAND_MAX * 14);
			m_pcDigits[n] = '0' + ((nRand < 6) ? nRand + 1 : 0);
		}
		else
		{
			// Choose digit 0 only.
			m_pcDigits[n] = '0';
		}
	}
	m_nDigitCount = nResolution + 1;
	m_pcDigits[m_nDigitCount] = 0; // terminate
}

/*! 
Allows PYXIS indices to be written to streams.

\param out		The stream to write to.
\param pyxIndex	The index to write to the stream.

\return The stream after the operation.
*/
std::ostream& operator <<(std::ostream& out, const PYXIndex& pyxIndex)
{
	out << pyxIndex.toString();
	return out;
}

/*!
Allows PYXIS indices to be read from streams.

\param input	The stream to read from.
\param pyxIndex	The index to read from the stream.

\return The stream after the operation.
*/
std::istream& operator >>(std::istream& input, PYXIndex& pyxIndex)
{
	std::string strIndex;
	input >> strIndex;
	pyxIndex = strIndex;
	return input;
}

/*!
Addition operator.

\param lhs	The first index to add.
\param rhs	The second index to add.

\return The addition of two indexes.
*/
PYXIndex operator +(PYXIndex lhs, const PYXIndex& rhs)
{
	return lhs += rhs;
}

/*!
Subtraction operator.

\param lhs	The minuend
\param rhs	The subtrahend

\return The substration of two indexes.
*/
PYXIndex operator -(PYXIndex lhs, const PYXIndex& rhs)
{
	return lhs -= rhs;
}
