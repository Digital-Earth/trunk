/******************************************************************************
compact_index.cpp

begin		: 2007-05-25
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/compact_index.h"

// pyxlib includes
#include "pyxis/derm/exceptions.h"
#include "pyxis/derm/index.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <algorithm>
#include <cassert>
#include <cstdlib>

//! Tester class
Tester<CompactIndex> gTester;

//! Test method
void CompactIndex::test()
{					
	const std::string kstrTestDigits("1-60102030405060");									  
											 
	// test that constructor creates null index
	CompactIndex index1;
	TEST_ASSERT(index1.isNull());
	TEST_ASSERT(index1.getResolution() == -1);

	// test copy constructor
	CompactIndex index2(index1);
	TEST_ASSERT(index1 == index2);
	TEST_ASSERT(index2.isNull());
	TEST_ASSERT(index2.getResolution() == -1);

	// test construction from string
	CompactIndex index3;
	index3 = kstrTestDigits;
	TEST_ASSERT(index3.toString() == kstrTestDigits);
	TEST_ASSERT(index3.getResolution() == kstrTestDigits.length() - 1);

	// test construction from PYXIcosIndex
	PYXIcosIndex index4(kstrTestDigits);
	index3 = index4;
	TEST_ASSERT(index3.getResolution() == index4.getResolution());
	TEST_ASSERT(index3.toString() == kstrTestDigits);
	index4.reset();
	index4 = index3;
	TEST_ASSERT(index3.getResolution() == index4.getResolution());
	TEST_ASSERT(index4.toString() == kstrTestDigits);

	// test copy assignment
	index2 = index3;
	TEST_ASSERT(index2 == index3);
	TEST_ASSERT(index2.toString() == kstrTestDigits);
	TEST_ASSERT(index2.getResolution() == index3.getResolution());

	// test equality and other operators
	index1.reset();
	index2.reset();
	TEST_ASSERT(index1 == index2);
	TEST_ASSERT(!(index1 < index2));
	TEST_ASSERT(!(index1 > index2));
	TEST_ASSERT(index1 >= index2);
	TEST_ASSERT(index1 <= index2);

	index1 = "1-0";
	TEST_ASSERT(!(index1 == index2));
	TEST_ASSERT(index1 != index2);
	TEST_ASSERT(!(index1 < index2));
	TEST_ASSERT(!(index1 <= index2));
	TEST_ASSERT(index1 > index2);
	TEST_ASSERT(index1 >= index2);

	index2 = "1-0";
	TEST_ASSERT(index1 == index2);
	TEST_ASSERT(!(index1 != index2));
	TEST_ASSERT(!(index1 < index2));
	TEST_ASSERT(index1 <= index2);
	TEST_ASSERT(!(index1 > index2));
	TEST_ASSERT(index1 >= index2);

	index2 = "2-0";
	TEST_ASSERT(!(index1 == index2));
	TEST_ASSERT(index1 != index2);
	TEST_ASSERT(index1 < index2);
	TEST_ASSERT(index1 <= index2);
	TEST_ASSERT(!(index1 > index2));
	TEST_ASSERT(!(index1 >= index2));

	// test reset()
	index3.reset();
	TEST_ASSERT(index3.isNull());
	TEST_ASSERT(index3.getResolution() < 0);

	// test isVertex() and isFace() and isPentagon()
	index1.reset();
	TEST_ASSERT(!index1.isVertex());
	TEST_ASSERT(!index1.isFace());

	index1 = "1";
	TEST_ASSERT(index1.isVertex());
	TEST_ASSERT(!index1.isFace());

	index1 = "A";
	TEST_ASSERT(!index1.isVertex());
	TEST_ASSERT(index1.isFace());

	index1 = "1-000000000000000000";
	TEST_ASSERT(index1.isPentagon());

	index1 = "A-00000";
	TEST_ASSERT(!index1.isPentagon());

	// test hasVertexChildren()
	index1.reset();
	TEST_ASSERT(!index1.hasVertexChildren());

	index1 = "01";
	TEST_ASSERT(index1.hasVertexChildren());

	index1 = "01-02";
	TEST_ASSERT(!index1.hasVertexChildren());

	index1 = "1-20";
	TEST_ASSERT(index1.hasVertexChildren());
	
	index1 = "A";
	TEST_ASSERT(!index1.hasVertexChildren());
	
	index1 = "A-0";
	TEST_ASSERT(index1.hasVertexChildren());

	// test isAncestorOf
	index1.reset();
	index2 = "01-03";
	TEST_ASSERT(!index1.isAncestorOf(index1));
	TEST_ASSERT(!index1.isAncestorOf(index2));
	TEST_ASSERT(!index2.isAncestorOf(index1));

	index1 = "A";
	index2 = "A-01";
	TEST_ASSERT(index1.isAncestorOf(index1));
	TEST_ASSERT(index1.isAncestorOf(index2));
	TEST_ASSERT(!index2.isAncestorOf(index1));

	index1 = "1-0";
	index2 = "1-000201";
	TEST_ASSERT(index1.isAncestorOf(index1));
	TEST_ASSERT(index1.isAncestorOf(index2));
	TEST_ASSERT(!index2.isAncestorOf(index1));

	index1 = "A";
	index2 = "1-0002";
	TEST_ASSERT(index1.isAncestorOf(index1));
	TEST_ASSERT(!index1.isAncestorOf(index2));
	TEST_ASSERT(!index2.isAncestorOf(index1));
}

/*!
Constructor creates a null index.
*/
CompactIndex::CompactIndex()
{
	reset();
}

