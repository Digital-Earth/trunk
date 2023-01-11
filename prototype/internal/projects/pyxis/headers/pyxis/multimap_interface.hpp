#if !defined(PYXIS__MULTIMAP_INTERFACE)
#define PYXIS__MULTIMAP_INTERFACE

namespace Pyxis
{
	template < typename Key, typename Value > struct MultimapInterface;
	template < typename Key, typename Value > struct MutableMultimapInterface;
}

#include "pyxis/collection_interface.hpp"

template < typename Key, typename Value >
struct Pyxis::MultimapInterface :
virtual CollectionInterface
{
	// TODO: Fill this in.
};

template < typename Key, typename Value >
struct Pyxis::MutableMultimapInterface :
virtual MultimapInterface< Key, Value >,
virtual MutableCollectionInterface
{
	// TODO: Fill this in.
};

#endif
