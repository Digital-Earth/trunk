/******************************************************************************
cell.cpp

begin		: 2004-11-17
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/cell.h"
#include "pyxis/region/circle_region.h"

// pyxlib includes
#include "pyxis/derm/iterator.h"
#include "pyxis/geometry/bounding_rects_calculator.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/utility/tester.h"

//! Tester class
Tester<PYXCell> gTester;

//! Test method
void PYXCell::test()
{
	// test an empty cell
	PYXCell emptyCell;

	TEST_ASSERT(emptyCell.isEmpty());
	TEST_ASSERT(emptyCell.getCellResolution() < 0);

	{
		// test an empty iterator
		PYXPointer<PYXIterator> spIt(emptyCell.getIterator());
		TEST_ASSERT(spIt->end());
		PYXIcosIndex index = spIt->getIndex();
		TEST_ASSERT(index.isNull());
	}

	{
		// test copying and equality
		PYXPointer<PYXCell> spCopy(PYXCell::create(emptyCell));
		assert(0 != spCopy);
		TEST_ASSERT(spCopy->isEmpty());
		TEST_ASSERT(emptyCell == *spCopy);

		// test intersection
		PYXPointer<PYXGeometry> spGeometry(emptyCell.intersection(*spCopy));
		TEST_ASSERT(spGeometry->isEmpty());
		TEST_ASSERT(!emptyCell.intersects(*spCopy));
	}

	{
		// test cloning and equality
		PYXPointer<PYXGeometry> spClone(emptyCell.clone());
		assert(0 != spClone);
		TEST_ASSERT(spClone->isEmpty());
		// Polymorphic equality operator not yet implemented.
		// TEST_ASSERT(emptyCell == *spClone);

		// test intersection
		PYXPointer<PYXGeometry> spGeometry(emptyCell.intersection(*spClone));
		TEST_ASSERT(spGeometry->isEmpty());
		TEST_ASSERT(!emptyCell.intersects(*spClone));
	}

	// test a cell with a value
	PYXIcosIndex index = "A-0";

	PYXCell cell;
	cell.setIndex(index);
	TEST_ASSERT(!cell.isEmpty());
	TEST_ASSERT(cell.getCellResolution() == index.getResolution());

	{
		// test iterator
		PYXPointer<PYXIterator> spIt(cell.getIterator());
		TEST_ASSERT(!spIt->end());
		TEST_ASSERT(spIt->getIndex() == index);
		spIt->next();
		TEST_ASSERT(spIt->end());
		PYXIcosIndex index = spIt->getIndex();
		TEST_ASSERT(index.isNull());
	}

	{
		// test copying and equality
		PYXPointer<PYXCell> spCopy = PYXCell::create(cell);
		assert(0 != spCopy);
		TEST_ASSERT(!spCopy->isEmpty());
		TEST_ASSERT(cell == *spCopy);

		// test intersection with the copy
		PYXPointer<PYXGeometry> spGeometry(cell.intersection(*spCopy));
		TEST_ASSERT(!spGeometry->isEmpty());
		try
		{
			TEST_ASSERT(cell == dynamic_cast<const PYXCell&>(*spGeometry));
		}
		catch (...)
		{
			TEST_ASSERT(false && "Incorrect type.");
		}
		TEST_ASSERT(!emptyCell.intersects(*spCopy));
	}

	{
		// test cloning and equality
		PYXPointer<PYXGeometry> spClone = cell.clone();
		assert(0 != spClone);
		TEST_ASSERT(!spClone->isEmpty());
		// Polymorphic equality operator not yet implemented.
		//TEST_ASSERT(cell == *spClone);

		// test intersection with the clone
		PYXPointer<PYXGeometry> spGeometry(cell.intersection(*spClone));
		TEST_ASSERT(!spGeometry->isEmpty());
		try
		{
			TEST_ASSERT(cell == dynamic_cast<const PYXCell&>(*spGeometry));
		}
		catch (...)
		{
			TEST_ASSERT(false && "Incorrect type.");
		}
		TEST_ASSERT(!emptyCell.intersects(*spClone));
	}

	{
		// test intersection with the empty cell
		PYXPointer<PYXGeometry> spGeometry(cell.intersection(emptyCell));
		TEST_ASSERT(spGeometry->isEmpty());
		TEST_ASSERT(!cell.intersects(emptyCell));
	}

	{
		// test intersection with an intersecting cell
		PYXCell cellTest;
		cellTest.setIndex(index);
		PYXPointer<PYXGeometry> spGeometry(cell.intersection(cellTest));
		TEST_ASSERT(!spGeometry->isEmpty());
		try
		{
			TEST_ASSERT(dynamic_cast<const PYXCell&>(*spGeometry).getIndex() == index);
		}
		catch (...)
		{
			TEST_ASSERT(false && "Incorrect type.");
		}
		TEST_ASSERT(cell.intersects(cellTest));
	}

	{
		// test intersection with a non-intersecting cell
		PYXIcosIndex index2 = "B-0";

		PYXCell cellTest;
		cell.setIndex(index2);
		PYXPointer<PYXGeometry> spGeometry(cell.intersection(cellTest));
		TEST_ASSERT(spGeometry->isEmpty());
		TEST_ASSERT(!cell.intersects(cellTest));
	}

	{
		// test less than operator
		PYXIcosIndex index1 = "A-0";
		PYXCell cell1(index1);
		PYXIcosIndex index2 = "B-0";
		PYXCell cell2(index2);

		TEST_ASSERT(cell1 < cell2);
		TEST_ASSERT(!(cell2 < cell1));
	}

	// empty the geometry and test
	cell.setEmpty();
	TEST_ASSERT(cell.isEmpty());
}

/*!
Equality operator.

\param	rhs	The cell to compare with this one.

\return	true if the two cells are equal, otherwise false.
*/
bool PYXCell::operator ==(const PYXCell& rhs) const
{
	return (m_index == rhs.m_index);
}

