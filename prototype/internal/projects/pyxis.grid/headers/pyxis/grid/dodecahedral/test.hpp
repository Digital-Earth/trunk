#if !defined(PYXIS__GRID__DODECAHEDRAL__TEST)
#define PYXIS__GRID__DODECAHEDRAL__TEST

namespace Pyxis
{
	namespace Grid
	{
		namespace Dodecahedral
		{
			struct Test;
		}
	}
}

#include "pyxis/test.hpp"
#include "pyxis/grid/dodecahedral/tree.hpp"

struct Pyxis::Grid::Dodecahedral::Test
{
	operator bool()
	{
		return (
			Pyxis::Test() &&
			Pyxis::Grid::Dodecahedral::Tree::Test());
	}
};

#endif
