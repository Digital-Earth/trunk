#if !defined(PYXIS__GLOBE__RASTER)
#define PYXIS__GLOBE__RASTER

#include "pyxis/forward_range_interface.hpp"
#include "pyxis/globe/raster_interface.hpp"
#include "pyxis/globe/tile.hpp"
#include "pyxis/globe/tree/subtree_set.hpp"
#include "pyxis/set_interface.hpp"
#include <boost/thread.hpp>

namespace Pyxis
{
	namespace Globe
	{
		// A set of cells at a single resolution.
		class Raster;
	}
}

// TODO: Replace Visitors with Ranges.
class Pyxis::Globe::Raster :
public MutableSetInterface< Tile const & >,
public RasterInterface
{
	// The resolution of each cell in the raster.
	// Note that this needs to be converted to level when interacting with the tree.
	Resolution resolution;
	
	// All of the cells in the raster have indexes that are defined as all the descendants
	// of the subtrees in the set, at the level represented by the resolution.
	Tree::SubtreeSet subtreeMap;

public:

	class Test;

	// TODO: Test this
	class Tiles :
	public ForwardRangeInterface< Tile const & >,
	boost::noncopyable // TODO (mutex is noncopyable)
	{
		Raster const & raster;
		Tree::SubtreeSet::Pairs pairs;

		// The current tile.  All mutables must be thread-safe.
		mutable boost::mutex tileMutex;
		mutable boost::intrusive_ptr< Tile const > tile;

	public: 

		explicit Tiles(Raster const & raster) :
		raster(raster),
		pairs(raster.subtreeMap),
		tileMutex(), tile()
		{}

		bool getIsEmpty() const
		{
			return pairs.getIsEmpty();
		}

		// Asserts that it is non-empty.
		void popFront()
		{
			pairs.popFront();
			{
				boost::mutex::scoped_lock lock(tileMutex);
				tile.reset();
			}
		}

		// Asserts that it is non-empty.
		Tile const & getFront() const
		{
			boost::mutex::scoped_lock lock(tileMutex);
			if (!tile)
			{
				Tree::SubtreeSet::Pair pair = pairs.getFront();

				// Get a copy of the index.
				Tree::Index index = pair.first();

				// Get the depth.
				assert(index.getStepCount().offset <= Tree::Level(raster.getResolution()).offset);
				Tree::Level depth(
					Tree::Level(raster.getResolution()).offset - index.getStepCount().offset);

				// Construct tile by swapping in cell, then set member.
				tile.reset(new Tile(index, depth));
			}
			assert(tile);
			return *tile;
		}
	};

	// Constructs an empty raster.
	explicit Raster(Resolution resolution) :
	resolution(resolution), subtreeMap()
	{}

	// Constructs a raster at the specified resolution,
	// defined by the subtree set.
	explicit Raster(Resolution resolution, Tree::SubtreeSet const & subtreeMap) :
	resolution(resolution), subtreeMap(subtreeMap)
	{}
	
	bool operator ==(SetInterface< Tile const & > const & set) const
	{
		Raster const * const that = dynamic_cast< Raster const * const >(&set);
		return (that && *this == *that);
	}

	bool operator !=(SetInterface< Tile const & > const & set) const
	{
		Raster const * const that = dynamic_cast< Raster const * const >(&set);
		return (!that || *this != *that);
	}

	std::ostream & write(std::ostream & output) const
	{
		(void)output;
		assert(0 && "Not implemented.");
		throw std::exception();
	}

	// Swaps the contents.
	void swap(Raster & with)
	{
		std::swap(resolution, with.resolution);
		subtreeMap.swap(with.subtreeMap);
	}

	// Intersects the raster with the given tile set.
	// The result is the tiles that exist in both.
	// This is not a "spatial intersection".
	void intersect(SetInterface< Tile const & > const & intersectee)
	{
		(void)intersectee;
		assert(0 && "Not implemented.");
		throw std::exception();
	}

	// Get the tile range.
	boost::intrusive_ptr< ForwardRangeInterface< Tile const & > > getTiles() const
	{
		return new Tiles(*this);
	}

	// Returns the resolution of this raster;
	// any cells within must have this resolution.
	Resolution getResolution() const
	{
		return resolution;
	}

	// Returns true if empty.
	bool getIsEmpty() const
	{
		return subtreeMap.getIsEmpty();
	}
	
	// Sets the raster to empty.
	void setIsEmpty()
	{
		subtreeMap.setIsEmpty();
	}

	// Returns true if full.
	bool getIsFull() const
	{
		return subtreeMap.getIsFull();
	}
	
	// Sets the raster to full.
	void setIsFull()
	{
		subtreeMap.setIsFull();
	}
	
	// Returns the number of cells in the raster.
	// Not cached.
	size_t getCount() const
	{
		return subtreeMap.getDescendantCount(Tree::Level(resolution));
	}

	// Inserts cells in this resolution defined by subtree, if any.
	void insert(Tree::Index const & subtree)
	{
		if (Tree::Level(this->resolution).offset < subtree.getStepCount().offset)
		{
			throw std::invalid_argument("The subtree is not an ancestor of any cells at this resolution.");
		}
		subtreeMap.insert(subtree);
	}

	// Inserts the cells defined by the subtree, if any in this resolution.
	void insert(Tile const & tile)
	{
		insert(tile.getIndex());
	}

	// Inserts the raster.
	// If the raster is of the wrong resolution, an exception is thrown.
	void insert(Raster const & raster)
	{
		if (raster.resolution != resolution)
		{
			throw std::invalid_argument("The raster has the wrong resolution.");
		}

		// TODO: Lockstep iteration.
		for (Tree::SubtreeSet::Pairs pairs(raster.subtreeMap); !pairs.getIsEmpty(); pairs.popFront())
		{
			Tree::SubtreeSet::Pair const & pair = pairs.getFront();
			subtreeMap.insert(pair.first(), pair.second());
		}
	}
	
	void insert(SetInterface< Tile const & > const & tiles)
	{
		MutableSetInterface< Tile const & >::insert(tiles);
	}

	void remove(Tree::Index const & subtree)
	{
		if (Tree::Level(this->resolution).offset < subtree.getStepCount().offset)
		{
			throw std::invalid_argument("The subtree is not an ancestor of any cells at this resolution.");
		}
		subtreeMap.remove(subtree);
	}

	// Removes the cells defined by the tile,
	// if any in this raster.
	void remove(Tile const & tile)
	{
		remove(tile.getIndex());
	}

	// Removes the raster.
	// If the raster is of the wrong resolution, does nothing.
	void remove(Raster const & raster)
	{
		if (raster.resolution == resolution)
		{
			// TODO: Lockstep iteration.
			for (Tree::SubtreeSet::Pairs pairs(raster.subtreeMap); !pairs.getIsEmpty(); pairs.popFront())
			{
				subtreeMap.remove(pairs.getFront().first());
			}
		}
	}
	
	void remove(SetInterface< Tile const & > const & tiles)
	{
		MutableSetInterface< Tile const & >::remove(tiles);
	}
	
	bool find(Tree::Index const & subtree) const
	{
		if (Tree::Level(this->resolution).offset < subtree.getStepCount().offset)
		{
			throw std::invalid_argument("The subtree is not an ancestor of any cells at this resolution.");
		}
		return subtreeMap.find(subtree);
	}

	// Returns true if the tile is contained.
	// If the tile is of the wrong resolution, returns false.
	bool find(Tile const & tile) const
	{
		return find(tile.getIndex());
	}

	// Returns true if intersects, and if provided,
	// populates positive with the part of intersectee that intersects,
	// and negative with the part that does not.
	bool getIntersection(RasterInterface const & intersectee,
		Raster * positive = 0,
		Raster * negative = 0) const
	{
		(void)intersectee;
		(void)positive;
		(void)negative;
		assert(0); // TODO
		return 0;
	}

	// Returns true if intersects.
	bool getIntersects(RasterInterface const & intersectee) const
	{
		return getIntersection(intersectee);
	}

	// Appends the boundary of the raster to the boundary argument,
	// and if provided, appends the remainder into interior.
	void getBoundary(Raster & boundary,
		Raster * interior = 0) const;

	// Negates the raster; every cell in the resolution that was contained
	// is now not, and every one that wasn't now is.
	void negate();

	// Adds or removes a layer of cells from the perimeter.
	// Negative = contract; positive = expand; 0 = identity.
	void buffer(long cellCount);

	// The order is implementation-defined.
	bool visit(FunctorInterface< bool, Tile const & > & visitor) const
	{
		for (Tiles tiles(*this); !tiles.getIsEmpty(); tiles.popFront())
		{
			if (!visitor(tiles.getFront()))
			{
				return false;
			}
		}
		return true;
	}
	
	// The order is implementation-defined.
	void visitAll(FunctorInterface< void, Tile const & > & visitor) const
	{
		for (Tiles tiles(*this); !tiles.getIsEmpty(); tiles.popFront())
		{
			visitor(tiles.getFront());
		}
	}
	
	boost::intrusive_ptr<
		ForwardRangeInterface< Tile const & > > getElements() const
	{
		return getTiles();
	}
};

class Pyxis::Globe::Raster::Test
{
public:

	operator bool ()
	{
		// TODO: Add tests

		return true;
	}
};

#endif
