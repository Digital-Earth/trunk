#if !defined(PYXIS__EMPTY)
#define PYXIS__EMPTY

#include "pyxis/is_singleton.hpp"

namespace Pyxis
{
	struct Empty;

	// The Empty class is a Singleton type.
	template <> struct IsSingleton< Empty >
	{
		BOOST_STATIC_CONSTANT(bool, value = true);
	};
}

// An empty struct.
// Useful as a default template argument.
struct Pyxis::Empty
{
	// Workaround for VC6 bug described here:
	// http://www.boost.org/doc/libs/release/libs/utility/compressed_pair.htm
	Empty & operator =(Empty const)
	{
		return *this;
	}

	bool operator ==(Empty const) const
	{
		return true;
	}

	bool operator !=(Empty const) const
	{
		return false;
	}

	bool operator <(Empty const) const
	{
		return false;
	}
};

#endif
