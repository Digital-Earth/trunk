#if !defined(PYXIS__GLOBE__REGION__RECTANGLE_REGION)
#define PYXIS__GLOBE__REGION__RECTANGLE_REGION

#include "pyxis/globe/region/polygon_region_interface.hpp"

namespace Pyxis
{
	namespace Globe
	{
		namespace Region
		{
			class RectangleRegion;
		}
	}
}

class Pyxis::Globe::Region::RectangleRegion : public PolygonRegionInterface
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
