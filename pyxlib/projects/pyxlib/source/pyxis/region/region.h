#ifndef PYXIS__REGION__REGION_H
#define PYXIS__REGION__REGION_H
/******************************************************************************
region.h

begin		: 2010-11-15
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/index.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/geometry/inner_tile_intersection_iterator.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/sphere_math.h"
#include "pyxis/utility/bounding_shape.h"
#include "pyxis/utility/wire_buffer.h"

#include "boost/function.hpp"
#include <vector>


struct PYXLIB_DECL IRegionVisitor : public PYXObject
{
	virtual bool isOptimal() const = 0;

	virtual PYXPointer<IRegionVisitor> trim(const PYXIcosIndex & index) const = 0;

	virtual PYXPointer<PYXInnerTileIntersectionIterator> getInnerTileIterator(const PYXInnerTile & tile) const = 0;

	virtual PYXInnerTileIntersection intersects(const PYXIcosIndex & cell) const = 0;

	virtual PYXInnerTileIntersection intersects(const PYXBoundingCircle & circle,double errorThreshold = 0) const = 0;
};

struct PYXLIB_DECL IRegion : public PYXObject
{
	virtual PYXPointer<IRegion> clone() const = 0;

	virtual int getVerticesCount() const = 0;

	virtual PYXPointer<IRegionVisitor> getVisitor() const = 0;
	
	virtual void serializeVisitor(PYXWireBuffer & buffer,const PYXPointer<IRegionVisitor> & visitor) const = 0;

	virtual PYXPointer<IRegionVisitor> deserializeVisitor(PYXWireBuffer & buffer) const = 0;
	virtual PYXPointer<IRegionVisitor> deserializeVisitor(PYXWireBuffer & buffer,const PYXIcosIndex & index) const
	{
		return deserializeVisitor(buffer);
	}
};

struct PYXCompletelyInsideVisitor : public IRegionVisitor
{
	static PYXPointer<PYXCompletelyInsideVisitor> create()
	{
		return PYXNEW(PYXCompletelyInsideVisitor);
	}

	PYXCompletelyInsideVisitor()
	{
	}

	virtual bool isOptimal() const 
	{ 
		return true;
	}

	virtual PYXPointer<IRegionVisitor> trim(const PYXIcosIndex & index) const 
	{
		return PYXCompletelyInsideVisitor::create();
	}

	virtual PYXPointer<PYXInnerTileIntersectionIterator> getInnerTileIterator(const PYXInnerTile & tile) const
	{
		return PYXInnerTileIntersectionIterator::SingleTileIterator::create(tile,knIntersectionComplete);
	}

	//NOTE: if you ask about cells outside, results are undefined (aka - it just return complete of everything)
	PYXInnerTileIntersection intersects(const PYXBoundingCircle & circle,double errorThreshold = 0) const
	{
		return knIntersectionComplete;
	}

	//NOTE: if you ask about cells outside, results are undefined (aka - it just return complete of everything)
	virtual PYXInnerTileIntersection intersects(const PYXIcosIndex & cell) const
	{
		return knIntersectionComplete;
	}
};
/*!
PYXRegion - basic class for represeting a region on earth.
The basic API allow the user to query about any given cell - and get back CellIntersectionState.
*/
class PYXLIB_DECL PYXRegion : public IRegion
{
public:
	enum CellIntersectionState
	{
		knNone,
		knPartial,
		knComplete
	};

public:
	virtual CellIntersectionState intersects(const PYXIcosIndex & index, bool asTile = false) const = 0;
};

/*!
PYXVectorRegion - basic class for represeting a vector region on earth.
This API extendes the PYXRegion API by querying the border and area contained by the region.

1. getDistanceToBorder(location,errorThreshold = 0) - distance to the border of the region
2. isPointContained(location,errorThreshold = 0) - true if the given point is inside the region.

The errorThreshold allow to specify the wanted accuarcy for this queries. 
This could all the Region to speed up the query time.

As a rule, intersects(Cell) - the error threhsold is around 10% of the cell radius.
*/
class PYXLIB_DECL PYXVectorRegion : public PYXRegion
{
public:
	//! distance in earth radians to the shape border
	virtual double getDistanceToBorder(const PYXCoord3DDouble & location,double errorThreshold = 0) const = 0;

	//! return true if loction is inside the region
	virtual bool isPointContained(const PYXCoord3DDouble & location,double errorThreshold = 0) const = 0;

	virtual CellIntersectionState intersects(const PYXIcosIndex & index, bool asTile=false) const;

	virtual CellIntersectionState intersects(const PYXBoundingCircle & circle,double errorThreshold = 0) const;

	//! return the bounding circle for this region
	virtual PYXBoundingCircle getBoundingCircle() const = 0;


	virtual PYXPointer<IRegionVisitor> getVisitor() const
	{
		PYXTHROW_NOT_IMPLEMENTED();
	}
	
	virtual void serializeVisitor(PYXWireBuffer & buffer,const PYXPointer<IRegionVisitor> & visitor) const
	{
		PYXTHROW_NOT_IMPLEMENTED();
	}

	virtual PYXPointer<IRegionVisitor> deserializeVisitor(PYXWireBuffer & buffer) const
	{
		PYXTHROW_NOT_IMPLEMENTED();
	}

};

