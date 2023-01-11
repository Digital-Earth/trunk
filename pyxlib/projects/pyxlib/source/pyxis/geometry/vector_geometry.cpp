/******************************************************************************
vector_geometry.cpp

begin		: 2010-11-15
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/vector_geometry.h"
#include "pyxis/geometry/vector_geometry2.h"
#include "pyxis/geometry/geometry_intersection_utils.h"

// pyxlib includes
#include "pyxis/geometry/exceptions.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/region/circle_region.h"
#include "pyxis/derm/child_iterator.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/utility/coord_lat_lon.h"
#include "pyxis/utility/sphere_math.h"
#include "pyxis/utility/tester.h"

//depth 6 tile have 1261 cells. However, raster a geometry into depth 6 we seldom get 1261 cells.
//on average, raster 6+3 resolutions deep will get us around 1000 tiles.
//A tradeoff between speed and details
const int SMALL_ENOUGH_TILE_DEPTH = 9;

// standard includes
#include <cfloat>

namespace
{
	//! Vector of resolution 2 indices with which to begin traversal.
	std::vector<PYXIcosIndex> vecRes2;
}
/*!
Module set-up.
*/
void PYXVectorGeometry::initStaticData()
{	
	// Initialize vecRes2.
	{
		PYXIcosIterator it(2);
		for (; !it.end(); it.next())
		{
			vecRes2.push_back(it.getIndex());
		}
	}
}

/*!
Module tear-down.
*/
void PYXVectorGeometry::freeStaticData()
{
}

PYXVectorGeometry::PYXVectorGeometry(const PYXBoundingCircle & circle)
{
	if (circle.isEmpty())
	{
		m_region.reset();
	}
	else if (circle.getRadius() == 0) 
	{
		m_region = PYXVectorPointRegion::create(circle.getCenter());
		m_nResolution = 24;
	}
	else 
	{
		m_region = PYXCircleRegion::create(circle.getCenter(),circle.getRadius());
		m_nResolution = std::max(2,PYXBoundingCircle::estimateResolutionFromRadius(circle.getRadius()));
	}
}

/*!
Get the intersection of this geometry and the specified geometry.

\param	geometry		The geometry to intersect with this one.
\param	bCommutative	Indicates whether the operation is to be commutative (true) or one-way (false).

\return	The intersection.
*/
PYXPointer<PYXGeometry> PYXVectorGeometry::intersection(	const PYXGeometry& geometry,
															bool bCommutative	) const
{
	if (!m_region)
	{
		return PYXEmptyGeometry::create();
	}

	const PYXTileCollection* const pTileCollection = 
		dynamic_cast<const PYXTileCollection*>(&geometry);
	if (pTileCollection)
	{
		return intersection(*pTileCollection);
	}

	const PYXTile* const pTile = dynamic_cast<const PYXTile*>(&geometry);
	if (pTile)
	{
		return intersection(*pTile);
	}

	const PYXCell* const pCell = dynamic_cast<const PYXCell*>(&geometry);
	if (pCell)
	{
		return intersection(*pCell);
	}

	if (!m_spGeometry)
	{
		createGeometry();
	}
	return m_spGeometry->intersection(geometry);
}

PYXPointer<PYXGeometry> PYXVectorGeometry::intersection(const PYXTileCollection& collection) const
{
	if (!m_region)
	{
		return PYXEmptyGeometry::create();
	}

	std::vector<PYXIcosIndex> vec;
	for (PYXPointer<PYXTileCollectionIterator> spIt = collection.getTileIterator();
		!spIt->end(); spIt->next())
	{
		vec.push_back(spIt->getTile()->getRootIndex());
	}
	PYXPointer<PYXTileCollection> tc = PYXTileCollection::create();
	tc->setCellResolution(collection.getCellResolution());
	intersectionImpl(*tc, vec, collection.getCellResolution());
	return tc;
}

PYXPointer<PYXGeometry> PYXVectorGeometry::intersection(const PYXTile& tile) const
{
	if (!m_region)
	{
		return PYXEmptyGeometry::create();
	}

	std::vector<PYXIcosIndex> vec(1, tile.getRootIndex());
	PYXPointer<PYXTileCollection> tc = PYXTileCollection::create();
	tc->setCellResolution(tile.getCellResolution());
	intersectionImpl(*tc, vec, tile.getCellResolution());
	return tc;
}