/*!
Construct the index from a string.

\param strIndex	The string.
*/
CompactIndex::CompactIndex(const std::string& strIndex)
{
	// call string assignment operator
	*this = strIndex;
}

/*!
Construct the index from a PYXIS index.

\param pyxIndex	The PYXIS index.
*/
CompactIndex::CompactIndex(const PYXIcosIndex& pyxIndex)
{
	// call PYXIS index assignment operator
	*this = pyxIndex;
}

/*!
Copy assignment.

\param	rhs	The index to copy.
*/
/*
CompactIndex& CompactIndex::operator =(const CompactIndex &rhs)
{
	if (this != &rhs)
	{
		// copy all bytes
		memcpy(m_pcBuffer, rhs.m_pcBuffer, sizeof(m_pcBuffer));
	}

	return *this;
}
*/

/*!
String assignment. The string must consist of digits in the range {0...7}
and the number of digits must be less than or equal to the maximum allowed.

\param	strIndex	The index.
*/
CompactIndex& CompactIndex::operator =(const std::string& strIndex)
{
	// allow PYXIS index to interpret string
	PYXIcosIndex pyxIndex(strIndex);

	// assign from PYXIS index
	*this = pyxIndex;

	return *this;
}

/*!
PYXIS index assignment.

\param	pyxIndex	The PYXIS index.
*/
CompactIndex& CompactIndex::operator =(const PYXIcosIndex& pyxIndex)
{
	// set index to null
	reset();

	if (!pyxIndex.isNull())
	{
		m_pcBuffer[0] = pyxIndex.getPrimaryResolution();
		m_pcBuffer[sizeof(m_pcBuffer) - 1] = pyxIndex.getResolution();
		int nBufferPos = 1;

		const PYXIndex& subIndex = pyxIndex.getSubIndex();
		int nPos = 0;
		int nDigitCount = subIndex.getDigitCount();

		unsigned int pnDigits[knDigitsPerByte];
		do
		{
			// get the 4 digits to be encoded in each byte, unused digits are set to zero
			for (int n = 0; n < knDigitsPerByte; ++n)
			{
				if (nPos < nDigitCount)
				{
					pnDigits[n] = subIndex.getDigit(nPos);
				}
				else
				{
					pnDigits[n] = 0;
				}

				++nPos;
			}

			// put the two pair-encodings into the byte
			m_pcBuffer[nBufferPos++] = packDigits(pnDigits);

		} while (nPos < nDigitCount);
	}

	return *this;
}

