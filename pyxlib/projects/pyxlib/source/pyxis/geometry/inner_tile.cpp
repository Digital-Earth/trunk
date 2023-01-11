/******************************************************************************
inner_tile.cpp

begin		: 2012-05-1
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/inner_tile.h"

// pyxlib includes
#include "pyxis/derm/exhaustive_iterator.h"
#include "pyxis/derm/iterator.h"
#include "pyxis/geometry/bounding_rects_calculator.h"
#include "pyxis/geometry/exceptions.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/region/circle_region.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/tester.h"

Tester<PYXInnerTile> tileTester;

//! Test method
void PYXInnerTile::test()
{
	// Basic tests
	{
		const PYXIcosIndex myRootIndex("A-0");
		const int myCellResolution = 6;
		PYXInnerTile myTile(myRootIndex, myCellResolution);

		PYXTileCollection tileCollection;
		myTile.copyTo(&tileCollection);

		PYXPointer< PYXTileCollectionIterator > spIt(tileCollection.getTileIterator());
		TEST_ASSERT(!spIt->end());
		PYXPointer< PYXTile > spTile = spIt->getTile();
		TEST_ASSERT(spTile);
		TEST_ASSERT(myTile.asTile().getCellCount() == spTile->getCellCount());
	}


	{
		//Tile("A") converted to InnerTile("A")
		auto tile = PYXTile(PYXIcosIndex("A"),11);
		auto innerTiles = PYXInnerTile::createInnerTiles(tile);

		TEST_ASSERT(innerTiles.size() == 1);
		TEST_ASSERT(innerTiles[0].intersects(tile));
		TEST_ASSERT(innerTiles[0].getRootIndex() == PYXIcosIndex("A"));
		TEST_ASSERT(innerTiles[0].getCellResolution() == tile.getCellResolution());
	}

	{
		//Tile("1") converted to InnerTile("1"),InnerTile("1-2"),InnerTile("1-3"),...
		auto tile = PYXTile(PYXIcosIndex("1"),11);
		auto innerTiles = PYXInnerTile::createInnerTiles(tile);

		TEST_ASSERT(innerTiles.size() == 6); //center + 5 vertices 
		for(auto & innerTile : innerTiles) {
			TEST_ASSERT(innerTile.intersects(tile));
			TEST_ASSERT(innerTile.getCellResolution() == tile.getCellResolution());
		}
	}

	{
		//Tile("1-0") converted to InnerTile("1")
		auto tile = PYXTile(PYXIcosIndex("1-0"),11);
		auto innerTiles = PYXInnerTile::createInnerTiles(tile);

		TEST_ASSERT(innerTiles.size() == 1);
		TEST_ASSERT(innerTiles[0].intersects(tile));
		TEST_ASSERT(innerTiles[0].getRootIndex() == PYXIcosIndex("1"));
		TEST_ASSERT(innerTiles[0].getCellResolution() == tile.getCellResolution());
	}

	{
		//Tile("1-00") converted to InnerTile("1-0")
		auto tile = PYXTile(PYXIcosIndex("1-00"),11);
		auto innerTiles = PYXInnerTile::createInnerTiles(tile);

		TEST_ASSERT(innerTiles.size() == 1);
		TEST_ASSERT(innerTiles[0].intersects(tile));
		TEST_ASSERT(innerTiles[0].getRootIndex() == PYXIcosIndex("1-0"));
		TEST_ASSERT(innerTiles[0].getCellResolution() == tile.getCellResolution());
	}

	{
		//Tile("A-01") converted to InnerTile("A-01")
		auto tile = PYXTile(PYXIcosIndex("A-01"),11);
		auto innerTiles = PYXInnerTile::createInnerTiles(tile);

		TEST_ASSERT(innerTiles.size() == 1);
		TEST_ASSERT(innerTiles[0].intersects(tile));
		TEST_ASSERT(innerTiles[0].getRootIndex() == PYXIcosIndex("A-01"));
		TEST_ASSERT(innerTiles[0].getCellResolution() == tile.getCellResolution());
	}
}

/*!
Constructor initializes member variables.

\param	index			The parent index
\param	nCellResolution	The resolution at which the cells reside.
*/
PYXInnerTile::PYXInnerTile(const PYXIcosIndex& index, int nCellResolution) :
	m_index(index),
	m_rootIndex(index),
	m_nCellResolution(0)
{
	if (!m_rootIndex.isNull() && m_rootIndex.hasVertexChildren() && m_rootIndex.getResolution() < nCellResolution)
	{
		m_rootIndex.incrementResolution();
	}
	assert(!m_index.isNull() && "Invalid argument.");
	assert((nCellResolution >= m_rootIndex.getResolution()) && "Cell resolution less than root resolution.");

	setCellResolution(nCellResolution);
	m_boundingCell.setIndex(m_index);
}

