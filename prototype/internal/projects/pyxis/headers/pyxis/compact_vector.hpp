#if !defined(PYXIS__COMPACT_VECTOR)
#define PYXIS__COMPACT_VECTOR

#include "pyxis/forward_range_interface.hpp"
#include <algorithm>
#include <boost/integer_traits.hpp>
#include <boost/static_assert.hpp>
#include <stdexcept>

namespace Pyxis
{
	template < typename Value, typename Key = std::size_t >
	class CompactVector;
}

// Key must be numeric, and Element must be a non-reference type.
template < typename Element, typename Key >
class Pyxis::CompactVector : public virtual Pointee
{
	// The size of the key cannot exceed the size of size_t.
	BOOST_STATIC_ASSERT(
		(boost::integer_traits< Key >::const_max) <=
		(boost::integer_traits< size_t >::const_max));

	friend class Elements;

	static void construct(Element & element)
	{
		new (&element) Element();
	}

	static void copyConstruct(Element & element, Element const & source)
	{
		new (&element) Element(source);
	}

	static void destruct(Element & element)
	{
		(void)element; // Workaround for Visual Studio bug: http://connect.microsoft.com/VisualStudio/feedback/details/435574/visual-c-gives-unexpected-warning-c4100-on-explicit-call-to-object-destructor
		element.~Element();
	}

	static void destructArray(Element * const array, Key const last)
	{
		assert(array);

		// Call destructor for each element.
		std::for_each(array, array + last + 1, &destruct);

		// Free the array.
		free(array);
	}
	
	// Array allocated via malloc/realloc.
	Element * array;
	
	// The offset of the last element.
	// Contents undefined if array is null.
	Key last;

	// Destructs the elements of the array and deallocates the storage.
	void destroy()
	{
		assert(array);

		destructArray(array, last);

		// Set the fields.
		array = 0;
		last = 0;
	}

public:

	class Elements :
	public virtual ForwardRangeInterface< Element >
	{
		Element const * iterator;
		Element const * const end;

	public:

		explicit Elements(CompactVector const & compactVector) :
		iterator(compactVector.array),
		end(iterator ? (iterator + compactVector.last + 1) : 0)
		{}

		bool getIsEmpty() const
		{
			return (end == iterator);
		}
		
		// Asserts that it is non-empty.
		void popFront()
		{
			assert(iterator && end && iterator < end);

			++iterator;
		}

		// Asserts that it is non-empty.
		Element getFront() const
		{
			assert(iterator && end && iterator < end);

			return *iterator;
		}
	};

	class Test
	{
	public:

		operator bool() const
		{
#if 0
			CompactVector< std::string, char unsigned > smallVector;

			if (smallVector.getCapacity() != (boost::integer_traits< char unsigned >::const_max + 1))
			{
				return PYXIS__ASSERT___FAIL("Wrong capacity.");
			}

			if (!smallVector.getIsEmpty())
			{
				return PYXIS__ASSERT___FAIL("Should be initially empty.");
			}
			if (smallVector.getCount() != 0)
			{
				return PYXIS__ASSERT___FAIL("Wrong count.");
			}

			// From empty, set an empty; ensure it's still empty.
			smallVector.setValue(2, "");
			if (!smallVector.getIsEmpty())
			{
				return PYXIS__ASSERT___FAIL("Should still be empty.");
			}
			if (smallVector.getCount() != 0)
			{
				return PYXIS__ASSERT___FAIL("Wrong count.");
			}

			// From empty, set a non-empty
			smallVector.setValue(3, "three");
			if (smallVector.getIsEmpty())
			{
				return PYXIS__ASSERT___FAIL("Should not be empty.");
			}
			if (smallVector.getCount() != 4)
			{
				return PYXIS__ASSERT___FAIL("Wrong count.");
			}
			if (smallVector.getValue(3) != "three")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}

			// Set non-empty - 1
			smallVector.setValue(1, "one");
			if (smallVector.getIsEmpty())
			{
				return PYXIS__ASSERT___FAIL("Should not be empty.");
			}
			if (smallVector.getCount() != 4)
			{
				return PYXIS__ASSERT___FAIL("Wrong count.");
			}
			if (smallVector.getValue(0) != "")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
			if (smallVector.getValue(1) != "one")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
			if (smallVector.getValue(2) != "")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
			if (smallVector.getValue(3) != "three")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}

