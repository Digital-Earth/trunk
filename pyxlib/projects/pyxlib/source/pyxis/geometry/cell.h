#ifndef PYXIS__GEOMETRY__CELL_H
#define PYXIS__GEOMETRY__CELL_H
/******************************************************************************
cell.h

begin		: 2004-11-17
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/derm/index.h"

// forward declarations
class PYXIterator;

//! Represents a single cell in a PYXIS grid.
/*!
PYXCell represents a single cell in a PYXIS grid.
*/
class PYXLIB_DECL PYXCell : public PYXGeometry
{
public:

	//! Test method
	static void test();

	//! Default creator
	static PYXPointer<PYXCell> create()
	{
		return PYXNEW(PYXCell);
	}

	//! Copy creator
	static PYXPointer<PYXCell> create(const PYXCell& rhs)
	{
		return PYXNEW(PYXCell, rhs);
	}

	//! Creator
	static PYXPointer<PYXCell> create(const PYXIcosIndex& index)
	{
		return PYXNEW(PYXCell, index);
	}

	//! Constructor
	PYXCell() {}

	//! Constructor
	explicit PYXCell(const PYXIcosIndex& index) : m_index(index) {}

	//! Destructor
	virtual ~PYXCell() {}

	//! Equality operator
	bool operator ==(const PYXCell& rhs) const;

	//! Inequality operator.
	bool operator !=(const PYXCell& rhs) const {return !(*this == rhs);}

	//! Less than operator
	bool operator <(const PYXCell& rhs) const;

	/*!
	Set the index of the cell.

	\param	index	The index.
	*/
	void setIndex(const PYXIcosIndex& index) {m_index = index;}

	/*!
	Get the index of the cell.
	
	\return	The index.
	*/
	const PYXIcosIndex& getIndex() const {return m_index;}

	/*!
	Create a copy of the geometry.
	
	\return	A copy of the geometry (ownership transferred).
	*/
	virtual PYXPointer<PYXGeometry> clone() const;

	/*!
	Is the geometry empty.

	\return	true if the geometry is empty (no cells) or false otherwise.
	*/
	virtual bool isEmpty() const {return m_index.isNull();}

	/*!
	Set the geometry to empty.
	*/
	virtual void setEmpty() {m_index.reset();}

	//! Get the PYXIS resolution of cells in the geometry.
	virtual int getCellResolution() const;

	//! Set the PYXIS resolution of cells in the geometry.
	virtual void setCellResolution(int nCellResolution);

	//! Determine the intersecting geometry.
	virtual PYXPointer<PYXGeometry> intersection(const PYXGeometry& geometry, bool bCommutative = true) const;

	//! Determine the intersecting geometry.
	PYXPointer<PYXGeometry> intersection(const PYXCell& cell) const;

	//! Return a boolean indication of intersection.
	virtual bool intersects(const PYXGeometry& geometry, bool bCommutative = true) const;

	//! Return a boolean indication of intersection.
	bool intersects(const PYXCell& cell) const;

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

	//! Get the bounding box for this geometry.
	virtual void getBoundingRects(const ICoordConverter* coordConvertor,
		PYXRect2DDouble* pRect1,
		PYXRect2DDouble* pRect2) const;

	virtual PYXBoundingCircle getBoundingCircle() const;

private:

	//! The index of this cell.
	PYXIcosIndex m_index;
};

//! Allows PYXIS cells to be written to streams.
PYXLIB_DECL std::ostream& operator <<(std::ostream& out, const PYXCell& pyxCell);

//! Allows PYXIS cells to be read from streams.
PYXLIB_DECL std::istream& operator >>(std::istream& input, PYXCell& pyxCell);

#endif // guard
