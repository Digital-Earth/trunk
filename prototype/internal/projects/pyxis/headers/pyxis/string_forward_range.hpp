#if !defined(PYXIS__STRING_FORWARD_RANGE)
#define PYXIS__STRING_FORWARD_RANGE

#include "pyxis/forward_range_interface.hpp"

namespace Pyxis
{
	// A forward range for an array of Value, terminated by Value().
	template < typename Type = char >
	class StringForwardRange;
}

template < typename Type > 
class Pyxis::StringForwardRange :
public virtual Pyxis::ForwardRangeInterface< Type >
{
	Type const * iterator;

public:

	explicit StringForwardRange(Type const & string) :
	iterator(&string)
	{}

public: // ForwardRangeInterface< Value >

	// Returns true if the range is empty.
	bool getIsEmpty() const
	{
		assert(iterator);
		return *iterator == Type();
	}
	
	// Pops the first element in the range.
	// Asserts that it is non-empty.
	void popFront()
	{
		assert(iterator);
		assert(*iterator != Type());
		++iterator;
	}

	// Gets the first element in the range.
	// Asserts that it is non-empty.
	Type getFront() const
	{
		assert(iterator);
		assert(*iterator != Type());
		return *iterator;
	}
};

#endif
