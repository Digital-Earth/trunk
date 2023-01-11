/******************************************************************************
geometry.cpp

begin		: 2004-10-18
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/geometry.h"

// pyxlib includes
#include "pyxis/geometry/tile.h"
#include "pyxis/geometry/inner_tile.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/iterator.h"
#include "pyxis/utility/tester.h"

//! Tester class
Tester<PYXEmptyGeometry> gEmptyGeometryTester;

//! Tester class
Tester<PYXGlobalGeometry> gGlobalGeometryTester;

/*!
Get the intersection of this geometry and the specified geometry.

\param	geometry		The geometry to intersect with this one.
\param	bCommutative	Indicates whether the operation is to be commutative (true) or one-way (false).

\return	The intersection.
*/
PYXPointer<PYXGeometry> PYXGeometry::intersection(	const PYXGeometry& geometry,
													bool bCommutative	) const
{
	if (!bCommutative)
	{
		// Convert both to tile collections to guarantee success.
		PYXTileCollection tc1;
		copyTo(&tc1);
		PYXTileCollection tc2;
		geometry.copyTo(&tc2);
		return tc1.intersection(tc2);
	}

	// Try the reverse call.
	return geometry.intersection(*this, false);
}

/*!
Determine if this geometry has any intersection with the specified geometry.

\param	geometry		The geometry to intersect with this one.
\param	bCommutative	Indicates whether the operation is to be commutative (true) or one-way (false).

\return true if any intersection exists or false if none is found.
*/
bool PYXGeometry::intersects(const PYXGeometry& geometry, bool bCommutative) const
{
	if (!bCommutative)
	{
		// Convert both to tile collections to guarantee success.
		PYXTileCollection tc1;
		copyTo(&tc1);
		PYXTileCollection tc2;
		geometry.copyTo(&tc2);
		return tc1.intersects(tc2);
	}

	// Try the reverse call.
	return geometry.intersects(*this, false);
}

/*! 
Determine this geometry contians the specified geometry.

\param	geometry		The geometry to test for intersection with this one.
	
\return true if this geometry contains the given geometry, otherwise false.
*/
//! Return a boolean indication of contains.
bool PYXGeometry::contains(const PYXGeometry& geometry) const
{
	if (isEmpty())
	{
		return false;
	}

	if (geometry.isEmpty())
	{
		return true;
	}

	PYXTileCollection tc1;
	copyTo(&tc1);
	PYXTileCollection tc2;
	geometry.copyTo(&tc2);
	return tc1.contains(tc2);
}

/*!
Get the disjunction (union) of this geometry and the specified geometry as a new
geometry.

\param	geometry		The geometry to unite with this one.

\return	The disjunction (union) of geometries.
*/
PYXPointer<PYXGeometry> PYXGeometry::disjunction(const PYXGeometry& geometry) const
{
	PYXTileCollection collection;
	geometry.copyTo(&collection);
	return collection.disjunction(*this);
}

PYXPointer<PYXInnerTileIntersectionIterator> PYXGeometry::getInnerTileIterator(const PYXInnerTile & tile) const
{
	PYXTHROW_NOT_IMPLEMENTED();
}






//! Get the bounding box for this geometry.
void PYXGeometry::getBoundingRects(const ICoordConverter* coordConvertor,
	PYXRect2DDouble* pRect1,
	PYXRect2DDouble* pRect2) const
{
	PYXTileCollection collection;
	copyTo(&collection);
	collection.getBoundingRects(coordConvertor, pRect1, pRect2);
}

//! Test method
void PYXEmptyGeometry::test()
{
	PYXPointer<PYXEmptyGeometry> spEmpty = PYXEmptyGeometry::create();
	PYXPointer<PYXGeometry> spGeometry = spEmpty->clone();
	TEST_ASSERT(!spEmpty->intersects(*spGeometry));
	TEST_ASSERT(spEmpty->intersection(*spGeometry)->isEmpty());
}

/*!
Create a copy of the geometry.

\return	A copy of the geometry.
*/
PYXPointer<PYXGeometry> PYXEmptyGeometry::clone() const
{
	return create(*this);
}

/*!
Is the geometry empty.

\return	true if the geometry is empty (no cells) or false otherwise.
*/
bool PYXEmptyGeometry::isEmpty() const
{
	return true;
}

/*!
Get the PYXIS resolution of cells in the geometry.

\return	The PYXIS resolution.
*/
int PYXEmptyGeometry::getCellResolution() const
{
	return 0;
}

/*!
Set the PYXIS resolution of cells in the geometry.

\param	nCellResolution	The cell resolution.
*/
void PYXEmptyGeometry::setCellResolution(int nCellResolution)
{
	// can't set cell resolution on an empty cell
	assert(false);
}

/*!
Get the intersection of this geometry and the specified geometry as a new
geometry.

\param	geometry		The geometry to intersect with this one.
\param	bCommutative	Indicates whether the operation is to be commutative (true) or one-way (false).

\return	The intersection of geometries.
*/
PYXPointer<PYXGeometry> PYXEmptyGeometry::intersection(	const PYXGeometry& geometry,
														bool bCommutative	) const
{
	return PYXEmptyGeometry::create();
}

/*!
Test for intersection with this empty geometry.

\return	false.
*/
bool PYXEmptyGeometry::intersects(const PYXGeometry& geometry, bool bCommutative) const
{
	return false;
}

