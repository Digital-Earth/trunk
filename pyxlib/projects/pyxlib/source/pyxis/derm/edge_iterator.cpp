/******************************************************************************
edge_iterator.cpp

begin		: 2004-11-10
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/edge_iterator.h"

// pyxlib includes
#include "pyxis/derm/dir_edge_iterator.h"
#include "pyxis/derm/exceptions.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"

//! Tester class
Tester<PYXEdgeIterator> gTester;

//! Test method
void PYXEdgeIterator::test()
{
	PYXIcosIndex edgeRoot = "A-0";

	// verify the count for different iterator depths
	TEST_ASSERT(PYXEdgeIterator::calcCellCount(edgeRoot, 6) == 126);
	TEST_ASSERT(PYXEdgeIterator::calcCellCount(edgeRoot, 7) == 126);
	TEST_ASSERT(PYXEdgeIterator::calcCellCount(edgeRoot, 8) == 510);
	TEST_ASSERT(PYXEdgeIterator::calcCellCount(edgeRoot, 9) == 510);

	// a pentagon will have less cells
	edgeRoot = "1-0";
	TEST_ASSERT(PYXEdgeIterator::calcCellCount(edgeRoot, 6) == 105);
	TEST_ASSERT(PYXEdgeIterator::calcCellCount(edgeRoot, 7) == 105);
 	TEST_ASSERT(PYXEdgeIterator::calcCellCount(edgeRoot, 8) == 425);
	TEST_ASSERT(PYXEdgeIterator::calcCellCount(edgeRoot, 9) == 425);

	int nDataRes = 7;
	edgeRoot = "1-0";
	PYXEdgeIterator itEdge(edgeRoot, nDataRes);
	PYXEdgeIterator itOtherEdge(edgeRoot, nDataRes);
	int nTotalCount = PYXDirEdgeIterator::calcCellCount(edgeRoot, nDataRes);

	// cycle through each of the cells
	PYXIcosIndex indexOne;
	PYXIcosIndex indexTwo;
	int nCount;
	for (; !itEdge.end(); itEdge.next())
	{
		nCount = itEdge.calcCurrentOffset();
		indexOne = itEdge.getIndex();
		if (!indexTwo.isNull())
		{
			TEST_ASSERT(indexTwo == indexOne);
		}

		itOtherEdge.setIteratorIndex(indexOne);
		TEST_ASSERT(nCount == itOtherEdge.calcCurrentOffset());
		itOtherEdge.next();
		indexTwo = itOtherEdge.getIndex();
	}

	{
		PYXIcosIndex index("1-20");
		int nResolution = 9;
		PYXEdgeIterator itEdge(index, nResolution);
		int n = 0;
		while (!itEdge.end())
		{
			TEST_ASSERT(!itEdge.end());
			PYXEdgeIterator itEdge2(index, nResolution);
			TEST_ASSERT(itEdge2.setIteratorIndex(itEdge.getIndex()));
			TEST_ASSERT(itEdge.calcCurrentOffset() == n);
			TEST_ASSERT(itEdge2.calcCurrentOffset() == n);
			itEdge.next();
			++n;
		}
		TEST_ASSERT(calcCellCount(index, nResolution) == n);
	}

	{
		PYXIcosIndex index("A-01");
		int nResolution = 9;
		PYXEdgeIterator itEdge(index, nResolution);
		int n = 0;
		while (!itEdge.end())
		{
			TEST_ASSERT(!itEdge.end());
			PYXEdgeIterator itEdge2(index, nResolution);
			TEST_ASSERT(itEdge2.setIteratorIndex(itEdge.getIndex()));
			TEST_ASSERT(itEdge.calcCurrentOffset() == n);
			TEST_ASSERT(itEdge2.calcCurrentOffset() == n);
			itEdge.next();
			++n;
		}
		TEST_ASSERT(calcCellCount(index, nResolution) == n);
	}
}

/*!
Construct an edge iterator with the passed parameters.  If the root index
is not an origin child the resolution of the root will be incremented.

\param rootIndex		The root index of the tile being iterated.
\param nDataResolution	The resolution on which to iterate.
*/
PYXEdgeIterator::PYXEdgeIterator(	const PYXIcosIndex& rootIndex, 
									int nDataResolution	)
{
	// verify passed information
	if (rootIndex.isNull())
	{
		PYXTHROW(PYXModelException, "Null index.");
	}
	
	// edge iterators must have origin child roots
	m_rootIndex = rootIndex;
	if (!m_rootIndex.hasVertexChildren())
	{
		m_rootIndex.incrementResolution();
	}

	m_bComplete = false;
	m_nDataResolution = nDataResolution;
	initDirIterators();
}

