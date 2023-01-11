/******************************************************************************
geometry_intersection_utils.cpp

begin		: 2016-03-26
copyright	: (C) 2016 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/geometry_intersection_utils.h"
#include "pyxis/geometry/inner_tile_intersection_iterator.h"
#include "pyxis/geometry/vector_geometry.h"
#include "pyxis/geometry/vector_geometry2.h"
#include "pyxis/region/circle_region.h"
#include "pyxis/utility/tester.h"

///////////////////////////////////////////////////////////////////////////

namespace { Tester<PYXGeometryIntersectionUtils> gTester; }

void PYXGeometryIntersectionUtils::test()
{
}

bool PYXGeometryIntersectionUtils::boundingCirclesIntersect(	const PYXGeometry& a,
																const PYXGeometry& b )
{
	auto circleA = a.getBoundingCircle();
	auto circleB = b.getBoundingCircle();
	return circleA.intersects( circleB );
}


bool PYXGeometryIntersectionUtils::intersectsAtInnerTile(	const PYXInnerTile& innerTile,
															const PYXGeometry& a,
															const PYXGeometry& b,
															Optimization optimization )
{
	const int TILE_DEPTH_CONSIDER_TO_BE_BIG = 10;

	//optimization for large inner tiles
	if (optimization == knMultiResolutionOptimization && innerTile.getDepth() > TILE_DEPTH_CONSIDER_TO_BE_BIG)
	{
		//create a tile that is half depth
		PYXInnerTile smallerInnerTile ( innerTile );
		smallerInnerTile.setCellResolution(innerTile.getCellResolution() - innerTile.getDepth() / 2);

		if ( ! PYXInnerTileIntersectionIterator::intersects(	*a.getInnerTileIterator(smallerInnerTile),
																*b.getInnerTileIterator(smallerInnerTile)  ))
		{
			//if smaller tile don't intersect, bigger tiles won't intersect as well
			return false;
		}
	}

	return PYXInnerTileIntersectionIterator::intersects(	*a.getInnerTileIterator(innerTile),
															*b.getInnerTileIterator(innerTile)  );
}

bool PYXGeometryIntersectionUtils::intersectsAtTile(	const PYXTile& tile,
														const PYXGeometry& a,
														const PYXGeometry& b,
														Optimization optimization)
{
	PYXPointer<PYXInnerTile> innerTile;
	if (PYXInnerTile::covertToInnerTile(tile,innerTile)) 
	{
		return intersectsAtInnerTile(*innerTile, a, b, optimization);
	} 
	else
	{
		for (auto innerTile2 : PYXInnerTile::createInnerTiles(tile)) 
		{
			if (intersectsAtInnerTile(innerTile2, a, b, optimization)) 
			{
				return true;
			}
		}
		return false;
	}
}


bool PYXGeometryIntersectionUtils::intersectsAtTiles(	const PYXTileCollection & tiles, 
														const PYXGeometry& a,
														const PYXGeometry& b,
														Optimization optimization  )
{
	for(auto iterator = tiles.getTileIterator(); !iterator->end(); iterator->next() ) 
	{
		if (PYXGeometryIntersectionUtils::intersectsAtTile(*iterator->getTile(), a, b, optimization)) 
		{
			return true;
		}
	}
	return false;
}

PYXPointer<PYXTileCollection> PYXGeometryIntersectionUtils::createSmallOverview( const PYXGeometry & geometry , int overviewTileCount) 
{
	auto overview = PYXTileCollection::create();

	auto geometryAsTileCollection = dynamic_cast<const PYXTileCollection *>(&geometry);
	if (geometryAsTileCollection) 
	{
		//estimate right resolution to use...
		auto geometryCount = geometryAsTileCollection->getGeometryCount();
		auto resolution = geometryAsTileCollection->getCellResolution();

		while (geometryCount > overviewTileCount)
		{
			resolution--;
			geometryCount /= 3;
		}

		//copy with the given resolutions
		geometryAsTileCollection->copyTo(overview.get(),resolution);
	} 
	else 
	{
		//resolution of depth 5 produce around 50 tiles to check intersection on
		const int DEFAULT_OVERVIEW_RESOLUTION_OFFSET = 5;
		const int DEFAULT_OVERVIEW_SIZE = 50;

		int resolutionOffset = DEFAULT_OVERVIEW_RESOLUTION_OFFSET;
		int estimatedSize = DEFAULT_OVERVIEW_SIZE;
		while (estimatedSize < overviewTileCount) 
		{
			resolutionOffset++;
			estimatedSize *= 3;
		}

		auto circle = geometry.getBoundingCircle();
		auto resolution = std::min(PYXMath::knMaxAbsResolution,circle.estimateResolutionFromRadius(circle.getRadius()) + resolutionOffset);
	
		geometry.copyTo(overview.get(),resolution);
	}

	//continue to decrease cell count until we reach the wanted limit
	while (overview->getCellResolution() > 5 && overview->getGeometryCount() > overviewTileCount)
	{
		overview->setCellResolution(overview->getCellResolution() - 1);
	}

	return overview;
}

PYXPointer<PYXRegion> PYXGeometryIntersectionUtils::extractRegion( const PYXGeometry & geometry )
{
	auto vectorGeom = dynamic_cast<const PYXVectorGeometry*>(&geometry);
	if (vectorGeom != nullptr)
	{
		return vectorGeom->getRegion();
	}

	auto vectorGeom2 = dynamic_cast<const PYXVectorGeometry2*>(&geometry);
	if (vectorGeom2 != nullptr)
	{
		return boost::dynamic_pointer_cast<PYXRegion>(vectorGeom2->getRegion());
	}

	return nullptr;
}

bool PYXGeometryIntersectionUtils::isCircle( const PYXGeometry & geometry )
{
	auto region = extractRegion(geometry);
	if (!region)
	{
		return false;
	}

	auto point = dynamic_cast<const PYXVectorPointRegion*>(region.get());
	if (point)
	{
		return true;
	}

	auto circle = dynamic_cast<const PYXCircleRegion*>(region.get());
	if (circle)
	{
		return true;
	}

	return false;
}