/*!
Convert an index to a string.

\return	The string.
*/
std::string CompactIndex::toString() const
{
	// allow PYXIS index to output string
	PYXIcosIndex pyxIndex(*this);

	return pyxIndex.toString();
}

/*!
Convert a compact index to a PYXIS index.
*/
CompactIndex::operator PYXIcosIndex() const
{
	PYXIcosIndex pyxIndex;

	if (!isNull())
	{
		pyxIndex.setPrimaryResolution(m_pcBuffer[0]);
		PYXIndex& subIndex = pyxIndex.getSubIndex();

		int nSubResolution = m_pcBuffer[sizeof(m_pcBuffer) - 1] - PYXIcosIndex::knMinSubRes;
		if (nSubResolution >= 0)
		{
			unsigned int pnDigits[knDigitsPerByte];

			int nByteCount = nSubResolution / knDigitsPerByte + 1;
			for (int nBufferPos = 1; nBufferPos <= nByteCount; ++nBufferPos)
			{
				// extract the digits from each byte
				unpackDigits(m_pcBuffer[nBufferPos], pnDigits);

				// insert the digits into the PYXIS index
				for (int n = 0; n < knDigitsPerByte; ++n)
				{
					if (nSubResolution-- < 0)
					{
						break;
					}

					subIndex.appendDigit(pnDigits[n]);
				}
			}
		}
	}

	return pyxIndex;
}

/*!
Equality operator. Two indices are equal if their digits are the same.

\return	true if the indices are equal, otherwise false.
*/
bool CompactIndex::operator ==(const CompactIndex& rhs) const
{
	// compare indices byte for byte
	return (memcmp(m_pcBuffer, rhs.m_pcBuffer, sizeof(m_pcBuffer)) == 0);
}

/*!
Inequality operator. See equality operator.

\return	true if the indices are not equal, otherwise false.
*/
bool CompactIndex::operator !=(const CompactIndex& rhs) const
{
	// compare indices byte for byte
	return (memcmp(m_pcBuffer, rhs.m_pcBuffer, sizeof(m_pcBuffer)) != 0);
}

/*!
Less than operator. Returns true if this index occurs before the specified
index in an depth-first sort.

\param	rhs	The index to compare.

\return true if this index is less than the index passed in.
*/
bool CompactIndex::operator <(const CompactIndex& rhs) const
{
	// compare indices byte for byte
	return (memcmp(m_pcBuffer, rhs.m_pcBuffer, sizeof(m_pcBuffer)) < 0);
}

/*!
Less than or equal operator. See less than and equality operators.

\param	rhs	The index to compare.

\return true if this index is less than or equal to the index passed in.
*/
bool CompactIndex::operator <=(const CompactIndex& rhs) const
{
	// compare indices byte for byte
	return (memcmp(m_pcBuffer, rhs.m_pcBuffer, sizeof(m_pcBuffer)) <= 0);
}

/*!
Greater than operator. Returns true if this index occurs after the specified
index in an depth-first sort.

\param	rhs	The index to compare.

\return true if this index is greater than the index passed in.
*/
bool CompactIndex::operator >(const CompactIndex& rhs) const
{
	// compare indices byte for byte
	return (memcmp(m_pcBuffer, rhs.m_pcBuffer, sizeof(m_pcBuffer)) > 0);
}

/*!
Greater than or equal operator. See greater than and equality operators.

\param	rhs	The index to compare.

\return true if this index is greater than or equal to the index passed in.
*/
bool CompactIndex::operator >=(const CompactIndex& rhs) const
{
	// compare indices byte for byte
	return (memcmp(m_pcBuffer, rhs.m_pcBuffer, sizeof(m_pcBuffer)) >= 0);
}