/*!
Less than operator.

\param	rhs	The cell to compare with this one.

\return	true If lhs < rhs
*/
bool PYXCell::operator <(const PYXCell& rhs) const
{
	return (m_index < rhs.m_index);
}

/*!
Create a copy of the geometry.

\return	A copy of the geometry (ownership transferred).
*/
PYXPointer<PYXGeometry> PYXCell::clone() const
{
	return create(*this);
}

/*!
Get the PYXIS resolution of cells in the geometry.

\return	The PYXIS resolution.
*/
int PYXCell::getCellResolution() const
{
	return m_index.getResolution();
}

/*!
Set the PYXIS resolution of cells in the geometry.

\param	nCellResolution	The cell resolution.
*/
void PYXCell::setCellResolution(int nCellResolution)
{
	m_index.setResolution(nCellResolution);
}

/*!
Get the intersection of this geometry and the specified geometry as a new
geometry. This method calculates the intersection of two cells. Intersection of
a cell with a more complex geometry is deferred to the more complex geometry.

For cell-cell intersection there are three cases:

1. The cells are the same -> return the cell.
2. One cell is a descendant of the other -> return the descendant.
3. There is no ancestor/descendant relationship between the cells -> return the
	empty geometry.

\param	geometry		The geometry to intersect with this one.
\param	bCommutative	Indicates whether the operation is to be commutative (true) or one-way (false).

\return	The intersection of geometries.
*/
PYXPointer<PYXGeometry> PYXCell::intersection(const PYXGeometry& geometry, bool bCommutative) const
{
	const PYXCell* const pCell = dynamic_cast<const PYXCell*>(&geometry);
	if (0 != pCell)
	{
		return intersection(*pCell);
	}

	return PYXGeometry::intersection(geometry, bCommutative);
}

