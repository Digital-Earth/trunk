#if !defined(PYXIS__GLOBE__FEATURE)
#define PYXIS__GLOBE__FEATURE

#include "pyxis/globe/feature_interface.hpp"
#include "pyxis/globe/region/region_interface.hpp"
#include <memory> // std::auto_ptr
#include <stdexcept>

namespace Pyxis
{
	namespace Globe
	{
		// A feature is a value and region pair.
		// The value uses value semantics.
		template < typename Value > class Feature;
	}
}

template < typename Value >
class Pyxis::Globe::Feature : public FeatureInterface
{
	// The region associated with the feature.
	boost::intrusive_ptr< Region::RegionInterface > region;

	// The value stored in the feature.
	Value value;

public:

	// Constructs a feature from the region and the value.
	explicit Feature(std::auto_ptr< Region::RegionInterface > region, Value value) :
	region(region.release()), value(value)
	{
		if (!this->region.get())
		{
			throw std::invalid_argument("Region cannot be null.");
		}
	}

	// Gets the region of the feature.
	Region::RegionInterface const & getRegion() const
	{
		assert(region);
		return *region;
	}

	// Gets the region of the feature.
	Region::RegionInterface & getRegion()
	{
		assert(region);
		return *region;
	}

	// Gets the value associated with the feature.
	Value getValue() const
	{
		return value;
	}
};

#endif
