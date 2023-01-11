/******************************************************************************
tile.cpp

begin		: 2004-11-15
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/tile.h"

// pyxlib includes
#include "pyxis/derm/exhaustive_iterator.h"
#include "pyxis/derm/iterator.h"
#include "pyxis/geometry/bounding_rects_calculator.h"
#include "pyxis/geometry/exceptions.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/region/circle_region.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/tester.h"

//! The difference between the data resolution and the tile resolution
const int PYXTile::knDefaultTileDepth = 11;

Tester<PYXTile> gTester;

//! Test method
void PYXTile::test()
{
	// Basic tests
	{
		const PYXIcosIndex myRootIndex("A-0");
		const int myCellResolution = 6;
		PYXTile myTile(myRootIndex, myCellResolution);

		PYXTileCollection tileCollection;
		myTile.copyTo(&tileCollection);

		PYXPointer< PYXTileCollectionIterator > spIt(tileCollection.getTileIterator());
		TEST_ASSERT(!spIt->end());
		PYXPointer< PYXTile > spTile = spIt->getTile();
		TEST_ASSERT(spTile);
		TEST_ASSERT(myTile.getCellCount() == spTile->getCellCount());
	}


	// TO DO: Add some tests
}

/*!
Constructor initializes member variables.

\param	index			The parent index
\param	nCellResolution	The resolution at which the cells reside.
*/
PYXTile::PYXTile(const PYXIcosIndex& index, int nCellResolution) :
	m_index(index),
	m_nCellResolution(0)
{
	assert(!m_index.isNull() && "Invalid argument.");
	assert((nCellResolution >= index.getResolution()) && "Cell resolution less than root resolution.");

	setCellResolution(nCellResolution);
}

/*!
Test if two tiles are equal.

\param	rhs	The tile to test against this one.

\return	true if the tiles are equal, otherwise false.
*/
bool PYXTile::operator ==(const PYXTile& rhs) const
{
	return (	(m_nCellResolution == rhs.m_nCellResolution) &&
				(m_index == rhs.m_index)	);
}

/*!
Test if two tiles are equal.

\param	rhs	The tile to test against this one.

\return	true if the tiles are unequal, otherwise false.
*/
bool PYXTile::operator !=(const PYXTile& rhs) const
{
	return !(*this == rhs);
}

/*!
Less than operator to facilitate sorting of PYXTiles.

\param	rhs	The tile to test against this one.

\return	true if this is less than the specified tile, otherwise false.
*/
bool PYXTile::operator <(const PYXTile& rhs) const
{
	if (m_nCellResolution < rhs.m_nCellResolution)
	{
		return true;
	}
	else if (rhs.m_nCellResolution < m_nCellResolution)
	{
		return false;
	}

	// The cell resolutions were the same, sort on index
    return m_index < rhs.m_index;
}

/*!
Assign one tile to another.
*/
PYXTile& PYXTile::operator=(const PYXTile& tile)
{
	this->m_index = tile.m_index;
	this->m_nCellResolution = tile.m_nCellResolution;
	return *this;
}

/*!
Create a copy of the geometry.

\return	A copy of the geometry (ownership transferred).
*/
PYXPointer<PYXGeometry> PYXTile::clone() const
{
	return create(*this);
}

/*!
Get the tile depth. The tile depth is the difference between the cell
resolution and the root index resolution.

\return	The tile depth.
*/
int PYXTile::getDepth() const
{
	return (m_nCellResolution - m_index.getResolution());
}

/*!
Set the PYXIS resolution of cells in the geometry.

\param	nCellResolution	The cell resolution.
*/
void PYXTile::setCellResolution(int nCellResolution)
{
	if (nCellResolution < m_index.getResolution())
	{
		PYXTHROW(	PYXGeometryException,
					"Invalid cell resolution: '" << nCellResolution << 
					"' for root '" << m_index << "'."	);
	}
	
	m_nCellResolution = nCellResolution;	
}

/*! 
If the root index of the tile is not an origin child the resolution of the 
root index is incremented.  If the resolution can't be incremented an exception
is thrown.
*/
void PYXTile::setOriginChildRoot()
{
	if (m_index.hasVertexChildren())
	{
		return;
	}

	// attempt to increment the resolution of the root index
	if (m_index.getResolution() < m_nCellResolution)
	{
		try
		{
			m_index.incrementResolution();
		}
		catch (PYXException& e)
		{
			PYXRETHROW(	e, 
						PYXGeometryException,
						"Failed to set resolution of tile root"	);
		}
	}
	else
	{
		PYXTHROW(	PYXGeometryException,
					"Can't setOriginChildRoot() on tile: '" << 
					*this << "'."	);
	}
}

