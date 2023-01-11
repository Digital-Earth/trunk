#if !defined(PYXIS__GRID__DODECAHEDRAL__TREE)
#define PYXIS__GRID__DODECAHEDRAL__TREE

#include <stack>
#include <stdexcept>
#include <queue>
#include <vector>
#include <boost/logic/tribool.hpp>
#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>
#include "pyxis/compact_set.hpp"
#include "pyxis/compact_vector.hpp"
#include "pyxis/dynamic_array.hpp"
#include "pyxis/empty.hpp"
#include "pyxis/functor_interface.hpp"
#include "pyxis/grid/cell.hpp"
#include "pyxis/grid/dodecahedral/cell_interface.hpp"
#include "pyxis/grid/dodecahedral/direction.hpp"
#include "pyxis/grid/dodecahedral/resolution.hpp"
#include "pyxis/grid/gazetteer.hpp"
#include "pyxis/grid/geometry.hpp"
#include "pyxis/grid/rosette.hpp"
#include "pyxis/multimap.hpp"
#include "pyxis/string_forward_range.hpp"

namespace Pyxis
{
	namespace Grid
	{
		namespace Dodecahedral
		{
			class Tree;
		}

		template <>
		class Cell< Dodecahedral::Tree >;

		template <>
		class Rosette< Dodecahedral::Tree >;

		template <>
		class Geometry< Dodecahedral::Tree >;

		template < typename Key >
		class Gazetteer< Dodecahedral::Tree, Key >;
	}
	
	template <>
	class CompactSet<
		Grid::Dodecahedral::Tree const & >;

	template <>
	class CompactSet<
		Grid::Rosette< Grid::Dodecahedral::Tree > const & >;

	template < typename Value >
	class Multimap<
		Grid::Dodecahedral::Tree const &,
		Value >;

	template < typename Value >
	class Multimap<
		Grid::Rosette< Grid::Dodecahedral::Tree > const &,
		Value >;

	template <>
	class Set<
		Grid::Dodecahedral::Tree const & >;

	template <>
	class Set<
		Grid::Rosette< Grid::Dodecahedral::Tree > const & >;
}

// A node in a tree-based hierarchical model of a dodecahedral grid.
/*
A tree contains a finite number of nodes, one of which is the root node of the tree.

The tree has the following parts:
-	Trunk: a vector of child offsets that describes the root node of the tree.
-	Branch: any subset of nodes within the tree.
-	Leaf: a node in the tree that has no children.

A cell is modeled by a tree trunk whose level corresponds to a resolution.
A rosette is modeled by a tree, truncated at a level, such that the leaves
correspond to cells in a resolution.

A tree is complete if its root node possesses the maximum child count.
Every branch of a complete tree is complete.
*/
class Pyxis::Grid::Dodecahedral::Tree :
public virtual Pointee
{
public:

	// A level.
	/*
	Levels greater than 4 correspond to resolutions:
	-	0: Root; Globe (no center point)
	-	1: Globe (center point, no antipodal vertex)
	-	2: Globe (center point, antipodal vertex)
	-	3: 3/4 Icosahedron pentagonal cells (partially overlapping)
	-	4: 1/4 Icosahedron pentagonal cells (partially overlapping)
	-	5: Dodecahedron pentagonal cells
	-	6: Truncated Icosahedron pentagonal and hexagonal cells
	*/
	typedef char unsigned Level;

	// A child count.
	typedef char ChildCount;

	// The offset of a child.
	typedef char ChildOffset;

	class Relative;
	class Trunk;
	class Leaves;
	class Test;

	// Returns the maximum level.
	static Level getMaximumLevel()
	{
		return boost::integer_traits< Level >::const_max;
	}

	// Converts a resolution to a level.
	static Level getLevel(Resolution const resolution)
	{
		enum { resolutionOffset = 5 };
		assert((int)resolution <= (int)getMaximumLevel() - resolutionOffset);
		return resolution + resolutionOffset;
	}

	// Returns true if a level represents a resolution.
	static bool getResolution(Level const level)
	{
		return getLevel(Resolution(0)) <= level;
	}

	// Converts a level to a resolution if possible. 
	static Resolution * getResolution(Resolution & resolution, Level const level)
	{
		if (!getResolution(level)) { return 0; }
		assert(getLevel(Resolution(0)) <= level);
		resolution = level - getLevel(Resolution(0));
		return &resolution;
	}

	// TODO: Test
	// Reads a tree from an index.
	friend std::istream & operator >>(std::istream & in, Tree & tree)
	{
		tree.read(
			std::istream_iterator< Direction >(in),
			std::istream_iterator< Direction >());
		return in;
	}

	// TODO: Test
	// Writes a unique index to the stream.
	// The index of the root tree is an empty direction sequence.
	friend std::ostream & operator <<(std::ostream & out, Tree const & tree)
	{
		tree.write(std::ostream_iterator< Direction >(out));
		return out;
	}

private:

	// The trunk.
	/*
	The offsets are stored such that the size
	of the vector corresponds to the level of the trunk.  This is done as follows:
	-	To grow a trunk (leaving leaves unchanged):
		-	To step the root node to the zero child:
			-	Push the zero offset.
		-	To step the root node to a non-zero child:
			-	Push the zero offset followed by the non-zero offset.
	-	To shrink a trunk (leaving leaves unchanged):
		-	To step the root node from the zero child:
			-	Pop the zero offset.
		-	To step the root node from a non-zero child:
			-	Pop the non-zero offset followed by the zero offset.
	*/
	std::vector< ChildOffset > trunk;
	
	// The highest non-full ancestor level.
	Level incompleteLevel;

protected:

	// Constructor for use by derived classes.
	// Sets the incomplete level for the derived tree.
	explicit Tree(Level const incompleteLevel) :
	trunk(), incompleteLevel(incompleteLevel)
	{}

	// Returns the child count of the root node.
	ChildCount getChildCount(Level const level) const
	{
		switch (level)
		{
		case 0: // Root; Globe ([])
			return 2; // [0] (centroid globe), [01] (vertex icosahedron)
		case 1: // Globe ([0])
			return 1; // [00] (centroid icosahedron)
		case 2: // Globe ([0p], where p is 0 or 1)
			return 6; // [0p0] (3/4 icosahedron), [0p0v] (5 1/4 icosahedra)
		case 3: // 3/4 Icosahedron ([0p0])
			return 1; // [0p00] (1/4 icosahedron)
		case 4: // 1/4 Icosahedron ([0p00], [0p0v])
			if (this->getChildOffset(level - 1)) // Non-polar
			{
				return 2; // [0p0v0] (dodecahedron pentagon), [0p0v01] (truncated icosahedron hexagon)
			}
			return 6; // [0p000] (dodecahedron pentagon), [0p000v] (truncated icosahedron hexagon)
		default: // Dodecahedron or greater; resolution 0 and up
			if (this->getIncompleteLevel() < level) // Non-pentagonal
			{
				return 7;
			}
			return 6;
		}
	}

protected:

	// Returns the child offset at the given level offset.
	// Asserts that the level offset is valid.
	virtual ChildOffset getChildOffset(Level const levelOffset) const
	{
		assert(levelOffset < this->getLevel());

		return this->trunk[levelOffset];
	}

	// Sets the last child offset.
	// Asserts that the level is non-zero.
	virtual void setChildOffset(ChildOffset const childOffset)
	{
		assert(!this->getIsRoot());
		
		assert(!this->trunk.empty());
		this->trunk.back() = childOffset;
	}
	
	// Pushes a child offset onto the end.
	// Asserts that this doesn't result in overflow.
	virtual void pushChildOffset(ChildOffset const childOffset)
	{
		assert(this->getLevel() < getMaximumLevel());
	
		this->trunk.push_back(childOffset); // Can throw.
	}
	
	// Pops a child offset from the end.
	// Asserts that the level is non-zero.
	virtual void popChildOffset()
	{
		assert(!this->getIsRoot());

		assert(!this->trunk.empty());
		this->trunk.pop_back();
	}
	
public:
	
	// Constructs the root tree.
	Tree() : trunk(), incompleteLevel() {}

	virtual bool operator !=(Tree const & tree) const
	{
		return !this->operator==(tree);
	}

	// TODO: Test
	// Reads a tree from an index.
	template < typename InputDirectionIterator >
	void read(
		InputDirectionIterator iterator,
		InputDirectionIterator const end)
	{
		if (end != iterator)
		{
			// Get the first direction, convert to child offset,
			// and throw if it is not zero.
			ChildOffset childOffset;
			if (!this->getChildOffset(childOffset, *iterator) || childOffset)
			{
				throw std::invalid_argument("Invalid index.");
			}
			assert(!childOffset);

			while (end != ++iterator)
			{
				// Read the next direction and convert to child offset.
				if (!this->getChildOffset(childOffset, *iterator))
				{
					throw std::invalid_argument("Invalid index.");
				}

				// If the child offset is zero, step to the previous zero.
				// Otherwise, step to this child offset.
				this->stepToChild(childOffset);

				// If the child offset is non-zero, advance the iterator
				// (returning if at end) and read the next zero.
				if (childOffset)
				{
					// If we're at the end, there is nothing more to add.
					if (end == ++iterator)
					{
						return;
					}

					// Read the next direction and convert to child offset.
					// The child offset must be zero, or the index is invalid.
					if (!this->getChildOffset(childOffset, *iterator) || childOffset)
					{
						throw std::invalid_argument("Invalid index.");
					}
					assert(!childOffset);
				}
			}

			// Step to the previous zero.
			this->stepToChild(0);
		}
	}
	
	// Convenience: reads a null-terminated index.
	void read(Direction const & index)
	{
		StringForwardRange< Direction > range(index); 
		read(range.begin(), range.end());
	}
	
	// TODO: Test
	template < typename OutputDirectionIterator >
	OutputDirectionIterator write(OutputDirectionIterator iterator) const
	{
		// TODO: Use a range that can be better optimized.
		Level const level(this->getLevel());
		for (Level offset(0); offset < level; ++offset)
		{
			*iterator++ = this->getDirection(this->getChildOffset(offset));
		}
		return iterator;
	}

	// TODO: Test
	// Convenience.
	// Writes the index to the string.
	std::string & write(std::string & index) const
	{
		this->write(std::back_inserter(index));
		return index;
	}

	// Converts child offset to direction.
	// TODO: Don't just do character to integer translation; use meaningful directions.
	Direction getDirection(ChildOffset const childOffset) const
	{
		return childOffset + '0';
	}

	// Converts direction to child offset.
	// TODO: Don't just do character to integer translation; use meaningful directions.
	ChildOffset * getChildOffset(ChildOffset & result, Direction const direction) const
	{
		if ('0' <= direction)
		{
			size_t const childOffset(direction - '0');
			if (childOffset < (size_t)this->getChildCount())
			{
				result = (ChildOffset)childOffset;
				return &result;
			}
		}
		return 0;
	}

	// Returns the level of the nearest incomplete root node ancestor in the trunk vector.
	Level getIncompleteLevel() const
	{
		return this->incompleteLevel;
	}

	// Returns true if the root node is complete.
	bool getIsComplete() const
	{
		return this->getIncompleteLevel() < this->getLevel();
	}
	
	// Returns true if this tree is the zero child of another.
	bool getIsZeroChild() const
	{
		return !this->getIsRoot() && !this->getChildOffset();
	}
	
	// Returns true if the tree (or part, if includePartial is true) is contained.
	// TODO: Test this.
	bool find(Tree const & that, bool const includePartial = false) const
	{
		// If that is ancestor of this, return includePartial.
		// Otherwise, return (that equals this, or is descendant of this).
		Level const thisLevel(this->getLevel());
		Level const thatLevel(that.getLevel());
		for (Level thisLevelOffset(0), thatLevelOffset(0);
			thisLevelOffset < thisLevel;
			++thisLevelOffset, ++thatLevelOffset)
		{
			assert(thatLevelOffset <= thatLevel);
			if (thatLevel == thatLevelOffset)
			{
				return includePartial;
			}
			if (this->getChildOffset(thisLevelOffset) != that.getChildOffset(thatLevelOffset))
			{
				return false;
			}
		}
		return true;
	}

	// Returns the leaf count if truncated at the given level.
	// If level <= getLevel(), returns 1.
	size_t getLeafCount(Level const level) const
	{
		if (getLevel() < level)
		{
			size_t leafCount = 0;
			
			assert(0); throw std::exception(); // TODO

			return leafCount;
		}
		return 1;
	}

	// Returns the child count of the tree.
	ChildCount getChildCount() const
	{
		return this->getChildCount(this->getLevel());
	}

	// Steps the root node to the specified child.
	// Throws if the child offset is invalid.
	// Returns false if at maximum level.
	bool stepToChild(ChildOffset const childOffset)
	{
		if (this->getChildCount() <= childOffset)
		{
			throw std::invalid_argument("The child offset is out of range.");
		}
		
		Level const parentLevel(this->getLevel());
		if (getMaximumLevel() - !!childOffset <= parentLevel)
		{
			return false;
		}

		this->pushChildOffset(0); // Can throw.
		if (childOffset)
		{
			this->pushChildOffset(childOffset); // Can throw.
		}

		if (parentLevel == this->incompleteLevel)
		{
			// Increment nonFullLevel for zero child offset.
			// If childOffset argument is non-zero, and  
			// the parent level is for an icosahedron or lower,
			// increment a second time.
			this->incompleteLevel += (1 + (childOffset && (parentLevel < 3)));
		}

		return true;
	}
	
	// Returns true if the root node has a parent node.
	bool getParentCount() const
	{
		return !this->getIsRoot();
	}

	// Steps the root node to the parent,
	// and sets the argument to the child offset stepped from.
	// Returns false if the trunk is empty.
	bool stepToParent(ChildOffset & childOffset)
	{
		if (this->getIsRoot())
		{
			return false;
		}

		childOffset = this->getChildOffset();
		this->popChildOffset();
		if (childOffset)
		{
			assert(!this->getIsRoot() && "Every non-zero is preceded by a zero.");
			this->popChildOffset();
		}

		this->incompleteLevel = std::min(this->getLevel(), this->incompleteLevel);

		return true;
	}

	// Steps the root node to the parent.
	// Returns false if the trunk is empty.
	bool stepToParent()
	{
		ChildOffset childOffset;
		return stepToParent(childOffset);
	}

	// Returns the number of siblings, including self.
	// The root tree is not a child, so it is not a sibling.
	ChildCount getSiblingCount() const
	{
		return (this->getIsRoot() ?
			0 :
			this->getChildCount(this->getLevel() - (1 + !!this->getChildOffset())));
	}

	// Gets the sibling offset of the current tree.
	bool getSiblingOffset(ChildOffset & siblingOffset) const
	{
		if (this->getIsRoot())
		{
			return false;
		}
		siblingOffset = this->getChildOffset();
		return true;
	}

	// Steps to the sibling.  Note that this can change the level;
	// non-zero children are one level higher than the zero child.
	// Returns false if it could not succeed (insufficient levels).
	// Sets the argument to the former sibling.
	// Throws if child offset is invalid (including if the tree is root).
	bool stepToSibling(
		ChildOffset const toSiblingOffset,
		ChildOffset & fromSiblingOffset)
	{
		Level const level(this->getLevel());
		if (!level)
		{
			throw std::invalid_argument("The root tree has no siblings.");
		}
		
		fromSiblingOffset = this->getChildOffset();
		if (toSiblingOffset)
		{
			assert((Level)!!fromSiblingOffset < level);
			ChildCount const siblingCount(this->getChildCount(level - (1 + !!fromSiblingOffset)));
			if (siblingCount <= toSiblingOffset)
			{
				throw std::invalid_argument("The sibling offset is out of range.");
			}

			if (fromSiblingOffset)
			{
				this->setChildOffset(toSiblingOffset);
			} else
			{
				if (getMaximumLevel() == level)
				{
					return false;
				}

				this->pushChildOffset(toSiblingOffset);

				this->incompleteLevel += (level == this->incompleteLevel && level < 4);
			}
		} else if (fromSiblingOffset)
		{
			this->popChildOffset();

			assert(this->incompleteLevel <= level);
			this->incompleteLevel -= (level == this->incompleteLevel);
		}
		return true;
	}

	// Steps to the sibling.  Note that this can change the level;
	// non-zero children are one level higher than the zero child.
	// Returns false if it could not succeed (insufficient levels).
	// Sets the argument to the former sibling.
	// Throws if child offset is invalid (including if the tree is root).
	bool stepToSibling(ChildOffset const toSiblingOffset)
	{
		ChildOffset fromSiblingOffset;
		return stepToSibling(toSiblingOffset, fromSiblingOffset);
	}

public:

	virtual bool operator ==(Tree const & tree) const
	{
		return this->trunk == tree.trunk;
	}

	// Constructs a relative tree.
	// The resulting relative requires that this tree outlives it.
	// The definition must follow that of Relative.
	virtual Relative getRelative() const;

	// Constructs an ancestor tree (or throws if level is a descendant level).
	// The resulting relative requires that this tree outlives it.
	// The definition must follow that of Relative.
	virtual Relative getRelative(Level const level) const;

	virtual bool getIsRoot() const
	{
		return this->trunk.empty();
	}

	virtual Level getLevel() const
	{
		return this->trunk.size();
	}

	// Returns the last child offset.
	// Asserts that the level is non-zero.
	virtual ChildOffset getChildOffset() const
	{
		assert(!this->getIsRoot());
		
		assert(!this->trunk.empty());
		return this->trunk.back();
	}
};