class PYXLIB_DECL PYXVectorRegionVisitor : public IRegionVisitor
{
public:
	static PYXPointer<PYXVectorRegionVisitor> create(const PYXPointer<const PYXVectorRegion> & region)
	{
		return PYXNEW(PYXVectorRegionVisitor,region);
	}

	PYXVectorRegionVisitor(const PYXPointer<const PYXVectorRegion> & region);

public:
	virtual bool isOptimal() const
	{
		return true;
	}

	virtual PYXPointer<IRegionVisitor> trim(const PYXIcosIndex & index) const;

	virtual PYXPointer<PYXInnerTileIntersectionIterator> getInnerTileIterator(const PYXInnerTile & tile) const;

	virtual PYXInnerTileIntersection intersects(const PYXIcosIndex & cell) const;

	virtual PYXInnerTileIntersection intersects(const PYXBoundingCircle & circle,double errorThreshold = 0) const;

private:
	class Iterator : public PYXInnerTileIntersectionIterator
	{
	public:
		static PYXPointer<Iterator> create(const PYXPointer<const PYXVectorRegion> & region,const PYXInnerTile & tile)
		{
			return PYXNEW(Iterator,region,tile);
		}

		Iterator(const PYXPointer<const PYXVectorRegion> & region,const PYXInnerTile & tile);

		virtual const PYXInnerTile & getTile() const;
		virtual const PYXInnerTileIntersection & getIntersection() const;
		virtual bool end() const;
		virtual void next();

	private:
		PYXPointer<const PYXVectorRegion> m_region;
		PYXInnerTile m_tile;
		PYXIcosIndex m_index;

		PYXInnerTile m_currentTile;
		PYXInnerTileIntersection m_currentIntersection;

		void findIntersection();
		void findNextInnerTile(PYXIcosIndex & index);
	};

private:
	PYXPointer<const PYXVectorRegion> m_region;
};

/*! 
PYXVectorBufferRegion - add a buffer around a region.
*/
class PYXLIB_DECL PYXVectorBufferRegion : public PYXVectorRegion
{
public:
	virtual PYXPointer<IRegion> clone() const;

	virtual int getVerticesCount() const
	{
		PYXTHROW_NOT_IMPLEMENTED();
	}

	//virtual PYXPointer<IRegionVisitor> getVisitor() const;

	//! distance in earth radians to the shape border
	virtual double getDistanceToBorder(const PYXCoord3DDouble & location,double errorThreshold = 0) const;

	//! return true if loction is inside the region
	virtual bool isPointContained(const PYXCoord3DDouble & location,double errorThreshold = 0) const;

	//! return the bounding circle for this region
	virtual PYXBoundingCircle getBoundingCircle() const;

private:
	//! distance in radians
	double m_bufferRadius;

	PYXPointer<PYXVectorRegion> m_region;

public:
	static PYXPointer<PYXVectorBufferRegion> create(const PYXPointer<PYXVectorRegion> & region,double bufferRadius)
	{
		return PYXNEW(PYXVectorBufferRegion,region,bufferRadius);
	}

	PYXVectorBufferRegion(const PYXPointer<PYXVectorRegion> & region,double bufferRadius);

	PYXVectorBufferRegion(const PYXPointer<PYXVectorBufferRegion> & region,double bufferRadius);
};

/*
PYXCollectionVectorRegion - a index based collection of non-intersection PYXVectorRegions
*/
class PYXLIB_DECL PYXCollectionVectorRegion : public PYXVectorRegion
{
public:
	virtual PYXPointer<IRegion> clone() const;

	virtual int getVerticesCount() const
	{
		PYXTHROW_NOT_IMPLEMENTED();
	}

	//virtual PYXPointer<IRegionVisitor> getVisitor() const;

	//! distance in radians
	virtual double getDistanceToBorder(const PYXCoord3DDouble & location,double errorThreshold = 0) const;

	virtual bool isPointContained(const PYXCoord3DDouble & location,double errorThreshold = 0) const;

	virtual PYXBoundingCircle getBoundingCircle() const;

private:
	PYXBoundingCircle m_boundingCircle;
	std::vector< PYXPointer<PYXVectorRegion> > m_regions;

public:
	static PYXPointer<PYXCollectionVectorRegion> create(const std::vector< PYXPointer<PYXVectorRegion> > & regions)
	{
		return PYXNEW(PYXCollectionVectorRegion,regions);
	}

	PYXCollectionVectorRegion(const std::vector< PYXPointer<PYXVectorRegion> > & regions);

	PYXCollectionVectorRegion()
	{
		PYXTHROW_NOT_IMPLEMENTED();
	}

public:
	PYXPointer<PYXVectorRegion> getRegion(int index) 
	{
		return m_regions[index];
	}

	int getRegionCount() const
	{
		return m_regions.size();
	}

	PYXPointer<const PYXVectorRegion> getRegion(int index) const
	{
		return m_regions[index];
	}

	void addRegion(const PYXPointer<PYXVectorRegion> & region)
	{
		m_regions.push_back(region);
	}

	void removeRegion(const PYXPointer<PYXVectorRegion> & region)
	{
		std::vector< PYXPointer<PYXVectorRegion> >::iterator it = std::find(m_regions.begin(),m_regions.end(),region);
		if (it!=m_regions.end())
		{
			m_regions.erase(it);
			findBoundingCircle();
		}
	}

protected:
	void findBoundingCircle();
};

#endif // guard