/*!
Get the cell that spatially contains all the cells in this tile.

\return	The cell.
*/
const PYXCell& PYXTile::getBoundingCell() const
{
	if (isEmpty() || !m_index.hasVertexChildren())
	{
		m_boundingCell.setIndex(m_index);
	}
	else
	{
		// index has vertex children, return parent index
		if (0 < m_index.getResolution())
		{
			PYXIcosIndex index = m_index;
			index.setResolution(m_index.getResolution() - 1);

			m_boundingCell.setIndex(index);
		}
		else
		{
			m_boundingCell.setEmpty();
		}
	}

	return m_boundingCell;
}

//! Helper function for finding the XY bounds of a tile
void PYXTile::addCornersToCalculator (PYXBoundingRectsCalculator* pCalc) const
{
	// we find the six cells at the "corners" of the tile and add them to the bounds calculator.
	for (unsigned int direction1 = 1; direction1 <= 6; direction1++)
	{
		if (PYXIcosMath::isValidDirection(m_index, static_cast<PYXMath::eHexDirection>(direction1)))
		{
			bool AddingZero = !m_index.hasVertexChildren();
			PYXIcosIndex cornerIndex1 = m_index.toString();
			PYXIcosIndex cornerIndex2 = m_index.toString();
			PYXIcosIndex cornerIndex3 = m_index.toString();
			int direction2 = direction1;
			int direction3 = direction1;
			for (int steps = 1; steps <= getDepth(); steps++)
			{
				if (AddingZero)
				{
					cornerIndex1.getSubIndex().appendDigit(0);
					cornerIndex2.getSubIndex().appendDigit(0);
					cornerIndex3.getSubIndex().appendDigit(0);
				}
				else
				{
					cornerIndex1.getSubIndex().appendDigit(direction1);
					cornerIndex2.getSubIndex().appendDigit(direction2);
					cornerIndex3.getSubIndex().appendDigit(direction3);
					direction2 = (direction1 + 4) % 6 + 1;
					direction3 = (direction1) % 6 + 1;
				}
				AddingZero = !AddingZero;
			}
			pCalc->addCell(PYXCell(cornerIndex1));
			pCalc->addCell(PYXCell(cornerIndex2));
			pCalc->addCell(PYXCell(cornerIndex3));
		}
	}
}

//! Get the bounding box for this geometry.
void PYXTile::getBoundingRects(const ICoordConverter* coordConvertor,
	PYXRect2DDouble* pRect1,
	PYXRect2DDouble* pRect2) const
{
	PYXBoundingRectsCalculator calculator(coordConvertor, *this);
	addCornersToCalculator(&calculator);
	calculator.getBoundingRects(pRect1, pRect2);
}

/*!
Get the intersection of this geometry and the specified geometry as a new
geometry. This method handles intersection of tiles and cells. For geometries
with different resolutions, the geometry of intersection is generated at the
higher resolution.

\param	geometry		The geometry to intersect with this one.
\param	bCommutative	Indicates whether the operation is to be commutative (true) or one-way (false).

\return	The intersection of geometries.
*/
PYXPointer<PYXGeometry> PYXTile::intersection(	const PYXGeometry& geometry,
												bool bCommutative	) const
{
	const PYXCell* const pCell = dynamic_cast<const PYXCell*>(&geometry);
	if (0 != pCell)
	{
		return intersection(*pCell);
	}

	const PYXTile* const pTile = dynamic_cast<const PYXTile*>(&geometry);
	if (0 != pTile)
	{
		return intersection(*pTile);
	}

	return PYXGeometry::intersection(geometry, bCommutative);
}

/*!
Get the intersection of this geometry and the specified geometry as a new
geometry. This method handles intersection of tiles. For geometries
with different resolutions, the geometry of intersection is generated at the
higher resolution.

\param	tile	The tile to intersect with this one.

\return	The intersection of geometries.
*/
PYXPointer<PYXGeometry> PYXTile::intersection(const PYXTile& tile) const
{
	if (&tile == this)
	{
		return tile.clone();
	}

	PYXPointer<PYXGeometry> spIntersection;

	if (m_index.isDescendantOf(tile.m_index))
	{
		spIntersection = clone();
		assert(0 != spIntersection);
	}
	else if (tile.m_index.isDescendantOf(m_index))
	{
		spIntersection = tile.clone();
		assert(0 != spIntersection);
	}

	// use the higher resolution for the new geometry
	if (0 != spIntersection)
	{
		int nResolution = std::max(getCellResolution(), tile.getCellResolution());
		spIntersection->setCellResolution(nResolution);

		return spIntersection;
	}

	return PYXEmptyGeometry::create();
}