// A relative tree.
/*
Contains a reference to an existing tree and a level
(the prefix), a sequence of child offsets (the suffix),
and the incomplete level of the whole.
The latter two are stored in the base class.
*/
class Pyxis::Grid::Dodecahedral::Tree::Relative :
public Tree
{
	// The prefix tree.
	Tree const * tree;

	// The prefix level.
	// This is the amount of the prefix tree to use.
	Level level;

protected:

	ChildOffset getChildOffset(Level const levelOffset) const
	{
		assert(this->tree);
		assert(levelOffset < this->getLevel());

		if (levelOffset < this->level)
		{
			return this->tree->getChildOffset(levelOffset);
		}
		return Tree::getChildOffset(levelOffset - this->level);
	}

	void setChildOffset(ChildOffset const childOffset)
	{
		assert(this->tree);
		assert(!this->getIsRoot());

		if (Tree::getIsRoot())
		{
			assert(this->level);
			if (childOffset != this->tree->getChildOffset(this->level - 1))
			{
				--this->level;
				Tree::pushChildOffset(childOffset); // Can throw.
			}

			return;
		}
		Tree::setChildOffset(childOffset);
	}

	void pushChildOffset(ChildOffset const childOffset)
	{
		assert(this->tree);
		assert(this->getLevel() < getMaximumLevel());

		if (Tree::getIsRoot() &&
			this->level < this->tree->getLevel() && 
			childOffset == this->tree->getChildOffset(this->level))
		{
			++this->level;

			return;
		}
		Tree::pushChildOffset(childOffset); // Can throw.
	}

	void popChildOffset()
	{
		assert(this->tree);
		assert(!this->getIsRoot());

		if (Tree::getIsRoot())
		{
			assert(this->level);
			--this->level;

			return;
		}
		Tree::popChildOffset();
	}

private:

	// Give the factory functions in Tree special access to the
	// following constructors.
	friend Relative Tree::getRelative() const;
	friend Relative Tree::getRelative(Level const) const;

	// Constructs a reference to the given tree.
	// Requires the tree to outlive this object.
	explicit Relative(Tree const & tree) : 
	Tree(tree.getIncompleteLevel()),
	tree(&tree),
	level(tree.getLevel())
	{}

	// Constructs a reference to the given tree,
	// truncated to 'level'.
	// Throws if level exceeds tree.getLevel().
	// Requires the tree to outlive this object.
	explicit Relative(Tree const & tree, Level const level) :
	Tree(std::min(level, tree.getIncompleteLevel())),
	tree(&tree),
	level((tree.getLevel() < level) ?
		throw std::invalid_argument("Level is out of range.") :
		level)
	{}

public:

	// Copies the relative tree, but truncates to the given level.
	explicit Relative(Relative const & relative, Level const level) :
	Tree(std::min(level, relative.getIncompleteLevel())),
	tree((relative.level < relative.getLevel()) ?
		&relative /* There is a suffix, so references must be chained. */:
		relative.tree /* There is no suffix, so references can be collapsed. */),
	level((relative.getLevel() < level) ?
		throw std::invalid_argument("Level is out of range.") :
		level)
	{}

	bool operator ==(Relative const & relative) const
	{
		return (
			this->tree == relative.tree &&
			this->level == relative.level &&
			Tree::operator ==(relative));
	}

	using Tree::stepToChild;

	// If this tree is an ancestor of another, steps to the child.
	bool stepToChild()
	{
		assert(this->tree);
	
		if (Tree::getIsRoot() &&
			this->level < this->tree->getLevel())
		{
			// Advance.
			++this->level;
			assert(!this->tree->getChildOffset(this->level - 1) &&
				"The child offset should be zero.");

			// If the next child offset is non-zero, advance again.
			if (this->level < this->tree->getLevel() &&
				this->tree->getChildOffset(this->level))
			{
				++this->level;
			}

			return true;
		}
		return false;
	}

public: // Tree

	virtual bool operator ==(Tree const & tree) const
	{
		Relative const * const relative(dynamic_cast< Relative const * >(&tree));
		return relative && this->operator==(*relative);
	}

	// Constructs a relative tree.
	// The resulting relative requires that this tree outlives it.
	// Any modern compiler should use return value optimization here.
	Relative getRelative() const
	{
		return Relative(*this);
	}

	// Constructs an ancestor tree (or throws if level is a descendant level).
	// The resulting relative requires that this tree outlives it.
	// Any modern compiler should use return value optimization here.
	Relative getRelative(Level const level) const
	{
		return Relative(*this, level);
	}

	bool getIsRoot() const
	{
		return !this->level && Tree::getIsRoot();
	}

	Level getLevel() const
	{
		return this->level + Tree::getLevel();
	}

	ChildOffset getChildOffset() const
	{
		assert(this->tree);
		assert(!this->getIsRoot());

		if (Tree::getIsRoot())
		{
			return this->tree->getChildOffset(this->level - 1);
		}
		return Tree::getChildOffset();
	}
};

// Any modern compiler should use return value optimization here.
inline Pyxis::Grid::Dodecahedral::Tree::Relative
Pyxis::Grid::Dodecahedral::Tree::getRelative() const
{
	return Relative(*this);
}

// Any modern compiler should use return value optimization here.
inline Pyxis::Grid::Dodecahedral::Tree::Relative
Pyxis::Grid::Dodecahedral::Tree::getRelative(
	Level const level) const
{
	return Relative(*this, level);
}

// The forward range of child offsets in the trunk of a tree.
class Pyxis::Grid::Dodecahedral::Tree::Trunk :
public virtual ForwardRangeInterface< ChildOffset >
{
	Tree const & tree;
	Level levelOffset;

public:

	// Requires the tree to outlive this object.
	explicit Trunk(Tree const & tree) :
	tree(tree), levelOffset()
	{}

public: // ForwardRangeInterface< ChildOffset >

	bool getIsEmpty() const
	{
		assert(this->levelOffset <= this->tree.getLevel());
		return this->tree.getLevel() <= this->levelOffset;
	}

	// Asserts that it is non-empty.
	void popFront()
	{
		assert(this->levelOffset < this->tree.getLevel());
		++this->levelOffset;
		this->levelOffset += (
			this->levelOffset < this->tree.getLevel() && this->tree.getChildOffset(this->levelOffset));
	}

	// Asserts that it is non-empty.
	ChildOffset getFront() const
	{
		assert(this->levelOffset < this->tree.getLevel());
		assert(!tree.getChildOffset(this->levelOffset));
		Level const nextLevelOffset = (1 + this->levelOffset);
		return this->tree.getChildOffset(
			this->levelOffset + (
				nextLevelOffset < this->tree.getLevel() && this->tree.getChildOffset(nextLevelOffset)));
	}
};

// The range of descendants of a tree at a depth.
class Pyxis::Grid::Dodecahedral::Tree::Leaves :
public virtual ForwardRangeInterface< Tree const & >
{
	Tree const & tree;
	
	Level level;

	boost::optional< Tree::Relative > leaf;

	void descend()
	{
		assert(this->leaf);
		assert(this->leaf->getLevel() <= this->level);

		while (this->leaf->getLevel() != this->level)
		{
			bool const stepped = this->leaf->stepToChild(0);
			assert(stepped);
		}
	}

public:

	explicit Leaves(Tree const & tree, Level const depth) :
	tree(tree),
	level(tree.getLevel()),
	leaf(tree.getRelative())
	{
		if (getMaximumLevel() - depth < this->level)
		{
			throw std::invalid_argument("Overflow.");
		}
		this->level += depth;
		this->descend();
	}

	Tree const & getTree() const
	{
		return this->tree;
	}
	
	Level getLevel() const
	{
		return this->level;
	}

public: // SetInterface< Tree const & >

	bool getIsEmpty() const
	{
		return !this->leaf;
	}

	// Asserts non-empty.
	Tree const & getFront() const
	{
		assert(this->leaf);

		return *this->leaf;
	}

	// Asserts non-empty.
	void popFront()
	{
		assert(this->leaf);
		assert(this->tree.getLevel() <= this->leaf->getLevel());

		for (ChildOffset siblingOffset;
			this->leaf->getSiblingOffset(siblingOffset);
			this->leaf->stepToParent())
		{
			if (++siblingOffset < this->leaf->getSiblingCount() &&
				this->leaf->stepToSibling(siblingOffset))
			{
				Level const leafLevel = this->leaf->getLevel();
				if (leafLevel <= this->tree.getLevel())
				{
					break;
				}
				if (leafLevel <= this->level)
				{
					this->descend();
					return;
				}
			}
		}

		this->leaf = boost::none;
	}
};

template < typename Value >
class Pyxis::Multimap<
	Pyxis::Grid::Dodecahedral::Tree const &,
	Value > :