/*!
Default Destructor
*/
PYXEdgeIterator::~PYXEdgeIterator()
{
	// free memory
	clear();
}

/*!
Create the sub-iterators for this tesselation.
*/
void PYXEdgeIterator::initDirIterators()
{
	// create an PYXDirEdgeIterator for each vertex child
	try
	{
		// determine type of tesselation that is defined
		if (PYXIcosMath::isDataContained(	m_rootIndex, 
											m_nDataResolution - 
											m_rootIndex.getResolution()	))
		{
			for (	PYXVertexIterator itVertex(m_rootIndex);
					!itVertex.end();
					itVertex.next()	)
			{
				PYXPointer<PYXDirEdgeIterator> spDirIterator(
					PYXDirEdgeIterator::create(	itVertex.getIndex(),
												m_nDataResolution,
												itVertex.getDirection()	)	);
				m_vecIterators.push_back(spDirIterator);
			}
		}
		else
		{
			// get the neighbours of the current root index
			IndexVector vecRoots;
			PYXIcosMath::getNeighbours(m_rootIndex, &vecRoots);

			// create an edge iterator for each
			PYXMath::eHexDirection nDirection;
			for (	IndexVector::iterator itIndex = vecRoots.begin();
					itIndex != vecRoots.end();
					itIndex++	)
			{
				// a null is expected on gap directions of pentagons
				if (!itIndex->isNull())
				{
					if (!PYXIcosMath::areSiblings(	(*itIndex),
													m_rootIndex, 
													&nDirection	))
					{
						PYXTHROW(	PYXModelException,
									"Index '" << *itIndex <<
									"' is not a neighbor of root index '" << m_rootIndex << "'."	);
					}

					PYXPointer<PYXDirEdgeIterator> spDirIterator(
						PYXDirEdgeIterator::create(	(*itIndex),
													m_nDataResolution,
													nDirection	)	);
					m_vecIterators.push_back(spDirIterator);
				}
			}
		}

		// calculate the number of unique values in a sub iterator.
		m_nSubIteratorSize = 
			PYXDirEdgeIterator::calcCellCount(	m_vecIterators[0]->getRoot(),
												m_nDataResolution	);
		
		// set the value for the first iterator index
		m_pyxIndex = m_vecIterators[0]->getIndex();
	}
	catch (...)
	{
		assert(false && "Unable to create sub-iterator for edge iterator.");
	}

	// reset the offset into the sub iterator list
	m_nSubOffset = 0;
}

/*!
Set the state information of the iterator as if the iterator had just been 
created.  Any previous end condition that was set for the iterator will be 
removed.
*/
void PYXEdgeIterator::restartIterator()
{
	// restart the first iterator only
	(*(m_vecIterators.begin()))->restartIterator();
	
	// reset the offset into the iterator vector
	m_nSubOffset = 0;
}