/*!
Get the intersection of this geometry and the specified geometry as a new
geometry. This method handles intersection of a tile and a cell. For geometries
with different resolutions, the geometry of intersection is generated at the
higher resolution.

\param	cell	The cell to intersect with this tile.

\return	The intersection of geometries.
*/
PYXPointer<PYXGeometry> PYXTile::intersection(const PYXCell& cell) const
{
	const PYXIcosIndex& index = cell.getIndex();

	if (	index == m_index ||
			index.isDescendantOf(m_index) ||
			index.isAncestorOf(m_index)	)
	{
		return cell.clone();
	}

	return PYXEmptyGeometry::create();
}

/*!
Determine if this geometry has any intersection with the specified geometry.

\param geometry			The geometry to intersect with this one.
\param	bCommutative	Indicates whether the operation is to be commutative (true) or one-way (false).

\return true if any intersection exists or false if none is found.
*/
bool PYXTile::intersects(const PYXGeometry& geometry, bool bCommutative) const
{
	const PYXCell* const pCell = dynamic_cast<const PYXCell*>(&geometry);
	if (0 != pCell)
	{
		return intersects(*pCell);
	}

	const PYXTile* const pTile = dynamic_cast<const PYXTile*>(&geometry);
	if (0 != pTile)
	{
		return intersects(*pTile);
	}

	return PYXGeometry::intersects(geometry, bCommutative);
}

/*!
Determine if this geometry has any intersection with the specified geometry.

\param tile		The tile to intersect with this one.

\return true if any intersection exists or false if none is found.
*/
bool PYXTile::intersects(const PYXTile& tile) const
{
	return	tile.m_index == m_index	||
			m_index.isAncestorOf(tile.m_index) ||
			m_index.isDescendantOf(tile.m_index);
}

/*!
Determine if this geometry has any intersection with the specified geometry.

\param cell		The cell to intersect with this tile.
 
\return true if any intersection exists or false if none is found.
*/
bool PYXTile::intersects(const PYXCell& cell) const
{
	const PYXIcosIndex& index = cell.getIndex();

	return	index == m_index ||
			index.isDescendantOf(m_index) ||
			index.isAncestorOf(m_index);
}

/*!
Get an iterator to the individual cells in the geometry.

\return	The iterator (ownership transferred)
*/
PYXPointer<PYXIterator> PYXTile::getIterator() const
{
	if (isEmpty())
	{
		return PYXEmptyIterator::create();
	}
	return PYXExhaustiveIterator::create(m_index, getCellResolution());
}

/*!
Copies a representation of this geometry into a tile collection.

\param	pTileCollection	The destination tile collection (must not be null).
*/
void PYXTile::copyTo(PYXTileCollection* pTileCollection) const
{
	copyTo(pTileCollection, getCellResolution());
}

/*!
Copies a representation of this geometry into a tile collection at the specified resolution.

\param	pTileCollection		The destination tile collection (must not be null).
\param	nTargetResolution	The target resolution.	
*/
void PYXTile::copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const
{
	assert(pTileCollection != 0);

	pTileCollection->clear();
	pTileCollection->addTile(getRootIndex(), nTargetResolution);
}

PYXBoundingCircle PYXTile::getBoundingCircle() const
{
	return PYXCircleRegion(getRootIndex(),true).getBoundingCircle();
}


/*! 
Allows PYXIS tile to be written to streams.

\param out		The stream to write to.
\param pyxTile	The tile to write to the stream.

\return The stream after the operation.
*/
std::ostream& operator <<(std::ostream& out, const PYXTile& pyxTile)
{
	out << pyxTile.getRootIndex() << " "
		<< pyxTile.getCellResolution();
	return out;
}

/*!
Allows PYXIS tile to be read from streams.

\param input	The stream to read from.
\param pyxTile	The tile to write to the stream.

\return The stream after the operation.
*/
std::istream& operator >>(std::istream& input, PYXTile& pyxTile)
{
	PYXIcosIndex pyxIndex;
	int nResolution;
	input >> pyxIndex >> nResolution;
	PYXTile tempTile(pyxIndex, nResolution);
	pyxTile = tempTile;
	return input;
}