public virtual Pyxis::MutableMultimapInterface<
	Pyxis::Grid::Dodecahedral::Tree const &,
	Value >
{
	typedef Grid::Dodecahedral::Tree Tree;
	typedef Tree::ChildOffset ChildOffset;
	typedef Tree::ChildCount ChildCount;
	typedef Tree::Level Level;
	typedef Tree::Relative Relative;

	// A node represents the values mapped to a tree.
	class Node :
	boost::noncopyable // Has special copy constructor and assign method that additionally take Tree.
	{
		// A vector of child offsets.
		typedef CompactVector< ChildOffset > Trunk;

		// A dynamically-allocated array of nodes.
		typedef DynamicArray< Node, ChildCount > Branches;

		// The set of values mapped to this node.
		CompactSet< Value > values;

		// The values mapped to a descendant.
		CompactSet< Value > descendantValues;

		// The single-child offsets that descend from this node.
		Trunk trunk;

		// The child nodes at the end of the trunk.
		Branches branches;

	private:

		// Helper.
		// Descends the tree to that of the parent node of the branches,
		// and returns the branch count.
		// Asserts that this is not a leaf.
		ChildCount descend(
			Tree & tree /* Pre: the node tree; post: the branch parent */) const
		{
			assert(!this->branches.getIsEmpty());

			// Descend to the parent node of the branches.
			for (typename Trunk::Elements trunkElements(this->trunk);
				!trunkElements.getIsEmpty(); trunkElements.popFront())
			{
				tree.stepToChild(trunkElements.getFront());
			}

			// Return the branch count.
			ChildCount const branchCount(tree.getChildCount());
			assert(1 < branchCount);
			return branchCount;
		}

	private: // Modifying the trunk

		// Appends the remainder of the tree as a trunk,
		// except for the last child offset, which becomes the branch.
		// Returns success.
		bool append(
			Relative & relative /* Pre: the child offsets to append. Post: the new branch. */)
		{
			assert(this->getIsLeaf());

			// Append to the trunk elements until we hit the
			// end of the relative.  The last child offset is not appended;
			// this will become the branch offset.
			for (std::vector< ChildOffset > trunkElements; ; )
			{
				Relative parent(relative);
				if (!relative.stepToChild())
				{
					// While the branch count is 1, step to the parent.
					// If we run out of parents, return false.
					for (; ; trunkElements.pop_back())
					{
						parent.stepToParent();
						ChildCount const branchCount(parent.getChildCount());
						if (1 < branchCount)
						{
							if (!trunkElements.empty())
							{
								ChildOffset const * const begin = &trunkElements[0];
								trunk.append(begin, begin + trunkElements.size());
							}
							this->branches.reset(branchCount);
							return true;
						}
						if (trunkElements.empty())
						{
							return false;
						}
					}
				}
				trunkElements.push_back(parent.getChildOffset());
			}
		}

		// Splits the trunk so that it is truncated to the given size,
		// and the remainder is put in the correct branch
		// (with all other branches remaining empty).
		// Returns true if successful.
		bool split(
			Level trunkElementCount /* The new trunk element count. */, 
			Relative & relative /* The new branch. */)
		{
			assert(!this->getIsLeaf());

			for (; ; --trunkElementCount)
			{
				relative.stepToParent();
				ChildCount const branchCount = relative.getChildCount();
				if (1 < branchCount)
				{
					// Create a temporary node, which will be swapped into a child.
					Node temporaryNode;

					// Copy the descendant values into the temporary node.
					temporaryNode.descendantValues = this->descendantValues;

					// Get the branch offset.
					ChildOffset const branchOffset = this->trunk[trunkElementCount];

					// Copy the end of this node's trunk to the temporary node's trunk.
					temporaryNode.trunk.append(
						&this->trunk[0] + trunkElementCount + 1 /* Skip branch offset */,
						&this->trunk[0] + this->trunk.getCount());

					// Truncate this node's trunk.
					this->trunk.truncate(this->trunk.getCount() - trunkElementCount);

					// Swap this node's branches with the temporary node's branches.
					this->branches.swap(temporaryNode.branches);

					// Create new branches in this node, with the correct branch count.
					assert(this->branches.getIsEmpty());
					this->branches.reset(branchCount);

					// Swap the correct branch node with the temporary node.
					this->branches[branchOffset].swap(temporaryNode);

					// Advance level to branch.
					relative.stepToChild();

					return true;
				}

				// If the trunk element count is 0, it is a no-op;
				// the tree for the branch offset has 1 child, which
				// corresponds to the current node.
				if (!trunkElementCount)
				{
					return false;
				}
			}
		}

		// Merges the branch with the trunk.
		void merge(
			ChildOffset branchOffset /* The branch offset to merge. */)
		{
			assert(!this->branches.getIsEmpty());

			// Get the branch.
			Node & branch = this->branches[branchOffset];

			// Append the branch step to the trunk,
			// and step the tree to the corresponding child.
			this->trunk.append(&branchOffset, &branchOffset + 1);

			// If the branch trunk is not empty,
			// append the branch trunk to the trunk,
			// and step the tree to the corresponding children.
			if (!branch.trunk.getIsEmpty())
			{
				this->trunk.append(
					&branch.trunk[0],
					&branch.trunk[0] + branch.trunk.getCount());
			}

			// Swap the branches out of the branch into a temporary.
			// Need to use a temporary to avoid a self-double-delete.
			// Postcondition: branch.branches is empty.
			Branches temporaryBranches;
			temporaryBranches.swap(branch.branches);

			// Swap the branches into this node.
			// Postcondition: temporary branches has original branches.
			this->branches.swap(temporaryBranches);
		}

	private:

		// Removes the value from each descendant,
		// then removes from descendant values.
		void removeFromDescendants(Value value,
			Relative relative /* The node tree. */)
		{
			if (this->descendantValues.find(value))
			{
				ChildCount branchCount(this->descend(relative));
				bool areBranchesEmpty = true;
				{
					do
					{
						ChildOffset const branchOffset = --branchCount;
						relative.stepToChild(branchOffset);
						{
							Node & branch = this->branches[branchOffset];
							branch.remove(value, relative);
							areBranchesEmpty = areBranchesEmpty && branch.getIsEmpty();
						}
						relative.stepToParent();
					} while (branchCount);
				}
				if (areBranchesEmpty)
				{
					this->setIsLeaf();
				} else
				{
					this->descendantValues.remove(value);
				}
			}
		}

		// Finds the descendant nodes that the value is mapped to.
		MutableSetInterface< Tree const & > & findInDescendants(Value value,
			MutableSetInterface< Tree const & > & results,
			Relative relative /* The node tree. */) const
		{
			if (this->descendantValues.find(value))
			{
				ChildCount branchCount(this->descend(relative));
				do
				{
					ChildOffset const branchOffset = --branchCount;
					relative.stepToChild(branchOffset);
					this->branches[branchOffset].find(value, results, relative);
					relative.stepToParent();
				} while (branchCount);
			}
			return results;
		}

	public:

		struct Child
		{
			Node const & node;
			Tree const & tree;
			
			explicit Child(Node const & node, Tree const & tree) :
			node(node), tree(tree)
			{}
		};

		class Children :
		public virtual ForwardRangeInterface< Child >
		{
			Node const & node;
			Relative relative;
			ChildCount childOffset;
			ChildCount childCount;

		public:

			// Call this one if it is any node.
			explicit Children(Node const & node, Tree const & tree) :
			node(node), relative(tree.getRelative()), childOffset(), childCount()
			{
				if (!this->node.getDescendantValues().getIsEmpty())
				{
					this->childCount = this->node.descend(this->relative);
					this->relative.stepToChild(0);
				}
			}

			Node const & getNode() const
			{
				return this->node;
			}
			
			Tree const & getTree() const
			{
				return this->relative;
			}
			
		public: // ForwardRangeInterface< Node const & >
		
			bool getIsEmpty() const
			{
				assert(this->childOffset <= this->childCount);

				return this->childCount == this->childOffset;
			}

			// Asserts that it is non-empty.
			void popFront()
			{
				assert(this->childOffset < this->childCount);
				
				// Update child offset and tree.
				if (++this->childOffset < this->childCount)
				{
					this->relative.stepToSibling(this->childOffset);
				}
			}

			// Asserts that it is non-empty.
			Child getFront() const
			{
				assert(this->childOffset < this->childCount);

				// Construct and return a child node from the child node and current tree.
				return Child(this->node.branches[this->childOffset], this->relative);
			}
		};

		// Constructs an empty node.
		Node() :
		values(), descendantValues(), trunk(), branches() {}

		// Copies the node.
		Node(Node const & node,
			Relative relative /* The node tree. */) :
		values(node.values),
		descendantValues(node.descendantValues),
		trunk(node.trunk),
		branches()
		{
			if (!node.branches.getIsEmpty())
			{
				ChildCount branchCount(node.descend(relative));
				this->branches.reset(branchCount);
				do
				{
					relative.stepToChild(--branchCount);
					this->branches[branchCount].assign(node.branches[branchCount], relative);
					relative.stepToParent();
				} while (branchCount);
			}
		}

		// Swaps the contents of the node.
		void swap(Node & that)
		{
			this->values.swap(that.values);
			this->descendantValues.swap(that.descendantValues);
			this->trunk.swap(that.trunk);
			this->branches.swap(that.branches);
		}

		// Assigns the node.
		void assign(Node const & that,
			Tree const & tree /* The node tree. */)
		{
			Node(that, tree.getRelative()).swap(*this);
		}

		// Writes a user-friendly string describing the node structure; for diagnostics.
		void write(std::ostream & output,
			Relative relative /* The node tree. */) const
		{
			// Write values.
			output << this->values;

			// Write values mapped to descendant(s).
			output << this->descendantValues;

			// If this is not a leaf, write.
			if (!this->branches.getIsEmpty())
			{
				// Write the trunk.
				for (typename Trunk::Elements trunkElements(this->trunk);
					!trunkElements.getIsEmpty(); trunkElements.popFront())
				{
					// Write the next child offset in the trunk.
					ChildOffset const trunkElement = trunkElements.getFront();
					std::string indent(relative.getLevel() + 1, '.');
					relative.stepToChild(trunkElement);
					output << std::endl << indent;
					if (trunkElement)
					{
						output << "0";
					}
					output << (int)trunkElement;
				}
				
				// Write the branches.
				{
					std::string indent(relative.getLevel() + 1, '.');
					ChildCount branchCount = relative.getChildCount();
					assert(1 < branchCount);
					ChildOffset branchOffset = 0;
					do
					{
						relative.stepToChild(branchOffset);
						output << std::endl << indent;
						if (branchOffset)
						{
							output << "0";
						}
						output << (int)branchOffset;
						this->branches[branchOffset].write(output, relative);
						relative.stepToParent();
					} while (++branchOffset < branchCount);
				}
			}
		}

		bool getIsLeaf() const
		{
			assert(!this->descendantValues.getIsEmpty() || (
				this->trunk.getIsEmpty() && this->branches.getIsEmpty()));
			return this->descendantValues.getIsEmpty();
		}

		void setIsLeaf()
		{
			this->descendantValues.setIsEmpty();
			this->trunk.setIsEmpty();
			this->branches.setIsEmpty();
		}

		// Returns true if empty.	
		bool getIsEmpty() const
		{
			return this->values.getIsEmpty() && getIsLeaf();
		}

		// Empties the collection.
		void setIsEmpty()
		{
			this->values.setIsEmpty();
			this->setIsLeaf();
		}
		
		// Returns the set of values mapped to this node.
		SetInterface< Value > const & getValues() const
		{
			return this->values;
		}
		
		// Returns the set of values mapped to descendant nodes, not including this node.
		SetInterface< Value > const & getDescendantValues() const
		{
			return this->descendantValues;
		}
		
	public: // Value

		// Inserts a value into this node.
		// Returns true if the values of this node were modified.
		bool insert(Value value,
			Tree const & tree /* The node tree. */)
		{
			if (!this->values.find(value))
			{
				this->removeFromDescendants(value, tree.getRelative());
				this->values.insert(value);
				return true;
			}
			return false;
		}

		// Remove the value from this node and all descendants.
		// Returns true if the values of this node were modified.
		bool remove(Value value,
			Tree const & tree /* The node tree. */)
		{
			if (this->values.find(value))
			{
				assert(!this->descendantValues.find(value));
				this->values.remove(value);
				return true;
			}
			this->removeFromDescendants(value, tree.getRelative());
			return false;
		}
		
		// Returns true if the value is mapped to this node
		// (or one of its descendants, if includePartial is true).
		bool find(Value value, bool const includePartial) const
		{
			return this->values.find(value) || (
				includePartial && this->descendantValues.find(value));
		}

		// Finds the descendant nodes explicitly mapped to the value,
		// and stores the tree of each in the results.
		// Returns a reference to the results.
		MutableSetInterface< Tree const & > & find(Value value,
			MutableSetInterface< Tree const & > & results,
			Tree const & tree /* The node tree. */) const
		{
			if (this->values.find(value))
			{
				results.insert(tree);
				return results;
			}
			return this->findInDescendants(value, results, tree.getRelative());
		}

	public: // Descendant value

		// Inserts the mapping of the value to the descendant node for the given tree.
		// Returns true if the value is now mapped to this node (and wasn't before).
		bool insertDescendant(
			Relative relative /* The descendant ancestor that describes this node. */,
			Value value)
		{
			// If the value is already mapped to the node, return false.
			if (this->values.find(value))
			{
				assert(!this->descendantValues.find(value));
				return false;
			}

			// Capture relative before descent.
			Relative const initial(relative);

			// Try to advance the tree to the next node.
			if (relative.stepToChild())
			{
				if (this->getIsLeaf())
				{
					// Append as a trunk and full branch.
					// If successful, the value is not mapped to the entire node.
					// If unsuccessful, fall through to map the value to the entire node.
					if (this->append(relative))
					{
						// Set the correct branch to full for value.
						ChildOffset const branchOffset(relative.getChildOffset());
						this->branches[branchOffset].values.insert(value);
						goto False;
					}
				} else
				{
					// Descend the trunk and split either at difference, or if tree runs out.
					{
						assert(this->trunk.getCount() <= boost::integer_traits< Level >::const_max);
						Level trunkElementCount = this->trunk.getCount();
						for (Level trunkElementOffset = 0; trunkElementOffset < trunkElementCount;
							++trunkElementOffset)
						{
							ChildOffset const childOffset = relative.getChildOffset();
							ChildOffset const trunkElement = this->trunk[trunkElementOffset];
							if (childOffset != trunkElement || !relative.stepToChild())
							{
								// Split trunk before difference.
								// The tree level given is that of the new branch.
								if (this->split(trunkElementOffset, relative))
								{
									// Break so that we continue on to the correct child.
									break;
								}

								// The split was unsucessful; map the value to the entire node.
								goto True;
							}
						}
					}

					// Insert into the correct branch.
					// If this returns false, there is no value collapsing opportunity; return false.
					assert(!this->branches.getIsEmpty());
					{
						ChildOffset const branchOffset(relative.getChildOffset());
						Node & branch = this->branches[branchOffset];
						if (!branch.insertDescendant(relative, value))
						{
							goto False;
						}
					}

					// Step up to the parent and get the branch count.
					relative.stepToParent();
					ChildCount const branchCount(relative.getChildCount());
					assert(1 < branchCount);

					// The branch is now full of the value.
					// If each branch is not full of the value, return false.
					{
						ChildCount branchOffset = branchCount;
						do
						{
							if (!this->branches[--branchOffset].values.find(value))
							{
								goto False;
							}
						} while (branchOffset);
					}

					// All branches are full of the value.
					// If the trunk isn't empty, split and collapse the value up to the next node.
					// Otherwise, fall through and insert the value for the entire node.
					assert(this->trunk.getCount() <= boost::integer_traits< Level >::const_max);
					Level trunkElementCount(this->trunk.getCount());
					if (trunkElementCount)
					{
						// All branches are full of the value; 
						// remove the value from each branch.
						{
							ChildCount branchOffset = branchCount;
							do
							{
								this->branches[--branchOffset].values.remove(value);
							} while (branchOffset);
						}

						// Split the trunk by lopping off the end; 
						// we are going to map the resulting branch to the value.
						// Relative refers to the branch beforehand, and the branch parent after.
						if (this->split(trunkElementCount - 1, relative))
						{
							// For the correct child branch, set the value to be fully inserted.
							this->branches[relative.getChildOffset()].values.insert(value);
							goto False;
						}
					}
				}
			}

		True:

			// The value is mapped to this entire node.
			this->removeFromDescendants(value, initial);
			this->values.insert(value);
			return true;

		False:

			// The value is not mapped to this entire node.
			this->descendantValues.insert(value);
			return false;
		}

		// Removes the mapping of the value from the descendant node for the given tree.
		// Returns true if the value is no longer mapped to this node.
		bool removeDescendant(
			Relative relative /* The descendant ancestor that describes this node. */,
			Value value)
		{
			// If it is already mapped to this node, we need to
			// remove it and insert siblings all the way down.
			if (this->values.find(value))
			{
				// Remove the value from this node.
				this->values.remove(value);

				// Insert the siblings all the way down.
				for (Node * node = this; ; )
				{
					if (node->branches.getIsEmpty())
					{
						// Push the trunk elements to the vector,
						// then append them all to the trunk at once
						// for performance.
						for (std::vector< ChildOffset > trunkElements; ; 
							trunkElements.push_back(relative.getChildOffset()))
						{
							// Get the branch count before stepping to the child.
							ChildCount const branchCount(relative.getChildCount());
							assert(branchCount);

							// Try to step to the child.
							if (!relative.stepToChild())
							{
								// The tree refers to this node.
								return true;
							}

							if (1 < branchCount)
							{
								// Append trunk elements to trunk.
								node->trunk.append(trunkElements.begin(), trunkElements.end());

								// Create branches.
								node->branches.reset(branchCount);

								break;
							}
						}
					} else
					{
						assert(this->trunk.getCount() <= boost::integer_traits< Level >::const_max);
						Level const trunkElementCount(this->trunk.getCount());
						for (Level trunkElementOffset(0); ; ++trunkElementOffset)
						{
							// Try to step to the child.
							if (!relative.stepToChild())
							{
								// The tree refers to this node.
								return true;
							}

							// If there's no more trunk, break.
							if (trunkElementCount == trunkElementOffset)
							{
								break;
							}

							// Split the trunk.  If successful, break.
							Relative temporaryRelative(relative);
							if (node->split(trunkElementOffset, temporaryRelative))
							{
								relative = temporaryRelative;
								break;
							}
						}
					}
					assert(!node->branches.getIsEmpty());

					// Get the child offset.
					ChildOffset const childOffset(relative.getChildOffset());

					// Get the branch count of the node.
					ChildCount branchCount;
					{
						Relative parent(relative);
						parent.stepToParent();
						branchCount = parent.getChildCount();
					}
					assert(1 < branchCount);

					// Fill the siblings.
					do
					{
						ChildOffset branchOffset(--branchCount);
						if (childOffset != branchOffset)
						{
							node->branches[branchOffset].values.insert(value);
						}
					} while (branchCount);

					// The value is not mapped to this entire node, but to descendant(s).
					node->descendantValues.insert(value);

					// Set the node to the branch node.
					node = &node->branches[childOffset];
				}

				return true;
			}

			// Remove from descendants.
			if (this->descendantValues.find(value))
			{
				// Capture initial level before descent.
				Relative const initial(relative);

				// Descend the trunk.
				for (typename Trunk::Elements trunkElements(this->trunk);
					relative.stepToChild();
					trunkElements.popFront())
				{
					// If we've reached the end of the trunk...
					if (trunkElements.getIsEmpty())
					{
						// It shouldn't have been called if this is a leaf.
						assert(!this->branches.getIsEmpty());

						// Remove from the correct branch.
						Node & branch = this->branches[relative.getChildOffset()];
						if (branch.removeDescendant(relative, value))
						{
							bool canCollapseValue(true);

							// States:
							//	-	None: leaf
							//	-	ChildOffset: merge
							//	-	ChildCount: nothing
							boost::optional< ChildCount > mergeCandidate;

							Relative parent(relative);
							parent.stepToParent();
							ChildCount const branchCount(parent.getChildCount());
							assert(1 < branchCount);
							{
								ChildCount branchOffset = 0;
								do
								{
									Node const & branch(this->branches[branchOffset]);
									if (!branch.getIsEmpty())
									{
										// If it contains the value, can't collapse.
										canCollapseValue = canCollapseValue && !branch.find(value, true);

										// If no merge candidate yet, and this is a candidate
										// (has no values mapped to the entire branch), set.
										// Otherwise, set to branch count to indicate no merge.
										mergeCandidate = ((!mergeCandidate && branch.values.getIsEmpty()) ?
											branchOffset : branchCount);
									}
								} while (++branchOffset < branchCount);
							}

							// If no merge candidate, make this a leaf and return true.
							if (!mergeCandidate)
							{
								this->setIsLeaf();
								return true;
							}

							// If there is a single merge candidate, merge.
							if (*mergeCandidate < branchCount)
							{
								this->merge(*mergeCandidate);
							}

							// If value is collapsible, collapse and return true.
							if (canCollapseValue)
							{
								// The value is not in any of the branches.
								assert(!this->values.find(value)); // Was checked above.
								this->descendantValues.remove(value);
								return true;
							}
						}
						return false;
					}

					// If the steps don't match, the index is not present.
					ChildOffset const childOffset = relative.getChildOffset();
					ChildOffset const trunkElement = trunkElements.getFront();
					if (childOffset != trunkElement)
					{
						return false;
					}
				}

				// Remove the value from the entire node.
				removeFromDescendants(value, initial);

				return true;
			}

			return false;
		}

		// For each value mapped to the descendant node for the given tree
		// (or a descendant thereof, if includePartial is true):
		// if it is not in results, insert it and call callback.
		bool visitDescendant(
			Relative relative /* The descendant ancestor that describes this node. */,
			MutableSetInterface< Value > & results, 
			FunctorInterface< bool, Value > & callback,
			bool const includePartial = false) const
		{
			// Visit values mapped to the whole tree.
			if (!this->values.visit(callback, results))
			{
				return false;
			}

			// Visit descendants.
			if (!this->descendantValues.getIsEmpty())
			{
				assert(!this->branches.getIsEmpty());

				// Try to advance the tree to the next node.
				// If we can't, the tree refers to this node.
				if (relative.stepToChild())
				{
					// Descend the trunk.
					// If we run out of steps in the index, 
					// it is an ancestor of the branches; fall through to partials.
					typename Trunk::Elements trunkElements(this->trunk);
					do
					{
						// Get the child offset.
						ChildOffset const childOffset(relative.getChildOffset());

						// If the trunk has been iterated,
						// visit the correct branch and return.
						if (trunkElements.getIsEmpty())
						{
							return this->branches[childOffset].visitDescendant(
								relative, results, callback, includePartial);
						}

						// Advance to the next childOffset in the trunk.
						ChildOffset const trunkElement(trunkElements.getFront());
						trunkElements.popFront();

						// If it differs from the tree child offset,
						// the tree isn't contained; return true.
						if (childOffset != trunkElement)
						{
							return true;
						}
					} while (relative.stepToChild());
				}
				
				if (includePartial)
				{
					return this->descendantValues.visit(callback, results);
				}
			}

			return true;
		}

		// Populates the results with all values mapped
		// to the descendant node for the given tree
		// (and its descendants, if includePartial is true),
		// and returns a reference to the results.
		MutableSetInterface< Value > & findDescendant(
			Relative relative /* The descendant ancestor that describes this node. */,
			MutableSetInterface< Value > & results,
			bool const includePartial = false) const
		{
			struct : virtual FunctorInterface< bool, Value >
			{
				bool operator ()(Value value) { return true; }
			} callback;
			this->visitDescendant(relative, results, callback, includePartial);
			return results;
		}

		// Returns true if the value is mapped
		// to the descendant node for the given tree
		// (or a descendant thereof, if includePartial is true).
		bool findDescendant(
			Relative relative /* The descendant ancestor that describes this node. */,
			Value value,
			bool const includePartial = false) const
		{
			Set< Value > results;
			struct Callback : virtual FunctorInterface< bool, Value >
			{
				Value value;
				explicit Callback(Value value) : value(value) {}
				bool operator ()(Value visitee) { return (visitee != this->value); }
			} callback(value);
			return !this->visitDescendant(relative, results, callback, includePartial);
		}

		// Returns true if there is at least one value mapped
		// to the descendant node for the given tree
		// (or a descendant thereof, if includePartial is true).
		bool findDescendant(
			Relative relative /* The descendant ancestor that describes this node. */,
			bool const includePartial = false) const
		{
			Set< Value > results;
			struct : virtual FunctorInterface< bool, Value >
			{
				bool operator ()(Value visitee) { return false; }
			} callback;
			return !this->visitDescendant(relative, results, callback, includePartial);
		}
	} node;

public:

	struct Element
	{
		Tree const & tree;
		SetInterface< Value > const & values;

		explicit Element(Tree const & tree, SetInterface< Value > const & values):
		tree(tree), values(values)
		{}
	};

	class Elements :
	public virtual ForwardRangeInterface< Element >
	{
		// A stack of node children ranges.
		// The top range is the range of children of the current node.
		std::stack< boost::intrusive_ptr< typename Node::Children > > stack;

		// The tree used for iteration.
		Tree tree;
	
		// Returns false if the stack was unmodified.
		bool descend(Node const & node, Tree const & tree)
		{
			if (!node.getValues().getIsEmpty())
			{
				this->stack.push(new typename Node::Children(node, tree));

				return true;
			}

			if (!node.getDescendantValues().getIsEmpty())
			{
				boost::intrusive_ptr< typename Node::Children > children(
					new typename Node::Children(node, tree));
				assert(children);

				this->stack.push(children);

				for (; ; children->popFront())
				{
					assert(!children->getIsEmpty() &&
						"Since this node has descendant values, "
						"one of the children should have descended.");

					typename Node::Child const child(children->getFront());
					if (this->descend(child.node, child.tree))
					{
						return true;
					}
				}
			}

			return false;
		}

	public:

		// Constructs the range.
		explicit Elements(Multimap const & multimap) : stack(), tree()
		{
			this->descend(multimap.node, this->tree);
		}

	public: // ForwardRangeInterface< Tree const & >
	
		bool getIsEmpty() const
		{
			return this->stack.empty();
		}

		// Asserts that it is non-empty.
		void popFront()
		{
			assert(!this->stack.empty());

			for (typename Node::Children * children(this->stack.top().get()); ;
				children->popFront())
			{
				assert(children);
				if (children->getIsEmpty())
				{
					this->stack.pop();
					if (this->stack.empty())
					{
						break;
					}
					children = this->stack.top().get();
					assert(children);
					assert(!children->getIsEmpty());
				} else
				{
					typename Node::Child const & child(children->getFront());
					if (this->descend(child.node, child.tree))
					{
						break;
					}
				}
			}
		}

		// Asserts that it is non-empty.
		Element getFront() const
		{
			assert(!this->stack.empty());

			assert(this->stack.top());
			typename Node::Children const & children(*this->stack.top());
			return Element(children.getTree(), children.getNode().getValues());
		}
	};

	// Constructs an empty multimap.
	Multimap() : node() {}
	
	// Copies the multimap.
	Multimap(Multimap const & that) : node()
	{
		Tree tree;
		Node(that.node, tree.getRelative()).swap(this->node);
	}

	friend std::ostream & operator <<(
		std::ostream & output,
		Multimap const & multimap)
	{
		output << "(";
		{
			Tree tree;
			multimap.node.write(output, tree.getRelative());
		}
		output << ")";
		return output;
	}

	void swap(Multimap & with)
	{
		this->node.swap(with.node);
	}
	
	Multimap & operator =(Multimap that)
	{
		this->swap(that);
		return *this;
	}

	// Inserts the given value mapping for the given tree
	// (and all descendants).
	void insert(Tree const & tree, Value value)
	{
		this->node.insertDescendant(tree.getRelative(0), value);
	}

	// Removes the given value mapping from the given tree
	// (and all descendants).
	void remove(Tree const & tree, Value value)
	{
		this->node.removeDescendant(tree.getRelative(0), value);
	}
	
	// Convenience method: removes the given value from all trees.
	void remove(Value value)
	{
		this->remove(Tree(), value);
	}

	// Populates the results set with all values mapped to the tree
	// (and descendants, if includePartial is true).
	MutableSetInterface< Value > & find(
		Tree const & tree, 
		MutableSetInterface< Value > & results,
		bool const includePartial = false) const
	{
		return this->node.findDescendant(tree.getRelative(0), results, includePartial);
	}

	// Returns true if the value is mapped to the tree
	// (or a descendant, if includePartial is true).
	bool find(
		Tree const & tree,
		Value value,
		bool const includePartial = false) const
	{
		return this->node.findDescendant(tree.getRelative(0), value, includePartial);
	}

	// Returns true if the tree 
	// (or a descendant, if includePartial is true)
	// is mapped to any value.
	bool find(
		Tree const & tree,
		bool const includePartial = false) const
	{
		return this->node.findDescendant(tree.getRelative(0), includePartial);
	}
	
	// Returns true if the value is mapped to any tree.
	// Same result as find(Tree(), value, true),
	// but allows for optimization.
	bool find(Value value) const
	{
		return this->node.find(value, true);
	}

	// Convenience; inserts, into results, the trees mapped to the value.
	MutableSetInterface< Tree const & > & find(Value value,
		MutableSetInterface< Tree const & > & results) const
	{
		return this->node.find(value, results, Tree());
	}
	
	// Visits the values in the given subtree.
	// If includePartial is true,
	// visits values in any descendant subtree as well.
	bool visit(
		Tree const & tree, 
		MutableSetInterface< Value > & results,
		FunctorInterface< bool, Value > & callback,
		bool const includePartial = false) const
	{
		return this->node.visitDescendant(tree.getRelative(0), results, callback, includePartial);
	}

public: // CollectionInterface

	// Returns true if the tree is empty.
	bool getIsEmpty() const
	{
		return this->node.getIsEmpty();
	}

	// Counts the trees that have at least one value mapped to them.
	// The count is not cached.
	size_t getCount() const
	{
		size_t count(0);
		for (Elements elements(*this); !elements.getIsEmpty(); elements.popFront())
		{
			++count;
		}
		return count;
	}

public: // MutableCollectionInterface

	// Makes the tree empty.
	void setIsEmpty()
	{
		this->node.setIsEmpty();
	}
	
public: // MultimapInterface< Tree const &, Value >

public: // MutableMultimapInterface< Tree const &, Value >

};

