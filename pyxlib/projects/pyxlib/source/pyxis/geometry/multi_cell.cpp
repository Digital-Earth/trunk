/******************************************************************************
multi_cell.cpp

begin		: 2005-09-21
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/multi_cell.h"

// pyxlib includes
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <algorithm>

//! Tester class
Tester<PYXMultiCell> gTester;

//! Test method
void PYXMultiCell::test()
{
	// test an empty multi-cell
	PYXMultiCell emptyMultiCell;

	TEST_ASSERT(emptyMultiCell.isEmpty());
	TEST_ASSERT(emptyMultiCell.getCellResolution() < 0);

	{
		// test an empty iterator
		PYXPointer<PYXIterator> spIt(emptyMultiCell.getIterator());
		TEST_ASSERT(spIt->end());
		PYXIcosIndex index = spIt->getIndex();
		TEST_ASSERT(index.isNull());
	}

	{
		// test copying and equality
		PYXPointer<PYXMultiCell> spCopy = PYXMultiCell::create(emptyMultiCell);
		assert(0 != spCopy);
		TEST_ASSERT(spCopy->isEmpty());
		TEST_ASSERT(emptyMultiCell == *spCopy);

		// test intersection
		PYXPointer<PYXGeometry> spGeometry(emptyMultiCell.intersection(*spCopy));
		TEST_ASSERT(spGeometry->isEmpty());
		TEST_ASSERT(!emptyMultiCell.intersects(*spCopy));
	}

	{
		// test cloning and equality
		PYXPointer<PYXGeometry> spClone = emptyMultiCell.clone();
		assert(0 != spClone);
		TEST_ASSERT(spClone->isEmpty());
		// Polymorphic equality operator not yet implemented.
		// TEST_ASSERT(emptyMultiCell == *spClone);

		// test intersection with clone
		PYXPointer<PYXGeometry> spGeometry(emptyMultiCell.intersection(*spClone));
		TEST_ASSERT(spGeometry->isEmpty());
		TEST_ASSERT(!emptyMultiCell.intersects(*spClone));
	}

	// test a multi-cell with values
	PYXIcosIndex index1 = "A-0";
	PYXIcosIndex index2 = "B-0";
	PYXIcosIndex index3 = "C-0";
	PYXIcosIndex index4 = "D-0";
	PYXIcosIndex index5 = "E-0";
	PYXIcosIndex index6 = "F-0";

	PYXMultiCell multiCell;
	multiCell.addIndex(index2);
	multiCell.addIndex(index3);
	multiCell.addIndex(index1);
	TEST_ASSERT(!multiCell.isEmpty());
	TEST_ASSERT(multiCell.getCellResolution() == index1.getResolution());

	{
		// test iterator and sorting
		PYXPointer<PYXIterator> spIt(multiCell.getIterator());
		TEST_ASSERT(!spIt->end());
		TEST_ASSERT(spIt->getIndex() == index1);
		spIt->next();
		TEST_ASSERT(spIt->getIndex() == index2);
		spIt->next();
		TEST_ASSERT(spIt->getIndex() == index3);
		spIt->next();
		TEST_ASSERT(spIt->end());
		PYXIcosIndex index = spIt->getIndex();
		TEST_ASSERT(index.isNull());
	}

	{
		// test copying and equality
		PYXPointer<PYXMultiCell> spCopy = PYXMultiCell::create(multiCell);
		assert(0 != spCopy);
		TEST_ASSERT(!spCopy->isEmpty());
		TEST_ASSERT(multiCell == *spCopy);

		// test intersection with the copy
		PYXPointer<PYXGeometry> spGeometry(multiCell.intersection(*spCopy));
		TEST_ASSERT(!spGeometry->isEmpty());
		try
		{
			TEST_ASSERT(multiCell == dynamic_cast<const PYXMultiCell&>(*spGeometry));
		}
		catch (...)
		{
			TEST_ASSERT(false && "Incorrect type.");
		}

		TEST_ASSERT(!emptyMultiCell.intersects(*spCopy));
	}

	{
		// test cloning and equality
		PYXPointer<PYXGeometry> spClone = multiCell.clone();
		assert(0 != spClone);
		TEST_ASSERT(!spClone->isEmpty());
		// Polymorphic equality operator not yet implemented.
		// TEST_ASSERT(multiCell == *spClone);

		// test intersection with the clone
		PYXPointer<PYXGeometry> spGeometry(multiCell.intersection(*spClone));
		TEST_ASSERT(!spGeometry->isEmpty());
		try
		{
			TEST_ASSERT(multiCell == dynamic_cast<const PYXMultiCell&>(*spGeometry));
		}
		catch (...)
		{
			TEST_ASSERT(false && "Incorrect type.");
		}

		TEST_ASSERT(!emptyMultiCell.intersects(*spClone));
	}

	{
		// test intersection with the empty set
		PYXPointer<PYXGeometry> spGeometry(multiCell.intersection(emptyMultiCell));
		TEST_ASSERT(spGeometry->isEmpty());
		TEST_ASSERT(!multiCell.intersects(emptyMultiCell));
	}

	{
		// test intersection with an intersecting cell
		PYXCell cell;
		cell.setIndex(index1);
		PYXPointer<PYXGeometry> spGeometry(multiCell.intersection(cell));
		PYXPointer<PYXGeometry> spGeometry2(multiCell.intersection((const PYXGeometry&)cell));
		TEST_ASSERT(!spGeometry->isEmpty());
		try
		{
			TEST_ASSERT(dynamic_cast<const PYXCell&>(*spGeometry).getIndex() == index1);
			TEST_ASSERT(dynamic_cast<const PYXCell&>(*spGeometry2).getIndex() == index1);
		}
		catch (...)
		{
			TEST_ASSERT(false && "Incorrect type.");
		}
		TEST_ASSERT(multiCell.intersects(cell));
		TEST_ASSERT(multiCell.intersects((const PYXGeometry&)cell));
	}

	{
		// test intersection with a multicell containing a single intersecting cell
		PYXMultiCell mc;
		mc.addIndex(index1);
		PYXPointer<PYXGeometry> spGeometry(multiCell.intersection(mc));
		PYXPointer<PYXGeometry> spGeometry2(multiCell.intersection((const PYXGeometry&)mc));
		TEST_ASSERT(!spGeometry->isEmpty());
		try
		{
			TEST_ASSERT(dynamic_cast<const PYXCell&>(*spGeometry).getIndex() == index1);
			TEST_ASSERT(dynamic_cast<const PYXCell&>(*spGeometry2).getIndex() == index1);
		}
		catch (...)
		{
			TEST_ASSERT(false && "Incorrect type.");
		}
		TEST_ASSERT(multiCell.intersects(mc));
		TEST_ASSERT(multiCell.intersects((const PYXGeometry&)mc));
	}

	{
		// test intersection with a multicell containing a single non-intersecting cell
		PYXMultiCell mc;
		mc.addIndex(index4);
		PYXPointer<PYXGeometry> spGeometry(multiCell.intersection(mc));
		PYXPointer<PYXGeometry> spGeometry2(multiCell.intersection((const PYXGeometry&)mc));
		TEST_ASSERT(spGeometry->isEmpty());
		TEST_ASSERT(spGeometry2->isEmpty());
		TEST_ASSERT(!multiCell.intersects(mc));
		TEST_ASSERT(!multiCell.intersects((const PYXGeometry&)mc));
	}

	{
		// test intersection with a non-intersecting cell
		PYXCell cell;
		cell.setIndex(index4);
		PYXPointer<PYXGeometry> spGeometry(multiCell.intersection(cell));
		TEST_ASSERT(spGeometry->isEmpty());
		TEST_ASSERT(!multiCell.intersects(cell));
	}

	{
		// test non-intersection with another multicell
		PYXMultiCell multiCell2;
		multiCell2.addIndex(index5);
		multiCell2.addIndex(index6);
		multiCell2.addIndex(index4);
		TEST_ASSERT(!multiCell2.isEmpty());
		TEST_ASSERT(multiCell2.getCellResolution() == index1.getResolution());
		TEST_ASSERT(!multiCell.intersects(multiCell2));
		TEST_ASSERT(!multiCell2.intersects(multiCell));
	}

	{
		// test intersection with another multicell
		PYXMultiCell multiCell2;
		multiCell2.addIndex(index5);
		multiCell2.addIndex(index6);
		multiCell2.addIndex(index2);
		TEST_ASSERT(!multiCell2.isEmpty());
		TEST_ASSERT(multiCell2.getCellResolution() == index1.getResolution());
		TEST_ASSERT(multiCell.intersects(multiCell2));
		TEST_ASSERT(multiCell2.intersects(multiCell));
	}

	{
		// test intersection with a tile
		PYXTile tile(index1, 10);
		TEST_ASSERT(multiCell.intersects(tile));
		TEST_ASSERT(multiCell.intersects((const PYXGeometry&)tile));
	}

	// empty the geometry and test
	multiCell.setEmpty();
	TEST_ASSERT(multiCell.isEmpty());

	//Test Multi Cell empty collection intersection.
	{
		PYXPointer<PYXMultiCell> spMultiCell = PYXMultiCell::create();
		TEST_ASSERT(spMultiCell->intersects(PYXCell(PYXIcosIndex("H-0000"))) == false);
		
	}
}

/*!
Test if two PYXMultiCells are equal.

\param	rhs	The multi-cell to compare with.

\return	true if the multi-cells are equal, otherwise false.
*/
bool PYXMultiCell::operator==(const PYXMultiCell& rhs) const
{
	return (m_setIndex == rhs.m_setIndex);
}