PYXPointer<PYXGeometry> PYXVectorGeometry::intersection(const PYXCell& cell) const
{
	if (!m_region)
	{
		return PYXEmptyGeometry::create();
	}

	PYXPointer<PYXGeometry> spGeom;
	
	if (m_region->intersects(cell.getIndex()) == PYXRegion::knNone)
	{
		spGeom = PYXEmptyGeometry::create();
	}
	else
	{
		spGeom = PYXCell::create(cell);
	}

	return spGeom;
}


/*!
Determine if this geometry has any intersection with the specified geometry.

\param	geometry		The geometry to intersect with this one.
\param	bCommutative	Indicates whether the operation is to be commutative (true) or one-way (false).

\return true if any intersection exists or false if none is found.
*/
bool PYXVectorGeometry::intersects(const PYXGeometry& geometry, bool bCommutative) const
{	
	if (!m_region)
	{
		return false;
	}

	const PYXCell* const pCell = dynamic_cast<const PYXCell*>(&geometry);
	if (pCell != NULL)
	{
		return m_region->intersects(pCell->getIndex()) != PYXRegion::knNone;
	}

	const PYXTile* const pTile = dynamic_cast<const PYXTile*>(&geometry);
	if (pTile != NULL)
	{
		return m_region->intersects(pTile->getRootIndex(),true) != PYXRegion::knNone;
	}

	const PYXTileCollection* const pTileCollection = dynamic_cast<const PYXTileCollection*>(&geometry);
	if (pTileCollection != NULL)
	{
		//optimization for point regions
		auto point = dynamic_cast<const PYXVectorPointRegion *>(m_region.get());
		if (point)
		{
			PYXIcosIndex index;
			SnyderProjection::getInstance()->xyzToPYXIS(point->getPoint(),&index,pTileCollection->getCellResolution());			
			return pTileCollection->intersects(index);
		}

		//our geometry is smaller than the tile collection resolution. it better to rasterize ourself.
		if (PYXBoundingCircle::estimateResolutionFromRadius(m_region->getBoundingCircle().getRadius()) >= pTileCollection->getCellResolution())
		{
			PYXTileCollection overview;
			copyTo(&overview,pTileCollection->getCellResolution());
			return overview.intersects(*pTileCollection);
		}
		
		int resolution = std::min(pTileCollection->getCellResolution(),PYXBoundingCircle::estimateResolutionFromRadius(m_region->getBoundingCircle().getRadius())+5);
		PYXTileCollection lowResRaster;
		copyTo(&lowResRaster,resolution);

		for (auto it = lowResRaster.getTileIterator();!it->end();it->next())
		{
			if (pTileCollection->intersects(*(it->getTile()))) 
			{
				return true;
			}
		}
		return false;
	}
	
	auto otherCircle = geometry.getBoundingCircle();
	auto thisCircle = getBoundingCircle();

	if (!thisCircle.intersects(otherCircle)) 
	{
		return false;
	}

	//if other geometry is a circle...
	if (PYXGeometryIntersectionUtils::isCircle(geometry)) 
	{
		auto vectorRegion = dynamic_cast<const PYXVectorRegion*>(m_region.get());
		if (vectorRegion) 
		{
			return vectorRegion->intersects(otherCircle) != PYXRegion::knNone;
		}
	}

	//if this geometry is a circle
	if (PYXGeometryIntersectionUtils::isCircle(*this))
	{
		auto vectorRegion = dynamic_cast<const PYXVectorRegion*>(PYXGeometryIntersectionUtils::extractRegion(geometry).get());
		if (vectorRegion)
		{
			return vectorRegion->intersects(thisCircle) != PYXRegion::knNone;
		}
	}

	PYXPointer<PYXTileCollection> tiles;
	if (thisCircle.getRadius() < otherCircle.getRadius()) 
	{
		tiles = PYXGeometryIntersectionUtils::createSmallOverview(*this);
	}
	else 
	{
		tiles = PYXGeometryIntersectionUtils::createSmallOverview(geometry);
	}

	//intersects resolutions is +SMALL_ENOUGH_TILE_DEPTH deep than overview resolution
	tiles->setCellResolution(std::min(PYXMath::knMaxAbsResolution,tiles->getCellResolution() + SMALL_ENOUGH_TILE_DEPTH));

	return PYXGeometryIntersectionUtils::intersectsAtTiles(*tiles, *this, geometry);
}


