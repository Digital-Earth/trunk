#ifndef PYXIS__DERM__PROGRESSIVE_ITERATOR_H
#define PYXIS__DERM__PROGRESSIVE_ITERATOR_H
/******************************************************************************
progressive_iterator.h

begin		: 2006-03-07
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/iterator.h"
#include "pyxis/derm/exhaustive_iterator.h"

/*!
A PYXProgressiveIterator iterates through all cells with the given root index in
breadth-first order, enumerating first the highest-order (leftmost) sub-index digit,
then the next-highest, and so on to the lowest-order (rightmost) digit.

PYXProgressiveIterator delegates to an internal PYXExhaustiveIterator which does
all the actual iteration work; hence PYXProgressiveIterator does not have to know
many internal details of the PYXIS system.

Given the PYXIS root index RI and ultimate resolution R, PYXProgressiveIterator
begins by computing the depth MD = R - [resolution of RI].  MD (max. depth) is also
the number of digits which must be enumerated.  The sequence of PYXIS indices
enumerated by the iterator is as follows:

\verbatim
special case D=0: generate root index RI
for D = 1 to MD
	run a depth-D PYXExhaustiveIterator (root index RI, depth D), and:
		if least-significant digit of index is nonzero (i.e., vertex child)
		then generate the index
		else suppress this index and continue
\endverbatim

Constructor options are provided to omit the nonzero-digit filtering (such that
centroid-child indices are included in the enumeration) and to extend all output
indices to the maximum depth MD.  The constructor will throw a PYXIndexException
if given a null root index RI.
*/
//! Iterates through all cells at a progressively higher depths.
class PYXLIB_DECL PYXProgressiveIterator : public PYXIterator
{
public:

	//! Unit test method
	static void test();

	//! Defines a number of modifiers that can modify the operation of the iterator.
	enum eIteratorFlags
	{ 	
		//! Don't repeat children, output at regular resolution.
		knDefaultBehaviour = 0,

		//! Centroid children are included.  Thus, "00" will follow "0".
		knRepeatChildren = 1,						
	
		//! When printing a non-leaf node, zero-extend to the full resolution.
		knExtendOutputToFullResolution = 2, 

		//! Repeats children, and extends.
		knRepeatAndExtend = (knRepeatChildren | knExtendOutputToFullResolution)
	};

	//! Constructor
	PYXProgressiveIterator(	const PYXIcosIndex& rootIndex,
							int nResolution,
							eIteratorFlags nFlags = knDefaultBehaviour);

	//! Destructor.
	virtual ~PYXProgressiveIterator() {;}

	//! Reset to the specified root index and resolution.
	void reset(	const PYXIcosIndex& rootIndex,
				int nResolution	);
	
	//! Return true if all cells have been covered, otherwise false.
	virtual bool end() const { return m_bAtEnd; }

	//! Move to the next cell
	virtual void next();

	//! Get the PYXIcosIndex for the current cell.
	virtual const PYXIcosIndex& getIndex() const { return m_index; }

	//! Get the current iteration depth
	int getCurrentDepth() const { return m_nDepth; }

private:

	//! Disable copy constructor
	PYXProgressiveIterator(const PYXProgressiveIterator&);

	//! Disable copy assignment
	void operator =(const PYXProgressiveIterator&);

	//! The root PYXIS index
	PYXIcosIndex m_rootIndex;

	//! A PYXExhaustiveIterator we use internally
	PYXExhaustiveIterator m_iterator;

	//! Resolution of root index
	int m_nRootRes;

    //! The iterator will continue until all children of this depth have 
	//! been traversed.  
	int m_nMaxRes;

	//! Difference between m_nMaxRes and m_nRootRes
	int m_nMaxDepth;

	//! Current depth: begins at 0 and proc
	int m_nDepth;

    //! Centroid children are included.  Thus, "00" will follow "0".
	const bool m_bRepeatChildren; 

	//! When printing a non-leaf node, zero-extend to the full resolution.
	//! Thus, "0" in a res-3 iterator becomes "000".
	const bool m_bExtendResolution; 

	//! The index which gets returned
	PYXIcosIndex m_index;

	//! Flag returned by end()
	bool m_bAtEnd;
};

#endif	//guard
