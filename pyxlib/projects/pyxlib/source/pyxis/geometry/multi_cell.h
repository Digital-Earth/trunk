#ifndef PYXIS__GEOMETRY__MULTI_CELL_H
#define PYXIS__GEOMETRY__MULTI_CELL_H
/******************************************************************************
multi_cell.h

begin		: 2005-09-21
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/geometry/cell.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/derm/iterator.h"

// standard includes
#include <set>

// forward declarations
class PYXCell;

//! Represents multiple cells in a PYXIS grid.
/*!
PYXMultiCell represents multiple, typically disconnected, cells in a PYXIS
grid.
*/
class PYXLIB_DECL PYXMultiCell : public PYXGeometry
{
public:

	//! Test method
	static void test();

	//! Default creator
	static PYXPointer<PYXMultiCell> create()
	{
		return PYXNEW(PYXMultiCell);
	}

	//! Copy creator
	static PYXPointer<PYXMultiCell> create(const PYXMultiCell& rhs)
	{
		return PYXNEW(PYXMultiCell, rhs);
	}

	//! Constructor
	PYXMultiCell() {}

	//! Destructor
	virtual ~PYXMultiCell() {}

	//! Equality operator
	bool operator ==(const PYXMultiCell& rhs) const;

	//! Inequality operator.
	bool operator !=(const PYXMultiCell& rhs) const {return !(*this == rhs);}

	/*!
	Add and index to the set of cells.

	\param	index	The index.
	*/
	void addIndex(const PYXIcosIndex& index) {m_setIndex.insert(index);}

	/*!
	Add a cell's index to the set of cells.

	\param  cell    The cell.
	*/
	void addCell(const PYXPointer<PYXCell> cell) {m_setIndex.insert(cell->getIndex());}

	/*!
	Create a copy of the geometry.
	
	\return	A copy of the geometry (ownership transferred).
	*/
	virtual PYXPointer<PYXGeometry> clone() const {return create(*this);}

	/*!
	Is the geometry empty.

	\return	true if the geometry is empty (no cells) or false otherwise.
	*/
	virtual bool isEmpty() const {return m_setIndex.empty();}

	/*!
	Set the geometry to empty.
	*/
	virtual void setEmpty() {m_setIndex.clear();}

	//! Get the PYXIS resolution of cells in the geometry.
	virtual int getCellResolution() const;

	//! Set the PYXIS resolution of cells in the geometry.
	virtual void setCellResolution(int nCellResolution);

	//! Determine the intersecting geometry.
	virtual PYXPointer<PYXGeometry> intersection(const PYXGeometry& geometry, bool bCommutative = true) const;

	//! Determine the intersecting geometry.
	PYXPointer<PYXGeometry> intersection(const PYXCell& cell) const;

	//! Determine the intersecting geometry.
	PYXPointer<PYXGeometry> intersection(const PYXMultiCell& multiCell) const;

	//! Return a boolean indication of intersection.
	virtual bool intersects(const PYXGeometry& geometry, bool bCommutative = true) const; 

	//! Return a boolean indication of intersection.
	bool intersects(const PYXCell& cell) const;

	//! Return a boolean indication of intersection.
	bool intersects(const PYXMultiCell& multiCell) const;

	//! Get an iterator to the individual cells in the geometry.
	virtual PYXPointer<PYXIterator> getIterator() const;

	//! Calculate a series of PYXIS indices around a geometry.
	/*!
	\param	pVecIndex	The container to hold the returned indices.
	*/
	virtual void calcPerimeter(std::vector<PYXIcosIndex>* pVecIndex) const
	{
		assert(pVecIndex != 0 && "Null pointer.");
		pVecIndex->clear();
		assert(false && "Not yet implemented.");
	}

	//! Copies a representation of this geometry into a tile collection.
	virtual void copyTo(PYXTileCollection* pTileCollection) const;

	//! Copies a representation of this geometry into a tile collection at the specified resolution.
	virtual void copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const;

	virtual PYXBoundingCircle getBoundingCircle() const;

private:

	//! The list of indices (cells)
	typedef std::set<PYXIcosIndex> IndexSet;

	//! Used for an intersection of cells.
	typedef std::vector<PYXIcosIndex> IndexVector;

	//! Iterates over the cells in a multi-cell.
	/*!
	The PYXMultiCellIterator iterates over all the cells in a PYXMultiCell.
	*/
	class PYXMultiCellIterator : public PYXIterator
	{
	public:

		//! Dynamic creator
		static PYXPointer<PYXMultiCellIterator> create(const IndexSet& setIndex)
		{
			return PYXNEW(PYXMultiCellIterator, setIndex);
		}

		//! Constructor
		PYXMultiCellIterator(const IndexSet& setIndex);

		//! Destructor
		virtual ~PYXMultiCellIterator() {}

		//! Move to the next cell.
		virtual void next();

		//! See if we have covered all the cells.
		virtual bool end() const;

		//! Get the PYXIS index for the current cell.
		virtual const PYXIcosIndex& getIndex() const;

	private:
		
		//! The current iterator
		IndexSet::const_iterator m_it;

		//! The end iterator
		IndexSet::const_iterator m_end;
	};

private:

	IndexSet m_setIndex;
};

#endif // guard
