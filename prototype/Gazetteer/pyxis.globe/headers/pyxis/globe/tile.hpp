#if !defined(PYXIS__GLOBE__TILE)
#define PYXIS__GLOBE__TILE

namespace Pyxis
{
	namespace Globe
	{
		/*
		A tile is a group of 1 or more cells within a resolution which 
		can each be defined in terms of a single index 
		via an implementation-defined algorithm.
		If the root index is at the same resolution, the tile only
		includes that cell.

		Some terms:
		-	Positive/negative:
			-	The positive tile is the cells in the resolution that are of the tile;
				the negative tile is the cells in the resolution that are not of the tile.
		-	Boundary/interior/exterior:
			-	The tile boundary is comprised of:
				(a) the positive cells that touch the negative tile, and
				(b) the negative cells that touch the positive tile.
			-	The tile interior is comprised of the positive cells that are not in the boundary.
			-	The tile exterior is comprised of the negative cells that are not in the boundary.
		*/
		class Tile;
	}
}

#include "pyxis/globe/cell.hpp"
#include "pyxis/forward_range_interface.hpp"
#include <boost/intrusive_ptr.hpp>

class Pyxis::Globe::Tile : public virtual Pointee
{
	Tree::Index index;

	Tree::Level depth;

public:

	static Tree::Level getLevel(
		Tree::Index const & index, Tree::Level const depth)
	{
		return Tree::Level(index.getStepCount().offset + depth.offset);
	}

	static bool getHasResolution(
		Tree::Index const & index, Tree::Level const depth)
	{
		return getLevel(index, depth).getHasResolution();
	}

	static boost::optional< Resolution > getResolution(
		Tree::Index const & index, Tree::Level const depth)
	{
		return getLevel(index, depth).getResolution();
	}

	// Constructs the tile.  Swaps in the index.
	explicit Tile(Tree::Index & index, Tree::Level depth = Tree::Level()) :
	index(), depth(depth)
	{
		if (!getHasResolution(index, depth))
		{
			throw std::out_of_range("The resulting level does not represent a valid resolution.");
		}
		this->index.swap(index);
	}

	// Returns a reference to the root index, which 
	// defines the set of cells in the tile
	// in an implementation-defined way.
	Tree::Index const & getIndex() const { return this->index; }

	Tree::Level getDepth() const { return this->depth; }

	// Returns the resolution of the cells within the tile.
	Resolution getResolution() const
	{
		boost::optional< Resolution > resolution(getResolution(this->index, this->depth));
		assert(resolution);
		return *resolution;
	}

	// The range of cells in the tile.
	class Cells :
	public ForwardRangeInterface< Cell >
	{
		Tile const & tile;

		// The range of index descendants at the depth.
		Tree::Index::Descendants descendants;

	public:

		// Constructs the cell range.
		explicit Cells(Tile const & tile) :
		tile(tile),
		descendants(tile.getIndex(), tile.getDepth())
		{}

		// Gets the tile that the cell range pertains to.
		Tile const & getTile() const { return tile; }

		// Returns true if there are no cells remaining in the range.
		bool getIsEmpty() const { return descendants.getIsEmpty(); }

		// Throws assert if empty.
		void popFront()
		{
			assert(!descendants.getIsEmpty());
			descendants.popFront();
		}

		// Throws assert if empty.
		// Note that it is valid to assign the return value of this function
		// to a const reference, resulting in extension of the value's lifetime.
		// See http://herbsutter.spaces.live.com/blog/cns!2D4327CC297151BB!378.entry for explanation.
		Cell getFront() const
		{
			assert(!descendants.getIsEmpty());
			Tree::Index index = descendants.getFront();
			return Cell(index); // Swaps index.
		}
	};
};

namespace Pyxis
{
	// A specialization of Set for Tile.
	template < typename Key > class Set;
	template <> class Set< Globe::Tile const & >;
}

#include "pyxis/set_interface.hpp"

template <>
class Pyxis::Set< Pyxis::Globe::Tile const & > :
public MutableSetInterface< Pyxis::Globe::Tile const & >
{
	// TODO
};

namespace Pyxis
{
	// A specialization of Map for Tile.
	template < typename Key, typename Value > class Map;
	template < typename Value > class Map< Globe::Tile const &, Value >;
}

#include "pyxis/map_interface.hpp"
#include "pyxis/globe/tree/subtree_map.hpp"
#include <map>

