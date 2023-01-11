#if !defined(PYXIS__MULTIMAP)
#define PYXIS__MULTIMAP

namespace Pyxis
{
	template < typename Key, typename Value > class Multimap;
}

#include "pyxis/multimap_interface.hpp"

template < typename Key, typename Value >
class Pyxis::Multimap :
public virtual MultimapInterface< Key, Value >
{
	// TODO: Wrap std::multimap.
};

#endif
