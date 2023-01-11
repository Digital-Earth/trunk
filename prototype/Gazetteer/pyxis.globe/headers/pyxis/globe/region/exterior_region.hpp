#if !defined(PYXIS__GLOBE__REGION__EXTERIOR_REGION)
#define PYXIS__GLOBE__REGION__EXTERIOR_REGION

#include "pyxis/globe/region/region_interface.hpp"
#include <memory>
#include <stdexcept>

namespace Pyxis
{
	namespace Globe
	{
		namespace Region
		{
			class ExteriorRegion;
		}
	}
}

// The exterior of a region.
// At a resolution, this includes all cells that are not in the raster.
class Pyxis::Globe::Region::ExteriorRegion : public RegionInterface
{
	boost::intrusive_ptr< RegionInterface > operand;

public:
	// Constructs an exterior of a region.
	explicit ExteriorRegion(std::auto_ptr< RegionInterface > operand) : operand(operand.release())
	{
		if (!operand.get())
		{
			throw std::invalid_argument("operand");
		}
	}

	// Returns the exterior of this region, which is the original region.
	RegionInterface const & getOperand() const { return *operand; }

	// Returns a simpler representation of the region.
	boost::intrusive_ptr< RegionInterface > normalize() const;

	boost::logic::tribool getIntersection(
		Raster const & intersectee,
		Raster * complete = 0,
		Raster * partial = 0,
		Raster * unknown = 0) const;
};

#endif
