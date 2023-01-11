#if !defined(PYXIS__DYNAMIC_ARRAY)
#define PYXIS__DYNAMIC_ARRAY

#include <algorithm>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>

namespace Pyxis
{
	template < typename Value, typename Count = std::size_t >
	class DynamicArray;
}

template < typename Value, typename Count >
class Pyxis::DynamicArray :
boost::noncopyable
{
#if !NDEBUG
	Count count;
#endif

	boost::scoped_array< Value > values;

public:

	DynamicArray() :
#if !NDEBUG
	count(),
#endif
	values()
	{}

	Value & operator [](Count const offset)
	{
#if !NDEBUG
		assert(offset < this->count);
#endif

		return this->values[offset];
	}

	Value const & operator [](Count const offset) const
	{
#if !NDEBUG
		assert(offset < this->count);
#endif

		return this->values[offset];
	}

	void swap(DynamicArray & that)
	{
#if !NDEBUG
		std::swap(this->count, that.count);
#endif
		this->values.swap(that.values);
	}

	bool getIsEmpty() const
	{
		return !this->values;
	}

	void setIsEmpty()
	{
#if !NDEBUG
		this->count = 0;
#endif
		this->values.reset();
	}

	void reset(Count count)
	{
#if !NDEBUG
		this->count = count;
#endif
		this->values.reset(new Value[count]);
	}
};

#endif
