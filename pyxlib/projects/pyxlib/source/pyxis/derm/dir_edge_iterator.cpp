/******************************************************************************
dir_edge_iterator.cpp

begin		: 2004-02-14
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/dir_edge_iterator.h"

// pyxlib includes
#include "pyxis/derm/exceptions.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/utility/tester.h"

//! The default starting count value for all of the nodes
static const int knInitCount = 0;

//! The amount to rotate the first value on a knFull
static const int knFullRotate = -2;

//! The maximum count of a knFull
static const int knFullMax = 4; 

//! The amount to rotate the first value on a knPartCCW node
static const int knPartCCWRotate = 0;

//! The amount to rotate the first value on a knPartCW node
static const int knPartCWRotate = -2;

//! The maximum count for either of the knPart types
static const int knPartMax = 2;

//! The the difference in resolution between nodes 
static const int knNodeDiff = 2;

//! Minimum tile depth for a PYXDirEdgeIterator
static const int knMinDirEdgeIteratorTileDepth = 2;

//! Tester class
Tester<PYXDirEdgeIterator> gTester;

//! Test method
void PYXDirEdgeIterator::test()
{
	// Create an iterator and step over ever point
	PYXIcosIndex edgeRoot = "A-0";
	int nDataRes = 5;
	PYXDirEdgeIterator itEdge(	edgeRoot, 
								nDataRes, 
								PYXMath::knDirectionOne	);
	PYXDirEdgeIterator itOtherEdge(	edgeRoot, 
									nDataRes, 
									PYXMath::knDirectionOne	);

	int nTotalCount = PYXDirEdgeIterator::calcCellCount(edgeRoot, nDataRes);
	TEST_ASSERT(nTotalCount == 21);

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
}

/*!
Construct an edge iterator with the passed parameters.

\param rootIndex		The root index of the tile sector being iterated.
\param nDataResolution	The resolution on which to iterate.
\param nDirection		The starting direction for the iterator
*/
PYXDirEdgeIterator::PYXDirEdgeIterator(	const PYXIcosIndex& rootIndex, 
										int nDataResolution,
										PYXMath::eHexDirection nDirection	)
{
	// determine if there is a valid index and resolution
	if (!rootIndex.isNull())
	{
		m_rootIndex = rootIndex;

		try 
		{
			int nDepth = nDataResolution - rootIndex.getResolution();
			if (	(nDepth >= knMinDirEdgeIteratorTileDepth) &&
					(nDataResolution <= PYXMath::knMaxAbsResolution)	)
			{
				m_nDataResolution = nDataResolution;

				// verify the data resolution is a contained data set
				if (!PYXIcosMath::isDataContained(m_rootIndex, nDepth))
				{
					PYXTHROW(	PYXModelException,
								"Invalid depth of '" << nDepth <<
								"' result in uncontained data resolution for root '" <<
								m_rootIndex << "'."	);
				}

				// initialize the node stack
				initializeNodes(nDirection);

				// set the iterator state
				m_bComplete = false;
			}
			else
			{
				PYXTHROW(	PYXModelException,
							"Invalid resolution of '" << m_rootIndex.getResolution() <<
							"' with depth of '" << nDepth << "' for edge iterator."	);
			}
		}
		catch (...)
		{
			assert(false && "Error while processing the edge iterator.");
		}
	}
	else
	{
		PYXTHROW(PYXModelException, "Null index.");
	}
}

/*!
Default Destructor
*/
PYXDirEdgeIterator::~PYXDirEdgeIterator()
{
	// free memory
	clear();
}

