#if !defined(PYXIS__MAP_INTERFACE)
#define PYXIS__MAP_INTERFACE

namespace Pyxis
{
	template < typename Return, typename Argument > struct FunctorInterface;

	template < typename Key, typename Value > struct MapInterface;
}

#include "pyxis/collection_interface.hpp"
#include <boost/compressed_pair.hpp>

template < typename Key, typename Value >
struct Pyxis::MapInterface :
virtual CollectionInterface
{
	typedef boost::compressed_pair< Key, Value > Pair;

	virtual Value getValue(Key key) const = 0;
	virtual bool visit(FunctorInterface< bool, Pair > & callback) const = 0;
};

#endif
