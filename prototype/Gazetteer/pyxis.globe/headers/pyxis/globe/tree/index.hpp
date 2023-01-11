#if !defined(PYXIS__GLOBE__TREE__INDEX)
#define PYXIS__GLOBE__TREE__INDEX

#include "pyxis/assert.hpp"
#include "pyxis/forward_range_interface.hpp"
#include "pyxis/functor_interface.hpp"
#include "pyxis/globe/tree/level.hpp"
#include "pyxis/globe/tree/step.hpp"
#include "pyxis/pointee.hpp"
#include <ostream>
#include <stdexcept>
#include <vector>

namespace Pyxis
{
	namespace Globe
	{
		namespace Tree
		{
			// A sequence of child offsets, starting from the root tree node,
			// that uniquely identifies a node in the tree.
			// Indexes for some nodes (whose level corresponds to a resolution)
			// are homomorphic to cells; the rest are homomorphic to "abstract cells".
			class Index;
		}
	}
}

class Pyxis::Globe::Tree::Index : public virtual Pointee
{
	static size_t getLevel0DescendantCount(Level depth)
	{
		if (0 == depth.offset) { return 1; }
		--depth.offset;
		return 2 * getLevel1DescendantCount(depth);
	}
	
	static size_t getLevel1DescendantCount(Level depth)
	{
		if (0 == depth.offset) { return 1; }
		--depth.offset;
		return getLevel2DescendantCount(depth);
	}
	
	static size_t getLevel2DescendantCount(Level depth)
	{
		if (0 == depth.offset) { return 1; }
		--depth.offset;
		return getLevel3PolarDescendantCount(depth) + (
			5 * getLevel3NonpolarDescendantCount(depth));
	}
	
	static size_t getLevel3NonpolarDescendantCount(Level depth)
	{
		if (0 == depth.offset) { return 1; }
		--depth.offset;
		return getLevel4NonpolarDescendantCount(depth);
	}
	
	static size_t getLevel3PolarDescendantCount(Level depth)
	{
		if (0 == depth.offset) { return 1; }
		--depth.offset;
		return getLevel4PolarDescendantCount(depth);
	}
	
	static size_t getLevel4NonpolarDescendantCount(Level depth)
	{
		if (0 == depth.offset) { return 1; }
		--depth.offset;
		return (getVertexHexagonDescendantCount(depth) +
			getPentagonDescendantCount(depth));
	}
	
	static size_t getLevel4PolarDescendantCount(Level depth)
	{
		return getPentagonDescendantCount(depth);
	}
	
	static size_t getPentagonDescendantCount(Level depth)
	{
		if (0 == depth.offset) { return 1; }
		--depth.offset;
		return getPentagonDescendantCount(depth) + (
			5 * getVertexHexagonDescendantCount(depth));
	}
	
	static size_t getVertexHexagonDescendantCount(Level depth)
	{
		if (0 == depth.offset) { return 1; }
		--depth.offset;
		return getCenterHexagonDescendantCount(depth);
	}
	
	static size_t getCenterHexagonDescendantCount(Level depth)
	{
		if (0 == depth.offset) { return 1; }
		--depth.offset;
		return getCenterHexagonDescendantCount(depth) + (
			6 * getVertexHexagonDescendantCount(depth));
	}

	// The sequence of child offsets that constitute the normalized index.
	std::vector< Step > steps;

	// The step count of the nearest non-hexagon ancestor index.
	// This is cached as an optimization for getIsHexagon().
	Level nonHexagonStepCount;

public:

	class Test;

	class Descendants;

	Descendants getDescendants(Level stepCount, Level depth) const;

	Descendants getDescendants(Level depth) const;

	size_t getDescendantCount(Level stepCount, Level depth) const
	{
		switch (stepCount.offset)
		{
		case 0:
			return getLevel0DescendantCount(depth);
		case 1:
			return getLevel1DescendantCount(depth);
		case 2:
			return getLevel2DescendantCount(depth);
		case 3:
			return (getIsCenter(stepCount) ?
				getLevel3PolarDescendantCount(depth) :
				getLevel3NonpolarDescendantCount(depth));
		case 4:
			return (getIsCenter(Level(stepCount.offset - 1)) ?
				getLevel4PolarDescendantCount(depth) :
				getLevel4NonpolarDescendantCount(depth));
		default:
			return (getIsHexagon(stepCount) ?
				(getIsCenter(stepCount) ?
					getCenterHexagonDescendantCount(depth) :
					getVertexHexagonDescendantCount(depth)) :
				getPentagonDescendantCount(depth));
		}
	}
	
