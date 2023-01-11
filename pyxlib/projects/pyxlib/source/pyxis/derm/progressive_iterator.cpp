/******************************************************************************
progressive_iterator.cpp

begin		: 2006-03-07
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/progressive_iterator.h"

// pyxlib includes
#include "pyxis/derm/exceptions.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/tester.h"

//! The unit test class
Tester<PYXProgressiveIterator> gTester;

//! Used in testing to compare a PYXProgressiveIterator its definition
static void checkDef (	const PYXIcosIndex& rootIndex,
						int nDepth,
						PYXProgressiveIterator::eIteratorFlags nFlags	)
{
	int nRootRes = rootIndex.getResolution();
	int nMaxRes = nRootRes + nDepth;
	PYXProgressiveIterator itPr(rootIndex, nMaxRes, nFlags);

	// depth 0 case: we must generate the root index itself
	PYXIcosIndex index = rootIndex;
	if ((nFlags & PYXProgressiveIterator::knExtendOutputToFullResolution) != 0)
	{
		index.setResolution(nMaxRes);
	}
	TEST_ASSERT(itPr.getIndex() == index);
	++itPr;

	// depth > 0
	for (int d = 1; d <= nDepth; ++d)
	{
		int res = nRootRes + d;
		// iterate exhaustively at res
		PYXExhaustiveIterator itEx(rootIndex,res);
		for (; !itEx.end(); ++itEx)
		{
			PYXIcosIndex index = itEx.getIndex();
			// filter: if we're repeating centroid children, don't filter at all
			//			otherwise, skip over indices with final zero digit
			if (	((nFlags & PYXProgressiveIterator::knRepeatChildren) != 0)
					||	!index.hasVertexChildren()	)
			{
				if ((nFlags & PYXProgressiveIterator::knExtendOutputToFullResolution) != 0)
				{
					index.setResolution(nMaxRes);
				}
				TEST_ASSERT(itPr.getIndex() == index);
				++itPr;
			}
		}
	}
	TEST_ASSERT(itPr.end());
}

//! Used in testing to compare a PYXProgressiveIterator against a PYXExhaustiveIterator
static void comparePrEx (	const PYXIcosIndex& rootIndex,
							int nDepth	)
{
	int nRootRes = rootIndex.getResolution();
	int nMaxRes = nRootRes + nDepth;
	int nCount = PYXIcosMath::getCellCount(rootIndex,nMaxRes);

	// Test 1: compare counts
	{
		int nCountPr = 0;
		PYXProgressiveIterator itPr(rootIndex,
									nMaxRes,
									PYXProgressiveIterator::knExtendOutputToFullResolution);
		for (; !itPr.end(); ++itPr)
		{
			TEST_ASSERT(itPr.getIndex().isDescendantOf(rootIndex));
			nCountPr++;
			if (nCountPr > nCount)
			{
				// possible infinite loop: skip any more tests
				TEST_ASSERT(false);
				return;
			}
		}
		TEST_ASSERT(nCountPr == nCount);
	}

	// Test 2: verify that same set of indices is generated
	{
		std::vector<bool> bGenerated(nCount,false);
		PYXProgressiveIterator itPr(	rootIndex,
										nMaxRes,
										PYXProgressiveIterator::knExtendOutputToFullResolution);
		for (; !itPr.end(); ++itPr)
		{
			int nOffset = PYXIcosMath::calcCellPosition(	rootIndex,
															itPr.getIndex()	);
			// verify no duplicates
			TEST_ASSERT(bGenerated[nOffset] == false);
			bGenerated[nOffset] = true;
		}
		for (int i=0; i<nCount; i++)
		{
			// verify nothing missed
			TEST_ASSERT(bGenerated[i]);
		}
	}
}

/*!
The unit test method for the class.
*/
void PYXProgressiveIterator::test()
{
	// test all behaviours against definition
	checkDef(PYXIcosIndex("A-0"),5,PYXProgressiveIterator::knDefaultBehaviour);
	checkDef(PYXIcosIndex("A-0"),5,PYXProgressiveIterator::knRepeatChildren);
	checkDef(PYXIcosIndex("A-0"),5,PYXProgressiveIterator::knExtendOutputToFullResolution);
	checkDef(PYXIcosIndex("A-0"),5,PYXProgressiveIterator::knRepeatAndExtend);

	// Compare a PYXProgressiveIterator against a PYXExhaustiveIterator
	comparePrEx(PYXIcosIndex("A-0"),5);			// hexagonal, centroid-child root
	comparePrEx(PYXIcosIndex("1"),5);			// pentagonal root
	comparePrEx(PYXIcosIndex("10-0101"),8);		// vertex-child root

	// Test end, next in a simple case.
	PYXIcosIndex testIndex( "A-006");
	PYXProgressiveIterator firstIterator( testIndex, 
		testIndex.getResolution() + 2);
	for (int nCount = 0; nCount < 7; ++nCount)
	{	
		TEST_ASSERT( !firstIterator.end());
		firstIterator.next();
	}
	TEST_ASSERT( firstIterator.end());

	// try to go past the end
	firstIterator.next();
	TEST_ASSERT( firstIterator.end());

	// Create an iterator on a valid index, but ask for a lower resolution than 
	//	the index.
	PYXProgressiveIterator iteratorOnEmptySet( testIndex, 
		testIndex.getResolution() - 1);
	TEST_ASSERT( iteratorOnEmptySet.end());

	// test exception throwing for null root index cases
    TEST_ASSERT_EXCEPTION( PYXProgressiveIterator itBad1(PYXIcosIndex(), 7), 
		PYXException);
	TEST_ASSERT_EXCEPTION( PYXProgressiveIterator itBad2(PYXIcosIndex(), 8), 
		PYXException);
	TEST_ASSERT_EXCEPTION( PYXProgressiveIterator itBad3(PYXIcosIndex(), -1), 
		PYXException);
}