template <>
class Pyxis::Set<
	Pyxis::Grid::Dodecahedral::Tree const & > :
public virtual Pyxis::MutableSetInterface<
	Pyxis::Grid::Dodecahedral::Tree const & >
{
	typedef Grid::Dodecahedral::Tree Tree;

	typedef Multimap< Tree const &, Empty > Multimap;

	Multimap multimap;

public:

	class Elements :
	public virtual ForwardRangeInterface< Tree const & >
	{
		static Tree const * getTree(Multimap::Elements const & elements)
		{
			return elements.getIsEmpty() ? 0 : &elements.getFront().tree;
		}

		Multimap::Elements elements;
		Tree const * tree;

	public:

		explicit Elements(Set const & set) :
		elements(set.multimap),
		tree(getTree(elements))
		{}

	public: // ForwardRangeInterface< Tree const & >
	
		bool getIsEmpty() const
		{
			return this->elements.getIsEmpty();
		}

		// Asserts that it is non-empty.
		void popFront()
		{
			this->elements.popFront();
			this->tree = getTree(this->elements);
		}

		// Asserts that it is non-empty.
		Tree const & getFront() const
		{
			assert(this->tree);
			return *tree;
		}
	};

	Set() : multimap() {}

	bool getIsFull() const
	{
		return this->multimap.find(Tree());
	}

	void setIsFull()
	{
		this->insert(Tree());
	}

	void swap(Set & that)
	{
		this->multimap.swap(that.multimap);
	}

	bool find(Tree const & tree, bool const includePartial) const
	{
		return this->multimap.find(tree, Empty(), includePartial);
	}

public: // CollectionInterface

	bool getIsEmpty() const
	{
		return this->multimap.getIsEmpty();
	}

	size_t getCount() const
	{
		return this->multimap.getCount();
	}

public: // MutableCollectionInterface

	void setIsEmpty()
	{
		this->remove(Tree());
	}

public: // SetInterface< Tree const & >

	boost::intrusive_ptr<
		ForwardRangeInterface< Tree const & > > getElements() const
	{
		return new Elements(*this);
	}

	bool find(Tree const & tree) const
	{
		return this->multimap.find(tree, Empty());
	}

public: // MutableSetInterface< Tree const & >

	using MutableSetInterface< Tree const & >::insert;

	void insert(Tree const & tree)
	{
		this->multimap.insert(tree, Empty());
	}

	using MutableSetInterface< Tree const & >::remove;

	void remove(Tree const & tree)
	{
		this->multimap.remove(tree, Empty());
	}

	void intersect(SetInterface< Tree const & > const & intersectee)
	{
		// Create intersection:
		// start with empty set, 
		// and add only those from this set that are in intersectee.
		Set intersection;
		for (Elements elements(*this); !elements.getIsEmpty(); elements.popFront())
		{
			Tree const & tree = elements.getFront();
			if (intersectee.find(tree)) { intersection.insert(tree); }
		}

		// Swap intersection with this.
		this->swap(intersection);
	}
};

template <>
class Pyxis::CompactSet<
	Pyxis::Grid::Dodecahedral::Tree const & > :
public Pyxis::Set<
	Grid::Dodecahedral::Tree const & >
{};

// A cell defined by an tree of level 5 or greater.
// Fulfills the Region concept.
template <>
class Pyxis::Grid::Cell< Pyxis::Grid::Dodecahedral::Tree > :
public virtual Pyxis::Grid::Dodecahedral::MutableCellInterface
{
	typedef Dodecahedral::Tree Tree;
	typedef Dodecahedral::Resolution Resolution;
	typedef Dodecahedral::ResolutionInteger ResolutionInteger;
	typedef Dodecahedral::Direction Direction;

	Tree & tree;

public:

	// Requires tree to outlive this object.
	explicit Cell(Tree & tree) : tree(tree)
	{
		if (!Tree::getResolution(tree.getLevel()))
		{
			throw std::invalid_argument(
				"The level does not correspond to a resolution.");
		}
	}

	Tree const & getTree() const
	{
		return this->tree;
	}

public: // TileInterface

	Resolution getResolution() const
	{
		Resolution resolution;
		assert(Tree::getResolution(this->tree.getLevel()));
		return *Tree::getResolution(resolution, this->tree.getLevel());
	}
	
public: // CellInterface

	// Converts overlap/neighbour/underlap offset to direction.
	Direction getOverlapDirection(
		OverlapOffset const overlapOffset) const;
	Direction getNeighbourDirection(
		NeighbourOffset const neighbourOffset) const;
	Direction getUnderlapDirection(
		UnderlapOffset const underlapOffset) const;

	// Converts direction to overlap/neighbour/underlap offset.
	OverlapOffset * getOverlapOffset(
		OverlapOffset & result, Direction const direction) const;
	NeighbourOffset * getNeighbourOffset(
		NeighbourOffset & result, Direction const direction) const;
	UnderlapOffset * getUnderlapOffset(
		UnderlapOffset & result, Direction const direction) const;

	bool getIsContained() const
	{
		return this->tree.getIsZeroChild();
	}

	NeighbourCount getNeighbourCount() const
	{
		return this->tree.getIsComplete() ? 6 : 5;
	}

	UnderlapCount getUnderlapCount() const
	{
		return this->getIsContained() ? 1 : 3;
	}

	OverlapCount getOverlapCount() const
	{
		return 1 + this->getNeighbourCount();
	}

public: // MutableCellInterface

	NeighbourOffset stepToNeighbour(NeighbourOffset const neighbourOffset)
	{
		if (this->getNeighbourCount() <= neighbourOffset)
		{
			throw std::invalid_argument("The neighbour offset is invalid.");
		}

		assert(0); throw std::exception(); // TODO
	}

	UnderlapOffset stepToOverlap(OverlapOffset const overlapOffset)
	{
		if (this->getOverlapCount() <= overlapOffset)
		{
			throw std::invalid_argument("The overlap offset is invalid.");
		}
		if (this->getResolution() == boost::integer_traits< ResolutionInteger >::const_max)
		{
			throw std::overflow_error("Resolution overflow.");
		}
		
		assert(0); throw std::exception(); // TODO
	}

	OverlapOffset stepToUnderlap(UnderlapOffset const underlapOffset)
	{
		if (this->getUnderlapCount() <= underlapOffset)
		{
			throw std::invalid_argument("The underlap offset is invalid.");
		}
		if (!this->getResolution())
		{
			throw std::underflow_error("Resolution underflow.");
		}

		assert(0); throw std::exception(); // TODO
	}

	bool stepToContained()
	{
		return this->tree.stepToChild(0);
	}

	bool stepToContainer()
	{
		// If this will drop below resolution 0, return false.
		if (this->getResolution() <= Resolution(!this->getIsContained()))
		{
			return false;
		}
		this->tree.stepToParent();
		assert(Tree::getResolution(this->tree.getLevel()));
		return true;
	}

public: // Region concept

	bool getIntersection(
		Geometry< Tree > const & intersectee,
		Geometry< Tree > * const complete = 0,
		Geometry< Tree > * const partial = 0,
		Geometry< Tree > * const negative = 0) const;
};

// Fulfills the Region concept.
template <>
class Pyxis::Grid::Rosette< Pyxis::Grid::Dodecahedral::Tree > :
public virtual Pyxis::Grid::Dodecahedral::TileInterface,
public virtual Pyxis::SetInterface<
	Pyxis::Grid::Cell< Pyxis::Grid::Dodecahedral::Tree > const & >
{
	typedef Dodecahedral::Tree Tree;
	typedef Dodecahedral::Resolution Resolution;
	typedef Cell< Tree > Cell;

	Tree const & tree;

	Resolution resolution;

public:

	// TODO: Implement and test.
	// A forward range of cells.
	class Cells :
	public virtual ForwardRangeInterface< Cell const & >
	{
	public:

		explicit Cells(Rosette const & rosette)
		{
			assert(0); throw std::exception(); // TODO
		}

	public: // ForwardRangeInterface< Cell const & >

		bool getIsEmpty() const
		{
			assert(0); throw std::exception(); // TODO
		}

		// Asserts that it is non-empty.
		void popFront()
		{
			assert(0); throw std::exception(); // TODO
		}

		// Asserts that it is non-empty.
		Cell const & getFront() const
		{
			assert(0); throw std::exception(); // TODO
		}
	};

	// Constructs a rosette with the cells at the resolution in the tree.
	// If the resolution level is lower than the tree trunk level,
	// the rosette contains a single cell.
	// Requires tree to outlive this object.
	explicit Rosette(Tree const & tree, Resolution const resolution) :
	tree(tree), resolution(resolution)
	{}

	Tree const & getTree() const
	{
		return this->tree;
	}

public: // TileInterface

	Resolution getResolution() const
	{
		return this->resolution;
	}

public: // SetInterface< Cell const & >

	boost::intrusive_ptr<
		ForwardRangeInterface< Cell const & > > getElements() const
	{
		return new Cells(*this);
	}

	bool getIsEmpty() const
	{
		return false;
	}

	size_t getCount() const
	{
		// Returns the number of leaves when truncating the tree
		// at the given resolution.
		// If the level is <= that of the tree, returns 1.
		return this->tree.getLeafCount(Tree::getLevel(this->resolution));
	}

	bool find(Cell const & cell) const
	{
		return (
			cell.getResolution() == this->resolution &&
			this->tree.find(cell.getTree(), true));
	}

public: // Region concept

	bool getIntersection(
		Geometry< Tree > const & intersectee,
		Geometry< Tree > * const complete = 0,
		Geometry< Tree > * const partial = 0,
		Geometry< Tree > * const negative = 0) const
	{
		assert(0); throw std::exception(); // TODO
	}
};