template < typename Value >
class Pyxis::Map< Pyxis::Globe::Tile const &, Value > :
public MapInterface< Pyxis::Globe::Tile const &, Value >
{
	// A map of resolution to SubtreeMap.
	typedef std::map<
		Globe::Resolution,
		boost::intrusive_ptr<
			Globe::Tree::SubtreeMap< Value >
		>
	> ResolutionMap;

	ResolutionMap resolutionMap;

	// Helper.
	Globe::Tree::SubtreeMap< Value > & getSubtreeMap(Globe::Resolution const resolution)
	{
		boost::intrusive_ptr<
			Globe::Tree::SubtreeMap< Value >
		> & subtreeMap = this->resolutionMap[resolution];
		if (!subtreeMap)
		{
			subtreeMap.reset(new Globe::Tree::SubtreeMap< Value >());
		}
		assert(subtreeMap);
		return *subtreeMap;
	}

	// TODO
	Map(Map const & that);
	Map & operator =(Map that);

public:

	typedef typename MapInterface< Globe::Tile const &, Value >::Pair Pair;

	typedef boost::compressed_pair< Globe::Tree::Index const &, Globe::Tree::Level > IndexDepthPair;
	typedef boost::compressed_pair< IndexDepthPair, Value > IndexDepthValuePair;

	Map() : resolutionMap() {}

	bool getIsEmpty() const { return this->resolutionMap.empty(); }

	size_t getCount() const
	{
		assert(0); throw std::exception(); // TODO
	}

	Value getValue(Globe::Tile const & tile) const
	{
		typename ResolutionMap::const_iterator const iterator = this->resolutionMap.find(
			tile.getResolution());
		if (iterator != this->resolutionMap.end())
		{
			Value value;
			assert(iterator->second);
			iterator->second->find(tile.getIndex(), &value);
			return value;
		}
		return Value();
	}

	Value getValue(Globe::Tree::Index const & index, Globe::Tree::Level level) const;

	void setValue(Globe::Tile const & tile)
	{
		typename ResolutionMap::iterator const iterator = this->resolutionMap.find(
			tile.getResolution());
		if (iterator != this->resolutionMap.end())
		{
			assert(iterator->second);
			Globe::Tree::SubtreeMap< Value > & subtreeMap = *(iterator->second);
			subtreeMap.remove(tile.getIndex());
			if (subtreeMap.getIsEmpty())
			{
				this->resolutionMap.erase(iterator);
			}
		}
	}

	void setValue(Globe::Tree::Index const & index, Globe::Tree::Level level) const;

	void setValue(Globe::Tile const & tile, Value value)
	{
		if (value == Value())
		{
			setValue(tile);
		} else 
		{
			getSubtreeMap(tile.getResolution()).insert(tile.getIndex(), value);
		}
	}

	void setValue(Globe::Tree::Index const & index, Globe::Tree::Level level, Value value) const;

	// Visits each tile/value pair.
	// TODO: Test this.
	bool visit(FunctorInterface< bool, Pair > & callback) const
	{
		typename ResolutionMap::const_iterator const end = this->resolutionMap.end();
		for (typename ResolutionMap::const_iterator iterator(this->resolutionMap.begin());
			iterator != end; ++iterator)
		{
			assert(iterator->second);
			Globe::Tree::SubtreeMap< Value > const & subtreeMap = *(iterator->second);
			for (typename Globe::Tree::SubtreeMap< Value >::Pairs indexValuePairs(subtreeMap);
				!indexValuePairs.getIsEmpty(); indexValuePairs.popFront())
			{
				typename Globe::Tree::SubtreeMap< Value >::Pair const & indexValuePair(
					indexValuePairs.getFront());
				Globe::Tree::Index index(indexValuePair.first()); // Copy; tile constructor will swap it in.
				if (!callback(Pair(Globe::Tile(index), indexValuePair.second()))) { return false; }
			}
		}
		return true;
	}

	bool visit(FunctorInterface< bool, IndexDepthValuePair > & callback) const;
};

namespace Pyxis
{
	// A specialization of Multimap for Tile.
	template < typename Key, typename Value > class Multimap;
	template < typename Value > class Multimap< Globe::Tile const &, Value >;
}

#include "pyxis/multimap_interface.hpp"
#include "pyxis/globe/tree/subtree_multimap.hpp"

