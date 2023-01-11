#if !defined(PYXIS__FORWARD_RANGE_INTERFACE)
#define PYXIS__FORWARD_RANGE_INTERFACE

#include "pyxis/pointee.hpp"

namespace Pyxis
{
	template < typename Value > struct ForwardRangeInterface;

#if 0	
	// TODO: Move to separate file
	template < typename Value, typename Count = size_t > 
	struct SkipForwardRangeInterface;
#endif
}

template < typename Value > 
struct Pyxis::ForwardRangeInterface : virtual Pointee
{
	// An iterator is provided to allow for usage
	// with std::algorithm and other code that 
	// requires iterators.
	// Advancing the iterator pops from the front of the range.
	class Iterator
	{
		ForwardRangeInterface * range;

	public:

		// Creates an iterator in the "end" state.
		Iterator() : range() {}

		// Creates an iterator for the range, or an
		// iterator in the "end" state if the range is empty.
		// Requires the range to outlive this object.
		explicit Iterator(ForwardRangeInterface< Value > & range) :
		range(&range) {}

		// Advances the iterator by popping from the front of the range.
		Iterator & operator ++()
		{
			assert(this->range);
			this->range->popFront();
			if (this->range->getIsEmpty())
			{
				this->range = 0;
			}
			return *this;
		}
	
		bool operator ==(Iterator const & that) const
		{
			return this->range == that.range;
		}
		
		bool operator !=(Iterator const & that) const
		{
			return this->range != that.range;
		}
		
		Value operator *() const
		{
			assert(this->range);
			return this->range->getFront();
		}
	};

	// Returns an iterator pointing to the first item in the range.
	// Uses RVO.
	Iterator begin() { return Iterator(*this); }

	// Returns an iterator pointing past the last item in the range.
	// Uses RVO.
	Iterator end() const { return Iterator(); }

	// Returns true if the range is empty.
	virtual bool getIsEmpty() const = 0;
	
	// Pops the first element in the range.
	// Asserts that it is non-empty.
	virtual void popFront() = 0;

	// Gets the first element in the range.
	// Asserts that it is non-empty.
	virtual Value getFront() const = 0;
};

#if 0
// Extends forward range by allowing "jumping ahead".
template < typename Value, typename Count > 
struct Pyxis::SkipForwardRangeInterface : ForwardRangeInterface< Value >
{
	// Returns the count.
	virtual Count getCount() const = 0;

	// Pops the first N elements.
	// Asserts that the count does not exceed the size.
	virtual void popFront(Count count) = 0;

	// Skips the first N elements and returns the one thereafter.
	// Asserts that the offset does not equal or exceed the size.
	virtual Value skipFront(Count count) const = 0;
};
#endif

#endif
