#if !defined(PYXIS__GLOBE__CELL)
#define PYXIS__GLOBE__CELL

#include "pyxis/globe/tree/index.hpp"

namespace Pyxis
{
	namespace Globe
	{
		class Cell;
	}
}

// A tile in a globe grid resolution tessellation.
// Although the cell has a simple geometry, it is distorted by sphere projection.
// The cell knows nothing about the tree; only that it has a defining index.
// There is no concept of normalized parents or children in the cell interface.
class Pyxis::Globe::Cell : public virtual Pointee
{
	Tree::Index index;

public:

	enum Intersection
	{
		noIntersection = 0,
		partialIntersection,
		completeIntersection
	};

	class Test;

	// Swap the cell contents.
	void swap(Cell & with)
	{
		index.swap(with.index);
	}

	// Gets the index that defines this cell.
	// The index is treated as a "black box" as far as the 
	// cell interface is concerned.
	Tree::Index & getIndex()
	{
		return this->index;
	}

	// Gets the index that defines this cell.
	// The index is treated as a "black box" as far as the 
	// cell interface is concerned.
	Tree::Index const & getIndex() const
	{
		return this->index;
	}

	// Swaps in the index and returns *this.
	Cell & swapIndex(Tree::Index & index)
	{
		this->index.swap(index);
		return *this;
	}

	// Returns the resolution of the cell.
	Resolution getResolution() const
	{
		assert(index.getStepCount().getResolution());
		return *index.getStepCount().getResolution();
	}

	// Returns true if the cell is a center child, or false if it is a vertex child.
	bool getIsCenter() const
	{
		return index.getIsCenter();
	}

	// Returns true if the cell is a hexagon, or false if it is a pentagon.
	bool getIsHexagon() const
	{
		return index.getIsHexagon();
	}

	// Returns the number of underlap cells
	// (all intersecting cells at the prior resolution).
	char getUnderlapCount() const
	{
		return index.getUnderlapCount();
	}

	// Steps to an underlap cell
	// (any intersecting cell at the prior resolution)
	// and sets the step to the return-trip step.
	// Throws if step is out of bounds.
	Cell & stepToUnderlap(char & step)
	{
		index.stepToUnderlap(step);
		return *this;
	}

	// Returns the number of overlap cells
	// (all intersecting cells at the next resolution).
	char getOverlapCount() const
	{
		return index.getOverlapCount();
	}

	// Steps to an overlap cell
	// (any intersecting cell at the next resolution)
	// and sets the step to the return-trip step.
	// Throws if step is out of bounds.
	Cell & stepToOverlap(char & step)
	{
		index.stepToOverlap(step);
		return *this;
	}

	// Returns the number of adjacent cells
	// (all cells at the same resolution that share an edge
	// and are not congruent).
	char getAdjacentCount() const
	{
		return index.getAdjacentCount();
	}

	// Steps to an adjacent cell
	// (any cell at the same resolution that shares an edge
	// and is not congruent)
	// and sets the step to the return-trip step.
	// Throws if step is out of bounds.  0 is not permitted.
	Cell & stepToAdjacent(char & step)
	{
		index.stepToAdjacent(step);
		return *this;
	}

	// Swaps the index.
	explicit Cell(Tree::Index & index) : index()
	{
		if (!index.getStepCount().getHasResolution())
		{
			throw std::out_of_range("The index has an invalid step count for a cell.");
		}
		this->index.swap(index);
	}

	template < typename InputIterator >
	explicit Cell(InputIterator begin, InputIterator end) : index(begin, end)
	{
		if (!index.getStepCount().getHasResolution())
		{
			throw std::out_of_range("The index has an invalid step count for a cell.");
		}
	}

	explicit Cell(char const * const string) : index(string)
	{
		if (!index.getStepCount().getHasResolution())
		{
			throw std::out_of_range("The index level does not represent a valid resolution.");
		}
	}
};

class Pyxis::Globe::Cell::Test
{
public:
	operator bool() const
	{
		{
			try
			{
				Cell cell("");
				return PYXIS__ASSERT___FAIL(
					"Created cell with invalid cell index.");
			} catch (...) {}
		}

		try
		{
			Cell cell("1");
			return PYXIS__ASSERT___FAIL(
				"Created cell with invalid cell index.");
		} catch (...) {}

		try
		{
			Cell cell("10");
			return PYXIS__ASSERT___FAIL(
				"Created cell with invalid cell index.");
		} catch (...) {}

		try
		{
			Cell cell("103");
			return PYXIS__ASSERT___FAIL(
				"Created cell with invalid cell index.");
		} catch (...) {}

		{
			Cell cell("1030");
			if (cell.getResolution())
			{
				return PYXIS__ASSERT___FAIL(
					"The resolution is incorrect.");
			}
		}

		return true;
	}
};

#endif