/*!
Get the PYXIS resolution of cells in the geometry.

\return	The PYXIS resolution or -1 if the geometry is empty.
*/
int PYXMultiCell::getCellResolution() const
{
	int nResolution = -1;

	if (!m_setIndex.empty())
	{
		nResolution = m_setIndex.begin()->getResolution();
	}

	return nResolution;
}

/*!
Set the PYXIS resolution of cells in the geometry.

\param	nCellResolution	The cell resolution.
*/
void PYXMultiCell::setCellResolution(int nCellResolution)
{
	IndexSet newIndices;
	IndexSet::iterator it = m_setIndex.begin();
	for (; it != m_setIndex.end(); ++it)
	{
		PYXIcosIndex index = *it;
		index.setResolution(nCellResolution);
		newIndices.insert(index);		
	}
	std::swap(m_setIndex,newIndices);
}

/*!
Get the intersection of this geometry and the specified geometry as a new
geometry. This method calculates the intersection of a multi-cell and a cell or
two multi-cells. Intersection of a multi-cell with a more complex geometry is
deferred to the more complex geometry.

\param	geometry		The geometry to intersect with this one.
\param	bCommutative	Indicates whether the operation is to be commutative (true) or one-way (false).

\return	The intersection of geometries.
*/
PYXPointer<PYXGeometry> PYXMultiCell::intersection(	const PYXGeometry& geometry,
													bool bCommutative	) const
{
	const PYXCell* const pCell = dynamic_cast<const PYXCell*>(&geometry);
	if (0 != pCell)
	{
		return intersection(*pCell);
	}

	const PYXMultiCell* const pMultiCell = dynamic_cast<const PYXMultiCell*>(&geometry);
	if (0 != pMultiCell)
	{
		return intersection(*pMultiCell);
	}

	return PYXGeometry::intersection(geometry, bCommutative);
}

