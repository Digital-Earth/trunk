#ifndef PYXIS__GEOMETRY__INNER_TILE_H
#define PYXIS__GEOMETRY__INNER_TILE_H
/******************************************************************************
inner_tile.h

begin		: 2012-05-1
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/child_iterator.h"

#include <vector>

/*!
PYXTile provides a means for specifying a collection of cells at a given
resolution that share the same parent index.
*/
//! A collection of cells at a given resolution with the same parent index.
class PYXLIB_DECL PYXInnerTile : public PYXGeometry
{
public:

	//! Test method
	static void test();

	//! Copy creator
	static PYXPointer<PYXInnerTile> create(const PYXInnerTile& tile)
	{
		return PYXNEW(PYXInnerTile, tile);
	}

	//! convert tile to match inner tile if possible. return null if tile root is prime hexagon (res 1)
	static bool covertToInnerTile(const PYXTile& tile, PYXPointer<PYXInnerTile> & result)
	{
		if(tile.getRootIndex().hasVertexChildren())
		{
			if(tile.getRootIndex().getResolution()>1)
			{
				PYXIcosIndex index= tile.getRootIndex();
				index.decrementResolution();
				result = PYXNEW(PYXInnerTile,index,tile.getCellResolution());
			}
			else 
			{
				result = nullptr;
			}
		}
		else 
		{
			result = PYXNEW(PYXInnerTile,tile.getRootIndex(),tile.getCellResolution());
		}
		//convert to bool.
		return result;
	}

	//! Creator
	static PYXPointer<PYXTile> create(const PYXIcosIndex& index, int nCellResolution)
	{
		return PYXNEW(PYXTile, index, nCellResolution);
	}

	static std::vector<PYXInnerTile> createInnerTiles(const PYXTile& tile)
	{
		std::vector<PYXInnerTile> result;
		if(tile.getRootIndex().hasVertexChildren())
		{
			if(tile.getRootIndex().getResolution()>1)
			{
				PYXIcosIndex index= tile.getRootIndex();
				index.decrementResolution();
				result.push_back(PYXInnerTile(index,tile.getCellResolution()));
			}
			else
			{
				PYXIcosIndex index= tile.getRootIndex();
				result.push_back(PYXInnerTile(index,tile.getCellResolution()));
				PYXChildIterator childIt(index);
				// skipping the first 0 child 
				childIt.next();
				for(;!childIt.end();childIt.next())
				{
					result.push_back(PYXInnerTile(childIt.getIndex(),tile.getCellResolution()));
				}
			}
		}
		else
		{
			result.push_back(PYXInnerTile(tile.getRootIndex(),tile.getCellResolution()));
		}
		return result;
	}

	//! Constructor
	PYXInnerTile(const PYXIcosIndex& index, int nCellResolution);

	//! Default constructor
	PYXInnerTile() : m_nCellResolution(0) {}

	//! Destructor
	virtual ~PYXInnerTile() {}

	//! Copy assignment.
	PYXInnerTile& operator=(const PYXInnerTile&);

	//! Equality operator
	bool operator ==(const PYXInnerTile& tile) const;

	//! Inequality operator
	bool operator !=(const PYXInnerTile& tile) const;

	//! Equality operator
	bool operator ==(const PYXTile& tile) const;

	//! Inequality operator
	bool operator !=(const PYXTile& tile) const;


	//! Less than operator
	bool operator <(const PYXInnerTile& tile) const;

	//! Create a copy of the geometry.
	virtual PYXPointer<PYXGeometry> clone() const;

	/*!
	Is the geometry empty.

	\return	true if the geometry is empty (no cells) or false otherwise.
	*/
	//! Determine if the geometry contains any cells
	virtual bool isEmpty() const {return m_index.isNull();}
	static bool isAncestorOf(PYXIcosIndex root,const PYXIcosIndex & index)
	{
		if(root==index)
		{
			return	true;
		}
		root.incrementResolution();
		return  root.isAncestorOf(index);
	}
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

	//! Get the cell that spatially contains all the cells in this tile.
	const PYXCell& getBoundingCell() const;

	//! Get the bounding box for this geometry.
	virtual void getBoundingRects(const ICoordConverter* coordConvertor,
		PYXRect2DDouble* pRect1,
		PYXRect2DDouble* pRect2) const;

	//! Get the intersection of this geometry and the specified geometry.
	virtual PYXPointer<PYXGeometry> intersection(const PYXGeometry& geometry, bool bCommutative = true) const;

	//! Get the intersection of this geometry and the specified geometry.
	PYXPointer<PYXGeometry> intersection(const PYXInnerTile& tile) const;

	//! Get the intersection of this geometry and the specified geometry.
	PYXPointer<PYXGeometry> intersection(const PYXCell& cell) const;

	//! Return a boolean indication of intersection.
	virtual bool intersects(const PYXGeometry& geometry, bool bCommutative = true) const;

	//! Return a boolean indication of intersection.
	bool intersects(const PYXInnerTile& tile) const;

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
					m_rootIndex.isAncestorOf(pyxIndex)	);
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
		return PYXIcosMath::getCellCount(m_rootIndex, getCellResolution());
	}

	PYXTile asTile() const 
	{
		return PYXTile(m_rootIndex,m_nCellResolution);
	}

private:
	//! The parent index
	PYXIcosIndex m_index;

	//! The root index of the tile that is equal to the inner tile
	PYXIcosIndex m_rootIndex;

	//! The PYXIS resolution at which the cells reside
	int	m_nCellResolution;

	//! The bounding cell for temporary use
	mutable PYXCell m_boundingCell;
};

//! Allows PYXIS tiles to be written to streams.
PYXLIB_DECL std::ostream& operator <<(std::ostream& out, const PYXInnerTile& pyxTile);

//! Allows PYXIS tiles to be read from streams.
PYXLIB_DECL std::istream& operator >>(std::istream& input, PYXInnerTile& pyxTile);

//========================================================
//			Utility Class for Inner Tile
//========================================================

class PYXLIB_DECL PYXPrimeInnerTileIterator : public PYXAbstractIterator
{
public:

	static void test();
	PYXPrimeInnerTileIterator():m_position(0)
	{
	}

	static PYXIcosIndex & getPrimeInnerTileRoot(const PYXIcosIndex & index)
	{
		return m_roots[getPrimeInnerTileRootPosition(index)];
	}

	static PYXIcosIndex & getPrimeInnerTileRootByPosition(unsigned int position)
	{
		return m_roots[position];
	}

	static unsigned int getPrimeInnerTileRootPosition(const PYXIcosIndex & index);

	static const unsigned int totalCount=92;

	static void initStaticData();

	static void freeStaticData();

	const PYXIcosIndex & getIndex() const
	{
		return m_roots[m_position];
	}

	const  unsigned int getPosition()
	{
		return m_position;
	}
	//! Move to the next item.
	virtual void next() 
	{
		m_position++;
	}
	/*!
	See if we have covered all the items.
	\return	true if all items have been covered, otherwise false.
	*/
	virtual bool end() const 
	{
		return m_position >= m_roots.size();
	}


private:
	static std::vector<PYXIcosIndex> m_roots;
	unsigned int m_position;
};

#endif // guard