	size_t getDescendantCount(Level depth) const
	{
		return getDescendantCount(getStepCount(), depth);
	}
	
	void swap(Index & with)
	{
		steps.swap(with.steps);
		std::swap(nonHexagonStepCount.offset, with.nonHexagonStepCount.offset);
	}

	std::vector< Step > const & getSteps() const
	{
		return steps;
	}

	Level getStepCount() const
	{
		assert(steps.size() <= (size_t const)boost::integer_traits< char >::const_max);
		return Level(steps.size());
	}

	bool getIsEmpty() const
	{
		return steps.empty();
	}

	Index & setIsEmpty()
	{
		steps.clear();
		nonHexagonStepCount.offset = 0;
		return *this;
	}

	// Returns true if the cell defined by the index prefix is a center cell.
	// Throws if level is too high.
	bool getIsCenter(Level const stepCount) const
	{
		switch (stepCount.offset)
		{
		case 0:
			return true;
		case 1:
			return false;
		default:
			return 0 == steps.at(stepCount.offset - 1);
		}
	}

	// Returns true if the cell defined by this index is a center cell.
	bool getIsCenter() const
	{
		return getIsCenter(getStepCount());
	}

	// Returns true if the cell defined by the index prefix is a hexagonal cell.
	// Throws if level is too high.
	bool getIsHexagon(Level const stepCount) const
	{
		return (nonHexagonStepCount.offset < stepCount.offset);
	}

	// Returns true if the cell defined by this index is a hexagonal cell.
	bool getIsHexagon() const
	{
		return getIsHexagon(getStepCount());
	}

	// Returns the count of child indices.
	char getChildCount(Level const stepCount) const
	{
		size_t const childCount = getDescendantCount(stepCount, Level(1));
		assert(childCount <= (size_t const)boost::integer_traits< char >::const_max);
		return childCount;
	}

	// Returns the count of child indices.
	char getChildCount() const
	{
		return getChildCount(getStepCount());
	}
	
	bool getIsAncestorOf(Index const & descendant,
		Level const descendantStepCount,
		Level const stepCount) const
	{
		if (descendantStepCount.offset < stepCount.offset) { return false; }

		std::vector< Step >::const_iterator const begin = steps.begin();
		return std::equal(begin, begin + stepCount.offset, descendant.steps.begin());
	}

	bool getIsAncestorOf(Index const & descendant,
		Level const descendantStepCount) const
	{
		return getIsAncestorOf(descendant, descendantStepCount, getStepCount());
	}

	bool getIsAncestorOf(Index const & descendant) const
	{
		return getIsAncestorOf(descendant, descendant.getStepCount(), getStepCount());
	}

	Index & stepToAncestor(Level const byStepCount)
	{
		size_t stepCount = steps.size();
		if ((size_t)byStepCount.offset < stepCount)
		{
			stepCount -= byStepCount.offset;
			steps.resize(stepCount);

			if (stepCount < (size_t)nonHexagonStepCount.offset)
			{
				assert(stepCount <= (size_t const)boost::integer_traits< char >::const_max);
				nonHexagonStepCount.offset = stepCount;
			}
		}
		else
		{
			setIsEmpty();
		}
		return *this;
	}

	// No-op if level 0.
	Index & stepToParent()
	{
		return stepToAncestor(Level(1));
	}
	
	// Steps to the center child.
	// Returns false if no movement.
	// Throws if overflow.
	Index & stepToChild()
	{
		size_t const stepCount = steps.size();
		if (stepCount == (size_t const)boost::integer_traits< char >::const_max)
		{
			throw std::overflow_error("Cannot step to child; overflow.");
		}

		// Push the child.
		// Do this first because it might throw, and we want a strong exception guarantee.
		steps.push_back(0);

		// Update highest non-hexagon step count as necessary.
		if ((size_t)nonHexagonStepCount.offset == stepCount)
		{
			++nonHexagonStepCount.offset;
		}

		return *this;
	}