template < typename Value >
class Pyxis::Multimap<
	Pyxis::Grid::Rosette< Pyxis::Grid::Dodecahedral::Tree > const &,
	Value > :
public virtual Pyxis::MutableMultimapInterface<
	Pyxis::Grid::Rosette< Pyxis::Grid::Dodecahedral::Tree > const &,
	Value >
{
	typedef Grid::Dodecahedral::Tree Tree;
	typedef Grid::Dodecahedral::Resolution Resolution;
	typedef Grid::Rosette< Tree > Rosette;

	typedef Multimap< Tree const &, Value > TreeMultimap;
	typedef std::map<
		Resolution,
		boost::intrusive_ptr< TreeMultimap > > Map;

	Map map;

	TreeMultimap & get(Resolution const resolution)
	{
		boost::intrusive_ptr< TreeMultimap > & multimap = this->map[resolution];
		if (!multimap) { multimap.reset(new TreeMultimap()); }
		return *multimap;
	}
	
	TreeMultimap * find(Resolution const resolution) const
	{
		typename Map::const_iterator const iterator = this->map.find(resolution);
		if (iterator == this->map.end()) { return 0; }
		assert(iterator->second);
		return iterator->second.get();
	}

public:

	Multimap() : map() {}

	// Performs a deep-copy.
	Multimap(Multimap const & that) :
	map()
	{
		typename Map::const_iterator const end(that.map.end());
		for (typename Map::const_iterator iterator(that.map.begin());
			iterator != end; ++iterator)
		{
			assert(iterator->second);
			this->map[iterator->first].reset(new TreeMultimap(*iterator->second));
		}
	}

	void swap(Multimap & that)
	{
		this->map.swap(that.map);
	}

	Multimap & operator =(Multimap that)
	{
		this->swap(that);
		return *this;
	}

	// Inserts a rosette->value mapping.
	void insert(Rosette const & rosette, Value value)
	{
		this->get(rosette.getResolution()).insert(rosette.getTree(), value);
	}

	// Removes all mappings to the given value.
	void remove(Value value)
	{
		// Iterate through each tree multimap and remove value.
		typename Map::iterator const end(this->map.end());
		for (typename Map::iterator iterator(this->map.begin());
			iterator != end; )
		{
			typename Map::iterator element(iterator++);
			assert(element->second);
			TreeMultimap & treeMultimap(*element->second);
			assert(!treeMultimap.getIsEmpty());
			treeMultimap.remove(value);
			if (treeMultimap.getIsEmpty())
			{
				// std::map::erase only invalidates an iterator referencing the erased element.
				this->map.erase(element);
			}
		}
	}

	// Finds all values mapped to the rosette.
	// If 'includePartial' is true, finds values mapped to rosettes within the rosette.
	MutableSetInterface< Value > & find(
		Rosette const & rosette, 
		MutableSetInterface< Value > & results,
		bool const includePartial = false) const
	{
		TreeMultimap const * const multimap(this->find(rosette.getResolution()));
		if (multimap)
		{
			return multimap->find(rosette.getTree(), results, includePartial);
		}
		return results;
	}

	bool find(
		Rosette const & rosette,
		Value value,
		bool const includePartial = false) const
	{
		TreeMultimap const * const multimap(this->find(rosette.getResolution()));
		if (multimap)
		{
			return multimap->find(rosette.getTree(), value, includePartial);
		}
		return false;
	}

	// Visits values mapped to the rosette.
	// Allows immediate response for each value.
	// If 'includePartial' is true, visits values mapped to rosettes within the rosette.
	bool visit(
		Rosette const & rosette, 
		MutableSetInterface< Value > & results,
		FunctorInterface< bool, Value > & callback,
		bool const includePartial = false) const
	{
		TreeMultimap const * const multimap(this->find(rosette.getResolution()));
		return (!multimap ||
			multimap->visit(rosette.getTree(), results, callback, includePartial));
	}

public: // CollectionInterface

	bool getIsEmpty() const
	{
		return this->map.empty();
	}

	size_t getCount() const
	{
		size_t count(0);
		typename Map::const_iterator const end(this->map.end());
		for (typename Map::const_iterator iterator(this->map.begin());
			iterator != end; ++iterator)
		{
			assert(iterator->second);
			count += iterator->second->getCount();
		}
		return count;
	}

public: // MutableCollectionInterface

	void setIsEmpty()
	{
		this->map.clear();
	}

public: // MultimapInterface< Rosette const &, Value >

public: // MutableMultimapInterface< Rosette const &, Value >

};

template <>
class Pyxis::Set<
	Pyxis::Grid::Rosette< Pyxis::Grid::Dodecahedral::Tree > const & > :
public virtual Pyxis::MutableSetInterface<
	Pyxis::Grid::Rosette< Pyxis::Grid::Dodecahedral::Tree > const & >
{
	typedef Grid::Dodecahedral::Tree Tree;
	typedef Grid::Dodecahedral::Resolution Resolution;
	typedef Grid::Rosette< Tree > Rosette;

	typedef Set< Tree const & > TreeSet;
	typedef std::map<
		Resolution,
		boost::intrusive_ptr< TreeSet > > Map;

	Map map;

protected:

	TreeSet & get(Resolution const resolution)
	{
		boost::intrusive_ptr< TreeSet > & set = this->map[resolution];
		if (!set) { set.reset(new TreeSet()); }
		return *set;
	}
	
	TreeSet * find(Resolution const resolution) const
	{
		Map::const_iterator const iterator = this->map.find(resolution);
		if (iterator == this->map.end()) { return 0; }
		assert(iterator->second);
		return iterator->second.get();
	}

public:

	// TODO: Implement and test.
	class Elements :
	public virtual ForwardRangeInterface< Rosette const & >
	{
	public:
	
		explicit Elements(Set const & set)
		{
			assert(0); throw std::exception(); // TODO
		}

	public: // ForwardRangeInterface< Rosette const & >
	
		bool getIsEmpty() const
		{
			assert(0); throw std::exception(); // TODO
		}

		// Asserts that it is non-empty.
		void popFront()
		{
			assert(0); throw std::exception(); // TODO
		}

		// Asserts that it is non-empty.
		Rosette const & getFront() const
		{
			assert(0); throw std::exception(); // TODO
		}
	};
	
	Set() : map() {}

	// Performs a deep-copy.
	Set(Set const & that)
	{
		Map::const_iterator const end(that.map.end());
		for (Map::const_iterator iterator(that.map.begin());
			iterator != end; ++iterator)
		{
			assert(iterator->second);
			this->map[iterator->first].reset(new TreeSet(*iterator->second));
		}
	}

	void swap(Set & that)
	{
		this->map.swap(that.map);
	}

	Set & operator =(Set that)
	{
		this->swap(that);
		return *this;
	}

public: // CollectionInterface

	bool getIsEmpty() const
	{
		return this->map.empty();
	}

	// Iterates and counts elements.
	size_t getCount() const
	{
		size_t count(0);
		Map::const_iterator const end(this->map.end());
		for (Map::const_iterator iterator(this->map.begin());
			iterator != end; ++iterator)
		{
			assert(iterator->second);
			count += iterator->second->getCount();
		}
		return count;
	}

public: // MutableCollectionInterface

	void setIsEmpty()
	{
		this->map.clear();
	}

public: // SetInterface< Rosette const & >

	boost::intrusive_ptr<
		ForwardRangeInterface< Rosette const & > > getElements() const
	{
		return new Elements(*this);
	}

	bool find(Rosette const & rosette) const
	{
		TreeSet const * const set(this->find(rosette.getResolution()));
		return set && set->find(rosette.getTree());
	}

public: // MutableSetInterface< Rosette const & >

	using MutableSetInterface< Rosette const & >::insert;

	void insert(Rosette const & rosette)
	{
		this->get(rosette.getResolution()).insert(rosette.getTree());
	}

	using MutableSetInterface< Rosette const & >::remove;

	void remove(Rosette const & rosette)
	{
		TreeSet * const set(this->find(rosette.getResolution()));
		if (set)
		{
			set->remove(rosette.getTree());
		}
	}

	void intersect(SetInterface< Rosette const & > const & intersectee)
	{
		assert(0); throw std::exception(); // TODO
	}
};

template <>
class Pyxis::CompactSet<
	Pyxis::Grid::Rosette< Pyxis::Grid::Dodecahedral::Tree > const & > :
public Pyxis::Set<
	Pyxis::Grid::Rosette< Pyxis::Grid::Dodecahedral::Tree > const & >
{};

// Fulfills the Region concept.
template <>
class Pyxis::Grid::Geometry< Pyxis::Grid::Dodecahedral::Tree > :
public virtual Pyxis::Grid::Dodecahedral::TileInterface,
public virtual Pyxis::MutableSetInterface<
	Pyxis::Grid::Rosette< Pyxis::Grid::Dodecahedral::Tree > const & >
{
	typedef Dodecahedral::Tree Tree;
	typedef Tree::Level Level;
	typedef Dodecahedral::Resolution Resolution;
	typedef Rosette< Tree > Rosette;

	Set< Tree const & > trees;

	Resolution resolution;

public:

	// TODO: Test this
	class Rosettes :
	public ForwardRangeInterface< Rosette const & >
	{
		Set< Tree const & >::Elements trees;
		Resolution const resolution;
		boost::optional< Rosette > rosette;

	public: 

		explicit Rosettes(Geometry const & geometry) :
		trees(geometry.getTrees()),
		resolution(geometry.getResolution()),
		rosette()
		{
			if (!this->trees.getIsEmpty())
			{
				this->rosette = boost::in_place(
					this->trees.getFront(), this->resolution);
			}
		}
		
	public: // ForwardRangeInterface< Rosette const & >	
		
		bool getIsEmpty() const
		{
			return this->trees.getIsEmpty();
		}

		// Asserts that it is non-empty.
		void popFront()
		{
			this->trees.popFront();

			if (this->trees.getIsEmpty())
			{
				this->rosette = boost::none;
			} else
			{
				this->rosette = boost::in_place(
					this->trees.getFront(), this->resolution);
			}
		}

		// Asserts that it is non-empty.
		Rosette const & getFront() const
		{
			assert(this->rosette);
			return *this->rosette;
		}
	};

	explicit Geometry(Resolution const resolution) :
	trees(), resolution(resolution)
	{}

	Set< Tree const & > const & getTrees() const
	{
		return this->trees;
	}

	// TODO: Add geometric operations (boundary, etc.).

	bool getIsFull() const
	{
		return this->trees.getIsFull();
	}

	void setIsFull()
	{
		this->trees.setIsFull();
	}

	bool find(Rosette const & rosette, bool const includePartial) const
	{
		return (
			rosette.getResolution() == this->resolution &&
			this->trees.find(rosette.getTree(), includePartial));
	}

	// Inserts the tree rosette
	// that is at the geometry resolution.
	void insert(Tree const & tree)
	{
		Level const level(Tree::getLevel(this->resolution));
		this->trees.insert((level < tree.getLevel()) ?
			tree.getRelative(level) :
			tree);
	}

	// Removes the tree rosette.
	void remove(Tree const & tree)
	{
		this->trees.remove(tree);
	}

public: // TileInterface

	Resolution getResolution() const
	{
		return this->resolution;
	}

public: // CollectionInterface

	bool getIsEmpty() const
	{
		return this->trees.getIsEmpty();
	}

	size_t getCount() const
	{
		return this->trees.getCount();
	}

public: // MutableCollectionInterface

	void setIsEmpty()
	{
		this->trees.setIsEmpty();
	}

public: // SetInterface< Rosette const & >

	boost::intrusive_ptr<
		ForwardRangeInterface< Rosette const & > > getElements() const
	{
		return new Rosettes(*this);
	}

	bool find(Rosette const & rosette) const
	{
		return this->find(rosette, false);
	}

public: // MutableSetInterface< Rosette const & >

	using MutableSetInterface< Rosette const & >::insert;

	// Throws if the rosette is the wrong resolution.
	void insert(Rosette const & rosette)
	{
		if (rosette.getResolution() != this->resolution)
		{
			throw std::invalid_argument(
				"The rosette resolution must match the geometry resolution.");
		}
		assert(rosette.getTree().getLevel() <= Tree::getLevel(this->resolution));
		this->trees.insert(rosette.getTree());
	}

	using MutableSetInterface< Rosette const & >::remove;

	void remove(Rosette const & rosette)
	{
		if (rosette.getResolution() == this->resolution)
		{
			this->remove(rosette.getTree());
		}
	}

	void intersect(SetInterface< Rosette const & > const & intersectee)
	{
		assert(0); throw std::exception(); // TODO
	}

public: // Region concept

	// Needs to know about relationships between rosettes.
	bool getIntersection(
		Geometry const & intersectee,
		Geometry * const complete = 0,
		Geometry * const partial = 0,
		Geometry * const negative = 0) const
	{
		assert(0); throw std::exception(); // TODO
	}
};

// The tree gazetteer stores spatially-indexed feature keys.
// A feature key is a unique identifier for a feature.
// It is up to the caller to ensure integrity of the keys,
// such that if the feature region associated with a key
// is modified, the key is removed from the gazetteer (since this
// invalidates the spatial indexing).
template < typename Key >
class Pyxis::Grid::Gazetteer<
	Pyxis::Grid::Dodecahedral::Tree,
	Key > :
