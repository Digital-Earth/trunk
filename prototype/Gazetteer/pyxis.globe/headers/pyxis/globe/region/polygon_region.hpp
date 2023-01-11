#if !defined(PYXIS__GLOBE__REGION__POLYGON_REGION)
#define PYXIS__GLOBE__REGION__POLYGON_REGION

#include "pyxis/globe/region/polygon_region_interface.hpp"

namespace Pyxis
{
	namespace Globe
	{
		namespace Region
		{
			class PolygonRegion;
		}
	}
}

// A closed polygon, regular or irregular, with no holes.
class Pyxis::Globe::Region::PolygonRegion : public PolygonRegionInterface
{
public:
	// Returns a simpler representation of the region, or null if none.
	boost::intrusive_ptr< RegionInterface > normalize() const;

	boost::logic::tribool getIntersection(
		Raster const & intersectee,
		Raster * complete = 0,
		Raster * partial = 0,
		Raster * unknown = 0) const;
};

#endif