// Maps a tile to multiple values.
template < typename Value >
class Pyxis::Multimap< Pyxis::Globe::Tile const &, Value > :
public MultimapInterface< Pyxis::Globe::Tile const &, Value >
{
	// A map of resolution to SubtreeMultimap.
	typedef std::map<
		Globe::Resolution,
		boost::intrusive_ptr<
			Globe::Tree::SubtreeMultimap< Value >
		>
	> ResolutionMap;

	ResolutionMap resolutionMap;

	// Helper.
	Globe::Tree::SubtreeMultimap< Value > & getSubtreeMultimap(Globe::Resolution const resolution)
	{
		boost::intrusive_ptr<
			Globe::Tree::SubtreeMultimap< Value >
		> & subtreeMultimap = this->resolutionMap[resolution];
		if (!subtreeMultimap)
		{
			subtreeMultimap.reset(new Globe::Tree::SubtreeMultimap< Value >());
		}
		assert(subtreeMultimap);
		return *subtreeMultimap;
	}

	// TODO
	Multimap(Multimap const & that);
	Multimap & operator =(Multimap that);

public:

	Multimap() : resolutionMap() {}

	// Inserts a tile->value mapping.
	void insert(Globe::Tile const & tile, Value value)
	{
		getSubtreeMultimap(tile.getResolution()).insert(tile.getIndex(), value);
	}

	// Inserts a tile->value mapping without requiring construction of the tile.
	void insert(Globe::Tree::Index const & index, Globe::Tree::Level const depth, Value value)
	{
		boost::optional< Globe::Resolution > resolution(Globe::Tile::getResolution(index, depth));
		if (!resolution)
		{
			throw std::out_of_range("The resulting level does not represent a valid resolution.");
		}
		getSubtreeMultimap(*resolution).insert(index, value);
	}

	// Removes a tile->value mapping.
	// TODO: Fill this in; remember to clear out the resolutionMap entry if empty.
	void remove(Globe::Tile const & tile, Value value);

	// Removes a tile->value mapping without requiring construction of the tile.
	// TODO: Fill this in; remember to clear out the resolutionMap entry if empty.
	void remove(Globe::Tree::Index const & index, Globe::Tree::Level depth, Value value);

	// Removes value mapping from all tiles inside the given subtree.
	// TODO: Test this.
	void remove(Globe::Tree::Index const & subtree, Value value)
	{
		typename ResolutionMap::iterator const end = this->resolutionMap.end();
		for (typename ResolutionMap::iterator iterator = this->resolutionMap.begin();
			iterator != end; )
		{
			// Grab a copy of the iterator, and increment original.
			typename ResolutionMap::iterator current(iterator);
			++iterator;

			assert(current->second);
			Globe::Tree::SubtreeMultimap< Value > & subtreeMultimap = *(current->second);
			
			subtreeMultimap.remove(subtree, value);
			if (subtreeMultimap.getIsEmpty())
			{
				// This is safe, because erasing from a map only invalidates the current iterator.
				this->resolutionMap.erase(current);
			}
		}
	}

	// Finds all values mapped to the tile.
	// If 'includePartial' is true, finds values mapped to tiles within the tile.
	MutableSetInterface< Value > & find(
		Globe::Tile const & tile,
		MutableSetInterface< Value > & results,
		bool includePartial) const
	{
		typename ResolutionMap::const_iterator const iterator = this->resolutionMap.find(tile.getResolution());
		assert(iterator == this->resolutionMap.end() || iterator->second);
		return (iterator == this->resolutionMap.end()) ? results : iterator->second->find(tile.getIndex(), results, includePartial);
	}

	// Finds all values mapped to the tile.
	// If 'includePartial' is true, finds values mapped to tiles within the tile.
	MutableSetInterface< Value > & find(
		Globe::Tree::Index const & index, Globe::Tree::Level depth,
		MutableSetInterface< Value > & results,
		bool includePartial) const;

	// Visits values mapped to the tile.
	// Allows immediate response for each value.
	// If 'includePartial' is true, visits values mapped to tiles within the tile.
	bool visit(Globe::Tile const & tile, 
		MutableSetInterface< Value > & results,
		FunctorInterface< bool, Value > & callback,
		bool includePartial) const
	{
		typename ResolutionMap::const_iterator const iterator = this->resolutionMap.find(tile.getResolution());
		assert(iterator == this->resolutionMap.end() || iterator->second);
		return (iterator == this->resolutionMap.end()) ? true : iterator->second->visit(tile.getIndex(), results, callback, includePartial);
	}

	// Visits values mapped to the tile.
	// Allows immediate response for each value.
	// If 'includePartial' is true, visits values mapped to tiles within the tile.
	bool visit(Globe::Tree::Index const & index, Globe::Tree::Level depth,
		MutableSetInterface< Value > & results,
		FunctorInterface< bool, Value > & callback,
		bool includePartial) const;

public: // CollectionInterface

	bool getIsEmpty() const
	{
		assert(0); throw std::exception(); // TODO
	}

	size_t getCount() const
	{
		assert(0); throw std::exception(); // TODO
	}
};

#endif
