#ifndef PYXIS__GEOMETRY__GEOMETRY_INTERSECTION_UTILS_H
#define PYXIS__GEOMETRY__GEOMETRY_INTERSECTION_UTILS_H
/******************************************************************************
geometry_intersection_utils.h

begin		: 2016-03-26
copyright	: (C) 2016 by the PYXIS innova
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/geometry/inner_tile.h"
#include "pyxis/region/region.h"


/*!
Various utilties for intersection testing between geometries.
*/
//! Various utilties for intersection testing between geometries..
class PYXLIB_DECL PYXGeometryIntersectionUtils
{
public:

	static void test(); // unit test

public:
	enum Optimization
	{
		knNoOptimization,
		knMultiResolutionOptimization
	};

public:

	//! check if the bounding circles of 2 geometires intersect
	static bool boundingCirclesIntersect(	const PYXGeometry& a,
											const PYXGeometry& b );


	//! check if two geometries intersect inside a given inner tile
	static bool intersectsAtInnerTile(	const PYXInnerTile& innerTile,
										const PYXGeometry& a,
										const PYXGeometry& b,
										Optimization optimization = knMultiResolutionOptimization  );

	//! check if two geometries intersect inside a given tile
	static bool intersectsAtTile(	const PYXTile& tile,
									const PYXGeometry& a,
									const PYXGeometry& b,
									Optimization optimization = knMultiResolutionOptimization  );


	//! check if two geometries intersect inside a set of tile collection
	static bool intersectsAtTiles(	const PYXTileCollection & tiles, 
									const PYXGeometry& a,
									const PYXGeometry& b,
									Optimization optimization = knMultiResolutionOptimization  );

	//! return a tile collection describing the given geometry with realtively small number of tiles
	static PYXPointer<PYXTileCollection> createSmallOverview(	const PYXGeometry & geometry,
																int overviewTileCount = 50 );


	//! try to extract a PYXRegion from the given geometry
	static PYXPointer<PYXRegion> extractRegion( const PYXGeometry & geometry );

	//! return true if geometry is a real circle (or point) to simplify intersections
	static bool isCircle( const PYXGeometry & geometry );
};

#endif // guard
