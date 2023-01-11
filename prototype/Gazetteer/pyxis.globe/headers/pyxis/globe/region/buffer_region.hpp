#if !defined(PYXIS__GLOBE__REGION__BUFFER_REGION)
#define PYXIS__GLOBE__REGION__BUFFER_REGION

#include "pyxis/globe/region/region_interface.hpp"
#include <memory>

namespace Pyxis
{
	template < typename UnsignedInteger > class Fraction;

	namespace Globe
	{
		namespace Region
		{
			class BufferRegion;
		}
	}
}

// A buffer of a specific distance around a region.
// A buffer around a point region is a circle.
class Pyxis::Globe::Region::BufferRegion : public RegionInterface
{
public:
	// Constructs a buffer on a region.
	explicit BufferRegion(std::auto_ptr< RegionInterface > operand, double bufferSizeInMetres);

	// Gets the distance as a fraction of the great circle arc.
	Fraction< size_t > getGreatCircleFraction() const;

	// Returns the original region being buffered.
	RegionInterface const & getOperand() const;

	// Returns a simpler representation of the region, or null if none.
	boost::intrusive_ptr< RegionInterface > normalize() const;

	boost::logic::tribool getIntersection(
		Raster const & intersectee,
		Raster * complete = 0,
		Raster * partial = 0,
		Raster * unknown = 0) const;
};

#endif