/*!
Test if two tiles are equal.

\param	rhs	The tile to test against this one.

\return	true if the tiles are equal, otherwise false.
*/
bool PYXInnerTile::operator ==(const PYXInnerTile& rhs) const
{
	return (	(m_nCellResolution == rhs.m_nCellResolution) &&
				(m_index == rhs.m_index)	);
}

/*!
Test if two tiles are equal.

\param	rhs	The tile to test against this one.

\return	true if the tiles are unequal, otherwise false.
*/
bool PYXInnerTile::operator !=(const PYXInnerTile& rhs) const
{
	return !(*this == rhs);
}

/*!
Test if two tiles are equal.

\param	rhs	The tile to test against this one.

\return	true if the tiles are equal, otherwise false.
*/
bool PYXInnerTile::operator ==(const PYXTile& rhs) const
{
	return (	(m_nCellResolution == rhs.getCellResolution()) &&
				(m_rootIndex == rhs.getRootIndex())	);
}

/*!
Test if two tiles are equal.

\param	rhs	The tile to test against this one.

\return	true if the tiles are unequal, otherwise false.
*/
bool PYXInnerTile::operator !=(const PYXTile& rhs) const
{
	return !(*this == rhs);
}

/*!
Less than operator to facilitate sorting of PYXInnerTiles.

\param	rhs	The tile to test against this one.

\return	true if this is less than the specified tile, otherwise false.
*/
bool PYXInnerTile::operator <(const PYXInnerTile& rhs) const
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
PYXInnerTile& PYXInnerTile::operator=(const PYXInnerTile& tile)
{
	this->m_index = tile.m_index;
	this->m_rootIndex = tile.m_rootIndex;
	this->m_nCellResolution = tile.m_nCellResolution;
	m_boundingCell.setIndex(m_index);
	return *this;
}

/*!
Create a copy of the geometry.

\return	A copy of the geometry (ownership transferred).
*/
PYXPointer<PYXGeometry> PYXInnerTile::clone() const
{
	return create(*this);
}

/*!
Get the tile depth. The tile depth is the difference between the cell
resolution and the root index resolution.

\return	The tile depth.
*/
int PYXInnerTile::getDepth() const
{
	return (m_nCellResolution - m_index.getResolution());
}

/*!
Set the PYXIS resolution of cells in the geometry.

\param	nCellResolution	The cell resolution.
*/
void PYXInnerTile::setCellResolution(int nCellResolution)
{
	if (nCellResolution < m_rootIndex.getResolution())
	{
		PYXTHROW(	PYXGeometryException,
					"Invalid cell resolution: '" << nCellResolution << 
					"' for root '" << m_index << "'."	);
	}

	//update cell resolution
	m_nCellResolution = nCellResolution;

	//fix rootIndex if needed
	m_rootIndex = m_index;
	if (!m_rootIndex.isNull() && m_rootIndex.hasVertexChildren() && m_rootIndex.getResolution() < nCellResolution)
	{
		m_rootIndex.incrementResolution();
	}
}


/*!
Get the cell that spatially contains all the cells in this tile.

\return	The cell.
*/
const PYXCell& PYXInnerTile::getBoundingCell() const
{
	return m_boundingCell;
}