/*! 
This method sets up the node stack to point at a specific index.  If the
passed index does not sit on the edge of the tesselation the iterator state
is set to complete.  Any end condition that was set for the iterator will be 
removed.

\param pyxIndex	The new value for the current position.

\return true if the iterator position was sucessfully set, otherwise false.
*/
bool PYXEdgeIterator::setIteratorIndex(const PYXIcosIndex& pyxIndex)
{
	if (pyxIndex.getResolution() == m_nDataResolution)
	{
		// cycle through each of the sub iterators
		m_nSubOffset = 0;
		for (	SubItVector::iterator itSubIt = m_vecIterators.begin();
				itSubIt != m_vecIterators.end();
				itSubIt++	)
		{
			if ((*itSubIt)->getRoot().isAncestorOf(pyxIndex))
			{
				// attempt to set the index in the sub iterator
				if ((*itSubIt)->setIteratorIndex(pyxIndex))
				{
					m_pyxIndex = pyxIndex;
					m_bComplete = false;
					return true;
				}

				// the index is not valid, exit the for loop
				break;
			}

			m_nSubOffset++;
		}
	}

	// index not found, invalidate the state of the iterator
	m_bComplete = true;
	m_pyxIndex.reset();
	return false;
}

/*! 
Calculate the overall offset of the iterator.

\return The offset from the default start position of the iterator or -1 if 
		the iterator is not properly initialized.
*/
int PYXEdgeIterator::calcCurrentOffset()
{
	if (!m_bComplete)
	{
		// calculate the offset of the current index
		int nOffset = m_vecIterators[m_nSubOffset]->calcCurrentOffset();

		// add up edge iterators that are completed
		if ((nOffset >= 0) && (m_nSubOffset > 0))
		{
			nOffset += (m_nSubIteratorSize * m_nSubOffset);			
		}
		
		return nOffset;		
	}

	return -1;
}

/*! 
Determine the total number of cells that would be touched by an edge iterator
for a given root index and depth.

\param rootIndex		The root index of the tesselation being iterated.
\param nDataResolution	The data resolution of the tesselation being iterated.

\return The number of unique cells on the edge of the tesselation or -1 if the
		cell and depth combination is invalid.
*/
int PYXEdgeIterator::calcCellCount(	const PYXIcosIndex& rootIndex,
									int nDataResolution	)
{
	// calculate the count on an individual sector of the iterator
	int nCount = PYXDirEdgeIterator::calcCellCount(	rootIndex, 
													nDataResolution - 
													PYXIcosIndex::knMinSubRes	);

	if (nCount == -1)
	{
		// If data isn't contained, use the neighbour.
		PYXIcosIndex neighbourIndex;
		neighbourIndex = PYXIcosMath::move(rootIndex, PYXMath::knDirectionTwo);
		nCount = PYXDirEdgeIterator::calcCellCount(	neighbourIndex,
													nDataResolution	);
	}

	if (nCount != -1)
	{
		nCount *= rootIndex.determineNumSides();
	}

	return nCount;
}

/*! 
Acquire the next index in the edge iterator series.  The method updates the
current index.
*/
void PYXEdgeIterator::next()
{
	if (!m_bComplete)
	{
		// increment the current sub iterator
		m_vecIterators[m_nSubOffset]->next();
		if (m_vecIterators[m_nSubOffset]->end())
		{
			// move to the next sub iterator if possible
			m_nSubOffset++;
			if (m_rootIndex.determineNumSides() <= m_nSubOffset)
			{
				// no more sub iterator, the end has been reached
				m_bComplete = true;
				m_pyxIndex.reset();
				return;
			}

			// the restart the next sub iterator
			m_vecIterators[m_nSubOffset]->restartIterator();
		}

		// store the current index
		m_pyxIndex = m_vecIterators[m_nSubOffset]->getIndex();
	}
}

/*! 
Return the current index pointed to by the iterator.  If the iterator is not
properly initialized or has reached the end of the iteration, a null index 
is returned.

\return The current index.
*/
const PYXIcosIndex& PYXEdgeIterator::getIndex() const
{	
	return m_pyxIndex;
}

/*! 
Determine if all of the cells have been covered.

\return true if the iteration is complete, otherwise false.
*/
bool PYXEdgeIterator::end() const
{
	return m_bComplete;
}

/*! 
Delete the state information and free associated memory.  Set the iterator
state to 'complete'.
*/
void PYXEdgeIterator::clear()
{
	// remove the pointers from the vector
	m_vecIterators.clear();
}
