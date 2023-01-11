#if !defined(PYXIS__GLOBE__REGION__POLYGON_REGION_INTERFACE)
#define PYXIS__GLOBE__REGION__POLYGON_REGION_INTERFACE

#include "pyxis/globe/region/region_interface.hpp"

namespace Pyxis
{
	template < typename Return, typename Argument > struct FunctorInterface;

	namespace Globe
	{
		namespace Region
		{
			struct ArcRegionInterface;

			struct PolygonRegionInterface;
		}
	}
}

// The base class for closed, filled polygons, mutable and immutable.
// Exposes a list of more than 2 ArcInterface instances.
// Order indicates interior.
// Does not contain any holes.
struct Pyxis::Globe::Region::PolygonRegionInterface : RegionInterface
{
	// Visits each arc in the polygon, in order to indicate which side
	// is interior.
	virtual bool visitForward(FunctorInterface< bool, ArcRegionInterface const & > & visitor) const;
};

#endif
