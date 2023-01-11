#if !defined(PYXIS__GRID__DODECAHEDRAL__RESOLUTION)
#define PYXIS__GRID__DODECAHEDRAL__RESOLUTION

#include <boost/strong_typedef.hpp>
#include "pyxis/grid/dodecahedral/resolution.hpp"

namespace Pyxis
{
	namespace Grid
	{
		namespace Dodecahedral
		{
			// A resolution is an unsigned numerical index for a tessellation in the globe.
			typedef char ResolutionInteger;
			BOOST_STRONG_TYPEDEF(ResolutionInteger, Resolution);
		}
	}
}

#endif
