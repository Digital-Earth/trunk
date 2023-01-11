#if !defined(PYXIS__GLOBE__REGION__INTERSECTION_REGION)
#define PYXIS__GLOBE__REGION__INTERSECTION_REGION

#include "pyxis/globe/region/region_interface.hpp"
#include <set>

namespace Pyxis
{
	namespace Globe
	{
		namespace Region
		{
			template < typename Region > class IntersectionRegion;
		}
	}
}

// An intersection of regions.
template < typename Region >
class Pyxis::Globe::Region::IntersectionRegion : public RegionInterface
{
public:
	// Constructs an intersection from a set of regions.
	explicit IntersectionRegion(std::set< boost::intrusive_ptr< Region > > const & operands);

	// Returns a simpler representation of the region, or null if none.
	boost::intrusive_ptr< RegionInterface > normalize() const;

	boost::logic::tribool getIntersection(
		Raster const & intersectee,
		Raster * complete = 0,
		Raster * partial = 0,
		Raster * unknown = 0) const;
};

#endif