			// Copy.
			{
				SmallVector< std::string > copy(smallVector);
				if (copy.getIsEmpty())
				{
					return PYXIS__ASSERT___FAIL("Should not be empty.");
				}
				if (copy.getCount() != 4)
				{
					return PYXIS__ASSERT___FAIL("Wrong count.");
				}
				if (copy.getValue(0) != "")
				{
					return PYXIS__ASSERT___FAIL("Wrong value.");
				}
				if (copy.getValue(1) != "one")
				{
					return PYXIS__ASSERT___FAIL("Wrong value.");
				}
				if (copy.getValue(2) != "")
				{
					return PYXIS__ASSERT___FAIL("Wrong value.");
				}
				if (copy.getValue(3) != "three")
				{
					return PYXIS__ASSERT___FAIL("Wrong value.");
				}
			}

			// Assignment.
			{
				SmallVector< std::string > assignment;
				assignment = smallVector;
				if (assignment.getIsEmpty())
				{
					return PYXIS__ASSERT___FAIL("Should not be empty.");
				}
				if (assignment.getCount() != 4)
				{
					return PYXIS__ASSERT___FAIL("Wrong count.");
				}
				if (assignment.getValue(0) != "")
				{
					return PYXIS__ASSERT___FAIL("Wrong value.");
				}
				if (assignment.getValue(1) != "one")
				{
					return PYXIS__ASSERT___FAIL("Wrong value.");
				}
				if (assignment.getValue(2) != "")
				{
					return PYXIS__ASSERT___FAIL("Wrong value.");
				}
				if (assignment.getValue(3) != "three")
				{
					return PYXIS__ASSERT___FAIL("Wrong value.");
				}
			}