	// Steps to the child.
	// If the argument is 0, steps to the center child.
	// Returns false if no movement.
	// Throws if out of memory.  Strong exception guarantee.
	Index & stepToChild(Step step)
	{
		if (0 == step)
		{
			stepToChild();
		} else if (getChildCount() <= step)
		{
			throw std::out_of_range("The step is out of range.");
		} else
		{
			// Push the child.
			// Do this first because it might throw, and we want a strong exception guarantee.
			steps.push_back(step);

			// If the index does not have a resolution, increment the non-hexagon step count.
			size_t const stepCount = steps.size();
			assert(stepCount <= (size_t const)boost::integer_traits< char >::const_max);
			if (!Level(stepCount).getHasResolution())
			{
				++nonHexagonStepCount.offset;
				assert(stepCount == (size_t)nonHexagonStepCount.offset);
			}
		}
		return *this;
	}

	// Steps as far as it can before it hits an invalid.
	// Returns true if completely successful.
	template < typename InputIterator >
	Index & stepToDescendant(InputIterator begin, InputIterator end)
	{
		assert(begin <= end);
		for (; begin != end; ++begin)
		{
			stepToChild(*begin);
		}
		return *this;
	}

	Index & stepToDescendant(char const * string)
	{
		if (string)
		{
			for (; *string; ++string)
			{
				if (*string < '0')
				{
					throw std::invalid_argument(
						"The string contained an invalid child step, and the operation could not be completed.");
				}
				stepToChild(*string - '0');
			}
		}
		return *this;
	}

	// Returns the number of underlap (lower-resolution) indices.
	char getUnderlapCount() const;

	// Steps to an underlap (lower-resolution) index and sets the step to the return-trip step.
	// Throws if step is out of bounds.
	Index & stepToUnderlap(char & step);

	// Returns the number of overlap (higher-resolution) indices.
	char getOverlapCount() const;

	// Steps to an overlap (higher-resolution) index and sets the step to the return-trip step.
	// Throws if step is out of bounds.
	Index & stepToOverlap(char & step);

	// Returns the number of adjacent indices.
	char getAdjacentCount() const;

	// Steps to an adjacent index and sets the step to the return-trip step.
	// Throws if step is out of bounds.  0 is not permitted.
	Index & stepToAdjacent(char & step);

	bool operator ==(Index const & index) const
	{
		return &index == this || index.steps == this->steps;
	}

	bool operator !=(Index const & index) const
	{
		return !(*this == index);
	}

	friend bool operator <(Index const & left, Index const & right)
	{
		// TODO: Audit this; implement with less arbitrary behaviour.
		return left.steps < right.steps;
	}

	// Constructs the root index.
	explicit Index() : steps(), nonHexagonStepCount() {}
	
	// Constructs the index by iterating the steps.
	template < typename InputIterator >
	explicit Index(InputIterator begin, InputIterator end) :
	steps(), nonHexagonStepCount()
	{
		stepToDescendant(begin, end);
	}
	
	// Constructs the index from the string.
	explicit Index(char const * const string) :
	steps(), nonHexagonStepCount()
	{
		stepToDescendant(string);
	}

	// Constructs the index by copying the first part of the index.
	explicit Index(Index const & index, Level stepCount) :
	steps(), nonHexagonStepCount()
	{
		if (index.getStepCount().offset < stepCount.offset)
		{
			throw std::invalid_argument("The step count is higher than that of the index.");
		}
		stepToDescendant(index.steps.begin(), index.steps.begin() + stepCount.offset);
	}

	// Writes a user-friendly version of the index to the output stream.
	friend std::ostream & operator <<(std::ostream & output, Index const & index)
	{
		std::vector< Step >::const_iterator const end = index.steps.end();
		for (std::vector< Step >::const_iterator iterator = index.steps.begin();
			iterator != end; ++iterator)
		{
			int digit = *iterator;
			output << digit;
		}
		return output;
	}
};

