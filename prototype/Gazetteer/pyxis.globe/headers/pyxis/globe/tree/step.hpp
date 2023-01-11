#if !defined(PYXIS__GLOBE__TREE__STEP)
#define PYXIS__GLOBE__TREE__STEP

#if 1 // TODO: Switch to strong typedef and fix all the breakages.

namespace Pyxis
{
	namespace Globe
	{
		namespace Tree
		{
			// A step describes a movement from a node to a non-disjoint node in the tree.
			// It can only have values in the range [0, 6].
			// It must be an unsigned char.
			typedef char unsigned Step;
		}
	}
}

#else 

#include <boost/strong_typedef.hpp>

namespace Pyxis
{
	namespace Globe
	{
		namespace Tree
		{
			// A step describes a movement from a node to a non-disjoint node in the tree.
			// It can only have values in the range [0, 6].
			// It must be an unsigned char.
			// Declared using strong typedef to allow for template specializations and overloads.
			BOOST_STRONG_TYPEDEF(char unsigned, Step);
		}
	}
}

#endif

#endif
