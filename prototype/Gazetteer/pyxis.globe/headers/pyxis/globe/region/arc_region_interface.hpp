#if !defined(PYXIS__GLOBE__REGION__ARC_REGION_INTERFACE)
#define PYXIS__GLOBE__REGION__ARC_REGION_INTERFACE

#include "pyxis/globe/region/region_interface.hpp"

namespace Pyxis
{
	template < typename UnsignedInteger > class Fraction;

	namespace Globe
	{
		namespace Region
		{
			struct ArcRegionInterface;

			class PointRegion;
		}
	}
}

// A line segment.
struct Pyxis::Globe::Region::ArcRegionInterface : RegionInterface
{
	// Returns the start point of the arc.
	virtual PointRegion const & getStartPoint() const = 0;

	// Returns the end point of the arc.
	virtual PointRegion const & getEndPoint() const = 0;

	// Returns a fraction indicating the fraction of the circle it is on, between 0 (a point) and 1 (the entire circle), inclusive.
	virtual Fraction< size_t > const & getCircleFraction() const = 0;
};

#endif