public virtual Pointee
{
	typedef Dodecahedral::Tree Tree;
	typedef Rosette< Tree > Rosette;
	typedef Geometry< Tree > Geometry;

	mutable boost::mutex mutex;

	class Cache
	{
		Multimap< Tree const &, Key > complete;
		Multimap< Tree const &, Key > negative;
		Multimap< Rosette const &, Key > partial;

	public:

		Cache() : complete(), negative(), partial() {}

		void removeComplete(Key const key)
		{
			this->complete.remove(key);
		}

		void removePartial(Key const key)
		{
			this->partial.remove(key);
		}

		void removeNegative(Key const key)
		{
			this->negative.remove(key);
		}

		void remove(Key const key)
		{
			this->removeComplete(key);
			this->removePartial(key);
			this->removeNegative(key);
		}

		void insertComplete(
			Geometry const & geometry,
			Key const key)
		{
			for (Geometry::Rosettes rosettes(geometry); 
				!rosettes.getIsEmpty(); rosettes.popFront())
			{
				this->complete.insert(rosettes.getFront().getTree(), key);
			}
		}

		void insertPartial(
			Geometry const & geometry,
			Key const key)
		{
			for (Geometry::Rosettes rosettes(geometry); 
				!rosettes.getIsEmpty(); rosettes.popFront())
			{
				this->partial.insert(rosettes.getFront(), key);
			}
		}
		
		void insertNegative(
			Geometry const & geometry,
			Key const key)
		{
			for (Geometry::Rosettes rosettes(geometry); 
				!rosettes.getIsEmpty(); rosettes.popFront())
			{
				this->negative.insert(rosettes.getFront().getTree(), key);
			}
		}

		// Determine intersection between the feature and the geometry,
		// and insert.
		// Region fulfills the Region concept.
		template < typename Region >
		bool insert(
			Geometry const & geometry,
			Key const key,
			Region const & region)
		{
			Geometry completeIntersection(geometry.getResolution());
			Geometry partialIntersection(geometry.getResolution());
			Geometry negativeIntersection(geometry.getResolution());

			// If 'getIntersection' returns a boost::logic::tribool,
			// the boolean value will be true on intersection and false otherwise.
			// http://www.boost.org/doc/html/tribool.html 
			bool const intersects(
				region.getIntersection(
					geometry,
					&completeIntersection,
					&partialIntersection,
					&negativeIntersection));
			if (intersects)
			{
				this->insertComplete(completeIntersection, key);
				this->insertPartial(partialIntersection, key);
			}
			this->insertNegative(negativeIntersection, key);
			return intersects;
		}

		// Visits partial and complete intersections of rosette.
		bool visitPositive(
			Rosette const & rosette,
			MutableSetInterface< Key > & results,
			FunctorInterface< bool, Key > & callback) const
		{
			// Visit keys for it in positive first, then visit keys for it in partial.
			// Both have "include partials" set to true to because a subrosette counts.
			return (
				this->complete.visit(rosette.getTree(), results, callback, true) &&
				this->partial.visit(rosette, results, callback, true));
		}

		// Visits partial and complete intersections of geometry.
		bool visitPositive(
			Geometry const & geometry,
			MutableSetInterface< Key > & results,
			FunctorInterface< bool, Key > & callback) const
		{
			for (Geometry::Rosettes rosettes(geometry); 
				!rosettes.getIsEmpty(); rosettes.popFront())
			{
				if (!this->visitPositive(rosettes.getFront(), results, callback))
				{
					return false;
				}
			}
			return true;
		}

		// Finds intersections of rosette.
		MutableSetInterface< Key > & findPositive(
			Rosette const & rosette,
			MutableSetInterface< Key > & results) const
		{
			// Find keys for it in positive first, then visit keys for it in partial.
			// Both have "include partials" set to true to because a subrosette counts.
			this->complete.find(rosette.getTree(), results, true);
			this->partial.find(rosette, results, true);
			return results;
		}

		// Finds intersections of geometry.
		MutableSetInterface< Key > & findPositive(
			Geometry const & geometry,
			MutableSetInterface< Key > & results) const
		{
			for (Geometry::Rosettes rosettes(geometry);
				!rosettes.getIsEmpty(); rosettes.popFront())
			{
				findPositive(rosettes.getFront(), results);
			}
			return results;
		}

		// Finds non-intersections of geometry.
		MutableSetInterface< Key > & findNegative(
			Geometry const & geometry,
			MutableSetInterface< Key > & results) const
		{
			Geometry::Rosettes rosettes(geometry);
			if (!rosettes.getIsEmpty())
			{
				// Find values mapped to entire first tile,
				// and populate results set directly.
				this->negative.find(rosettes.getFront().getTree(), results, false);

				// For each remaining tile:
				// intersect results with values mapped to entire tile.
				for (; rosettes.popFront(), !rosettes.getIsEmpty(); )
				{
					Pyxis::Set< Key > temporary;
					this->negative.find(rosettes.getFront().getTree(), temporary, false);
					results.intersect(temporary);
				}
			}
			return results;
		}

		// TODO: Test.
		boost::logic::tribool find(
			Geometry const & geometry,
			Key const key) const
		{
			boost::logic::tribool result(false);

			for (Geometry::Rosettes rosettes(geometry);
				!rosettes.getIsEmpty(); rosettes.popFront())
			{
				Rosette const & rosette(rosettes.getFront());
				Tree const & tree(rosette.getTree());
				if (this->complete.find(tree, key, true) ||
					this->partial.find(rosette, key, true))
				{
					return true;
				}
				if (!this->negative.find(tree, key, false))
				{
					result = boost::logic::indeterminate;
				}
			}

			return result;
		}
	} cache;

public:

	// A range of keys in the gazetteer whose features intersect the given geometry.
	// Once this reports that it is empty, iteration is completed
	// and no insertions to the gazetteer will be reflected here.
	class Keys :
	public virtual ForwardRangeInterface< Key >
	{
		// A thread worker that adds results to the queue.
		// This is also used as the gazetteer callback for when a feature is found.
		class Worker :
		boost::noncopyable,
		public virtual FunctorInterface< bool, Key > // For gazetteer to call.
		{
			Gazetteer const & gazetteer;
			Geometry const & geometry;
			MutableSetInterface< Key > & visited;
			boost::mutex & queueMutex;
			std::queue< Key > & queue;
			boost::condition_variable & condition; // Notifies on push and finish (one or more).
			boost::detail::atomic_count isFinished;

		public:

			// The objects referred to must be alive for the lifetime of this worker,
			// which is on a different thread.
			explicit Worker(
				Gazetteer const & gazetteer,
				Geometry const & geometry,
				MutableSetInterface< Key > & visited,
				boost::mutex & queueMutex,
				std::queue< Key > & queue,
				boost::condition_variable & condition) :
			gazetteer(gazetteer),
			geometry(geometry),
			visited(visited),
			queueMutex(queueMutex),
			queue(queue),
			condition(condition),
			isFinished(0)
			{}

			void setIsFinished()
			{
				// This may fire more than once, if multiple threads flag finish.
				++this->isFinished;
				this->condition.notify_one(); // There is only ever a maximum of one thread waiting.
			}

			bool getIsFinished() const
			{
				return !!this->isFinished;
			}

			// Called by the thread constructor.
			void operator ()()
			{
				this->gazetteer.visit(this->geometry, this->visited, *this);
				this->setIsFinished();
			}

		public: // FunctorInterface< bool, Feature const & >

			// Called by the gazetteer, serially, when a feature is found.
			bool operator ()(Key key)
			{
				if (this->isFinished)
				{
					return false; // Avoid a push if we're already finished.
				}
				{
					boost::mutex::scoped_lock lock(this->queueMutex);
					this->queue.push(key);
				}
				this->condition.notify_one(); // There is only ever a maximum of one thread waiting.
				return !this->isFinished;
			}
		};

		mutable boost::mutex queueMutex;
		mutable std::queue< Key > queue;

		mutable boost::condition_variable condition;
		Worker worker;
		boost::thread thread; // This lifetime of this thread is contained by that of this object.

	public:

		// The gazetteer, geometry and visited set must be alive and unmodified
		// for the lifetime of this object, or undefined behaviour can result.
		explicit Keys(
			Gazetteer const & gazetteer,
			Geometry const & geometry,
			MutableSetInterface< Key > & visited) :
		queueMutex(), queue(),
		condition(),
		worker(gazetteer, geometry, visited, queueMutex, queue, condition),
		thread(boost::ref(worker))
		{}

		~Keys()
		{
			// Set the finished flag, used by the thread.
			this->worker.setIsFinished();

			// Wait until the thread finishes.  This guarantees that
			// everything the thread references stays in scope.
			try
			{
				this->thread.join();
			} catch (boost::thread_interrupted const &)
			{
			} catch (...)
			{
				// Should not throw any other exceptions.
				assert(0);
			}
		}

	public: //	ForwardRangeInterface< Key >

		bool getIsEmpty() const
		{
			boost::mutex::scoped_lock lock(this->queueMutex);
			while (this->queue.empty())
			{
				if (this->worker.getIsFinished())
				{
					return this->queue.empty();
				}
				this->condition.wait(lock);
			}
			return false;
		}

		// Asserts non-empty.
		Key getFront() const
		{
			boost::mutex::scoped_lock lock(this->queueMutex);
			while (this->queue.empty())
			{
				assert(!this->worker.getIsFinished() && "The range is empty.");
				this->condition.wait(lock);
			}
			return this->queue.front();
		}

		// Asserts non-empty.
		void popFront()
		{
			boost::mutex::scoped_lock lock(this->queueMutex);
			while (this->queue.empty())
			{
				assert(!this->worker.getIsFinished() && "The range is empty.");
				this->condition.wait(lock);
			}
			this->queue.pop();
		}
	};

	explicit Gazetteer() : mutex(), cache() {}

	// Removes the key from the gazetteer.
	void remove(Key const key)
	{
		boost::mutex::scoped_lock lock(this->mutex);
		this->cache.remove(key);
	}

	// Inserts information about intersection with the geometry.
	// Returns true if the feature intersects the geometry.
	template < typename Region >
	bool insert(
		Geometry const & geometry,
		Key const key,
		Region const & region)
	{
		boost::mutex::scoped_lock lock(this->mutex);
		return this->cache.insert(geometry, key, region);
	}

	// Visits all keys whose regions are known to either intersect
	// or not intersect the geometry, and calls callback for each
	// positive intersection.
	// The intersections are handled first, followed by the non-intersections.
	// Returns true if all callbacks returned true.
	bool visit(
		Geometry const & geometry,
		MutableSetInterface< Key > & visited,
		FunctorInterface< bool, Key > & callback) const
	{
		boost::mutex::scoped_lock lock(this->mutex);
		if (this->cache.visitPositive(geometry, visited, callback))
		{
			this->cache.findNegative(geometry, visited);
			return true;
		}
		return false;
	}

	// Returns true if the key is known to intersect the geometry,
	// false if it is known to not intersect,
	// or boost::logic::indeterminate if positive intersection is unknown.
	boost::logic::tribool find(
		Geometry const & geometry,
		Key const key) const
	{
		boost::mutex::scoped_lock lock(this->mutex);
		return this->cache.find(geometry, key);
	}
};

// The tree testing class.
class Pyxis::Grid::Dodecahedral::Tree::Test
{
	class TreeLeavesTest
	{
		static size_t countLeaves(Tree const & tree, Level depth)
		{
			size_t leafCount = 0;
			for (Tree::Leaves leaves(tree, depth); !leaves.getIsEmpty(); leaves.popFront())
			{
				++leafCount;
			}
			return leafCount;
		}

	public:

