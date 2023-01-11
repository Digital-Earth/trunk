#if !defined(PYXIS__GLOBE__REGION__RASTER_REGION)
#define PYXIS__GLOBE__REGION__RASTER_REGION

#include "pyxis/globe/raster.hpp"
#include "pyxis/globe/region/region_interface.hpp"

namespace Pyxis
{
	namespace Globe
	{
		namespace Region
		{
			class RasterRegion;
		}
	}
}

// Wraps a raster as a region, conferring the abilities of a region
// (e.g. intersection with any raster at any resolution) to a raster.
class Pyxis::Globe::Region::RasterRegion : public RegionInterface
{
	// The underlying raster.
	Raster raster;

public:

	Raster & getRaster()
	{
		return raster;
	}

	Raster const & getRaster() const
	{
		return raster;
	}

	// Creates an empty raster.
	explicit RasterRegion(Resolution resolution) : raster(resolution) {}

	// Returns null pointer; a raster cannot be simplified to another region type.
	boost::intrusive_ptr< RegionInterface > normalize() const
	{
		return boost::intrusive_ptr< RegionInterface >();
	}

	boost::logic::tribool getIntersection(
		Raster const & intersectee,
		Raster * complete = 0,
		Raster * partial = 0,
		Raster * unknown = 0) const
	{
		if (intersectee.getIsEmpty() || raster.getIsEmpty())
		{
			return false;
		}

		if (intersectee.getIsFull())
		{
			if (complete)
			{
				complete->insert(raster);
			}
			return true;
		}

		if (raster.getIsFull())
		{
			if (complete)
			{
				complete->insert(intersectee);
			}
			return true;
		}

		// If the raster is the same resolution, do a cell intersection.
		// Otherwise, walk the tree from the lower to the higher resolution one,
		// and determine spatial intersection.
		assert(0); // TODO

		return boost::logic::indeterminate;
	}
};

#endif
