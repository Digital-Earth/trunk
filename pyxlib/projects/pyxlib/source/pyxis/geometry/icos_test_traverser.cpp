/******************************************************************************
icos_intersection_traverser.cpp

begin		: 2005-06-21
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/icos_test_traverser.h"

// pyxlib includes
#include "pyxis/geometry/test.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/derm/index.h"
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/utility/tester.h"

//! Tester class
Tester<PYXIcosTestTraverser> gTester;

static const PYXIcosIndex kTestIndex("F-010300405");

//! Test method for tester that finds only the test index.
class PYXTest1 : public PYXTest
{
public:
	//! Perform the test.
	virtual eTestResult doTestIndex(	const PYXIcosIndex& index,
										bool* pbAbort	)
	{
		eTestResult nResult = knNo;

		if (index.isAncestorOf(kTestIndex))
		{
			if (index == kTestIndex)
			{
				nResult = knYes;
			}
			else
			{
				nResult = knMaybe;
			}
		}

		return nResult;
	}
};

//! Test method
void PYXIcosTestTraverser::test()
{
	// test the tester
	PYXIcosTestTraverser traverser;
	int nDataResolution = kTestIndex.getResolution();
	int nTargetResolution = nDataResolution - 5;

	{
		// set up test
		PYXTest1 test1;
		test1.setDataResolution(nDataResolution);
		test1.setTargetResolution(nTargetResolution);

		// set up traverser
		traverser.setTest(test1);
		traverser.traverse();

		// check results
		PYXPointer<PYXTileCollection> spTileCollection = traverser.getTileCollection();
		TEST_ASSERT(spTileCollection != 0);

		// should find only the test index
		TEST_ASSERT(spTileCollection->getGeometryCount() == 1);
		PYXPointer<PYXCell> spCell(PYXCell::create(kTestIndex));
		TEST_ASSERT(spTileCollection->intersects(*spCell));
	}
}

/*!
\return		True if the traversal should be aborted; false otherwise.
*/
bool PYXIcosTestTraverser::doBeginTraversal()
{
	assert((m_pTest != 0) && "No test specified.");

	m_pTest->initialize();

	m_spTileCollection = PYXTileCollection::create(false);

	return false;
}

/*!
\param index	The index to process.
\return The traversal result.
*/
PYXIcosTraverser::eTraversalResult PYXIcosTestTraverser::doProcessIndex(const PYXIcosIndex& index)
{
	eTraversalResult nResult = knContinue;

	// Perform the test
	bool bAbort = false;
	PYXTest::eTestResult nTestResult = m_pTest->testIndex(index, &bAbort);

	if (bAbort)
	{
		return knAbort;
	}

	if (index.getResolution() == m_pTest->getTargetResolution())
	{
		// We are at target recording resolution
		nResult = knPrune;
		if (nTestResult == PYXTest::knYes ||
			nTestResult == PYXTest::knYesComplete)
		{
			// Record the result 
			m_spTileCollection->addTile(index, m_pTest->getDataResolution());
		}
		else if (nTestResult == PYXTest::knMaybe)
		{
			// This shouldn't happen now that maybe is resolved by the test itself
			assert(false);
		}
	}
	else
	{
		// We aren't at target recording resolution
		if (nTestResult == PYXTest::knNo)
		{
			nResult = knPrune;
		}
		else if (nTestResult == PYXTest::knYesComplete)
		{
			nResult = knPrune;
			if (index.hasVertexChildren())
			{
				// Record the centroid child
				PYXIcosIndex index2(index);
				index2.incrementResolution();
				m_spTileCollection->addTile(index2, m_pTest->getDataResolution());

				// Test the vertex children
				for (PYXVertexIterator it(index); !it.end(); it.next())
				{
					eTraversalResult nChildResult = traverseImpl(it.getIndex());
					if (nChildResult == knAbort)
					{
						return knAbort;
					}
				}
			}
			else
			{
				// Record the result
				m_spTileCollection->addTile(index, m_pTest->getDataResolution());
			}
		}
	}

	return nResult;
}

/*!
Releases the result set from the traverser.

\return The tile collection (ownership transferred).
*/
PYXPointer<PYXTileCollection> PYXIcosTestTraverser::getTileCollection()
{
	return m_spTileCollection;
}

/*!
\param	test	The test to perform on each index (ownership retained).
*/
void PYXIcosTestTraverser::setTest(PYXTest& test)
{
	m_pTest = &test;
}