/*!
Constructor creates an iterator for all the cells that share the same root index
at the specified resolution.  Throws PYXIndexException if root index is null.
Iteration proceeds in a breadth-first manner.

\param rootIndex		The PYXIS icos index from which to reference
\param nResolution		The resolution over which to iterate.
\param nFlags			Behaviour flags (repeat children, extend)
*/
PYXProgressiveIterator::PYXProgressiveIterator(
								const PYXIcosIndex& rootIndex,
								int nResolution, 
								eIteratorFlags nFlags /* = IteratorFlags::Default */) :
	m_iterator(rootIndex, nResolution),
    m_bRepeatChildren( (nFlags & knRepeatChildren)!= 0 ),
	m_bExtendResolution( (nFlags & knExtendOutputToFullResolution) != 0)
{
	reset(rootIndex, nResolution);
}

/*!
Resets the iterator for given root index and resolution.  Throws PYXIndexException
when given a null root index.

\param rootIndex		The PYXIS icos index from which to reference
\param nResolution		The resolution over which to iterate.
*/
void PYXProgressiveIterator::reset(	const PYXIcosIndex& rootIndex,
									int nResolution	)
{
	if (rootIndex.isNull())
	{
		PYXTHROW(	PYXIndexException,
					"Attempt to create PYXProgressiveIterator with null root index."	);
	}
	assert(nResolution <= PYXMath::knMaxAbsResolution);

	// determine root and cell resolutions
	m_rootIndex = rootIndex;
	m_nRootRes = rootIndex.getResolution();
	m_nMaxRes = nResolution;
	m_nMaxDepth = m_nMaxRes - m_nRootRes;

	// prepare to iterate beginning at depth 0
	m_nDepth = 0;
	m_iterator.reset(rootIndex, m_nRootRes);
	m_bAtEnd = ((m_nMaxDepth < 0) || m_iterator.end());

	// set initial value of m_index
	m_index = m_iterator.getIndex();
	if (m_bExtendResolution)
	{
		m_index.setResolution(m_nMaxRes);
	}
}

/*!
Advance to the next cell.
*/
void PYXProgressiveIterator::next()
{
	while (!m_bAtEnd)
	{
		// advance the underlying iterator until we get a valid index
		for (++m_iterator; !m_iterator.end(); ++m_iterator)
		{
			PYXIcosIndex index = m_iterator.getIndex();
			if (m_bRepeatChildren || !index.hasVertexChildren())
			{
				// valid index: return immediately
				m_index = index;
				if (m_bExtendResolution)
				{
					m_index.setResolution(m_nMaxRes);
				}
				return;
			}
		}

		// if we hit the end, do we need to advance the depth?
		if (m_iterator.end())
		{
			// yes, we do: if we've already hit the maximum, we're done
			if (m_nDepth >= m_nMaxDepth)
			{
				m_bAtEnd = true;
			}
			// there's another depth still to go
			else
			{
				++m_nDepth;
				m_iterator.reset(m_rootIndex, m_nRootRes + m_nDepth);
				if (m_bRepeatChildren)
				{
					m_index = m_iterator.getIndex();
					if (m_bExtendResolution)
					{
						m_index.setResolution(m_nMaxRes);
					}
					return;
				}
			}
		}
	}
}
