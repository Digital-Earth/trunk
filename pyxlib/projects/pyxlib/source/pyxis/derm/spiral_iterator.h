#ifndef PYXIS__DERM__SPIRAL_ITERATOR_H
#define PYXIS__DERM__SPIRAL_ITERATOR_H
/******************************************************************************
spiral_iterator.h

begin		: 2003-12-17
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyxis/derm/cursor.h"
#include "pyxis/derm/iterator.h"

// standard includes
#include <vector>

/*!
Starting from a center data cell, the iterator visits the cells that make
up the tile, in an outward spiral pattern.

When the Hexagonal Class (I or II) of the root index and
data cells are different, the spiral iteration follows the form of
a hexagon of the same class as the hexagon representing the root index.
Its last spiral follows the edges of the hexagon that represents
the root index. The iterator will not visit outside the
bounds of the root index hexagon.

When the Hexagonal Class of the root index and data cells
are the same, the spiral iteration will extend outside the hexagonal
bounds of the root index, in order to ensure that it visits all
of the data cells.

TODO: fix the SpiralIterator for the case when the Hexagonal Classes of
the root index and data cells are the same, so that it visits the
data cells in a pattern that closely follows the root index representation
(ie: not visiting extra cells).

*/
//! Iterate outwards in a spiraling hexagon pattern to fill a parent hexagon.
class PYXLIB_DECL PYXSpiralIterator : public PYXIterator
{
public:

	//! Spiral Iterator Constant
	static const int knMaxSpiralingDepth;

	//! Dynamic creator
	static PYXPointer<PYXSpiralIterator> create(
		const PYXIcosIndex& rootIndex,
		int nDepth,
		int nExtraSpirals = 0)
	{
		return PYXNEW(PYXSpiralIterator, rootIndex, nDepth, nExtraSpirals);
	}

	//! Test method
	static void test();

	//! Constructor
	PYXSpiralIterator(	const PYXIcosIndex& rootIndex,
						int nDepth,
						int nExtraSpirals =  0	);

	//! Destructor
	virtual ~PYXSpiralIterator();

	//! Move to the next cell
	virtual void next();

	//! See if we have covered all the cells
	virtual bool end() const {return m_index.isNull();}

	//! Get the PYXIS index for the current cell.
	virtual const PYXIcosIndex& getIndex() const {return m_spiralCursor.getIndex();}

	//! Reset the iterator to spiral around a given.
	void reset(	const PYXIcosIndex& rootIndex,
				int nSpiralCount,
				int nExtraSpirals = 0	);

	//! Retrieve the two direction factor for the current index.
	int getTwoDirectionFactor() const {return m_nTwoFactor;}

	//! Retrieve the six direction factor for the current index.
	int getSixDirectionFactor() const {return m_nSixFactor;}

private:

	//! Disable copy constructor
	PYXSpiralIterator(const PYXSpiralIterator&);

	//! Disable copy assignment
	void operator=(const PYXSpiralIterator&);

	//! Move to the next spiral about the origin hexagon.
	void incrementSpiral();

	//! Adjust the two and six directions according to the direction of movement.
	void applyFactors(); 

	//! Get the number of spirals iterate around center cell.
	int getNumberOfSpirals(int nDepth);

	//! Initialize class.
	static void initStaticData();

	//! Free any static data
	static void freeStaticData() { m_vecNumberSpirals.clear(); }

	//! The root PYXIS index
	PYXIcosIndex m_rootIndex;

	//! The current PYXIS index
	PYXIcosIndex m_index;

	//! The current offset in the six direction
	int m_nSixFactor;

	//! The current offset in the two direction
	int m_nTwoFactor;

	//! The number of times to spiral around the origin
	int m_nTotalSpirals;
	
	//! The current spiral count for the iterator.
	int m_nSpiralCount;

	//! The number of turns that have been made by the spiral cursor.
	int m_nTurnCount;

	//! The total number of turns to be made by the iterator
	int m_nTotalTurns;

	//! The number of cells left on the current leg of the spiral
	int m_nLegCount;

	//! The cursor that moves in a circular direction.
	PYXCursor m_spiralCursor;

	//! The cursor that move out from the centre of the spiral
	PYXCursor m_axialCursor;

	//! Vector of the number spirals that need to be drawn for a mesh cell.
	static std::vector<int> m_vecNumberSpirals;

	//! Allows PYXLibInstance to initialize the static data.
	friend class PYXLibInstance;
};

#endif	// PYX_SPIRAL_ITERATOR_H
