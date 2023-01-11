#if !defined(PYXIS__GLOBE__COVERAGE_INTERFACE)
#define PYXIS__GLOBE__COVERAGE_INTERFACE

#include "pyxis/pointee.hpp"

namespace Pyxis
{
	namespace Globe
	{
		template < typename Value > struct CoverageInterface;

		namespace Region
		{
			struct RegionInterface;
		}
	}
}

// A data structure for which every cell of every raster of its region
// has an associated value.
// Optimized for space, allowing for large coverages.
// For a tree indexing approach, this may store a set of value tiles for each resolution
// (or combine value tiles from all resolutions into a single tree, where the index node stores
// a depth-indexed array of leaf value arrays for the root index).
template < typename Value >
struct Pyxis::Globe::CoverageInterface : virtual Pointee
{
	// Gets the region of the coverage.
	virtual Region::RegionInterface const & getRegion() const = 0;

#if 0 // TODO: Add this when Raster is templated on Value.
	// Gets the coverage intersection with a raster.
	virtual boost::logic::tribool getIntersection(
		RasterInterface const & intersectee,
		Raster< Value > * complete = 0,
		Raster< Value > * partial = 0,
		Raster< Value > * unknown = 0) const = 0;
#endif
};

#endif
