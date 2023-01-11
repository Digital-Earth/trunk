/******************************************************************************
spiral_iterator.cpp

begin		: 2007-01-31
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "spiral_iterator.h"

// local includes
//#include "exceptions.h"
#include "pyxis/derm/cursor.h"
#include "pyxis/derm/exceptions.h"
#include "pyxis/derm/hexagon.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"

//! Tester class
Tester<PYXSpiralIterator> gTester;

//! The maximum depth used in created number of spirals for varying depths
const int PYXSpiralIterator::knMaxSpiralingDepth = 20;

//! Number of extra spirals vector
std::vector<int> PYXSpiralIterator::m_vecNumberSpirals;

//! Test method
void PYXSpiralIterator::test()
{

	PYXSpiralIterator itSpiral(PYXIcosIndex("A-0"), 0, 0);
	int nCount = 0;
	for (; !itSpiral.end(); ++itSpiral)
	{
		++nCount;
	}
	TEST_ASSERT(nCount == 1);

	itSpiral.reset(PYXIcosIndex("A-0"), 1, 0);
	nCount = 0;
	for (; !itSpiral.end(); ++itSpiral)
	{
		++nCount;
	}
	TEST_ASSERT(nCount == 7);

	nCount = 0;
	itSpiral.reset(PYXIcosIndex("A-0"), 2, 0);
	for (; !itSpiral.end(); ++itSpiral)
	{
		++nCount;
	}
	TEST_ASSERT(nCount == 19);

	nCount = 0;
	itSpiral.reset(PYXIcosIndex("A-0"), 3, 0);
	for (; !itSpiral.end(); ++itSpiral)
	{
		++nCount;
	}
	TEST_ASSERT(nCount == 37);

	nCount = 0;
	itSpiral.reset(PYXIcosIndex("1-0"), 1, 0);
	for (; !itSpiral.end(); ++itSpiral)
	{
		++nCount;
	}
	TEST_ASSERT(nCount == 6);

	nCount = 0;
	itSpiral.reset(PYXIcosIndex("1-0"), 2, 0);
	for (; !itSpiral.end(); ++itSpiral)
	{
		++nCount;
	}
	TEST_ASSERT(nCount == 16);

	nCount = 0;
	itSpiral.reset(PYXIcosIndex("12-0"), 3, 0);
	for (; !itSpiral.end(); ++itSpiral)
	{
		++nCount;
	}
	TEST_ASSERT(nCount == 31);

	nCount = 0;
	itSpiral.reset(PYXIcosIndex("1-00"), 1);
	for (; !itSpiral.end(); ++itSpiral)
	{
		++nCount;
	}
	TEST_ASSERT(nCount == 6);

	nCount = 0;
	itSpiral.reset(PYXIcosIndex("1-00"), 2);
	for (; !itSpiral.end(); ++itSpiral)
	{
		++nCount;
	}
	TEST_ASSERT(nCount == 16);

	nCount = 0;
	itSpiral.reset(PYXIcosIndex("12-00"), 3);
	for (; !itSpiral.end(); ++itSpiral)
	{
		++nCount;
	}
	TEST_ASSERT(nCount == 31);
}

/*!
Create a new iterator initialized with a starting index and an iteration 
depth. If the root index is invalid or the iteration resolution is beyond 
the maximum resolution an exception is thrown.

\param rootIndex		The PYXIS icos index from which to reference.
\param nDepth			The depth below the root index to iterate.
\param nExtraSpirals	The number of extra spirals to include in the iteration.
*/
PYXSpiralIterator::PYXSpiralIterator(	const PYXIcosIndex& rootIndex,
										int nDepth,
										int nExtraSpirals)
{
	reset(rootIndex, nDepth, nExtraSpirals);
}

PYXSpiralIterator::~PYXSpiralIterator()
{
}

