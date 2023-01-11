#if !defined(PYXIS__GLOBE__REGION__POINT_REGION)
#define PYXIS__GLOBE__REGION__POINT_REGION

#include "pyxis/globe/region/region_interface.hpp"

namespace Pyxis
{
	namespace Globe
	{
		namespace Region
		{
			class PointRegion;
		}
	}
}

class Pyxis::Globe::Region::PointRegion : public RegionInterface
{
public:
	// Returns a null pointer; a point cannot be simplified.
	boost::intrusive_ptr< RegionInterface > normalize() const
	{
		return boost::intrusive_ptr< RegionInterface >();
	}

	boost::logic::tribool getIntersection(
		Raster const & intersectee,
		Raster * complete = 0,
		Raster * partial = 0,
		Raster * unknown = 0) const;
};

#endif