//! Get the bounding box for this geometry.
void PYXInnerTile::getBoundingRects(const ICoordConverter* coordConvertor,
									PYXRect2DDouble* pRect1,
									PYXRect2DDouble* pRect2) const
{
	asTile().getBoundingRects(coordConvertor,pRect1,pRect2);
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
PYXPointer<PYXGeometry> PYXInnerTile::intersection(	const PYXGeometry& geometry,
												   bool bCommutative	) const
{
	const PYXInnerTile* const pTile = dynamic_cast<const PYXInnerTile*>(&geometry);
	if (0 != pTile)
	{
		return asTile().intersection(pTile->asTile(),bCommutative);;
	}
	return asTile().intersection(geometry,bCommutative);
}

//! Get the intersection of this geometry and the specified geometry.
PYXPointer<PYXGeometry> PYXInnerTile::intersection(const PYXInnerTile& tile) const
{
	return asTile().intersection(tile.asTile());
}

//! Get the intersection of this geometry and the specified geometry.
PYXPointer<PYXGeometry> PYXInnerTile::intersection(const PYXCell& cell) const
{
	return asTile().intersection(cell);
}


/*!
Determine if this geometry has any intersection with the specified geometry.

\param geometry			The geometry to intersect with this one.
\param	bCommutative	Indicates whether the operation is to be commutative (true) or one-way (false).

\return true if any intersection exists or false if none is found.
*/
bool PYXInnerTile::intersects(const PYXGeometry& geometry, bool bCommutative) const
{
	const PYXInnerTile* const pTile = dynamic_cast<const PYXInnerTile*>(&geometry);
	if (0 != pTile)
	{
		return asTile().intersects(pTile->asTile(),bCommutative);;
	}
	return asTile().intersects(geometry,bCommutative);
}

/*!
Determine if this geometry has any intersection with the specified geometry.

\param tile		The tile to intersect with this one.

\return true if any intersection exists or false if none is found.
*/
bool PYXInnerTile::intersects(const PYXInnerTile& tile) const
{
	return asTile().intersects(tile);
}

/*!
Determine if this geometry has any intersection with the specified geometry.

\param cell		The cell to intersect with this tile.

\return true if any intersection exists or false if none is found.
*/
bool PYXInnerTile::intersects(const PYXCell& cell) const
{
	return asTile().intersects(cell);
}

/*!
Get an iterator to the individual cells in the geometry.

\return	The iterator (ownership transferred)
*/
PYXPointer<PYXIterator> PYXInnerTile::getIterator() const
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
void PYXInnerTile::copyTo(PYXTileCollection* pTileCollection) const
{
	copyTo(pTileCollection, getCellResolution());
}

/*!
Copies a representation of this geometry into a tile collection at the specified resolution.

\param	pTileCollection		The destination tile collection (must not be null).
\param	nTargetResolution	The target resolution.	
*/
void PYXInnerTile::copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const
{
	assert(pTileCollection != 0);

	pTileCollection->clear();
	pTileCollection->addTile(m_rootIndex, nTargetResolution);
}

PYXBoundingCircle PYXInnerTile::getBoundingCircle() const
{
	return PYXCircleRegion(getRootIndex(),false).getBoundingCircle();
}


/*! 
Allows PYXIS tile to be written to streams.

\param out		The stream to write to.
\param pyxTile	The tile to write to the stream.

\return The stream after the operation.
*/
std::ostream& operator <<(std::ostream& out, const PYXInnerTile& pyxTile)
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
std::istream& operator >>(std::istream& input, PYXInnerTile& pyxTile)
{
	PYXIcosIndex pyxIndex;
	int nResolution;
	input >> pyxIndex >> nResolution;
	PYXInnerTile tempTile(pyxIndex, nResolution);
	pyxTile = tempTile;
	return input;
}

//========================================================
//			Utility Class for Inner Tile
//========================================================

Tester<PYXPrimeInnerTileIterator> gTester;

void PYXPrimeInnerTileIterator::test()
{
	for(int i=0;i<1000;i++)
	{
		PYXIcosIndex index;
		index.randomize(i % 20 + 4);
		int resultIndex= getPrimeInnerTileRootPosition(index);

		PYXIcosIndex result= getPrimeInnerTileRoot(index);

		TEST_ASSERT(result.isAncestorOf(index));
	}

	return ;
}
unsigned int PYXPrimeInnerTileIterator::getPrimeInnerTileRootPosition(const PYXIcosIndex & index)
{
	if(index.isFace())
	{
		return index.getPrimaryResolution()-PYXIcosIndex::kcFaceFirstChar + 12*6;
	}
	else
	{
		unsigned int location = (index.getPrimaryResolution()-PYXIcosIndex::knFirstVertex) *6;
		unsigned int firstDigit =0;
		if(index.getSubIndex().getDigitCount()>0)
			{
				firstDigit = index.getSubIndex().getDigit(0);
			}
		unsigned int gapDigit = index.isNorthern()?1:4;
		if (firstDigit > gapDigit)
		{
			firstDigit--;
		}
		return location+ firstDigit;
	}
}

std::vector<PYXIcosIndex> PYXPrimeInnerTileIterator::m_roots;

void PYXPrimeInnerTileIterator::initStaticData()
{
	// adding vertices
	for(PYXIcosVertexIterator icosVertexIterator (1);
		!icosVertexIterator.end();
		icosVertexIterator.next())
	{
		const PYXIcosIndex & tmpIndex = icosVertexIterator.getIndex();
		m_roots.push_back( tmpIndex );

		for(PYXChildIterator childItr(tmpIndex);!childItr.end();childItr.next())
		{
			const PYXIcosIndex & childIndex = childItr.getIndex();
			if(!childIndex.isMajor())
			{
				m_roots.push_back( childIndex );
			}
		}
	}

	// adding faces
	for(PYXIcosFaceIterator icosFaceIterator (1);
		!icosFaceIterator.end();
		icosFaceIterator.next())
	{
		m_roots.push_back( icosFaceIterator.getIndex() );
	}
}

void PYXPrimeInnerTileIterator::freeStaticData()
{
	m_roots.clear();
}
