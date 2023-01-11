/******************************************************************************
exhaustive_iterator.cpp

begin		: 2003-12-17
copyright	: (C) 2003 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/exhaustive_iterator.h"

// pyxlib includes
#include "pyxis/derm/exceptions.h"
#include "pyxis/derm/hexagon.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <algorithm>

//! Tester class
Tester<PYXExhaustiveIterator> gTester;

//! Test method
void PYXExhaustiveIterator::test()
{
	// test resolution
	const int knTestResolution = 7;

	{
		// choose a vertex so gap detection is tested
		PYXIcosIndex rootIndex("01-0");
	
		PYXExhaustiveIterator it(rootIndex, knTestResolution);
		TEST_ASSERT(!it.end());

		testIteratorRecursive(	rootIndex,
								knTestResolution - rootIndex.getResolution(),
								it	);

		TEST_ASSERT(it.end());

		// try to go past the end
		it.next();
		TEST_ASSERT(it.end());
	}

	{
		// choose a vertex child as root
		PYXIcosIndex rootIndex("A");

		PYXExhaustiveIterator it(rootIndex, knTestResolution);
		TEST_ASSERT(!it.end());

		testIteratorRecursive(	rootIndex,
								knTestResolution - rootIndex.getResolution(),
								it	);

		TEST_ASSERT(it.end());

		// try to go past the end
		it.next();
		TEST_ASSERT(it.end());
	}

	// test some invalid iterators
	TEST_ASSERT_EXCEPTION( PYXExhaustiveIterator itBad1(PYXIcosIndex(), knTestResolution), PYXIndexException);
	TEST_ASSERT_EXCEPTION( PYXExhaustiveIterator itBad2(PYXIcosIndex(), knTestResolution + 1), PYXIndexException);
	TEST_ASSERT_EXCEPTION( PYXExhaustiveIterator itBad3(PYXIcosIndex(), -1), PYXIndexException);
}

/*!
A recursive method that tests the PYXIS cell iterator.

\param index	The index prefix
\param nDepth	The current depth (relative to the root)
\param it		The iterator to test
*/
void PYXExhaustiveIterator::testIteratorRecursive(	const PYXIcosIndex& index,
													int nDepth,
													PYXExhaustiveIterator& it	)
{
	assert(!index.isNull());

	if (0 == nDepth)
	{
		if (PYXIcosMath::isValidIndex(index))
		{
			TEST_ASSERT(index == it.getIndex());
			it.next();
		}
	}
	else if (1 == nDepth)
	{
		for (int nIndex = 0; nIndex <= Hexagon::knNumSides; ++nIndex)
		{
			PYXIcosIndex tempIndex(index);
			tempIndex.getSubIndex().appendDigit(static_cast<unsigned int>(nIndex));

			if (PYXIcosMath::isValidIndex(tempIndex))
			{
				TEST_ASSERT(tempIndex == it.getIndex());
				it.next();
			}
		}
	}
	else
	{
		// repeat for resolution n - 1 with 0 appended
		PYXIcosIndex tempIndex(index);
		tempIndex.getSubIndex().appendDigit(0);
		PYXExhaustiveIterator::testIteratorRecursive(tempIndex, nDepth - 1, it);

		if (index.hasVertexChildren())
		{
			// repeat for resolution n - 2 with 01 -> 06 appended
			for (int nIndex = 1; nIndex <= Hexagon::knNumSides; ++nIndex)
			{
				PYXIcosIndex tempIndex(index);
				tempIndex.getSubIndex().appendDigit(static_cast<unsigned int>(nIndex));
				tempIndex.getSubIndex().appendDigit(0);
				PYXExhaustiveIterator::testIteratorRecursive(tempIndex, nDepth - 2, it);
			}
		}
	}
}

/*!
Constructor creates an iterator for all the cells that share the same root
index at the specified resolution. This iterator does not support a null root
index. Iteration proceeds in a breadth first manner.

\param rootIndex		The PYXIS icos index from which to reference
\param nResolution		The resolution over which to iterate.
*/
PYXExhaustiveIterator::PYXExhaustiveIterator(	const PYXIcosIndex& rootIndex,
												int nResolution	)
{
	reset(rootIndex, nResolution);
}

PYXExhaustiveIterator::~PYXExhaustiveIterator()
{
}

/*!
Resets the iterator for all the cells that share the same root index at
the specified resolution. This iterator does not support a null root
index. Iteration proceeds in a breadth first manner.

\param rootIndex		The PYXIS icos index from which to reference
\param nResolution		The resolution over which to iterate.
*/
void PYXExhaustiveIterator::reset(	const PYXIcosIndex& rootIndex,
									int nResolution	)
{
	if(rootIndex.isNull())
	{
		PYXTHROW(PYXIndexException, "Exhaustive iterator does not support a null root index.");
	}

	m_rootIndex = rootIndex;
	m_index = rootIndex;

	// keep track of the centroid child
	m_bRootIsVertexChild = !rootIndex.hasVertexChildren();

	// keep track of the cell gap
	m_bIsPentagon = PYXIcosMath::getCellGap(rootIndex, &m_nCellGap);

	// set the end condition
	m_index = PYXIcosIndex();

	if (!rootIndex.isNull())
	{
		// root must be at least resolution 1
		assert(rootIndex.getResolution() >= 1);

		// force minimum resolution to be resolution of root index
		nResolution = std::max(nResolution, rootIndex.getResolution());
	
		// initialize for the resolution
		initForResolution(nResolution);
	}
}

/*!
Initialize to begin iterating over the cells at the specified resolution.

\param	nResolution	The resolution over which to iterate.
*/
void PYXExhaustiveIterator::initForResolution(int nResolution)
{
	// build initial index for resolution
	m_index = m_rootIndex;
	m_index.setResolution(nResolution);

	PYXIndex& subIndex = m_index.getSubIndex();
	m_pszMax = &(subIndex.m_pcDigits[subIndex.m_nDigitCount - 1]);
	m_pszMin = &(subIndex.m_pcDigits[m_rootIndex.getSubIndex().m_nDigitCount]);
	m_pszLeading = m_pszMax;
}

/*!
Move to the next cell.
*/
void PYXExhaustiveIterator::next()
{
	if (!end())
	{
		// work from the last to first digit
		char* ptr = m_pszMax;
		for (; ptr >= m_pszMin; --ptr)
		{
			// if more significant digit is zero, increment current digit
			if ((ptr == m_pszMin) || (*(ptr - 1) == '0'))
			{
				(*ptr)++;

				// if the leading digit is in the gap, move to next digit
				if (m_bIsPentagon)
				{
					if (ptr <= m_pszLeading)
					{
						m_pszLeading = ptr;

						if (m_nCellGap == static_cast<PYXMath::eHexDirection>(*ptr - '0'))
						{
							(*ptr)++;
						}
					}
				}

				// check for a carry condition
				if ((Hexagon::knNumSides + '0') < *ptr)
				{
					*ptr = '0';

					// remain in loop, there is a carry
				}
				else
				{
					// exit loop, we are done
					break;
				}
			}
		}

		// check the end condition
		if (	(ptr < m_pszMin) ||	
				(	(ptr == m_pszMin) &&
					(	('0' == *m_pszMin) ||
						m_bRootIsVertexChild	)	)	)
		{
			// set the index to null to trigger the iteration end condition.
			m_index = PYXIcosIndex();
		}
	}
}
