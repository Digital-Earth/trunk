#if !defined(PYXIS__GLOBE__REGION__GREAT_ARC_REGION)
#define PYXIS__GLOBE__REGION__GREAT_ARC_REGION

#include "pyxis/globe/region/arc_region_interface.hpp"

namespace Pyxis
{
	namespace Globe
	{
		namespace Region
		{
			class GreatArcRegion;
		}
	}
}

class Pyxis::Globe::Region::GreatArcRegion : public ArcRegionInterface
{
public:
	// Returns the start point of the arc.
	PointRegion const & getStartPoint() const;

	// Returns the end point of the arc.
	PointRegion const & getEndPoint() const;

	// Returns a fraction indicating the fraction of the great circle, between 0 (a point) and 1 (the entire circle), inclusive.
	Fraction< size_t > const & getCircleFraction() const;

	// Normalizes the arc.  If the circle fraction is 0, returns a point.
	boost::intrusive_ptr< RegionInterface > normalize() const;

	boost::logic::tribool getIntersection(
		Raster const & intersectee,
		Raster * complete = 0,
		Raster * partial = 0,
		Raster * unknown = 0) const;
};

#endif
