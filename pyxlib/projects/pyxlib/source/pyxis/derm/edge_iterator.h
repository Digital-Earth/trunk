#ifndef PYXIS__DERM__EDGE_ITERATOR_H
#define PYXIS__DERM__EDGE_ITERATOR_H
/******************************************************************************
edge_iterator.h

begin		: 2004-11-10
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/iterator.h"

// standard includes
#include <vector>

// forward declarations
class PYXDirEdgeIterator;

/*!
A collection of directional edge iterators that is assembled to iterate about
a defined tesselation.  
If a contained tesselation is defined by the root index and depth, the six (or 
five if a pentagon) directional edge iterators for that tesselation makes up 
the overall edge iterator.  
If the defined root and depth indicate an uncontained resolution edge iterators
on the six (or five) neighbouring tesselations that border the defined 
tesselation makes up the overall edge iterator.
*/
//!	A collection of directional edge iterators that iterate about a tesselation.
class PYXLIB_DECL PYXEdgeIterator : public PYXIterator
{
public:

	//! Test method
	static void test();

	//! Dynamic creator
	static PYXPointer<PYXEdgeIterator> create(const PYXIcosIndex& rootIndex, int nDataResolution)
	{
		return PYXNEW(PYXEdgeIterator, rootIndex, nDataResolution);
	}

	//! Create a new iterator.
	PYXEdgeIterator(const PYXIcosIndex& rootIndex, int nDataResolution);

	//! Reset the state of the current iterator using the current values.
	void restartIterator();

	//! Set the index of the iterator to a specific edge index value.
	bool setIteratorIndex(const PYXIcosIndex& pyxIndex);

	//! Default destructor.
	virtual ~PYXEdgeIterator();

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
	inline PYXIcosIndex getRoot() {return m_rootIndex;}

private:

	//! Disable default constructor.
	PYXEdgeIterator();

	//! Disable copy constructor.
	PYXEdgeIterator(const PYXEdgeIterator&);

	//! Disable copy assignment.
	void operator=(const PYXEdgeIterator&);

	//! Create the sub-iterators for this tesselation.
	void initDirIterators();

	//! Remove the state information from the iterator and free memory.
	void clear();

	//! The root index for the tesselation.
	PYXIcosIndex m_rootIndex;

	//! The absolute resolution around which to iterate.
	int m_nDataResolution;

	//! The current index for the iteration.
	PYXIcosIndex m_pyxIndex;

	//! Set to true when the iteration cycle is complete.
	bool m_bComplete;

	//! The offset of the current sub iterator in the vector
	int m_nSubOffset;

	//! The maximum count of a sub iterator
	int m_nSubIteratorSize;

	//! Vector of PYXIcosIndex values.
	typedef std::vector<PYXIcosIndex> IndexVector;

	//! Vector of NodeStates, one for each resolution of the tesselation.
	typedef std::vector< PYXPointer<PYXDirEdgeIterator> > SubItVector;

	//! Vector of node information for the iterator.
	SubItVector m_vecIterators;
};

#endif // guard