		operator bool() const
		{
			Tree tree;
			assert(tree.getLevel() == 0);
			if (1 != countLeaves(tree, 0))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}
			if (1 != countLeaves(tree, 1))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}
			if (2 != countLeaves(tree, 2))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}
			if (2 != countLeaves(tree, 3))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}
			if (12 != countLeaves(tree, 4))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}
			if (12 != countLeaves(tree, 5))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}
			if (32 != countLeaves(tree, 6))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}

			tree.stepToChild(0);
			assert(tree.getLevel() == 1);
			if (1 != countLeaves(tree, 0))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}
			if (2 != countLeaves(tree, 1))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}
			if (2 != countLeaves(tree, 2))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}
			if (12 != countLeaves(tree, 3))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}
			if (12 != countLeaves(tree, 4))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}
			if (32 != countLeaves(tree, 5))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}

			tree.stepToSibling(1);
			assert(tree.getLevel() == 2);
			if (1 != countLeaves(tree, 0))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}
			if (1 != countLeaves(tree, 1))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}
			if (6 != countLeaves(tree, 2))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}
			if (6 != countLeaves(tree, 3))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}
			if (16 != countLeaves(tree, 4))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}

			tree.stepToSibling(0);
			tree.stepToChild(0);
			assert(tree.getLevel() == 2);
			if (1 != countLeaves(tree, 0))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}
			if (1 != countLeaves(tree, 1))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}
			if (6 != countLeaves(tree, 2))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}
			if (6 != countLeaves(tree, 3))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}
			if (16 != countLeaves(tree, 4))
			{
				return PYXIS__ASSERT___FAIL("Wrong leaf count.");
			}

			return true;
		}
	};

	class TreeMultimapTest
	{
		Direction const & treeIndex;
		
		// Returns any uncle of the tree; returns the same one each time.
		static Tree & getTreeUncle(Tree & tree, ChildOffset & childOffset)
		{
			assert(1 < tree.getLevel());
			ChildOffset const parentChildOffset(
				tree.getChildOffset(tree.getLevel() - 2));

			tree.stepToParent();
			tree.stepToParent();

			ChildCount const grandparentChildCount(tree.getChildCount());
			assert(2 < grandparentChildCount);
			for (childOffset = 0; childOffset < grandparentChildCount; ++childOffset)
			{
				if (childOffset != parentChildOffset)
				{
					break;
				}
			}
			assert(childOffset != parentChildOffset);
			tree.stepToChild(childOffset);
			return tree;
		}

		// Get a sibling of parent other than uncle; returns the same one each time.
		static Tree & getTreeAunt(Tree & tree, ChildOffset & childOffset)
		{
			assert(1 < tree.getLevel());
			ChildOffset const parentChildOffset(
				tree.getChildOffset(tree.getLevel() - 2));
		
			tree.stepToParent();
			tree.stepToParent();

			ChildCount grandparentChildCount = tree.getChildCount();
			assert(2 < grandparentChildCount);
			do
			{
				childOffset = --grandparentChildCount;
				if (childOffset != parentChildOffset)
				{
					break;
				}
			} while (grandparentChildCount);
			assert(childOffset != parentChildOffset);
			tree.stepToChild(childOffset);
			return tree;
		}

		// Returns any descendant of the tree; returns the same one each time.
		static Tree & getTreeDescendant(Tree & tree)
		{
			tree.stepToChild(0);
			tree.stepToChild(0);
			return tree;
		}

		// Returns the grandparent of the index.
		static Tree & getTreeGrandparent(Tree & tree)
		{
			assert(1 < tree.getLevel() &&
				"The tree level must be 2 or higher.");
			tree.stepToParent();
			tree.stepToParent();
			return tree;
		}

	public:

		TreeMultimapTest() : treeIndex(*"0100040") {}

		operator bool() const
		{
			char valueX('X');
			char valueA('A');
			char valueB('B');
			char valueC('C');

			Tree tree;
			tree.read(this->treeIndex);

			ChildCount const childCount = tree.getChildCount();
			assert(2 < childCount &&
				"This test will only work with a tree that has more than 2 children.");

			// Create supporting test data.
			Tree treeDescendant(tree);
			getTreeDescendant(treeDescendant);
			Tree treeGrandparent(tree);
			getTreeGrandparent(treeGrandparent);
			Tree treeUncle(tree);
			ChildOffset uncleOffset;
			getTreeUncle(treeUncle, uncleOffset);
			Tree treeAunt(tree);
			ChildOffset auntOffset;
			getTreeAunt(treeAunt, auntOffset);

			// Create empty.
			Multimap< Tree const &, char > map;
			if (!map.getIsEmpty())
			{
				return PYXIS__ASSERT___FAIL(
					"The default constructed map was not empty.");
			}

			// Insert.
			{
				// Insert a node with more than 2 children.
				{
					std::cout << "+[" << tree << "]=>" << valueX << std::endl;
					map.insert(tree, valueX);
					std::cout << map << std::endl;
				}

				// Find the values for the tree and verify.
				{
					Set< char > results;
					if (map.find(tree, results, true).getCount() != 1) return PYXIS__ASSERT___FAIL("Insertion failed.");
					if (!results.find(valueX)) return PYXIS__ASSERT___FAIL("Retrieval failed.");
				}

				// Find the trees for the value and verify.
				{
					Set< Tree const & > results;
					if (map.find(valueX, results).getCount() != 1) return PYXIS__ASSERT___FAIL("Insertion failed.");
					if (!results.find(tree)) return PYXIS__ASSERT___FAIL("Retrieval failed.");
				}

				// Negative test (not all children have the value):
				{
					// Map all the children, but two, to value A.
					for (ChildOffset childOffset = 2; childOffset < childCount; ++childOffset)
					{
						tree.stepToChild(childOffset);
						std::cout << "+[" << tree << "]=>" << valueA << std::endl;
						map.insert(tree, valueA);
						tree.stepToParent();
					}
					std::cout << map << std::endl;

					// Verify that they're there, and that the 2 aren't.
					for (ChildOffset childOffset = 0; childOffset < 2; ++childOffset)
					{
						tree.stepToChild(childOffset);
						Set< char > results;
						if (map.find(tree, results, true).getCount() != 1) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueX)) return PYXIS__ASSERT___FAIL("");
						tree.stepToParent();
					}
					for (ChildOffset childOffset = 2; childOffset < childCount; ++childOffset)
					{
						tree.stepToChild(childOffset);
						Set< char > results;
						if (map.find(tree, results, true).getCount() != 2) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueX)) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueA)) return PYXIS__ASSERT___FAIL("");
						tree.stepToParent();
					}

					// Map one of the remaining children to value B.
					tree.stepToChild(0);
					std::cout << "+[" << tree << "]=>" << valueB << std::endl;
					map.insert(tree, valueB);
					std::cout << map << std::endl;
					tree.stepToParent();

					// Verify that it's there, as well as the value A nodes.
					{
						ChildOffset childOffset = 0;
						tree.stepToChild(childOffset);
						Set< char > results;
						if (map.find(tree, results, true).getCount() != 2) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueX)) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueB)) return PYXIS__ASSERT___FAIL("");
						tree.stepToParent();
					}
					{
						ChildOffset childOffset = 1;
						tree.stepToChild(childOffset);
						Set< char > results;
						if (map.find(tree, results, true).getCount() != 1) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueX)) return PYXIS__ASSERT___FAIL("");
						tree.stepToParent();
					}
					for (ChildOffset childOffset = 2; childOffset < childCount; ++childOffset)
					{
						tree.stepToChild(childOffset);
						Set< char > results;
						if (map.find(tree, results, true).getCount() != 2) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueX)) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueA)) return PYXIS__ASSERT___FAIL("");
						tree.stepToParent();
					}

					// Map the remaining empty child to value A.
					tree.stepToChild(1);
					std::cout << "+[" << tree << "]=>" << valueA << std::endl;
					map.insert(tree, valueA);
					std::cout << map << std::endl;
					tree.stepToParent();

					// Confirm that it was inserted, and that consolidation didn't happen.
					if (map.find(tree, valueA, false)) return PYXIS__ASSERT___FAIL("");
					{
						ChildOffset childOffset = 0;
						tree.stepToChild(childOffset);
						Set< char > results;
						if (map.find(tree, results, true).getCount() != 2) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueX)) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueB)) return PYXIS__ASSERT___FAIL("");
						tree.stepToParent();
					}
					for (ChildOffset childOffset = 1; childOffset < childCount; ++childOffset)
					{
						tree.stepToChild(childOffset);
						Set< char > results;
						if (map.find(tree, results, true).getCount() != 2) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueX)) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueA)) return PYXIS__ASSERT___FAIL("");
						tree.stepToParent();
					}
				}

				// Value consolidation test.
				{
					// Map all the children, but one, to value B.  (The first element is already set.)
					for (ChildOffset childOffset = 2; childOffset < childCount; ++childOffset)
					{
						tree.stepToChild(childOffset);
						std::cout << "+[" << tree << "]=>" << valueB << std::endl;
						map.insert(tree, valueB);
						tree.stepToParent();
					}
					std::cout << map << std::endl;

					// Verify that the inserted values are there, as well as values already inserted.
					{
						tree.stepToChild(0);
						Set< char > results;
						if (map.find(tree, results, true).getCount() != 2) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueX)) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueB)) return PYXIS__ASSERT___FAIL("");
						tree.stepToParent();
					}
					{
						tree.stepToChild(1);
						Set< char > results;
						if (map.find(tree, results, true).getCount() != 2) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueX)) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueA)) return PYXIS__ASSERT___FAIL("");
						tree.stepToParent();
					}
					for (ChildOffset childOffset = 2; childOffset < childCount; ++childOffset)
					{
						tree.stepToChild(childOffset);
						Set< char > results;
						if (map.find(tree, results, true).getCount() != 3) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueX)) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueA)) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueB)) return PYXIS__ASSERT___FAIL("");
						tree.stepToParent();
					}

					// Map the remaining child to value B.
					tree.stepToChild(1);
					std::cout << "+[" << tree << "]=>" << valueB << std::endl;
					map.insert(tree, valueB);
					std::cout << map << std::endl;
					tree.stepToParent();

					// Confirm that it was inserted, and that value consolidation happened.
					if (!map.find(tree, valueB, false)) return PYXIS__ASSERT___FAIL("");
					{
						tree.stepToChild(0);
						Set< char > results;
						if (map.find(tree, results, true).getCount() != 2) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueX)) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueB)) return PYXIS__ASSERT___FAIL("");
						tree.stepToParent();
					}
					for (ChildOffset childOffset = 1; childOffset < childCount; ++childOffset)
					{
						tree.stepToChild(childOffset);
						Set< char > results;
						if (map.find(tree, results, true).getCount() != 3) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueX)) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueA)) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueB)) return PYXIS__ASSERT___FAIL("");
						tree.stepToParent();
					}
				}

				// Full consolidation test.
				{
					// Insert value A in the last node.  This should result in consolidation.
					tree.stepToChild(0);
					std::cout << "+[" << tree << "]=>" << valueA << std::endl;
					map.insert(tree, valueA);
					std::cout << map << std::endl;
					tree.stepToParent();

					// Confirm state.
					if (!map.find(tree, valueA, false)) return PYXIS__ASSERT___FAIL("");
					for (ChildOffset childOffset = 0; childOffset < childCount; ++childOffset)
					{
						tree.stepToChild(childOffset);
						Set< char > results;
						if (map.find(tree, results, true).getCount() != 3) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueX)) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueA)) return PYXIS__ASSERT___FAIL("");
						if (!results.find(valueB)) return PYXIS__ASSERT___FAIL("");
						tree.stepToParent();
					}
				}

				// Insert ancestor.
				{
					// Insert valueB.
					std::cout << "+[" << treeGrandparent << "]=>" << valueB << std::endl;
					map.insert(treeGrandparent, valueB);
					std::cout << map << std::endl;

					// Confirm.
					if (!map.find(treeGrandparent, valueB, false)) return PYXIS__ASSERT___FAIL("");
					if (map.find(treeGrandparent, valueA, false)) return PYXIS__ASSERT___FAIL("");
					Set< char > values;
					if (map.find(treeGrandparent, values, true).getCount() != 3) return PYXIS__ASSERT___FAIL("");
				}
				
				// Insert another, not in lineage, with same value.
				{
					// Insert valueA.
					std::cout << "+[" << treeUncle << "]=>" << valueA << std::endl;
					map.insert(treeUncle, valueA);
					std::cout << map << std::endl;
					
					// Confirm.
					{
						Set< char > values;
						if (map.find(treeUncle, values, true).getCount() != 2) return PYXIS__ASSERT___FAIL("");
					}
					if (!map.find(treeUncle, valueA, false)) return PYXIS__ASSERT___FAIL("");
					{
						Set< Tree const & > trees;
						if (map.find(valueA, trees).getCount() != 2) return PYXIS__ASSERT___FAIL("");
						if (!trees.find(treeUncle)) return PYXIS__ASSERT___FAIL("");
						if (!trees.find(tree)) return PYXIS__ASSERT___FAIL("");
					}
				}
			}
			
			// Find.
			{
				// By tree: implicit value, explicit leaf.
				{
					// Find tree
					if (!map.find(tree, true)) return PYXIS__ASSERT___FAIL("");
				
					// Find values at tree
					Set< char > values;
					map.find(tree, values, true);
					
					// Verify valueA, valueB, and valueX.
					if (values.getCount() != 3) return PYXIS__ASSERT___FAIL("");
				}
				
				// By tree: implicit value, explicit leaf descendant.
				{
					// Find tree descendant.
					if (!map.find(treeDescendant, true)) return PYXIS__ASSERT___FAIL("");

					// Find values at tree descendant.
					// Verify valueA, valueB, and valueX.
					Set< char > values;
					if (map.find(tree, values, true).getCount() != 3) return PYXIS__ASSERT___FAIL("");
				}
				
				// Negative by index.
				{
					// Boolean form
					if (!map.find(treeAunt, true)) return PYXIS__ASSERT___FAIL("");
					
					// Results form
					Set< char > values;
					if (map.find(treeAunt, values, true).getCount() != 1) return PYXIS__ASSERT___FAIL("");
					if (!values.find(valueB)) return PYXIS__ASSERT___FAIL("");
				}
				
				// By value.
				{
					if (!map.find(valueX)) return PYXIS__ASSERT___FAIL("");
					if (!map.find(valueA)) return PYXIS__ASSERT___FAIL("");
					if (!map.find(valueB)) return PYXIS__ASSERT___FAIL("");

					{
						Set< Tree const & > trees;
						if (map.find(valueX, trees).getCount() != 1) return PYXIS__ASSERT___FAIL("");
					}
					{
						Set< Tree const & > trees;
						if (map.find(valueA, trees).getCount() != 2) return PYXIS__ASSERT___FAIL("");
					}
					{
						Set< Tree const & > trees;
						if (map.find(valueB, trees).getCount() != 1) return PYXIS__ASSERT___FAIL("");
					}
				}
				
				// Negative by value.
				{
					if (map.find(valueC)) return PYXIS__ASSERT___FAIL("");

					Set< Tree const & > trees;
					if (!map.find(valueC, trees).getIsEmpty()) return PYXIS__ASSERT___FAIL("");
				}
			}
			
			// Copy.
			{
				Multimap< Tree const &, char > copy(map);
				Set< char > values;
				Tree root;
				{
					Set< char > copyValues;
					map.find(root, values, true);
					copy.find(root, copyValues, true);
					if (values != copyValues) return PYXIS__ASSERT___FAIL("");
				}
#if 0 // TODO: Add these when additional equality is implemented.
				for (typename Pxyis::Set< char >::Elements elements(values);
					!elements.getIsEmpty(); elements.popFront())
				{
					char value = elements.getFront();
					Set< Tree const & > trees;
					Set< Tree const & > copyTrees;
					if (map.find(value, trees, true) != copy.find(value, copyTrees, true)) return PYXIS__ASSERT___FAIL("");
				}
				if (copy != map) return PYXIS__ASSERT___FAIL("");
#endif
			}

			// Remove.
			{
				// Negative, with sibling.
				{
					// Remove valueA from aunt.
					std::cout << "-[" << treeAunt << "]=>" << valueA << std::endl;
					map.remove(treeAunt, valueA);
					std::cout << map << std::endl;

					// Verify that there is no change.
					Set< Tree const & > trees;
					if (map.find(valueA, trees).getCount() != 2) return PYXIS__ASSERT___FAIL("");
					if (!trees.find(tree)) return PYXIS__ASSERT___FAIL("");
				}

				// Value deconsolidation.
				{
					// Remove valueB from aunt.
					std::cout << "-[" << treeAunt << "]=>" << valueB << std::endl;
					map.remove(treeAunt, valueB);
					std::cout << map << std::endl;
					
					// Verify addition of valueB to its siblings, and removal from parent.
					Tree treeCopy(treeGrandparent);

					ChildCount childCount = treeCopy.getChildCount();
					assert(2 < childCount);

					if (map.find(treeCopy, valueB, false)) return PYXIS__ASSERT___FAIL("");
					Set< Tree const & > trees;
					if (map.find(valueB, trees).getCount() != size_t(childCount - 1))
					{
						return PYXIS__ASSERT___FAIL("");
					}
					do
					{
						ChildOffset const childOffset = --childCount;
						treeCopy.stepToChild(childOffset);
						if (childOffset == auntOffset)
						{
							if (map.find(treeCopy, valueB, false)) return PYXIS__ASSERT___FAIL("");
						} else
						{
							if (!map.find(treeCopy, valueB, false)) return PYXIS__ASSERT___FAIL("");
						}
						treeCopy.stepToParent();
					} while (childCount);
				}

				// Full deconsolidation.
				{
					// Remove valueA from tree descendant.
					Tree treeChild(tree);
					ChildCount childCount = tree.getChildCount();
					assert(childCount);
					ChildOffset childOffset = childCount - 1;
					treeChild.stepToChild(childOffset);
					std::cout << "-[" << treeChild << "]=>" << valueA << std::endl;
					map.remove(treeChild, valueA);
					std::cout << map << std::endl;

					// Verify removal of valueA at tree decendant, and addition at all its siblings.
					if (map.find(tree, valueA, false)) return PYXIS__ASSERT___FAIL("");
					if (map.find(treeChild, valueA, true)) return PYXIS__ASSERT___FAIL("");
					Set< Tree const & > trees;
					if (map.find(valueA, trees).getCount() != (size_t)childCount)
					{
						return PYXIS__ASSERT___FAIL("");
					}
					do
					{
						ChildOffset const offset = --childCount;
						treeChild.stepToParent();
						treeChild.stepToChild(offset);
						if (offset == childOffset)
						{
							if (map.find(treeChild, valueA, true)) return PYXIS__ASSERT___FAIL("");
						} else
						{
							if (!map.find(treeChild, valueA, false)) return PYXIS__ASSERT___FAIL("");
						}
					} while (childCount);
				}

				// Remove leaf.
				{
					// Remove valueA from a child.
					Tree treeChild(tree);
					treeChild.stepToChild(0);
					std::cout << "-[" << treeChild << "]=>" << valueA << std::endl;
					map.remove(treeChild, valueA);
					std::cout << map << std::endl;
					
					// Verify single removal.
					if (map.find(treeChild, valueA, true)) return PYXIS__ASSERT___FAIL("");
					ChildCount const childCount = tree.getChildCount();
					assert(childCount);
					for (ChildOffset childOffset = 1; childOffset < childCount - 1; ++childOffset)
					{
						treeChild.stepToParent();
						treeChild.stepToChild(childOffset);
						if (!map.find(treeChild, valueA, false)) return PYXIS__ASSERT___FAIL("");
					}
				}
				
				// Remove ancestor with no differing values below.
				{
					// Remove valueA from tree.
					std::cout << "-[" << tree << "]=>" << valueA << std::endl;
					map.remove(tree, valueA);
					std::cout << map << std::endl;
					
					// Verify that it only exists at uncle.
					{
						Set< Tree const & > trees;
						if (map.find(valueA, trees).getCount() != 1) return PYXIS__ASSERT___FAIL("");
						if (!trees.find(treeUncle)) return PYXIS__ASSERT___FAIL("");
					}
					
					// Remove valueA.
					std::cout << "-[]=>" << valueA << std::endl;
					map.remove(valueA);
					std::cout << map << std::endl;
					
					// Verify that valueA is removed entirely from tree.
					if (map.find(valueA)) return PYXIS__ASSERT___FAIL("");
					if (!map.find(valueX)) return PYXIS__ASSERT___FAIL("");
					if (!map.find(valueB)) return PYXIS__ASSERT___FAIL("");
				}

				// Remove ancestor with differing values below.
				{
					// Remove valueX from tree.
					std::cout << "-[" << tree << "]=>" << valueX << std::endl;
					map.remove(tree, valueX);
					std::cout << map << std::endl;
					
					// Verify that valueB remains at parent.
					if (map.find(valueX)) return PYXIS__ASSERT___FAIL("");
					Set< Tree const & > trees;
					if (map.find(valueB, trees).getCount() <= 1) return PYXIS__ASSERT___FAIL("");
				}

				// Remove ancestor that clears the map.
				{
					// Remove valueB from root.
					std::cout << "-[]=>" << valueB << std::endl;
					map.remove(valueB);
					std::cout << map << std::endl;

					// Verify that the tree is cleared.
					if (!map.getIsEmpty()) return PYXIS__ASSERT___FAIL("");
				}
			}

			// Merge.
			// TODO: Add automated verification
			{
				std::cout << "+[0003]=>" << valueA << std::endl;
				{
					Tree temporary;
					temporary.read(*"0003");
					map.insert(temporary, valueA);
				}
				std::cout << map << std::endl;

				std::cout << "+[000401]=>" << valueA << std::endl;
				{
					Tree temporary;
					temporary.read(*"000401");
					map.insert(temporary, valueA);
				}
				std::cout << map << std::endl;

				std::cout << "-[0003]=>" << valueA << std::endl;
				{
					Tree temporary;
					temporary.read(*"0003");
					map.remove(temporary, valueA);
				}
				std::cout << map << std::endl;
			}

			// Captured bug found in real-world test.
			/*
			({}{0}
			 [0]{0}{}
			 [1]{}{0}
			  (0)
			   [0]{0}{}
			   [1]{0}{}
			   [2]{}{0}
				(0)
				 [0]
				  [0]
				   [0]{0}{}
				   [1]{0}{}
				   [2]{0}{}
				   [3]{0}{}
				   [4]{0}{}
				   [5]{}{0}
					(0)
					 [0]{}{}
					 [1]{0}{}
					 [2]{0}{}
					 [3]{0}{}
					 [4]{0}{}
					 [5]{0}{}
					 [6]{0}{}
			   [3]{}{}
			   [4]{}{}
			   [5]{}{})
			   
			+[102001]{0}

			*/
			// TODO: Add automated verification
			std::cout << "CLEAR" << std::endl;
			map.setIsEmpty();
			std::cout << map << std::endl;
			{
				// Set it up.
				{
					Direction const & index(*"00");
					std::cout << "+[" << &index << "]=>" << valueA << std::endl;
					Tree temporary;
					temporary.read(index);
					map.insert(temporary, valueA);
					std::cout << map << std::endl;
				}
				{
					Direction const & index(*"0100"); 
					std::cout << "+[" << index << "]=>" << valueA << std::endl;
					Tree temporary;
					temporary.read(index);
					map.insert(temporary, valueA);
					std::cout << map << std::endl;
				}
				{
					Direction const & index(*"0101"); 
					std::cout << "+[" << index << "]=>" << valueA << std::endl;
					Tree temporary;
					temporary.read(index);
					map.insert(temporary, valueA);
					std::cout << map << std::endl;
				}
				{
					Direction const & index(*"0102000"); 
					std::cout << "+[" << index << "]=>" << valueA << std::endl;
					Tree temporary;
					temporary.read(index);
					map.insert(temporary, valueA);
					std::cout << map << std::endl;
				}
				{
					Direction const & index(*"0102000500"); 
					std::cout << "-[" << index << "]=>" << valueA << std::endl;
					Tree temporary;
					temporary.read(index);
					map.remove(temporary, valueA);
					std::cout << map << std::endl;
				}

				// Insert the problem.
				{
					Direction const & index(*"0102001"); 
					std::cout << "+[" << index << "]=>" << valueA << std::endl;
					Tree temporary;
					temporary.read(index);
					map.insert(temporary, valueA);
					std::cout << map << std::endl;
				}
			}

			return true;
		}
	};

	class TreeSetTest
	{
	public:

		operator bool()
		{
			// Captured from bug.
			{
				Set< Tree const & > set;
				set.setIsFull();
				
				Tree tree;
				tree.read(*"00000000000");

				set.remove(tree);
			}

			// TODO: Add more tests.

			return true;
		}
	};

	static bool verify(Tree const & tree)
	{
		switch (tree.getLevel())
		{
		case 0:
			if (tree.getParentCount())
			{
				return PYXIS__ASSERT___FAIL(
					"The level 0 parent count is incorrect.");
			}
			if (tree.getChildCount() != 2)
			{
				return PYXIS__ASSERT___FAIL(
					"The level 0 child count is incorrect.");
			}
			if (tree.getSiblingCount() != 0)
			{
				return PYXIS__ASSERT___FAIL(
					"The level 0 sibling count is incorrect.");
			}
			return true;
		case 1:
			if (tree.getChildCount() != 1)
			{
				return PYXIS__ASSERT___FAIL(
					"The level 1 child count is incorrect.");
			}
			if (tree.getSiblingCount() != 2)
			{
				return PYXIS__ASSERT___FAIL(
					"The level 1 sibling count is incorrect.");
			}
			break;
		case 2:
			if (tree.getChildCount() != 6)
			{
				return PYXIS__ASSERT___FAIL(
					"The level 2 child count is incorrect.");
			}
			break;
		case 3:
			if (tree.getChildCount() != 1)
			{
				return PYXIS__ASSERT___FAIL(
					"The level 3 child count is incorrect.");
			}
			break;
		case 4:
			if (tree.getIsZeroChild())
			{
				if (tree.getChildCount() != 6)
				{
					return PYXIS__ASSERT___FAIL(
						"The level 4 contained child count is incorrect.");
				}
			} else
			{
				if (tree.getChildCount() != 2)
				{
					return PYXIS__ASSERT___FAIL(
						"The level 4 uncontained child count is incorrect.");
				}
			}
			break;
		case 5:
			if (tree.getChildCount() != 6)
			{
				return PYXIS__ASSERT___FAIL(
					"The level 5 contained child count is incorrect.");
			}
			break;
		case 6:
			if (tree.getIsZeroChild())
			{
				if (tree.getChildCount() != 6)
				{
					return PYXIS__ASSERT___FAIL(
						"The level 6 contained child count is incorrect.");
				}
			} else
			{
				if (tree.getChildCount() != 7)
				{
					return PYXIS__ASSERT___FAIL(
						"The level 6 uncontained child count is incorrect.");
				}
			}
			break;
		default:
			if (tree.getIsComplete())
			{
				if (tree.getChildCount() != 7)
				{
					return PYXIS__ASSERT___FAIL(
						"The complete child count is incorrect.");
				}
			} else
			{
				if (tree.getChildCount() != 6)
				{
					return PYXIS__ASSERT___FAIL(
						"The incomplete child count is incorrect.");
				}
			}
			break;
		}

		if (tree.getParentCount() != 1)
		{
			return PYXIS__ASSERT___FAIL(
				"The parent count is incorrect.");
		}
		return true;
	}

	static bool test(Tree & tree, Level const maximumLevel)
	{
		if (!verify(tree)) return false;
		
		Level const level(tree.getLevel());
		if (level < maximumLevel)
		{
			ChildOffset siblingOffset;
			if (tree.getSiblingOffset(siblingOffset))
			{
				for (ChildCount siblingCount(tree.getSiblingCount()); siblingCount; )
				{
					ChildOffset newSiblingOffset = --siblingCount;
					tree.stepToSibling(newSiblingOffset);
					{
						ChildOffset verificationSiblingOffset;
						if (!tree.getSiblingOffset(verificationSiblingOffset) || 
							verificationSiblingOffset != newSiblingOffset)
						{
							return PYXIS__ASSERT___FAIL("");
						}
					}
					if (!verify(tree))
					{
						return PYXIS__ASSERT___FAIL("");
					}
					
					// Return.
					tree.stepToSibling(siblingOffset);
					{
						ChildOffset verificationSiblingOffset;
						if (!tree.getSiblingOffset(verificationSiblingOffset) || 
							verificationSiblingOffset != siblingOffset)
						{
							return PYXIS__ASSERT___FAIL("");
						}
					}
					if (tree.getLevel() != level)
					{
						return PYXIS__ASSERT___FAIL("");
					}
				}
			} else
			{
				if (level)
				{
					return PYXIS__ASSERT___FAIL("");
				}
			}
			for (ChildCount childCount(tree.getChildCount()); childCount; )
			{
				ChildOffset childOffset = --childCount;
				tree.stepToChild(childOffset);
				if (!test(tree, maximumLevel))
				{
					return PYXIS__ASSERT___FAIL("");
				}
				
				// Return.
				ChildOffset oldChildOffset;
				tree.stepToParent(oldChildOffset);
				if (oldChildOffset != childOffset)
				{
					return PYXIS__ASSERT___FAIL("");
				}
				if (tree.getLevel() != level)
				{
					return PYXIS__ASSERT___FAIL("");
				}
			}
		}
		return true;
	}

	static bool verifyLevel0(Tree const & tree)
	{
		if (tree.getLevel())
		{
			return PYXIS__ASSERT___FAIL(
				"The level is incorrect.");
		}
		return verify(tree);
	}
	
	// Restores tree to original value.
	static bool testLevel0(Tree & tree)
	{
		if (!verifyLevel0(tree))
		{
			return PYXIS__ASSERT___FAIL(
				"The level 0 verification failed.");
		}
		if (tree.stepToParent())
		{
			return PYXIS__ASSERT___FAIL(
				"Stepped level 0 tree to parent; should fail.");
		}
		return true;
	}
	
	static bool verifyLevel1(Tree const & tree)
	{
		if (tree.getLevel() != 1)
		{
			return PYXIS__ASSERT___FAIL(
				"The level is incorrect.");
		}
		return verify(tree);
	}
	
	// Restores tree to original value.
	static bool testLevel1(Tree & tree)
	{
		ChildOffset siblingOffset;
		if (!tree.getSiblingOffset(siblingOffset))
		{
			return PYXIS__ASSERT___FAIL(
				"Get sibling offset for level 1 tree failed.");
		}
		if (!verifyLevel1(tree))
		{
			return PYXIS__ASSERT___FAIL(
				"The level 1 verification failed.");
		}
		{
			ChildCount siblingCount(tree.getSiblingCount());
			assert(siblingCount);
			try
			{
				tree.stepToSibling(siblingCount);
				return PYXIS__ASSERT___FAIL(
					"An exception was not thrown on invalid sibling.");
			} catch (...)
			{}
			do
			{
				tree.stepToSibling(--siblingCount);
				if (!verify(tree))
				{
					return PYXIS__ASSERT___FAIL(
						"The level 1 tests failed.");
				}
			} while (siblingCount);
		}
		tree.stepToSibling(siblingOffset);
		if (!verifyLevel1(tree))
		{
			return PYXIS__ASSERT___FAIL(
				"The level 1 tests failed.");
		}
		return true;
	}

	// Because this is used as a template argument,
	// it must be declared outside of a function.
	struct TestRegion
	{
		bool getIntersection(
			Geometry< Tree > const & intersectee,
			Geometry< Tree > * const complete = 0,
			Geometry< Tree > * const partial = 0,
			Geometry< Tree > * const negative = 0) const
		{
			if (complete)
			{
				complete->insert(intersectee);
			}
			return true;
		}
	};

	// TODO: Fill these out into proper unit tests.
	bool testGazetteer()
	{
		enum { testKey = 42 };

		Gazetteer< Tree, size_t > gazetteer;

		Resolution resolution(5);
		Geometry< Tree > geometry(resolution);
		geometry.insert(Tree());

		// Test insert.
		{
			TestRegion testRegion;
			if (!gazetteer.insert(geometry, testKey, testRegion))
			{
				return PYXIS__ASSERT___FAIL("");
			}
		}

		// Test visit.
		{
			struct Predicate : virtual FunctorInterface< bool, size_t >
			{
				bool operator ()(size_t key)
				{
					// TODO: Do stuff.
					return true;
				}
			} callback;

			Set< size_t > visited;
			bool complete = gazetteer.visit(geometry, visited, callback);
			(void)complete;

			if (visited.getCount() != 1)
			{
				return PYXIS__ASSERT___FAIL("");
			}
		}

		// Test key range.
		{
			Set< size_t > visited;
			for (Gazetteer< Tree, size_t >::Keys keys(gazetteer, geometry, visited);
				!keys.getIsEmpty(); keys.popFront())
			{
				size_t key(keys.getFront());
				if (key != testKey)
				{
					return PYXIS__ASSERT___FAIL("");
				}
			}

			if (visited.getCount() != 1)
			{
				return PYXIS__ASSERT___FAIL("");
			}
		}

		// Test remove.
		gazetteer.remove(testKey);

		return true;
	}

