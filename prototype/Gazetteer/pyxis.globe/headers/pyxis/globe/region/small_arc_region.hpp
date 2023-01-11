#if !defined(PYXIS__GLOBE__REGION__SMALL_ARC_REGION)
#define PYXIS__GLOBE__REGION__SMALL_ARC_REGION

#include "pyxis/globe/region/arc_region_interface.hpp"

namespace Pyxis
{
	namespace Globe
	{
		namespace Region
		{
			class SmallArcRegion;
		}
	}
}

class Pyxis::Globe::Region::SmallArcRegion : public ArcRegionInterface
{
public:
	// Returns the center point of the small circle.
	PointRegion const & getCenterPoint() const;

	// Returns true if the arc proceeds in a clockwise direction around the circle from the start point.
	bool getIsClockwise() const;

	// Returns the start point of the arc.
	PointRegion const & getStartPoint() const;

	// Returns the end point of the arc.
	PointRegion const & getEndPoint() const;

	// Returns a fraction indicating the fraction of the small circle, between 0 (a point) and 1 (the entire circle), inclusive.
	Fraction< size_t > const & getCircleFraction() const;

	// Normalize the arc.  If the circle fraction is 0, or the center point is the same as (or opposite) the start point, return a point.
	boost::intrusive_ptr< RegionInterface > normalize() const;

	boost::logic::tribool getIntersection(
		Raster const & intersectee,
		Raster * complete = 0,
		Raster * partial = 0,
		Raster * unknown = 0) const;
};

#endif