/*!
Reset the index to null.
*/
void CompactIndex::reset()
{
	// set all bytes except resolution byte to zero
	memset(m_pcBuffer, 0, sizeof(m_pcBuffer) - 1);

	// set resolution byte to -1
	m_pcBuffer[sizeof(m_pcBuffer) - 1] = -1;
}

/*!
Does this index represent a vertex at resolution 1.

\return	True if this index is a vertex or false not.
*/
bool CompactIndex::isVertex() const
{
	return (	(m_pcBuffer[0] >= PYXIcosIndex::knFirstVertex) &&
				(m_pcBuffer[0] <= PYXIcosIndex::knLastVertex)	);
}

/*!
Determine if this index is on a PYXIS pole.

\return true if this index is on a PYXIS pole
*/
bool CompactIndex::isPolar() const
{
	if (isNull())
	{
		// exception thrown to mimic behaviour of PYXIcosIndex
		PYXTHROW(PYXIndexException, "Null index.");
	}

	return	(	(m_pcBuffer[0] == PYXIcosIndex::knVertexPole1) ||
				(m_pcBuffer[0] == PYXIcosIndex::knVertexPole2)	);

}

/*!
Does this index represent a face at resolution 1.

\return true if this index is a face or false if the index is a vertex or if
		the index is null.
*/
bool CompactIndex::isFace() const
{
	return ((m_pcBuffer[0] >= PYXIcosIndex::kcFaceFirstChar) &&
			(m_pcBuffer[0] <= PYXIcosIndex::kcFaceLastChar));
}

/*!
Determine if this index is a centroid child of its parent. At resolution one
cells centred on icosahedron vertices (pentagons) are considered to be centroid
children and cells centred on icosahedron faces (hexagons) are considered to be
vertex children.

\return	true if this index is a centroid child otherwise false.
*/
bool CompactIndex::hasVertexChildren() const
{
	bool bIsCentroidChild = false;

	if (!isNull())
	{
		// if the subindex is non-null then defer to it
		int nSubRes = getResolution() - PYXIcosIndex::knMinSubRes;
		if (nSubRes >= 0)
		{
			// subindex has vertex children if it ends in zero
			int nLastByte = nSubRes / knDigitsPerByte;
			int nLastDigit = nSubRes - (nLastByte * knDigitsPerByte);

			// unpack the last byte of the subindex
			unsigned int pnDigits[knDigitsPerByte];
			unpackDigits(m_pcBuffer[nLastByte + 1], pnDigits);
			if (pnDigits[nLastDigit] == 0)
			{
				bIsCentroidChild = true;
			}
		}
		else if (isVertex())
		{
			// the only false for the first 2 resolutions is on a face
			bIsCentroidChild = true;
		}
	}

	return bIsCentroidChild;
}

