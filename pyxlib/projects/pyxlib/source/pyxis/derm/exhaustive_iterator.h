#ifndef PYXIS__DERM__EXHAUSTIVE_ITERATOR_H
#define PYXIS__DERM__EXHAUSTIVE_ITERATOR_H
/******************************************************************************
exhaustive_iterator.h

begin		: 2003-12-17
copyright	: (C) 2003 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/iterator.h"
#include "pyxis/derm/sub_index_math.h"

// standard includes

/*!
The PYXIterator iterates through all the cells with the given root index
at the specified resolution. The iterator proceeds in a well defined order
based on the two resolutions lower than the current resolution. This results in
the following pattern:

\verbatim
Resolution 0: 0
Resolution 1: 00, 01, 02, 03, 04, 05, 06
Resolution 2: 000, 001, 002, 003, 004, 005, 006, (Resolution 1 with 0 prepended)
                   010, 020, 030, 040, 050, 060  (Resolution 0 with 01->06 prepended)
...
Resolution n: Resolution n-1 with 0 prepended,
			  Resolution n-2 with 01 prepended,
			  Resolution n-2 with 02 prepended,
			  ...
			  Resolution n-2 with 06 prepended
\endverbatim
*/
//! Iterates through all the cells at a given resolution.
class PYXLIB_DECL PYXExhaustiveIterator : public PYXIterator
{
public:

	//! Dynamic creator
	static PYXPointer<PYXExhaustiveIterator> create(
		const PYXIcosIndex& rootIndex,
		int nResolution	)
	{
		return PYXNEW(PYXExhaustiveIterator, rootIndex, nResolution);
	}

	//! Test method
	static void test();

	//! PYXIS Errors

	//! Constructor
	PYXExhaustiveIterator(	const PYXIcosIndex& rootIndex,
							int nResolution	);

	//! Destructor
	virtual ~PYXExhaustiveIterator();

	//! Move to the next cell
	virtual void next();

	//! See if we have covered all the cells
	virtual bool end() const {return m_index.isNull();}

	//! Set the iterator to the end condition.
	void setEnd() {m_index = PYXIcosIndex();}

	//! Get the PYXIS index for the current cell.
	virtual const PYXIcosIndex& getIndex() const {return m_index;}

	//! Reset to the specified root index and resolution.
	void reset(const PYXIcosIndex& rootIndex, int nResolution);

private:

	//! Disable copy constructor
	PYXExhaustiveIterator(const PYXExhaustiveIterator&);

	//! Disable copy assignment
	void operator=(const PYXExhaustiveIterator&);

	//! A recursive method for testing the PYXIS iterator.
	static void testIteratorRecursive(	const PYXIcosIndex& index,
										int nDepth,
										PYXExhaustiveIterator& it	);

	//! Initialize for the specified resolution.
	void initForResolution(int nResolution);

	//! The root PYXIS index
	PYXIcosIndex m_rootIndex;

	//! The current PYXIS index
	PYXIcosIndex m_index;

	//! Is the root index a vertex child
	bool m_bRootIsVertexChild;

	//! The cell gap is or is not a pentagon
	bool m_bIsPentagon;

	//! The cell gap for the root index
	PYXMath::eHexDirection m_nCellGap;

	//! Pointers used in generating the indices
	char* m_pszMax;
	char *m_pszMin;
	char *m_pszLeading;
};

#endif // guard