public:

	operator bool()
	{
		try
		{
			Tree tree;
			if (!testLevel0(tree))
			{
				return PYXIS__ASSERT___FAIL(
					"The level 0 tests failed.");
			}
			tree.stepToChild(0);
			if (!testLevel1(tree))
			{
				return PYXIS__ASSERT___FAIL(
					"The level 1 tests failed.");
			}
			tree.stepToParent();
			if (!testLevel0(tree))
			{
				return PYXIS__ASSERT___FAIL(
					"The level 0 tests failed.");
			}

			if (!test(tree, 8))
			{
				return PYXIS__ASSERT___FAIL("");
			}
			
		} catch (...)
		{
			return PYXIS__ASSERT___FAIL(
				"An exception was thrown during the Tree test.");
		}
	
#if 0 //// From old code

		try
		{
			// Level 0 index child count.
			if (getChildCount(Index()) != 2)
			{
				return PYXIS__ASSERT___FAIL(
					"The level 0 index child count is incorrect.");
			}

			// Level 1 polar index child count.
			if (getChildCount(Index("0")) != 1)
			{
				return PYXIS__ASSERT___FAIL(
					"The level 1 polar index child count is incorrect.");
			}
			if (getChildCount(Index("1")) != 1)
			{
				return PYXIS__ASSERT___FAIL(
					"The level 1 polar index child count is incorrect.");
			}

			// Level 2 polar index child count.
			if (getChildCount(Index("10")) != 6)
			{
				return PYXIS__ASSERT___FAIL(
					"The level 2 polar index center child count is incorrect.");
			}

			// Level 3 polar index child count.
			if (getChildCount(Index("100")) != 1)
			{
				return PYXIS__ASSERT___FAIL(
					"The level 3 polar index center child count is incorrect.");
			}

			// Level 3 non-polar index child count.
			if (getChildCount(Index("105")) != 1)
			{
				return PYXIS__ASSERT___FAIL(
					"The level 3 non-polar index center child count is incorrect.");
			}

			// Level 4 polar index child count.
			if (getChildCount(Index("1000")) != 6)
			{
				return PYXIS__ASSERT___FAIL(
					"The level 4 polar index child count is incorrect.");
			}

			// Level 4 non-polar index child count.
			if (getChildCount(Index("1050")) != 2)
			{
				return PYXIS__ASSERT___FAIL(
					"The level 4 non-polar index child count is incorrect.");
			}

			// Level > 4 pentagon index child count.
			if (getChildCount(Index("10500")) != 6)
			{
				return PYXIS__ASSERT___FAIL(
					"The pentagon (level > 4) index child count is incorrect.");
			}

			// Vertex hexagon index child count.
			if (getChildCount(Index("10501")) != 1)
			{
				return PYXIS__ASSERT___FAIL(
					"The vertex hexagon index child count is incorrect.");
			}

			// Center hexagon index child count.
			if (getChildCount(Index("105010")) != 7)
			{
				return PYXIS__ASSERT___FAIL(
					"The center hexagon index child count is incorrect.");
			}

			// Test "getIsHexagon()".
			// stepToAncestor, stepToChild (both forms).
			{
				Index index;
				if (index.getIsHexagon())
				{
					return PYXIS__ASSERT___FAIL(
						"Should not be a hexagon.");
				}

				index.stepToChild(1);
				if (index.getIsHexagon())
				{
					return PYXIS__ASSERT___FAIL(
						"Should not be a hexagon.");
				}

				index.stepToChild(0);
				if (index.getIsHexagon())
				{
					return PYXIS__ASSERT___FAIL(
						"Should not be a hexagon.");
				}

				index.stepToChild(5);
				if (index.getIsHexagon())
				{
					return PYXIS__ASSERT___FAIL(
						"Should not be a hexagon.");
				}

				index.stepToChild(0);
				if (index.getIsHexagon())
				{
					return PYXIS__ASSERT___FAIL(
						"Should not be a hexagon.");
				}

				index.stepToChild(0);
				if (index.getIsHexagon())
				{
					return PYXIS__ASSERT___FAIL(
						"Should not be a hexagon.");
				}

				index.stepToChild(3); // 105003
				if (!index.getIsHexagon())
				{
					return PYXIS__ASSERT___FAIL(
						"Should be a hexagon.");
				}

				// Step up past boundary.
				index.stepToAncestor(Level(4)); // 10
				if (index.getStepCount().offset != 2)
				{
					return PYXIS__ASSERT___FAIL(
						"Step to ancestor was incorrect.");
				}
				if (index.getIsHexagon())
				{
					return PYXIS__ASSERT___FAIL(
						"Should not be a hexagon.");
				}

				index.stepToChild(0);
				if (index.getIsHexagon())
				{
					return PYXIS__ASSERT___FAIL(
						"Should not be a hexagon.");
				}

				try
				{
					index.stepToChild(5);
					return PYXIS__ASSERT___FAIL(
						"Denormalized index.");
				}
				catch (...)
				{}
				if (index.getStepCount().offset != 3)
				{
					return PYXIS__ASSERT___FAIL(
						"Exception did not restore original state.");
				}

				index.stepToChild(0);
				if (index.getIsHexagon())
				{
					return PYXIS__ASSERT___FAIL(
						"Should not be a hexagon.");
				}

				index.stepToChild(2);
				if (!index.getIsHexagon())
				{
					return PYXIS__ASSERT___FAIL(
						"Should be a hexagon.");
				}

				index.stepToParent();
				if (index.getIsHexagon())
				{
					return PYXIS__ASSERT___FAIL(
						"Should not be a hexagon.");
				}
			}
			
			// Test descendants.
			{
				// At a given resolution, iterate through all descendants.
				struct DescendantTester : FunctorInterface< bool, Index const & >
				{
					Index const & root;
					Level depth;
					explicit DescendantTester(Index const & root, Level depth) : root(root), depth(depth) {}
					bool operator ()(Index const & index)
					{
						if (!root.getIsAncestorOf(index))
						{
							return PYXIS__ASSERT___FAIL(
								"Not a descendant.");
						}
						if (index.getStepCount().offset != root.getStepCount().offset + depth.offset)
						{
							return PYXIS__ASSERT___FAIL(
								"Wrong depth.");
						}
						return true;
					}
				};
				
				// Root index.
				{
					Index index;
					for (char depth = 0; depth < 8; ++depth)
					{
						DescendantTester descendantTester(index, Level(depth));
						if (!testDescendants(index, Level(depth), descendantTester))
						{
							return PYXIS__ASSERT___FAIL(
								"Descendant test failed.");
						}
					}
				}

				// Each index at resolution 5.
				{
					Level level(5);
					Index ancestorIndex;
					size_t descendantCount = ancestorIndex.getDescendantCount(level);
					for (Index::Descendants descendants = ancestorIndex.getDescendants(level);
						!descendants.getIsEmpty(); descendants.popFront())
					{
						Index const & index = descendants.getFront();
						for (char depth = 0; depth < 3; ++depth)
						{
							DescendantTester descendantTester(index, Level(depth));
							if (!testDescendants(index, Level(depth), descendantTester))
							{
								return PYXIS__ASSERT___FAIL(
									"Descendant test failed.");
							}
						}
						--descendantCount;
					}
					if (descendantCount)
					{
						return PYXIS__ASSERT___FAIL(
							"Wrong descendant count.");
					}
				}
			}

		} catch (...)
		{
			return false;
		}

#endif	

		if (!TreeLeavesTest())
		{
			return false;
		}

		if (!TreeMultimapTest())
		{
			return false;
		}

		if (!TreeSetTest())
		{
			return false;
		}

		// Test geometry.
		{
			Tree tree;
			tree.read(*"0102000300005000001020302");

			Resolution resolution;
			assert(Tree::getResolution(tree.getLevel()));
			Tree::getResolution(resolution, tree.getLevel());

			Geometry< Tree > singleCellGeometry(resolution);
			singleCellGeometry.insert(tree);

			Geometry< Tree > fullGeometry(resolution);
			fullGeometry.insert(Tree());

			fullGeometry.remove(singleCellGeometry);
		}

		// Test gazetteer.
		if (!testGazetteer())
		{
			return false;
		}

		return true;
	}
};

#endif
