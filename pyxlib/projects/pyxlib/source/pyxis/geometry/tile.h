#ifndef PYXIS__GEOMETRY__TILE_H
#define PYXIS__GEOMETRY__TILE_H
/******************************************************************************
tile.h

begin		: 2004-11-15
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/geometry/cell.h"
#include "pyxis/geometry/geometry.h"

// Forward declarations
class PYXBoundingRectsCalculator;

/*!
PYXTile provides a means for specifying a collection of cells at a given
resolution that share the same parent index.
*/
//! A collection of cells at a given resolution with the same parent index.
class PYXLIB_DECL PYXTile : public PYXGeometry
{
public:

	//! Test method
	static void test();

	// Tile depth constants
	static const int knDefaultTileDepth;

	//! Copy creator
	static PYXPointer<PYXTile> create(const PYXTile& tile)
	{
		return PYXNEW(PYXTile, tile);
	}

	//! Creator
	static PYXPointer<PYXTile> create(const PYXIcosIndex& index, int nCellResolution)
	{
		return PYXNEW(PYXTile, index, nCellResolution);
	}

	//! Constructor
	PYXTile(const PYXIcosIndex& index, int nCellResolution);
	
	//! Default constructor
	PYXTile() : m_nCellResolution(0) {}

	//! Destructor
	virtual ~PYXTile() {}

	//! Copy assignment.
	PYXTile& operator=(const PYXTile&);

	//! Equality operator
	bool operator ==(const PYXTile& tile) const;

	//! Inequality operator
	bool operator !=(const PYXTile& tile) const;

	//! Less than operator
	bool operator <(const PYXTile& tile) const;

	//! Create a copy of the geometry.
	virtual PYXPointer<PYXGeometry> clone() const;

	/*!
	Is the geometry empty.

	\return	true if the geometry is empty (no cells) or false otherwise.
	*/
	//! Determine if the geometry contains any cells
	virtual bool isEmpty() const {return m_index.isNull();}

	/*!
	Get the cell resolution.

	\return	The cell resolution.
	*/
	virtual int getCellResolution() const
	{
		return m_nCellResolution;
	}

	//! Set the PYXIS resolution of cells in the geometry.
	virtual void setCellResolution(int nCellResolution);

	//! Set the root index of the tile to be an origin child
	virtual void setOriginChildRoot();

	//! Get the cell that spatially contains all the cells in this tile.
	const PYXCell& getBoundingCell() const;

	//! Get the bounding box for this geometry.
	virtual void getBoundingRects(const ICoordConverter* coordConvertor,
		PYXRect2DDouble* pRect1,
		PYXRect2DDouble* pRect2) const;

	//! Get the intersection of this geometry and the specified geometry.
	virtual PYXPointer<PYXGeometry> intersection(const PYXGeometry& geometry, bool bCommutative = true) const;

	//! Get the intersection of this geometry and the specified geometry.
	PYXPointer<PYXGeometry> intersection(const PYXTile& tile) const;

	//! Get the intersection of this geometry and the specified geometry.
	PYXPointer<PYXGeometry> intersection(const PYXCell& cell) const;

	//! Return a boolean indication of intersection.
	virtual bool intersects(const PYXGeometry& geometry, bool bCommutative = true) const;

	//! Return a boolean indication of intersection.
	bool intersects(const PYXTile& tile) const;

	//! Return a boolean indication of intersection.
	bool intersects(const PYXCell& cell) const;

	//! Get an iterator to the individual cells in the geometry.
	virtual PYXPointer<PYXIterator> getIterator() const;

	/*!
	\param	pVecIndex	The container to hold the returned indices.
	*/
	//! Calculate a series of PYXIS indices around a geometry.
	virtual void calcPerimeter(std::vector<PYXIcosIndex>* pVecIndex) const
	{
		assert(pVecIndex != 0 && "Null pointer.");
		pVecIndex->clear();
		assert(false && "Not yet implemented.");
	}

	virtual PYXBoundingCircle getBoundingCircle() const;

	//! Copies a representation of this geometry into a tile collection.
	virtual void copyTo(PYXTileCollection* pTileCollection) const;

	//! Copies a representation of this geometry into a tile collection at the specified resolution.
	virtual void copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const;

	/*
	This method will only match for a child cell that is at the cell
	resolution.

	\param pyxIndex	The index to test for containment within the tile.

	\return true if the tile is contained within the tile.
	*/
	//! Determine if an index is contained within the tile geometry.
	bool hasIndex(const PYXIcosIndex& pyxIndex) const
	{
		return (	pyxIndex.getResolution() == m_nCellResolution &&
					m_index.isAncestorOf(pyxIndex)	);
	}

	/*!
	Get the root index.

	\return	The root index.
	*/
	//! Get the root index of the tile
	const PYXIcosIndex& getRootIndex() const
	{
		return m_index;
	}

	//! Get the tile depth
	int getDepth() const;

	//! Get the number of cells covered by this tile
	int getCellCount() const
	{
		return PYXIcosMath::getCellCount(getRootIndex(), getCellResolution());
	}


private:

	//! Helper function for finding the XY bounds of a tile
	void PYXTile::addCornersToCalculator(PYXBoundingRectsCalculator* pCalc) const;

	//! The parent index
	PYXIcosIndex m_index;

	//! The PYXIS resolution at which the cells reside
	int	m_nCellResolution;

	//! The bounding cell for temporary use
	mutable PYXCell m_boundingCell;
};

//! Allows PYXIS tiles to be written to streams.
PYXLIB_DECL std::ostream& operator <<(std::ostream& out, const PYXTile& pyxTile);

//! Allows PYXIS tiles to be read from streams.
PYXLIB_DECL std::istream& operator >>(std::istream& input, PYXTile& pyxTile);

#endif // guard