/*
Determine if an index is the centroid child of an icosahedron vertex at any
resolution.

\return true if the index is a pentagon otherwise false.
*/
bool CompactIndex::isPentagon() const
{
	bool bIsPentagon = false;

	if (!isNull())
	{
		// if the parent of this index is centred on an icosahedron vertex
		if (isVertex())
		{
			int nSubRes = getResolution() - PYXIcosIndex::knMinSubRes;
			if (nSubRes < 0)
			{
				// we are on a pentagon on the primary resolution
				bIsPentagon = true;
			}
			else
			{
				// if the subindex is all 0's then we are a direct descendant
				bIsPentagon = true;

				int nBytesToCompare = nSubRes / knDigitsPerByte + 1;
				for (int n = 1; n <= nBytesToCompare; ++n)
				{
					if (m_pcBuffer[n] != 0)
					{
						bIsPentagon = false;
						break;
					}
				}
			}
		}
	}
	
	return bIsPentagon;
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
bool CompactIndex::isAncestorOf(const CompactIndex& index) const
{
	bool bIsAncestor = false;

	// the null index is a parent of nothing
	if (!isNull() && !index.isNull())
	{
		int nResolution = getResolution();
		if (nResolution <= index.getResolution())
		{
			// calculate the number of bytes that can be compared directly
			int nSubRes = nResolution - PYXIcosIndex::knMinSubRes;
			size_t nBytesToCompare = (nSubRes + 1) / knDigitsPerByte + 1;
			if (memcmp(m_pcBuffer, index.m_pcBuffer, nBytesToCompare) == 0)
			{
				bIsAncestor = true;

				// if necessary, unpack remaining byte to compare digits
				size_t nDigitsToCompare = nSubRes - ((nBytesToCompare - 1) * knDigitsPerByte) + 1;
				if (nDigitsToCompare > 0)
				{
					unsigned int pnDigits1[knDigitsPerByte];
					unsigned int pnDigits2[knDigitsPerByte];
					unpackDigits(m_pcBuffer[nBytesToCompare], pnDigits1);
					unpackDigits(m_pcBuffer[nBytesToCompare], pnDigits2);

					for (size_t n = 0; n < nDigitsToCompare; ++n)
					{
						if (pnDigits1[n] != pnDigits2[n])
						{
							bIsAncestor = false;
							break;
						}
					}
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
bool CompactIndex::isDescendantOf(const CompactIndex& index) const
{
	// call the ancestor method with the parameters switched
	return index.isAncestorOf(*this);
}

/*!
Pack 4 digits into a byte. The first two digits are pairwise encoded into the
top nibble and the second two digits are pairwise encoded into the low nibble.

\param pnDigits	Array of exactly knDigitsPerByte digits (in)

\return	The byte
*/
signed char CompactIndex::packDigits(unsigned int pnDigits[]) const
{
	assert((pnDigits != 0) && "Invalid argument.");

	// pair-encode the first two digits
	unsigned int nHighNibble = pnDigits[0] | pnDigits[1];
	if (pnDigits[0] != 0)
	{
		nHighNibble |= 0x8;
	}

	// pair-encode the last two digits
	unsigned int nLowNibble = pnDigits[2] | pnDigits[3];
	if (pnDigits[2] != 0)
	{
		nLowNibble |= 0x8;
	}

	return ((nHighNibble << 4) | nLowNibble);
}

/*!
Unpack 4 digits from a byte. The first two digits are pairwise encoded into the
top nibble and the second two digits are pairwise encoded into the low nibble.

\param cByte	The byte.
\param pnDigits	Array of exactly knDigitsPerByte digits (in/out)

\return	The byte
*/
void CompactIndex::unpackDigits(signed char cByte, unsigned int pnDigits[]) const
{
	assert((pnDigits != 0) && "Invalid argument.");

	// get the pair-encoded digits from the high nibble
	unsigned int nHighNibble = cByte >> 4;
	if (nHighNibble & 0x8)
	{
		pnDigits[0] = nHighNibble & 0x7;
		pnDigits[1] = 0;
	}
	else
	{
		pnDigits[0] = 0;
		pnDigits[1] = nHighNibble;
	}

	// get the pair-encoded digits from the low nibble
	unsigned int nLowNibble = cByte & 0xF;
	if (nLowNibble & 0x8)
	{
		pnDigits[2] = nLowNibble & 0x7;
		pnDigits[3] = 0;
	}
	else
	{
		pnDigits[2] = 0;
		pnDigits[3] = nLowNibble;
	}
}

/*! 
Allows indices to be written to streams.

\param out		The stream to write to.
\param index	The index to write to the stream.

\return The stream after the operation.
*/
std::ostream& operator <<(std::ostream& out, const CompactIndex& index)
{
	out << index.toString();
	return out;
}

/*!
Allows indices to be read from streams.

\param input	The stream to read from.
\param index	The index to read from the stream.

\return The stream after the operation.
*/
std::istream& operator >>(std::istream& input, CompactIndex& index)
{
	std::string strIndex;
	input >> strIndex;
	index = strIndex;
	return input;
}