class Pyxis::Globe::Tree::Index::Descendants :
public ForwardRangeInterface< Index const & >
{
	boost::optional< Index > index;
	Level indexStepCount;
	Level iterationLevel;

	void finish()
	{
		index = boost::none;
		indexStepCount.offset = 0;
		iterationLevel.offset = 0;
	}

	void descend()
	{
		assert(index);
		while (index->getStepCount().offset < iterationLevel.offset)
		{
			index->stepToChild();
		}
	}

	void advance()
	{
		assert(index);
		for (std::vector< Step > const & steps = index->getSteps();
			(size_t)indexStepCount.offset < steps.size(); )
		{
			// Get the last step and increment.
			char const step = steps.back() + 1;
			
			// Step to parent.
			index->stepToParent();

			// If there is another sibling, step down to it,
			// then continue to descend to the desired depth.
			// Otherwise, loop and advance the new last step.							
			if (step < index->getChildCount())
			{
				index->stepToChild(step);
				descend();
				return;
			}
		}

		finish();
	}

public:

	// Copies the index in, to be used as the index for descending.
	explicit Descendants(Index const & index, Level indexStepCount, Level depth) :
	index(index),
	indexStepCount(indexStepCount),
	iterationLevel(indexStepCount.offset + depth.offset)
	{
		if (index.getStepCount().offset < indexStepCount.offset)
		{
			throw std::invalid_argument(
				"The index step count argument cannot exceed the actual step count of the index.");
		}
		descend();
	}

	// Copies the index in, to be used as the index for descending.
	explicit Descendants(Index const & index, Level depth) :
	index(index),
	indexStepCount(index.getStepCount()),
	iterationLevel(indexStepCount.offset + depth.offset)
	{
		descend();
	}

	bool getIsEmpty() const
	{
		return index == boost::none;
	}

	// Asserts that it is non-empty.
	void popFront()
	{
		advance();
	}

	// Asserts that it is non-empty.
	Index const & getFront() const
	{
		assert(index);
		return *index;
	}
};

// Uses RVO.
inline Pyxis::Globe::Tree::Index::Descendants
Pyxis::Globe::Tree::Index::getDescendants(Level stepCount, Level depth) const
{
	return Descendants(*this, stepCount, depth);
}

// Uses RVO.
inline Pyxis::Globe::Tree::Index::Descendants
Pyxis::Globe::Tree::Index::getDescendants(Level depth) const
{
	return Descendants(*this, depth);
}

class Pyxis::Globe::Tree::Index::Test
{
	static size_t getDescendantCountByIteration(Index const & index, Level depth)
	{
		size_t result = 0;

		for (Index::Descendants descendants = index.getDescendants(depth);
			!descendants.getIsEmpty(); descendants.popFront())
		{
			++result;
		}
		
		return result;
	}
	
	static size_t getDescendantCount(Index const & index, Level depth)
	{
		size_t const descendantCount = index.getDescendantCount(depth);
		size_t const descendantCountByIteration = getDescendantCountByIteration(index, depth);
		if (descendantCount != descendantCountByIteration)
		{
			PYXIS__ASSERT___FAIL(
				"The descendant count and the iterated count don't match.");
			throw std::exception();
		}
		return descendantCount;
	}
	
	static char getChildCount(Index const & index)
	{
		size_t const childCount = index.getChildCount();
		if (childCount != getDescendantCount(index, Level(1)))
		{
			PYXIS__ASSERT___FAIL(
				"The child count and the descendant count at depth 1 don't match.");
			throw std::exception();
		}
		return childCount;
	}
	
	// For an index, verify that each of its descendants at a depth match using 
	// both approaches, and verify that all descendants are hit.
	static bool testDescendants(Index const & index, Level depth,
		FunctorInterface< bool, Index const & > & descendantFunctor)
	{
		size_t descendantCount = getDescendantCount(index, depth);
		
		for (Index::Descendants descendants = index.getDescendants(depth);
			!descendants.getIsEmpty(); descendants.popFront())
		{
			Index const & descendant = descendants.getFront();
			if (!descendantFunctor(descendant))
			{
				return PYXIS__ASSERT___FAIL(
					"The descendant functor returned false.");
			}

			--descendantCount;
		}
		
		if (descendantCount)
		{
			return PYXIS__ASSERT___FAIL(
				"The descendant count didn't match the number of iterated descendants.");
		}
		
		return true;
	}
	
public:

	operator bool ()
	{
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

		return true;
	}
};

#endif