			// Set last to empty; ensure it resizes
			smallVector.setValue(3, "");
			if (smallVector.getIsEmpty())
			{
				return PYXIS__ASSERT___FAIL("Should not be empty.");
			}
			if (smallVector.getCount() != 2)
			{
				return PYXIS__ASSERT___FAIL("Wrong count.");
			}
			if (smallVector.getValue(0) != "")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
			if (smallVector.getValue(1) != "one")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
			if (smallVector.getValue(2) != "")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
			if (smallVector.getValue(3) != "")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}

			// Set last to non-empty; ensure it resizes
			smallVector.setValue(3, "three");
			if (smallVector.getIsEmpty())
			{
				return PYXIS__ASSERT___FAIL("Should not be empty.");
			}
			if (smallVector.getCount() != 4)
			{
				return PYXIS__ASSERT___FAIL("Wrong count.");
			}
			if (smallVector.getValue(0) != "")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
			if (smallVector.getValue(1) != "one")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
			if (smallVector.getValue(2) != "")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
			if (smallVector.getValue(3) != "three")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}

			// Test visitForward to completion.
			struct Visitor : virtual FunctorInterface< bool, SmallVector< std::string >::Visitee >
			{
				size_t count;
				explicit Visitor() : count() {}
				bool operator ()(SmallVector< std::string >::Visitee pair)
				{
					++count;
					switch (pair.first())
					{
					case 0:
						return pair.second() == "" || PYXIS__ASSERT___FAIL("Wrong value.");
					case 1:
						return pair.second() == "one" || PYXIS__ASSERT___FAIL("Wrong value.");
					case 2:
						return pair.second() == "" || PYXIS__ASSERT___FAIL("Wrong value.");
					case 3:
						return pair.second() == "three" || PYXIS__ASSERT___FAIL("Wrong value.");
					};
					return PYXIS__ASSERT___FAIL("Too many values.");
				}
			} visitor;
			if (!smallVector.visitForward(visitor))
			{
				return PYXIS__ASSERT___FAIL("The visiting test failed.");
			}
			if (visitor.count != 4)
			{
				return PYXIS__ASSERT___FAIL("The visitor didn't iterate enough times.");
			}

			// Test visitForward with cancel.
			struct ExitingVisitor : virtual FunctorInterface< bool, SmallVector< std::string >::Visitee >
			{
				size_t count;
				explicit ExitingVisitor() : count() {}
				bool operator ()(SmallVector< std::string >::Visitee pair)
				{
					++count;
					return !pair.first();
				}
			} exitingVisitor;
			if (smallVector.visitForward(exitingVisitor))
			{
				return PYXIS__ASSERT___FAIL("The exiting visitor didn't exit.");
			}
			if (exitingVisitor.count != 2)
			{
				return PYXIS__ASSERT___FAIL("The exiting visitor didn't iterate enough times.");
			}

			// Test visitAllForward.
			struct AllVisitor : virtual FunctorInterface< void, SmallVector< std::string >::Visitee >
			{
				size_t count;
				explicit AllVisitor() : count() {}
				void operator ()(SmallVector< std::string >::Visitee pair)
				{
					++count;
				}
			} allVisitor;
			smallVector.visitAllForward(allVisitor);
			if (allVisitor.count != 4)
			{
				return PYXIS__ASSERT___FAIL("The visitor didn't iterate enough times.");
			}

			// Set 2nd last to empty; ensure it doesn't resize
			smallVector.setValue(1, "");
			if (smallVector.getIsEmpty())
			{
				return PYXIS__ASSERT___FAIL("Should not be empty.");
			}
			if (smallVector.getCount() != 4)
			{
				return PYXIS__ASSERT___FAIL("Wrong count.");
			}
			if (smallVector.getValue(0) != "")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
			if (smallVector.getValue(1) != "")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
			if (smallVector.getValue(2) != "")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
			if (smallVector.getValue(3) != "three")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}

			// Set last to empty; ensure that it empties
			smallVector.setValue(3, "");
			if (!smallVector.getIsEmpty())
			{
				return PYXIS__ASSERT___FAIL("Should be empty.");
			}
			if (smallVector.getCount() != 0)
			{
				return PYXIS__ASSERT___FAIL("Wrong count.");
			}
			if (smallVector.getValue(0) != "")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
			if (smallVector.getValue(1) != "")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
			if (smallVector.getValue(2) != "")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
			if (smallVector.getValue(3) != "")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}

			// Test adding one at maximum
			smallVector.setValue(smallVector.getCapacity() - 1, "maximum");
			if (smallVector.getIsEmpty())
			{
				return PYXIS__ASSERT___FAIL("Should not be empty.");
			}
			if (smallVector.getCount() != smallVector.getCapacity())
			{
				return PYXIS__ASSERT___FAIL("Wrong count.");
			}
			if (smallVector.getValue(smallVector.getCapacity() - 1) != "maximum")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}

			// Test adding one at maximum - 1
			smallVector.setValue(smallVector.getCapacity() - 4, "maximum - 3");
			if (smallVector.getIsEmpty())
			{
				return PYXIS__ASSERT___FAIL("Should not be empty.");
			}
			if (smallVector.getCount() != smallVector.getCapacity())
			{
				return PYXIS__ASSERT___FAIL("Wrong count.");
			}
			if (smallVector.getValue(smallVector.getCapacity() - 4) != "maximum - 3")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
			if (smallVector.getValue(smallVector.getCapacity() - 1) != "maximum")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}

			// Test setting capacity to empty; verify resize
			smallVector.setValue(smallVector.getCapacity() - 1, "");
			if (smallVector.getIsEmpty())
			{
				return PYXIS__ASSERT___FAIL("Should not be empty.");
			}
			if (smallVector.getCount() != smallVector.getCapacity() - 3)
			{
				return PYXIS__ASSERT___FAIL("Wrong count.");
			}
			if (smallVector.getValue(smallVector.getCapacity() - 4) != "maximum - 3")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
			if (smallVector.getValue(smallVector.getCapacity() - 1) != "")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}

			// Test setting capacity to non-empty; verify resize
			smallVector.setValue(smallVector.getCapacity() - 1, "maximum again");
			if (smallVector.getIsEmpty())
			{
				return PYXIS__ASSERT___FAIL("Should not be empty.");
			}
			if (smallVector.getCount() != smallVector.getCapacity())
			{
				return PYXIS__ASSERT___FAIL("Wrong count.");
			}
			if (smallVector.getValue(smallVector.getCapacity() - 4) != "maximum - 3")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
			if (smallVector.getValue(smallVector.getCapacity() - 1) != "maximum again")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}

			// Test setIsEmpty().
			smallVector.setIsEmpty();
			if (!smallVector.getIsEmpty())
			{
				return PYXIS__ASSERT___FAIL("Should be empty.");
			}
			if (smallVector.getCount() != 0)
			{
				return PYXIS__ASSERT___FAIL("Wrong count.");
			}
			if (smallVector.getValue(smallVector.getCapacity() - 4) != "")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
			if (smallVector.getValue(smallVector.getCapacity() - 1) != "")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}

			// Test swap.
			smallVector.setValue(0, "zero");
			SmallVector< std::string > anotherSmallVector;
			anotherSmallVector.setValue(8, "eight");
			smallVector.swap(anotherSmallVector);
			if (smallVector.getIsEmpty())
			{
				return PYXIS__ASSERT___FAIL("Should not be empty.");
			}
			if (smallVector.getCount() != 9)
			{
				return PYXIS__ASSERT___FAIL("Wrong count.");
			}
			if (smallVector.getValue(8) != "eight")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
			if (anotherSmallVector.getIsEmpty())
			{
				return PYXIS__ASSERT___FAIL("Should not be empty.");
			}
			if (anotherSmallVector.getCount() != 1)
			{
				return PYXIS__ASSERT___FAIL("Wrong count.");
			}
			if (anotherSmallVector.getValue(0) != "zero")
			{
				return PYXIS__ASSERT___FAIL("Wrong value.");
			}
