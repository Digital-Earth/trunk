#ifndef PYXIS__DERM__DIR_EDGE_ITERATOR_H
#define PYXIS__DERM__DIR_EDGE_ITERATOR_H
/******************************************************************************
dir_edge_iterator.h

begin		: 2004-02-14
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/iterator.h"
#include "pyxis/derm/sub_index_math.h"

/*!
This class iterates about the edge of a single sector of a contained data 
resolution (one sixth the edge of a standard tesselation).  The iteration 
moves in a counter clockwise direction with respect to the overall tesselation.

ex:		Root value = A-01, Depth = 4
		A-010505, A-010506, A-010501, ..., A-010303.

The default end condition of the iterator is when all of the cells for the 
sector have been visited once.
*/
//! Iterate over the edge of a single sector of a 'contained' tesselation.
class PYXLIB_DECL PYXDirEdgeIterator : public PYXIterator
{
public:

	//! Test method
	static void test();

	//! Types of nodes.
	enum eIndexType 
	{ 
		//! The first node in the list (is also a knFull type)
		knStart = 0,

		//! Has 5 children on the edge
		knFull, 

		//! Counts up to the parent direction
		knPartCCW,

		//! Counts down to the parent direction
		knPartCW
	};

	//! Dynamic creator
	static PYXPointer<PYXDirEdgeIterator> create(
		const PYXIcosIndex& rootIndex, 
		int nDataResolution,
		PYXMath::eHexDirection nDirection	)
	{
		return PYXNEW(PYXDirEdgeIterator, rootIndex, nDataResolution, nDirection);
	}

	//! Create a new iterator.
	PYXDirEdgeIterator(	const PYXIcosIndex& rootIndex, 
						int nDataResolution,
						PYXMath::eHexDirection nDirection	);

	//! Destructor.
	virtual ~PYXDirEdgeIterator();

	//! Reset the state of the current iterator using the current values.
	void restartIterator();

	//! Set the index of the iterator to a specific edge index value.
	bool setIteratorIndex(const PYXIcosIndex& pyxIndex);

	//! Move to the next cell.
	virtual void next();

	//! Returns true when all indices have been processed.
	bool end() const;
	
	//! Get the PYXIS index for the current cell.
	virtual const PYXIcosIndex& getIndex() const;

	//! Determine the offset of the current index in the iteration.
	int calcCurrentOffset();

	//! Calculate the number of cells on the edge of a tesselation.
	static int calcCellCount(	const PYXIcosIndex& rootIndex,
								int nDataResolution	);

	//! Return the root index for the iterator
	inline PYXIcosIndex getRoot() {return m_rootIndex;};

private:

	//! Disable default constructor.
	PYXDirEdgeIterator();

	//! Disable copy constructor.
	PYXDirEdgeIterator(const PYXDirEdgeIterator&);

	//! Disable copy assignment.
	void operator=(const PYXDirEdgeIterator&);

	//! Initialize the stack with correct information for the nodes.
	void initializeNodes(PYXMath::eHexDirection nDirection);

	//! Remove the state information from the iterator and free memory.
	void clear();

	//! Calculate the number of cells on the edge of a tesselation
	static int calcCellCount(	int nFullCount,
								int nPartCount,
								int nDepth	);

	//! The root index for the tesselation.
	PYXIcosIndex m_rootIndex;

	//! The absolute resolution around which to iterate.
	int m_nDataResolution;

	//! Set to true when the iteration cycle is complete.
	bool m_bComplete;

	//! The current index for the iteration.
	PYXIcosIndex m_pyxIndex;

	//! Represents the state of a single node in the calling hierarchy.
	/*!
	This class stores the information specific to maintaining 
	the position of a PYXEdgeIterator
	*/
	class StateNode
	{		
	public:	

		//! Default constructor.
		StateNode(	const PYXIcosIndex& pyxIndex,
					eIndexType nType,
					PYXMath::eHexDirection nDirection	);

		//! Default destructor.
		virtual ~StateNode() {};

		//! Initialize a node with information.
		void setNode(	
			const PYXIcosIndex& pyxIndex, 
			eIndexType nType,
			PYXMath::eHexDirection nDirection	);

		//! Determine the current index value of a node.
		void getIndex(PYXIcosIndex* pIndex);

		//! Determine the type of node that will be created by current index.
		eIndexType getChildType();

		//! Increase the iteration for this node
		bool increment();

		//! Retrieve direction with which to create the next node.
		PYXMath::eHexDirection getCurrentDirection();

		//! Determine the resolution of the indices produced by this node.
		inline int getChildResolution() 
		{
			return (m_nodeRootIndex.getResolution() + 1);
		};

		//! Retrieve the root index for this node.
		inline const PYXIcosIndex& getRoot() {return m_nodeRootIndex;};

		//! Retrieve the type of node.
		inline eIndexType getType() {return m_nNodeType;};

		//! Retrieve the count of the node
		inline int getCount() {return m_nPos;};

		//! Resets the count of the node to 0
		void resetCount();

	private:
		
		//! Disable default constructor.
		StateNode();
		
		//! Disable copy constructor.
		StateNode(const StateNode&);

		//! Disable copy assignment.
		void operator=(const StateNode&);

		//! The root index of this node (resolution above the node data).
		PYXIcosIndex m_nodeRootIndex;
		
		//! The direction passed down from the parent node.
		PYXMath::eHexDirection m_nDirection;

		//! The type of node.
		eIndexType	m_nNodeType;

		//! The sequential count of the node.
		int m_nPos;
	};

	// TODO: Consider making this a boost::ptr_vector for safety.
	//! Vector of StateNodes, one for each resolution of the tesselation.
	typedef std::vector <StateNode*> NodeVector;

	//! Vector of node information for the iterator.
	NodeVector m_vecNodes;
};

#endif // guard
