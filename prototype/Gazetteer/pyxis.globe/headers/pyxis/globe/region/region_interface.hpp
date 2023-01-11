#if !defined(PYXIS__GLOBE__REGION__REGION_INTERFACE)
#define PYXIS__GLOBE__REGION__REGION_INTERFACE

#include "pyxis/pointee.hpp"
#include <boost/logic/tribool.hpp>
#include <boost/intrusive_ptr.hpp>

namespace Pyxis
{
	namespace Globe
	{
		class Raster;

		namespace Region
		{
			struct RegionInterface;
		}
	}
}

/*
In a given resolution, the following terms apply to regions:
-	Positive/Negative intersection:
	-	Positive:
		-	The set of cells that intersect the region.
	-	Negative:
		-	The set of cells that do not intersect the region.
-	Boundary/Interior/Exterior:
	-	Boundary:
		-	The set of cells that are
			(1) in the positive intersection and touch the negative intersection, and
			(2) in the negative intersection and touch the positive intersection.
		-	If two regions have intersecting boundaries, they may touch or overlap.
			If they do not, they do not touch or overlap.
	-	Interior:
		-	The set of cells that are in the positive intersection and not in the boundary.
	-	Exterior:
		-	The set of cells that are in the negative intersection and not in the boundary.
*/
struct Pyxis::Globe::Region::RegionInterface : virtual Pointee
{
	// Determines the intersection, and appends the result to the optional
	// complete, partial, and unknown rasters.
	// Note that these can be the same raster if desired.
	// Returns true (intersects), false (doesn't intersect), or indeterminate
	// (no cells intersect, and at least one is unknown).
	virtual boost::logic::tribool getIntersection(
		Raster const & intersectee,
		Raster * complete = 0,
		Raster * partial = 0,
		Raster * unknown = 0) const = 0;
	/*
	{
		boost::logic::tribool result = intersectionCache.getIntersection(
			intersectee, complete, partial, unknown);
		
		if (unknown)
		{
			// Rasterize vector geometry for unknown part, and add each cell to intersectionCache.
			// Update result.
		}

		return result;
	}
	*/

#if 0 // TODO: Add later.

	// Determines the boundary intersection, and appends the result to the optional
	// complete, partial, and unknown rasters.
	// Returns true (intersects), false (doesn't intersect), or indeterminate
	// (no cells intersect, and at least one is unknown).
	virtual boost::logic::tribool getBoundaryIntersection(
		Raster const & intersectee,
		Raster * complete = 0,
		Raster * partial = 0,
		Raster * unknown = 0) const = 0;

#endif

	// Attempts to create a "normalized" version of this region in its current state.
	// Some operations will result in a simple region; for example, intersection of
	// two rectangles may result in a rectangle.
	// Further, some region types will have degenerate cases that simplify to other region types;
	// for example, a rectangle whose vertices are congruent normalizes to a point.
	// If the region can't be normalized to anything simpler (or normalization hasn't been
	// implemented for the type), returns a null pointer.
	virtual boost::intrusive_ptr< RegionInterface > normalize() const = 0;
	
#if 0 // TODO: Add this later
	// Returns true if the region has an area, and false if not.
	virtual bool getHasArea() const = 0;
#endif
};

#endif
