#if !defined(PYXIS__GLOBE__TEST)
#define PYXIS__GLOBE__TEST

#include "pyxis/test.hpp"
#include "pyxis/globe/tree/subtree_map.hpp"
#include "pyxis/globe/tree/subtree_multimap.hpp"
#include "pyxis/globe/raster.hpp"
#include "pyxis/globe/gazetteer.hpp"

namespace Pyxis
{
	namespace Globe
	{
		struct Test;
	}
}

struct Pyxis::Globe::Test
{
	operator bool()
	{
		return (
			Pyxis::Test() &&
			Pyxis::Globe::Tree::Index::Test() &&
			Pyxis::Globe::Tree::SubtreeMap<>::Test() &&
			Pyxis::Globe::Tree::SubtreeMultimap< std::string >::Test() &&
			Pyxis::Globe::Cell::Test() &&
			Pyxis::Globe::Raster::Test() &&
			Pyxis::Globe::Gazetteer< std::size_t, Pyxis::Empty >::Test());
	}
};

#endif
