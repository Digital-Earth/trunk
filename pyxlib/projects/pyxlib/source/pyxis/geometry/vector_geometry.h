#ifndef PYXIS__GEOMETRY__VECTOR_GEOMETRY_H
#define PYXIS__GEOMETRY__VECTOR_GEOMETRY_H
/******************************************************************************
vector_geometry.h

begin		: 2010-11-15
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/index.h"
#include "pyxis/geometry/cell.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/region/region.h"

/*!
A Light Wieght class to transfrom PYXVectorRegion into PYXGeometry
*/
//! A Light Wieght class to transfrom PYXVectorRegion into PYXGeometry
class PYXLIB_DECL PYXVectorGeometry : public PYXGeometry
{
public:

	//! Test method
	//static void test();

	//! Create a geometry.
	static PYXPointer<PYXVectorGeometry> create(const PYXPointer<PYXVectorRegion> & region,int resolution)
	{
		return PYXNEW(PYXVectorGeometry,region,resolution);
	}

	//! Create a geometry.
	static PYXPointer<PYXVectorGeometry> create(const PYXBoundingCircle & circle)
	{
		return PYXNEW(PYXVectorGeometry,circle);
	}

	/*!
	Create a copy of the geometry.

	\return	A copy of the geometry.
	*/
	//! Create a copy of the geometry.
	static PYXPointer<PYXVectorGeometry> create(const PYXVectorGeometry & rhs)
	{
		return PYXNEW(PYXVectorGeometry, rhs);
	}

	//! Constructor
	PYXVectorGeometry(const PYXPointer<PYXVectorRegion> & region,int resolution) : m_region(region), m_nResolution(resolution) {}

	PYXVectorGeometry(const PYXBoundingCircle & circle);

	//! Copy constructor
	PYXVectorGeometry(const PYXVectorGeometry& rhs) :
		m_region(dynamic_cast<PYXVectorRegion*>(rhs.m_region->clone().get())),
		m_nResolution(rhs.m_nResolution),
		m_spGeometry(rhs.m_spGeometry ? rhs.m_spGeometry->clone() : PYXPointer<PYXGeometry>())
	{
	}

	//! Destructor
	virtual ~PYXVectorGeometry() {}
	
	const PYXPointer<PYXVectorRegion> & getRegion() const { return m_region; }

	void setRegion(const PYXPointer<PYXVectorRegion> & region)
	{
		m_region = region;
		m_spGeometry.reset();
	}

	/*!
	Create a copy of the geometry.

	\return	A copy of the geometry.
	*/
	//! Create a copy of the geometry.
	virtual PYXPointer<PYXGeometry> clone() const;

	/*!
	Is the geometry empty?

	\return	true if the geometry is empty (no cells) or false otherwise.
	*/
	//! Is the geometry empty?
	virtual bool isEmpty() const { return false; }

	//! Get the cell resolution.
	virtual int getCellResolution() const;

	//! Set the PYXIS resolution of cells in the geometry.
	virtual void setCellResolution(int nCellResolution);

	//! Get the intersection of this geometry and the specified geometry.
	virtual PYXPointer<PYXGeometry> intersection(const PYXGeometry& geometry, bool bCommutative = true) const;

	//! Get the intersection of this geometry and the specified geometry.
	PYXPointer<PYXGeometry> intersection(const PYXTileCollection& collection) const;

	//! Get the intersection of this geometry and the specified geometry.
	PYXPointer<PYXGeometry> intersection(const PYXTile& tile) const;

	//! Get the intersection of this geometry and the specified geometry.
	PYXPointer<PYXGeometry> intersection(const PYXCell& cell) const;

	//! Is the specified geometry contained by this one?
	virtual bool contains(const PYXGeometry& geometry) const;
	
	//! Does the specified geometry intersect with this one?
	virtual bool intersects(const PYXGeometry& geometry, bool bCommutative = true) const; 

	//! Get an iterator to the individual cells in the geometry.
	virtual PYXPointer<PYXIterator> getIterator() const;

	/*!
	Get an iterator to the individual cells in the geometry.

	\return	The iterator (ownership transferred)
	*/
	virtual PYXPointer<PYXInnerTileIntersectionIterator> getInnerTileIterator(const PYXInnerTile & tile) const;


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

	virtual PYXBoundingCircle getBoundingCircle() const
	{
		return m_region->getBoundingCircle();
	}

	//! Copies a representation of this geometry into a tile collection.
	virtual void copyTo(PYXTileCollection* pTileCollection) const;

	//! Copies a representation of this geometry into a tile collection at the specified resolution.
	virtual void copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const;


private:
	void intersectionImpl(PYXTileCollection & tc, std::vector<PYXIcosIndex> vec, int nTargetResolution) const;

	//! Create the geometry for this spherical polygon.
	void createGeometry() const;

protected:

	//Emptry constructor. to allow derived classes to create them self from streams.
	PYXVectorGeometry() : m_region(),m_nResolution(0)
	{
	}
	
public:
	//! Module set-up.
	static void initStaticData();

	//! Module tear-down.
	static void freeStaticData();

private:
	//! the cell resolution to use
	int m_nResolution;

	//! The real vector region
	PYXPointer<PYXVectorRegion> m_region;

	//! The geometry (tile collection).
	mutable PYXPointer<PYXGeometry> m_spGeometry;
};

#endif // guard
