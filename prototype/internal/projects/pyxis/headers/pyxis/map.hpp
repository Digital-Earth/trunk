#if !defined(PYXIS__MAP)
#define PYXIS__MAP

#include "pyxis/functor_interface.hpp"
#include "pyxis/map_interface.hpp"
#include <boost/compressed_pair.hpp>
#include <map>

namespace Pyxis
{
	template < typename Key, typename Value > class Map;
}

template < typename Key, typename Value >
class Pyxis::Map : public virtual MapInterface< Key, Value >
{
	typedef boost::compressed_pair< Key, Value > Pair;

	std::map< Key, Value > map;

public:

	explicit Map() : map() {}

	bool operator ==(Map const & another) const
	{
		// TODO: Verify that this checks equality of each member
		return map == another.map;
	}

	bool operator !=(Map const & another) const
	{
		// TODO: Verify that this checks inequality of each member
		return map != another.map;
	}

	bool getIsEmpty() const
	{
		return map.empty();
	}
	
	void setIsEmpty()
	{
		map.clear();
	}

	size_t getCount() const
	{
		assert(0); throw std::exception(); // TODO
	}

	Value getValue(Key key) const
	{
		typename std::map< Key, Value >::const_iterator const iterator = map.find(key);
		return (iterator == map.end()) ? Value() : iterator->second;
	}
	
	void setValue(Key key, Value value = Value())
	{
		map[key] = value;
	}

	bool visit(FunctorInterface< bool, Pair > & visitor) const
	{
		typename std::map< Key, Value >::const_iterator const end = map.end();
		for (typename std::map< Key, Value >::const_iterator iterator = map.begin();
			iterator != end; ++iterator)
		{
			if (!visitor(Pair(iterator->first, iterator->second)))
			{
				return false;
			}
		}
		return true;
	}

	void visitAll(FunctorInterface< void, Pair > & visitor) const
	{
		typename std::map< Key, Value >::const_iterator const end = map.end();
		for (typename std::map< Key, Value >::const_iterator iterator = map.begin();
			iterator != end; ++iterator)
		{
			visitor(Pair(iterator->first, iterator->second));
		}
	}
};

#endif