/*!
Set the state information of the iterator as if the iterator had just been 
created.  Any previous end condition that was set for the iterator will be 
removed.
*/
void PYXDirEdgeIterator::restartIterator()
{
	// reset the count on the root node
	PYXDirEdgeIterator::NodeVector::iterator itNode = m_vecNodes.begin();
	(*itNode)->resetCount();

	PYXIcosIndex pyxIndex;
	while (*itNode != m_vecNodes.back())
	{ 
		// use the current vector node to initialize the next
		StateNode* pNode = *itNode;
		itNode++;
		pNode->getIndex(&pyxIndex);
		(*itNode)->setNode(	pyxIndex, 
							pNode->getChildType(), 
							pNode->getCurrentDirection()	);
	}
	
	// re-initialize the state variables
	m_bComplete = false;
	m_vecNodes.back()->getIndex(&m_pyxIndex);
}

/*! 
This method sets up the node stack to point at a specific index.  If the
passed index does not sit on the edge of the tesselation the iterator is
state is set to complete.  Any end condition that was set for the iterator 
will be removed.

\param pyxIndex	The new value for the current position.

\return true if the iterator position was sucessfully set, otherwise false.
*/
bool PYXDirEdgeIterator::setIteratorIndex(const PYXIcosIndex& pyxIndex)
{
	// verify the passed index is valid
	PYXDirEdgeIterator::NodeVector::iterator itNode = m_vecNodes.begin();
	m_bComplete = false;
	if (	pyxIndex.getResolution() == m_nDataResolution	&&
			(*itNode)->getRoot().isAncestorOf(pyxIndex)	)
	{
		// get the relative resolution from the root.
		PYXIndex relIndex = PYXIcosMath::calcDescendantIndex(	
														(*itNode)->getRoot(),
														pyxIndex	);

		// verify it is a contained data resolution (assumes origin child root)
		if (relIndex.getResolution() % 2 == 0)
		{
			// process each of the nodes in the vector
			PYXIcosIndex tempNodeIndex = (*itNode)->getRoot();
			int nDirection;

			// reset the root node before we loop
			while (itNode != m_vecNodes.end())
			{
				// strip the next digit pair
				nDirection = relIndex.stripLeft();
				if (	(0 == nDirection) ||
						(!relIndex.isNull()) && (0 != relIndex.stripLeft())	)
				{
						// the index is not on the edge
						m_bComplete = true;
						break;
				}

				// build the index we are looking for
				PYXIcosMath::zoomIntoNeighbourhood(	
							&tempNodeIndex, 
							static_cast<PYXMath::eHexDirection>(nDirection)	);

				// determine if the index can be created by the current root
				(*itNode)->resetCount();
				PYXIcosIndex pyxIndex;
				(*itNode)->getIndex(&pyxIndex);
				while (	tempNodeIndex != pyxIndex &&
						(*itNode)->increment()	)
				{
					(*itNode)->getIndex(&pyxIndex);
				}

				if (tempNodeIndex == pyxIndex)
				{
					// set the values for the next node if applicable
					StateNode* pNode = *itNode;
					itNode++;
					if (itNode != m_vecNodes.end())
					{
						(*itNode)->setNode(	tempNodeIndex,
											pNode->getChildType(),
											pNode->getCurrentDirection()	);
					}

					// move on to the next node
					tempNodeIndex.incrementResolution();
				}
				else
				{
					// the index is not on the edge
					m_bComplete = true;
					break;
				}
			}
		}
		else
		{
			m_bComplete = true;
		}
	}
	else
	{
		m_bComplete = true;
	}
	
	// if this code is reached, the passed value was not an edge index
	if (m_bComplete)
	{
		m_pyxIndex.reset();
		return false;
	}
	else
	{
		m_vecNodes.back()->getIndex(&m_pyxIndex);
		return true;
	}
}

