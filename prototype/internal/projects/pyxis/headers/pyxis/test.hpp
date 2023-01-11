#if !defined(PYXIS__TEST)
#define PYXIS__TEST

#include "pyxis/empty.hpp"
#include "pyxis/compact_vector.hpp"
#include "pyxis/visit.hpp"

namespace Pyxis
{
	struct Test;
}

struct Pyxis::Test
{
	operator bool()
	{
		return (
			Pyxis::CompactVector< Pyxis::Empty >::Test() &&
			Pyxis::Visit::Test());
	}
};

#endif