/*!
Get the intersection of this geometry and the specified geometry as a new
geometry. This method calculates the intersection of a multi-cell and a cell.

\param	cell	The cell to intersect with this one.

\return	The intersection of geometries.
*/
PYXPointer<PYXGeometry> PYXMultiCell::intersection(const PYXCell& cell) const
{
	// check if the cell is in this set
	if (m_setIndex.find(cell.getIndex()) != m_setIndex.end())
	{
		return cell.clone();
	}

	return PYXEmptyGeometry::create();
}

/*!
Get the intersection of this geometry and the specified geometry as a new
geometry. This method calculates the intersection of a two multi-cells.

\param	multiCell	The multi-cell to intersect with this one.

\return	The intersection of geometries.
*/
PYXPointer<PYXGeometry> PYXMultiCell::intersection(const PYXMultiCell& multiCell) const
{
	// find the intersection of sets
	IndexVector vecIntersection;
	vecIntersection.resize(std::min(	m_setIndex.size(),
										multiCell.m_setIndex.size()	));
	IndexVector::const_iterator itEnd =
		set_intersection(	m_setIndex.begin(),
							m_setIndex.end(),
							multiCell.m_setIndex.begin(),
							multiCell.m_setIndex.end(),
							vecIntersection.begin()	);

	if (itEnd - vecIntersection.begin() == 1)
	{
		// only a single cell of intersection
		PYXPointer<PYXCell> spCell = PYXCell::create();
		assert(0 != spCell);

		spCell->setIndex(*(vecIntersection.begin()));

		return spCell;
	}
	if (itEnd - vecIntersection.begin() > 1)
	{
		// multiple cells of intersection
		PYXPointer<PYXMultiCell> spMultiCell = PYXMultiCell::create();
		assert(0 != spMultiCell);

		for (	IndexVector::const_iterator it = vecIntersection.begin();
				it != itEnd;
				++it	)
		{
			spMultiCell->m_setIndex.insert(*it);
		}

		return spMultiCell;
	}

	return PYXEmptyGeometry::create();
}