bool PYXVectorGeometry::contains(const PYXGeometry& geometry) const
{
	const PYXCell* const pCell = dynamic_cast<const PYXCell*>(&geometry);
	if (pCell != NULL)
	{
		return m_region->intersects(pCell->getIndex(),false) == PYXRegion::knComplete;
	}
	
	const PYXTile* const pTile = dynamic_cast<const PYXTile*>(&geometry);
	if (pTile != NULL)
	{
		return m_region->intersects(pTile->getRootIndex(),true) == PYXRegion::knComplete;
	}

	if (!m_spGeometry)
	{
		createGeometry();
	}
	return m_spGeometry->contains(geometry);
}



/*!
Get the cell resolution.

\return	The cell resolution.
*/
int PYXVectorGeometry::getCellResolution() const
{
	return m_nResolution;
}

/*!
Set the PYXIS resolution of cells in the geometry. This method changes the
resolution of each of the nodes in the line.

\param	nCellResolution	The cell resolution.
*/
void PYXVectorGeometry::setCellResolution(int nCellResolution)
{
	assert(0 <= nCellResolution && nCellResolution < PYXMath::knMaxAbsResolution);

	m_nResolution = nCellResolution;
	m_spGeometry.reset();
}

/*!
Get an iterator to the individual cells in the geometry.

\return	The iterator (ownership transferred)
*/
PYXPointer<PYXIterator> PYXVectorGeometry::getIterator() const
{
	createGeometry();
	return m_spGeometry->getIterator();
}

/*!
Get an iterator to the individual cells in the geometry.

\return	The iterator (ownership transferred)
*/
PYXPointer<PYXInnerTileIntersectionIterator> PYXVectorGeometry::getInnerTileIterator(const PYXInnerTile & tile) const
{
	return m_region->getVisitor()->getInnerTileIterator(tile);
}

/*!
Create a copy of the geometry.

\return	A copy of the geometry.
*/
//! Create a copy of the geometry.
PYXPointer<PYXGeometry> PYXVectorGeometry::clone() const
{
	return create(*this);
}

/*!
Copies a representation of this geometry into a tile collection.

\param pTileCollection	The tile collection.
*/
void PYXVectorGeometry::copyTo(PYXTileCollection* pTileCollection) const
{
	copyTo(pTileCollection, getCellResolution());
}

/*!
Copies a representation of this geometry into a tile collection at the specified resolution.

\param	pTileCollection		The tile collection.
\param	nTargetResolution	The target resolution.
*/
void PYXVectorGeometry::copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const
{
	assert(pTileCollection != 0);

	pTileCollection->clear();

	if (m_spGeometry)
	{
		m_spGeometry->copyTo(pTileCollection, nTargetResolution);
	}
	else 
	{
		intersectionImpl(*pTileCollection,vecRes2,nTargetResolution);
	}
}

void PYXVectorGeometry::createGeometry() const
{
	static boost::recursive_mutex s_mutex[32];

	boost::recursive_mutex::scoped_lock lock(s_mutex[(((int)(this))/256)%32] );

	if (!m_spGeometry)
	{
		if (!m_region)
		{
			m_spGeometry = PYXEmptyGeometry::create();
		}
		else
		{
			PYXPointer<PYXTileCollection> tc = PYXTileCollection::create();
			tc->setCellResolution(m_nResolution);
			intersectionImpl(*tc,vecRes2,m_nResolution); 
			m_spGeometry = tc;
		}
	}
}

/*!
Traverse the intermediate data structures using 3D math.
*/
void PYXVectorGeometry::intersectionImpl(PYXTileCollection & tc, std::vector<PYXIcosIndex> vec, int nTargetResolution) const
{
	// Traversal.
	while (!vec.empty())
	{
		PYXIcosIndex& index = vec.back();
		if (index.getResolution() != nTargetResolution)
		{
			PYXRegion::CellIntersectionState intersectionState = m_region->intersects(index,true);

			if (intersectionState == PYXRegion::knNone)
			{
				vec.pop_back();
			}
			else if (intersectionState == PYXRegion::knComplete)
			{
				tc.addTile(index, nTargetResolution);
				vec.pop_back();
			}
			else
			{
				PYXChildIterator it(index);
				vec.pop_back();
				for (; !it.end(); it.next())
				{
					vec.push_back(it.getIndex());
				}
			}
		}
		else
		{
			// Classify points at data resolution.
			if ( m_region->intersects(index) != PYXRegion::knNone)
			{
				tc.addTile(index, nTargetResolution);
			}
			vec.pop_back();
		}
	}
}