#if !defined(PYXIS__GLOBE__FEATURE_INTERFACE)
#define PYXIS__GLOBE__FEATURE_INTERFACE

#include "pyxis/pointee.hpp"

namespace Pyxis
{
	namespace Globe
	{
		struct FeatureInterface;
		
		namespace Region
		{
			struct RegionInterface;
		}
	}
}

struct Pyxis::Globe::FeatureInterface : virtual Pointee
{
	virtual Region::RegionInterface const & getRegion() const = 0;
};

#endif