/*!
Reset the existing iterator to the first index in an iteration for an index at
a specified depth. If the root index is invalid or the iteration resolution is 
beyond the maximum resolution an exception is thrown.

\param rootIndex		The PYXIS icos index from which to reference
\param nDepth			The depth below the root index to iterate.
\param nExtraSpirals	The number of extra spirals to include in the iteration.
*/
void PYXSpiralIterator::reset(	const PYXIcosIndex& rootIndex,
								int nDepth,
								int nExtraSpirals	)
{
	if (rootIndex.isNull())
	{
		PYXTHROW(PYXIndexException, "Null index invalid for spiral iterator.");
	}
	int nIterationResolution = rootIndex.getResolution() + nDepth;
	if (	nIterationResolution < PYXIcosIndex::knMinSubRes ||
			PYXMath::knMaxAbsResolution < nIterationResolution ||
			nIterationResolution < rootIndex.getResolution()	)
	{
		PYXTHROW(	PYXIndexException, 
					"Invalid resolution for iteration '" << 
					nIterationResolution <<
					"'."	);				
	}
	assert(nDepth <= knMaxSpiralingDepth);
	
	// set the current index to the centre of the spiral
	m_rootIndex = rootIndex;
	m_index = rootIndex;
	m_index.setResolution(nIterationResolution);

	// reset the factor counters
	m_nSixFactor = 0;
	m_nTwoFactor = 0;

	// Calculate the number of spirals needed to paint the cell.
	m_nTotalSpirals = getNumberOfSpirals(nDepth) + nExtraSpirals;

	m_nSpiralCount = 0;

	// set up the cursor (axial only, no spiral needed at the origin)
	m_axialCursor.setIndex(m_index);
	m_axialCursor.setDir(PYXMath::knDirectionTwo);
	m_spiralCursor.setIndex(m_index);

	m_nLegCount = m_nSpiralCount;

	m_nTotalTurns = Hexagon::knNumSides - 1;
	if (m_rootIndex.isPentagon())
	{
		--m_nTotalTurns;
	}

	// the origin is a single cell and does not make any turns
	m_nTurnCount = m_nTotalTurns;
}

// Return the number of spirals to iterate around center point.
int PYXSpiralIterator::getNumberOfSpirals(int nDepth)
{	
	return m_vecNumberSpirals[nDepth];
}


/*!
Calculate the number of spirals that need to be drawn to fill in the mesh cell.
*/
void PYXSpiralIterator::initStaticData()
{	
	// Calculate the number of spirals that needed to completely cover a cell.
	m_vecNumberSpirals.push_back(0);
	m_vecNumberSpirals.push_back(1);
	m_vecNumberSpirals.push_back(2);
	m_vecNumberSpirals.push_back(3);

	for(int nDepth = 4;
		nDepth <= knMaxSpiralingDepth; 
		nDepth++	)
	{
		m_vecNumberSpirals.push_back( m_vecNumberSpirals[nDepth -2] * 3);
	}
}

/*!
Move to the next cell.
*/
void PYXSpiralIterator::next()
{
	if (!end())
	{
		// take the next step along the leg
		if (m_nLegCount != 0)
		{
			--m_nLegCount;
			m_spiralCursor.forward();
			applyFactors();
		}
		else
		{
			// the leg was finished, are there more turns to make
			if (m_nTurnCount < m_nTotalTurns)
			{
				// turn left and reset the leg
				m_spiralCursor.left();
				++m_nTurnCount;
				m_nLegCount = m_nSpiralCount - 1;
				m_spiralCursor.forward();	
				applyFactors();
			}
			else
			{
				// leg and turns complete, check spirals
				if (m_nSpiralCount < m_nTotalSpirals)
				{
					incrementSpiral();
				}
				else
				{
					// the end of the itertion is reached
					m_index.reset();
				}
			}				
		}
	}
}

/*!
Increment the state of the iterater to the next spiral out from the centre
of the iteration.
*/
void PYXSpiralIterator::incrementSpiral()
{
	assert(m_nSpiralCount < m_nTotalSpirals);
	
	// move out to the next ring
	++m_nSpiralCount;
	m_axialCursor.forward();
	--m_nSixFactor;
 
	// set up the spiral cursor
	m_spiralCursor.setIndex(m_axialCursor.getIndex());
	m_spiralCursor.setDir(
		PYXIcosMath::rotateDirection(m_axialCursor.getIndex(), m_axialCursor.getDir(), 2));
	
	// move forward since the spiral will finish in this position.
	m_spiralCursor.forward();

	m_nLegCount = m_nSpiralCount - 1;
	m_nTurnCount = 0;
}

/*!
Adjust the two and six direction factors according to the direction of travel.
*/
void PYXSpiralIterator::applyFactors()
{
	switch(m_nTurnCount)
	{
	case 0:
		{
			--m_nTwoFactor;
			--m_nSixFactor;
			break;
		}
	case 1:
		{
			--m_nTwoFactor;
			break;
		}
	case 2:
		{
			m_nSixFactor++;
			break;
		}
	case 3:
		{
			m_nTwoFactor++;
			m_nSixFactor++;
			break;
		}
	case 4:
		{
			++m_nTwoFactor;

			// if the spiral is a hexagon
			if (m_nTurnCount == m_nTotalTurns)
			{
				--m_nSixFactor;
			}
			break;
		}
	case 5:
		{
			--m_nSixFactor;
			break;
		}
	default:
		{
			assert(false && "Invalid case for spiral iterator factoring.");
			break;
		}
	}
}