/*!
Determine if this geometry has any intersection with the specified geometry.

\param	geometry		The geometry to intersect with this one.
\param	bCommutative	Indicates whether the operation is to be commutative (true) or one-way (false).

\return true if any intersection exists or false if none is found.
*/
bool PYXMultiCell::intersects(const PYXGeometry& geometry, bool bCommutative) const
{
	const PYXCell* const pCell = dynamic_cast<const PYXCell*>(&geometry);
	if (0 != pCell)
	{
		return intersects(*pCell);
	}

	const PYXMultiCell* const pMultiCell = dynamic_cast<const PYXMultiCell*>(&geometry);
	if (0 != pMultiCell)
	{
		return intersects(*pMultiCell);
	}

	// treat multi-cell as collection of single cells
	PYXCell cell;
	for (	IndexSet::const_iterator it = m_setIndex.begin();
			it != m_setIndex.end();
			++it	)
	{
		cell.setIndex(*it);
		if (geometry.intersects(cell))
		{
			return true;
		}
	}

	return false;
}

/*!
Determine if this geometry has any intersection with the specified geometry.

\param cell	The cell to intersect with this one.
 
\return true if any intersection exists or false if none is found.
*/
bool PYXMultiCell::intersects(const PYXCell& cell) const
{
	if (this->isEmpty())
	{
		return false;
	}

	if (this->getCellResolution() < cell.getCellResolution())
	{
		PYXCell newCell(cell.getIndex());
		newCell.setCellResolution(this->getCellResolution());
		return (m_setIndex.find(newCell.getIndex()) != m_setIndex.end());
	}
	else if (cell.getCellResolution() < this->getCellResolution())
	{
		for (IndexSet::const_iterator it = m_setIndex.begin();
			it != m_setIndex.end(); ++it)
		{
			if ((*it).isDescendantOf(cell.getIndex()))
			{
				return true;
			}
		}
		return false;
	}

	// the resolutions are equal
	return (m_setIndex.find(cell.getIndex()) != m_setIndex.end());
}

/*!
Determine if this geometry has any intersection with the specified geometry.

\param multiCell	The multi-cell to intersect with this one.
 
\return true if any intersection exists or false if none is found.
*/
bool PYXMultiCell::intersects(const PYXMultiCell& multiCell) const
{
	IndexSet::const_iterator it1 = m_setIndex.begin();
	IndexSet::const_iterator it2 = multiCell.m_setIndex.begin();
	while (	(it1 != m_setIndex.end()) &&
			(it2 != multiCell.m_setIndex.end())	)
	{
		if (*it1 < *it2)
		{
			++it1;
		}
		else if (*it2 < *it1)
		{
			++it2;
		}
		else
		{
			// cells are equal and we've found an intersection
			return true;
		}
	}

	return false;
}

/*!
Get an iterator to the individual cells in the geometry.

\return	The iterator (ownership transferred)
*/
PYXPointer<PYXIterator> PYXMultiCell::getIterator() const
{
	return PYXMultiCellIterator::create(m_setIndex);
}

/*!
Copies a representation of this geometry into a tile collection.

\param	pTileCollection	The destination tile collection (must not be null).
*/
void PYXMultiCell::copyTo(PYXTileCollection* pTileCollection) const
{
	copyTo(pTileCollection, getCellResolution());	
}

/*!
Copies a representation of this geometry into a tile collection at the specified resolution.

\param	pTileCollection		The destination tile collection (must not be null).
\param	nTargetResolution	The target resolution.
*/
void PYXMultiCell::copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const
{
	assert(pTileCollection != 0);

	PYXTileCollection tileCollection;
	pTileCollection->clear();
	
	IndexSet::const_iterator it = m_setIndex.begin();
	for (; it != m_setIndex.end(); ++it)
	{
		tileCollection.addTile(*it, getCellResolution());
	}

	tileCollection.copyTo(pTileCollection, nTargetResolution);
}

PYXBoundingCircle PYXMultiCell::getBoundingCircle() const
{
	PYXBoundingCircle circle;

	IndexSet::const_iterator it = m_setIndex.begin();
	for (; it != m_setIndex.end(); ++it)
	{
		circle += PYXCell(*it).getBoundingCircle();
	}
	return circle;
}

/*!
Constructor initializes member variables.
*/
PYXMultiCell::PYXMultiCellIterator::PYXMultiCellIterator(const IndexSet& setIndex) :
	m_it(setIndex.begin()),
	m_end(setIndex.end())
{
}

/*!
Move to the next cell.
*/
void PYXMultiCell::PYXMultiCellIterator::next()
{
	if (!end())
	{
		++m_it;
	}
}

/*!
See if we have covered all the cells.

\return true if all cells have been covered, otherwise false.
*/
bool PYXMultiCell::PYXMultiCellIterator::end() const
{
	return (m_it == m_end);
}

/*!
Get the PYXIS index for the current cell.

\return	The current index or the null index if iteration is complete.
*/
const PYXIcosIndex& PYXMultiCell::PYXMultiCellIterator::getIndex() const
{
	if (end())
	{
		return getNullIndex();
	}

	return *m_it;
}