/*! 
Every time this method is called the current state of the iterator will be
examined and the offset of the iterator will be calculated.

\return The offset of the current value from the start of a standard iterator
		or -1 if the iterator is not properly initialized.
*/
int PYXDirEdgeIterator::calcCurrentOffset()
{
	// verify the state is valid
	if (m_bComplete)
	{
		return -1;
	}

	// leave the last node out of the depth count since it is not recursive
	size_t nDepth = (m_vecNodes.size() - 1) * knNodeDiff;
	int nCount = 0;
	NodeVector::iterator itNode =  m_vecNodes.begin();

	// count recursively
	while ((nDepth > 0) && (*itNode))
	{
		int nFullCount = 0;
		int nPartCount = 0;

		switch ((*itNode)->getCount())
		{
		case 4:
			nFullCount++;

			// deliberate fall through to next count
		case 3:
			
			if ((*itNode)->getType() <= knFull)
			{
				// if the type is knFull or knStart	
				nFullCount++;
			}
			else
			{
				nPartCount++;
			}

			// deliberate fall through to next count

		case 2:	

			// count of 2 is always a knFull value
			nFullCount++;

			// deliberate fall through to next count

		case 1:			

			// count of 1 is always a knPart value
			nPartCount++;
			break;
			
		case 0:

			// have no complete children so just move on to next node
			break;
		
		default:
			assert(false && "Invalid node count.");
			break;
		}

		// calculate and add the count to the running total 
		nCount += calcCellCount(	nFullCount, 
									nPartCount, 
									static_cast<int>(nDepth)	);

		// decrement the resolution depth by 2 for each node
		nDepth -= knNodeDiff;
		itNode++;
	}

	// add the end node value
	nCount += (*itNode)->getCount();
	return nCount;
}

/*! 
Determine the total number of cells that would be touched by an edge iterator
for a given root index and depth.

\param rootIndex		The root index of the tesselation being iterated.
\param nDataResolution	The resolution of the tesselation being iterated.

\return The number of unique cells on the edge of the tesselation or -1 if the
		cell and depth combination is invalid.
*/
int PYXDirEdgeIterator::calcCellCount(	const PYXIcosIndex& rootIndex,
										int nDataResolution	)
{
	int nDepth = nDataResolution - rootIndex.getResolution();

	if (!PYXIcosMath::isDataContained(rootIndex, nDepth))
	{
		return -1;
	}

	// calculate the number of indices along the edge of a single sector
	return calcCellCount(1, 0, nDepth);
}

/*! 
Calculate the number of cells on the edge of a tesselation given the 
starting counts of partial and full cells.

\param nFullCount	The number of knFull cells to calculate for the depth
\param nPartCount	The number of knPartCW and knPartCCW cells to calculate
\param nDepth		The number of resolutions down to calculate (nodes * 2)

\return The number of indices below the specified nodes at the specified depth.
*/
int PYXDirEdgeIterator::calcCellCount(	int nFullCount,
										int nPartCount,
										int nDepth	)
{
	// cycle through each of the layers of nodes incrementing the counts
	int nTempCount;
	for (; nDepth > 0; nDepth -= knNodeDiff)
	{
		nTempCount = (3 * nFullCount) + nPartCount;
		nPartCount = (2 * nPartCount) + (2 * nFullCount);
		nFullCount = nTempCount;
	}
	return nFullCount + nPartCount;	
}									

/*! 
Acquire the next index in the edge iterator series.  The method updates the
current index.
*/
void PYXDirEdgeIterator::next()
{
	if (!m_bComplete)
	{
		assert( !m_vecNodes.empty() && "Can't be empty");
		
		// start with the end node and work back to the beginning.
		NodeVector::iterator itNode =  m_vecNodes.end() - 1;
		StateNode* pNode;

		while (!(*itNode)->increment())
		{
			// move to the previous element in the vector
			if (itNode != m_vecNodes.begin())
			{
				itNode--;
			}
			else
			{
				// set the state to complete
				m_pyxIndex.reset();
				m_bComplete = true;
				return;
			}
		}

		PYXIcosIndex pyxIndex;
		while (*itNode != m_vecNodes.back())
		{ 
			// use the current vector node to initialize the next
			pNode = *itNode;
			itNode++;
			pNode->getIndex(&pyxIndex);
			(*itNode)->setNode(	pyxIndex, 
								pNode->getChildType(), 
								pNode->getCurrentDirection()	);
		}

		// calculate the current index
		m_vecNodes.back()->getIndex(&m_pyxIndex);
	}
}