/*!
Get the intersection of this geometry and the specified geometry as a new
geometry. This method calculates the intersection of two cells.

For cell-cell intersection there are three cases:

1. The cells are the same -> return the cell.
2. One cell is a descendant of the other -> return the descendant.
3. There is no ancestor/descendant relationship between the cells -> return the
	empty geometry.

\param	cell	The cell to intersect with this one.

\return	The intersection of geometries.
*/
PYXPointer<PYXGeometry> PYXCell::intersection(const PYXCell& cell) const
{
	if (m_index.isDescendantOf(cell.m_index))
	{
		return clone();
	}
	if (cell.m_index.isDescendantOf(m_index))
	{
		return cell.clone();
	}

	return PYXEmptyGeometry::create();
}

/*!
Determine if this geometry has any intersection with the specified geometry.

\param	geometry		The geometry to intersect with this one.
\param	bCommutative	Indicates whether the operation is to be commutative (true) or one-way (false).

\return true if any intersection exists or false if none is found.
*/
bool PYXCell::intersects(const PYXGeometry& geometry, bool bCommutative) const
{
	const PYXCell* const pCell = dynamic_cast<const PYXCell*>(&geometry);
	if (0 != pCell)
	{
		return intersects(*pCell);
	}

	return PYXGeometry::intersects(geometry, bCommutative);
}

/*!
Determine if this geometry has any intersection with the specified geometry.

\param cell		The cell to intersect with this one.
 
\return true if any intersection exists or false if none is found.
*/
bool PYXCell::intersects(const PYXCell& cell) const
{
	return m_index.isDescendantOf(cell.m_index) || cell.m_index.isDescendantOf(m_index);
}

/*!
Get an iterator to the individual cells in the geometry.

\return	The iterator (ownership transferred)
*/
PYXPointer<PYXIterator> PYXCell::getIterator() const
{
	if (isEmpty())
	{
		return PYXEmptyIterator::create();
	}
	return PYXSingleIterator::create(m_index);
}

/*!
Copies a representation of this geometry into a tile collection.

\param	pTileCollection	The destination tile collection (must not be null).
*/
void PYXCell::copyTo(PYXTileCollection* pTileCollection) const
{
	copyTo(pTileCollection, getCellResolution());
}

/*!
Copies a representation of this geometry into a tile collection at the specified resolution.

\param	pTileCollection		The destination tile collection (must not be null).
\param	nTargetResolution	The target resolution.
*/
void PYXCell::copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const
{
	assert(pTileCollection != 0);

	pTileCollection->clear();

	PYXIcosIndex index = getIndex();
	index.setResolution(nTargetResolution);
	pTileCollection->addTile(index, nTargetResolution);	
}

PYXBoundingCircle PYXCell::getBoundingCircle() const 
{
	return PYXCircleRegion(getIndex(),false).getBoundingCircle();
}


/*! 
Allows PYXIS cells to be written to streams.

\param out		The stream to write to.
\param pyxCell	The cell to write to the stream.

\return The stream after the operation.
*/
std::ostream& operator<< (std::ostream& out, const PYXCell& pyxCell)
{
	out << pyxCell.getIndex();
	return out;
}

/*!
Allows PYXIS cells to be read from streams.

\param input	The stream to read from.
\param pyxCell	The cell to write to the stream.

\return The stream after the operation.
*/
std::istream& operator>> (std::istream& input, PYXCell& pyxCell)
{
	PYXIcosIndex pyxIndex;
	input >> pyxIndex;
	pyxCell.setIndex(pyxIndex);
	return input;
}

//! Get the bounding box for this geometry.
void PYXCell::getBoundingRects(const ICoordConverter* coordConvertor,
	PYXRect2DDouble* pRect1,
	PYXRect2DDouble* pRect2) const
{
	PYXBoundingRectsCalculator calculator(coordConvertor, *this);
	calculator.addCell(*this);
	calculator.getBoundingRects(pRect1, pRect2);
}
