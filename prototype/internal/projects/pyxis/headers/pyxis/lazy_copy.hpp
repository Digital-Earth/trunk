#if !defined(PYXIS__LAZY_COPY)
#define PYXIS__LAZY_COPY

#include "pyxis/functor_interface.hpp"
#include <boost/intrusive_ptr.hpp>

namespace Pyxis
{
	// This class wraps a value in a structure that
	// only copies the underlying value for modifications.
	// Value requirements:
	//	-	Default constructor.
	//	-	Equality (and inequality) operators
	//	-	Non-throwing "swap" function, which swaps the contents.
	//	-	Must expose a reference count through getReferenceCount().
	// This class, like most in C++, is not inherently thread-safe, and all
	// accesses from multiple threads must be locked.
	template < typename Type > class LazyCopy;
}

template < typename Type > class Pyxis::LazyCopy
{
	boost::intrusive_ptr< Type > pointer;
	
public:

	// Constructs with the default constructor.
	explicit LazyCopy() :
	pointer(new Type())
	{}

	// Swaps the value.
	explicit LazyCopy(Type & value) :
	pointer(new Type())
	{
		value.swap(*pointer);
	}
	
	// Gets a const reference to the value.
	Type const & operator *() const
	{
		assert(pointer);
		return *pointer;
	}

	// Allows access to const members of the value.
	Type const * operator ->() const
	{
		assert(pointer);
		return pointer.operator ->();
	}

	// Modifies it by applying a functor to it.
	// Copies if neccessary.
	// The passed-in functor SHOULD NOT retain a reference to the value;
	// modifying it after the fact may cause multiple instances
	// to be modified.
	template < typename Return >
	Return modify(FunctorInterface< Return, Type & > & functor)
	{
		assert(pointer);
		if (1 < pointer->getReferenceCount())
		{
			pointer.reset(new Type(*pointer));
		}
		return functor(*pointer);
	}

	bool operator ==(LazyCopy const & another) const
	{
		assert(pointer);
		assert(another.pointer);
		return *pointer == *another.pointer;
	}
	
	bool operator !=(LazyCopy const & another) const
	{
		assert(pointer);
		assert(another.pointer);
		return *pointer != *another.pointer;
	}
};

#endif
