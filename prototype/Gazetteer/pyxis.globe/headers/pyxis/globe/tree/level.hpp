#if !defined(PYXIS__GLOBE__TREE__LEVEL)
#define PYXIS__GLOBE__TREE__LEVEL

#include "pyxis/globe/resolution.hpp"
#include <boost/optional.hpp>

namespace Pyxis
{
	namespace Globe
	{
		namespace Tree
		{
			// A level in the tree, corresponding to a step count.
			// The root level (e.g. of a path with an empty step vector) is 0.
			// Tree nodes at level >= 4 represent cells at resolution 0, and
			// tree nodes at level < 4 are abstract (representing "overlapping" cells)
			// and exist to link the resolution 0 cells in a similar 
			// hierarchical relationship.
			struct Level;
		}
	}
}

struct Pyxis::Globe::Tree::Level
{
	BOOST_STATIC_CONSTANT(char, resolution0LevelOffset = 4);

	char offset;

	bool getHasResolution() const
	{
		return resolution0LevelOffset <= offset;  
	}

	boost::optional< Resolution > getResolution() const
	{
		if (getHasResolution())
		{
			return Resolution(offset - resolution0LevelOffset);
		}
		return boost::none;
	}
	
	explicit Level(char offset = 0) : offset(offset) {}

	explicit Level(Resolution resolution) :
	offset(resolution + resolution0LevelOffset) {}
};

#endif