/*!
Get an iterator to the individual cells in the geometry.

\return	The iterator (ownership transferred)
*/
PYXPointer<PYXIterator> PYXEmptyGeometry::getIterator() const
{
	return PYXEmptyIterator::create();
}

/*!
Calculate a series of PYXIS indices around a geometry.

\param	pVecIndex	The container to hold the returned indices.
*/
inline void PYXEmptyGeometry::calcPerimeter(std::vector<PYXIcosIndex>* pVecIndex) const
{
	assert(pVecIndex != 0 && "Invalid argument.");
	pVecIndex->clear();
}

/*!
Copies a representation of this geometry into a tile collection.

\param	pTileCollection	The destination tile collection (must not be null).
*/
void PYXEmptyGeometry::copyTo(PYXTileCollection* pTileCollection) const
{
	assert(pTileCollection != 0);

	pTileCollection->clear();
}

/*!
Copies a representation of this geometry into a tile collection at the specified resolution.

\param	pTileCollection		The destination tile collection (must not be null).
\param	nTargetResolution	The target resolution.
*/
void PYXEmptyGeometry::copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const
{
	copyTo(pTileCollection);
}

PYXBoundingCircle PYXEmptyGeometry::getBoundingCircle() const 
{
	return PYXBoundingCircle();
}


//! Test method
void PYXGlobalGeometry::test()
{
	PYXGlobalGeometry global(5);
	PYXPointer<PYXGeometry> spGeometry = global.clone();
	TEST_ASSERT(global.intersects(*spGeometry));
	TEST_ASSERT(!(global.intersection(*spGeometry)->isEmpty()));
}

//! Deserialization constructor
PYXGlobalGeometry::PYXGlobalGeometry(std::basic_istream< char>& in) : m_nCellResolution(in.get())
{
}

/*!
Create a copy of the geometry.

\return	A copy of the geometry.
*/
PYXPointer<PYXGeometry> PYXGlobalGeometry::clone() const
{
	return create(*this);
}

//! Serialize.
void PYXGlobalGeometry::serialize(std::basic_ostream< char>& out) const
{
	out.put(m_nCellResolution);
}

/*!
Is the geometry empty.

\return	true if the geometry is empty (no cells) or false otherwise.
*/
bool PYXGlobalGeometry::isEmpty() const
{
	return false;
}

/*!
Get the PYXIS resolution of cells in the geometry.

\return	The PYXIS resolution.
*/
int PYXGlobalGeometry::getCellResolution() const
{
	return m_nCellResolution;
}

/*!
Set the PYXIS resolution of cells in the geometry.

\param	nCellResolution	The cell resolution.
*/
void PYXGlobalGeometry::setCellResolution(int nCellResolution)
{
	m_nCellResolution = nCellResolution;
}

/*!
Get the intersection of this geometry and the specified geometry as a new
geometry.

\param	geometry		The geometry to intersect with this one.
\param	bCommutative	Indicates whether the operation is to be commutative (true) or one-way (false).

\return	The intersection of geometries.
*/
PYXPointer<PYXGeometry> PYXGlobalGeometry::intersection(const PYXGeometry& geometry,
														bool bCommutative	) const
{
	return geometry.clone();
}

/*!
Test for intersection with this empty geometry.

\return	false.
*/
bool PYXGlobalGeometry::intersects(const PYXGeometry& geometry, bool bCommutative) const
{
	return true;
}

/*!
Get an iterator to the individual cells in the geometry.

\return	The iterator (ownership transferred)
*/
PYXPointer<PYXIterator> PYXGlobalGeometry::getIterator() const
{
	return PYXIcosIterator::create(m_nCellResolution);
}

/*!
Calculate a series of PYXIS indices around a geometry.

\param	pVecIndex	The container to hold the returned indices.
*/
inline void PYXGlobalGeometry::calcPerimeter(std::vector<PYXIcosIndex>* pVecIndex) const
{
	assert(pVecIndex != 0 && "Invalid argument.");
	pVecIndex->clear();
}

/*!
Copies a representation of this geometry into a tile collection.

\param	pTileCollection	The destination tile collection (must not be null).
*/
void PYXGlobalGeometry::copyTo(PYXTileCollection* pTileCollection) const
{
	copyTo(pTileCollection, m_nCellResolution);	
}

/*!
Copies a representation of this geometry into a tile collection at the specified resolution.

\param	pTileCollection		The destination tile collection (must not be null).
\param	nTargetResolution	The target resolution.
*/
void PYXGlobalGeometry::copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const
{
	assert(pTileCollection != 0);

	PYXTileCollection tileCollection;
	pTileCollection->clear();

	for (PYXIcosIterator it(1); !it.end(); it.next())
	{
		tileCollection.addTile(PYXTile::create(it.getIndex(), m_nCellResolution));
	}

	tileCollection.copyTo(pTileCollection, nTargetResolution);
}

PYXBoundingCircle PYXGlobalGeometry::getBoundingCircle() const 
{
	return PYXBoundingCircle::global();
}


/*!
Check for equivalence of geometries.

\param	lhs	Any geometry.
\param	rhs	Any other geometry.

\return	True if the geometries are equivalent.
*/
bool operator ==(const PYXGeometry& lhs, const PYXGeometry& rhs)
{
	// Convert both to tile collections to guarantee success.
	PYXTileCollection tcLHS;
	lhs.copyTo(&tcLHS);
	PYXTileCollection tcRHS;
	rhs.copyTo(&tcRHS);
	return tcLHS == tcRHS;
}
