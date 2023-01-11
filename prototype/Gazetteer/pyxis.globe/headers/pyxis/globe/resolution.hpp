#if !defined(PYXIS__GLOBE__RESOLUTION)
#define PYXIS__GLOBE__RESOLUTION

#include <boost/strong_typedef.hpp>

namespace Pyxis
{
	namespace Globe
	{
		// A resolution is an unsigned numerical index for a tessellation in the globe.
		typedef char ResolutionInteger;
		BOOST_STRONG_TYPEDEF(ResolutionInteger, Resolution);
	}
}

#endif