/*! 
Return the current index pointed to by the iterator.  If the iterator is not
properly initialized or has reached the end of the iteration, a null index 
is returned.

\return The current index.
*/
const PYXIcosIndex& PYXDirEdgeIterator::getIndex() const
{	
	return m_pyxIndex;
}

/*! 
Determine if all of the cells have been covered.

\return true if the iteration is complete, otherwise false.
*/
bool PYXDirEdgeIterator::end() const
{
	return m_bComplete;
}

/*! 
Initialize the stack with correct information for the nodes.

\param nDirection	The direction value for the first node in the list.
*/
void PYXDirEdgeIterator::initializeNodes(PYXMath::eHexDirection nDirection)
{
	// remove any existing nodes 
	clear();

	// create the root node
	PYXDirEdgeIterator::StateNode* pNode;
	// TODO: Exception safety.    Consider replacing m_vecNodes with boost::ptr_vector.
	pNode = new PYXDirEdgeIterator::StateNode(m_rootIndex, knStart, nDirection);
	m_vecNodes.push_back(pNode);

	// continue creating nodes until the data resolution is reached
	PYXIcosIndex pyxIndex;
	while (pNode->getChildResolution() < m_nDataResolution)
	{
		// use the information from the last node to create the new one
		pNode->getIndex(&pyxIndex);
		// TODO: Exception safety.  Consider replacing m_vecNodes with boost::ptr_vector.
		pNode = new PYXDirEdgeIterator::StateNode(	pyxIndex, 
													pNode->getChildType(),
													pNode->getCurrentDirection()	);

		// put the newly created node in the vector
		m_vecNodes.push_back(pNode);
	}

	// initialize the first index value
	m_vecNodes.back()->getIndex(&m_pyxIndex);
}

/*! 
Delete the state information and free associated memory.  Set the iterator
state to 'complete'.
*/
void PYXDirEdgeIterator::clear()
{
	// set the appropriate member variables
	m_pyxIndex.reset();
	m_bComplete = true;

	// free memory associated with the nodes
	NodeVector::iterator itNode = m_vecNodes.begin();
	for (; itNode != m_vecNodes.end(); ++itNode)
	{
		delete(*itNode);
	}

	// empty the vector
	m_vecNodes.clear();
}

/*!
The constructor sets the initial state of the node in the same manner as
the setNode method.

\param pyxIndex		The index that created this node.
\param nType		The type of node that is to be created.
\param nDirection	The root direction of the new node. (default knDirectionone)

\sa PYXMath::eHexDirection
*/
PYXDirEdgeIterator::StateNode::StateNode(	const PYXIcosIndex& pyxIndex,
											eIndexType nType,
											PYXMath::eHexDirection nDirection	)
{
	setNode(pyxIndex, nType, nDirection);
}

/*! 
Initialize a node with information.

\param pyxIndex		The index that created this node.
\param nType		The type of node that is to be created.
\param nDirection	The root direction of the new node.
*/
void PYXDirEdgeIterator::StateNode::setNode(	const PYXIcosIndex& pyxIndex,
												eIndexType nType,
												PYXMath::eHexDirection nDirection	)
{
	// a null index is invalid
	if (pyxIndex.isNull())
	{
		PYXTHROW(PYXModelException, "Null index.");
	}

	// a zero direction should never be passed
	if (nDirection == PYXMath::knDirectionZero)
	{
		PYXTHROW(PYXModelException, "Invalid direction: '" << nDirection << "'."	);
	}

	// the type and direction are a direct copy
	m_nNodeType = nType;
	m_nDirection = nDirection;
	m_nodeRootIndex = pyxIndex;

	if (nType == knStart)
	{
		// the root value of a node must be an origin child hexagon
		if (!pyxIndex.hasVertexChildren())
		{
			m_nodeRootIndex.incrementResolution();
		}
	}
	else
	{
		// increment the resolution to simplify later calculations.
		m_nodeRootIndex.incrementResolution();
	}

	// initialize the start count
	m_nPos = knInitCount;

}