#endif

			return true;
		}
	};

	static size_t getCapacity()
	{
		return boost::integer_traits< Key >::const_max + 1;
	}
	
	CompactVector() : array(), last() {}

	CompactVector(CompactVector const & compactVector) :
	array(), last(compactVector.last)
	{
		if (compactVector.array)
		{
			// Allocate the array.
			array = (Element * const)malloc((last + 1) * sizeof(Element));
			if (!array)
			{
				throw std::bad_alloc();
			}

			// Populate each element with copy-constructed value.
			for (size_t offset = 0; offset <= last; ++offset)
			{
				copyConstruct(array[offset], compactVector.array[offset]);
			}
		}
	}

	~CompactVector()
	{
		if (array) { destructArray(array, last); }
	}
	
	void swap(CompactVector & with)
	{
		std::swap(array, with.array);
		std::swap(last, with.last);
	}

	CompactVector & operator =(CompactVector const & compactVector)
	{
		if (this != &compactVector)
		{
			CompactVector(compactVector).swap(*this);
		}
		return *this;
	}

	size_t getCount() const
	{
		return array ? last + 1 : 0;
	}
	
	bool getIsEmpty() const
	{
		return !array;
	}
	
	void setIsEmpty()
	{
		if (array) { destroy(); }
	}

	// Unchecked access.
	Element & operator [](Key key)
	{
		assert(array && key <= last);
		return array[key];
	}

	// Unchecked access.
	Element const & operator [](Key key) const
	{
		assert(array && key <= last);
		return array[key];
	}
	
	// Checked access.
	Element & get(Key const key)
	{
		if (!array || last < key)
		{
			throw std::out_of_range("Key out of range.");
		}
		return (*this)[key];
	}
	
	// Checked access.
	Element const & get(Key const key) const
	{
		if (!array || last < key)
		{
			throw std::out_of_range("Key out of range.");
		}
		return (*this)[key];
	}

	// The input iterators must support forward iteration, subtraction, and comparison,
	// and begin cannot be greater than end.
	template < typename InputIterator >
	void append(InputIterator begin, InputIterator const end)
	{
		assert(begin <= end && "The iterators are invalid.");

		// Get amount to append.
		size_t const amount = (end - begin);
		if (!amount) { return; }
		
		// Get count.
		size_t const count = getCount();
		
		// Compute new count.
		size_t const newCount = count + amount;
		assert(0 < newCount);
		if (boost::integer_traits< Key >::const_max < (newCount - 1))
		{
			throw std::out_of_range("The new size exceeds the capacity.");
		}

		// Allocate array with new size.
		Element * newArray = (Element * const)malloc(newCount * sizeof(Element));
		if (!newArray)
		{
			throw std::bad_alloc();
		}

		// Iterate through the items to append, and copy construct them
		// into the tail of new array.
		{
			assert(begin != end);
			size_t offset = count;
			do
			{
				assert(offset < newCount);
				copyConstruct(newArray[offset], *begin);
				++offset;
			} while (++begin != end);
		}
		
		// Copy original array contents into it, and deallocate array.
		if (array)
		{
			for (size_t offset = 0; offset < count; ++offset)
			{
				copyConstruct(newArray[offset], array[offset]);
			}
			free(array);
		}

		// Set the fields.
		array = newArray;
		last = newCount - 1;
	}
	
	// If the amount is too high, sets the array to empty.
	void truncate(size_t const amount)
	{
		if (!array || !amount) { return; }

		// If truncating the full array or more, destroy.
		if (last < amount)
		{
			destroy();
			return;
		}

		// Get count.
		size_t const count = getCount();
		
		// Compute new count.
		size_t const newCount = count - amount;

		// Destruct tail.
		std::for_each(array + newCount, array + count, &destruct);

		// Allocate array with new size.
		Element * newArray = (Element * const)malloc(newCount * sizeof(Element));
		if (!newArray)
		{
			throw std::bad_alloc();
		}

		// Copy original array contents into it, and deallocate array.
		if (array)
		{
			for (size_t offset = 0; offset < newCount; ++offset)
			{
				copyConstruct(newArray[offset], array[offset]);
			}
			free(array);
		}

		// Set the fields.
		array = newArray;
		last = newCount - 1;
	}
};

#endif