/*! 
By examining the current state of the nodes this method determines which
index it currently references.

\param	pIndex	The current index of the node (out).
*/
void PYXDirEdgeIterator::StateNode::getIndex(PYXIcosIndex* pIndex)
{
	*pIndex = m_nodeRootIndex;
	PYXMath::eHexDirection nZoomDirection;

	switch (m_nNodeType)
	{
	// This node contains 5 child indices.
	case knStart:
		// deliberate fall through

	case knFull:
		nZoomDirection = PYXMath::rotateDirection(	
										m_nDirection, 
										m_nPos + knFullRotate	);
		break;

	case knPartCCW:
		nZoomDirection = PYXMath::rotateDirection(	
										m_nDirection, 
										m_nPos + knPartCCWRotate	);
		break;

	case knPartCW:
		nZoomDirection = PYXMath::rotateDirection(	
										m_nDirection, 
										m_nPos + knPartCWRotate	);
		break;

	default:
		PYXTHROW(PYXException, "Invalid node type.");
		break;
	}

	PYXIcosMath::zoomIntoNeighbourhood(pIndex, nZoomDirection);
}

/*! 
Retrieve the least significant digit of the current index of a node

\return The direction that the subsequent node should be created with.
*/
PYXMath::eHexDirection PYXDirEdgeIterator::StateNode::getCurrentDirection()
{
	// return the least significant digit of the current index
	PYXIcosIndex pyxIndex;
	getIndex(&pyxIndex);
	PYXMath::eHexDirection nDirection = PYXMath::knDirectionZero;
	PYXIcosMath::directionFromParent(pyxIndex, &nDirection);
	return nDirection;
}

/*! 
This method determines what type of child node is created for the 
current state and index value of this node.

\return The type of child that would be created by this node
*/
PYXDirEdgeIterator::eIndexType PYXDirEdgeIterator::StateNode::getChildType()
{
	switch (m_nNodeType)
	{
	case knStart:
		// deliberate fall through to type knFull

	// the first and last value are knPartxx types and the rest are knFull
	case knFull:
		if (knInitCount == m_nPos)
		{
			return knPartCCW;
		}
		else if (knFullMax == m_nPos)
		{
			return knPartCW;
		}
		else
		{
			return knFull;
		}

		break;

	// these types reproduce themselves in the same pattern
	case knPartCCW:
	case knPartCW:
		if (knInitCount == m_nPos)
		{
			return knPartCCW;
		}
		else if (knPartMax == m_nPos)
		{
			return knPartCW;
		}
		else
		{
			return knFull;
		}
		break;

	default:
		assert(false && "Invalid node type.");
		break;
	}

	return knFull;
}

/*! 
Increase the iteration for this node verifying the maximum count for the node 
type has not been exceeded.

\return	true if the iteration was successful or false if the node was at its
		maximum count.
*/
bool PYXDirEdgeIterator::StateNode::increment()
{	
	bool bValidCount = false;

	// Verify the incremented value will be a valid direction.
	switch (m_nNodeType)
	{
	case knStart:
		// deliberate fall through

	case knFull:
		if (m_nPos < knFullMax)
		{
			bValidCount = true;
			m_nPos++;
		}
		break;

	case knPartCCW:
	case knPartCW:
		if (m_nPos < knPartMax)
		{
			bValidCount = true;
			m_nPos++;
		}
		break;

	default:
		assert(false && "Invalid node type.");
		break;
	}

	return bValidCount;
}

/*! 
Resets the count of the node to 0 for standard nodes and an appropriate
value for the start nodes.
*/
void PYXDirEdgeIterator::StateNode::resetCount()
{
	m_nPos = 0;
}
