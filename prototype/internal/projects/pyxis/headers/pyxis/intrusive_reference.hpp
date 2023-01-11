#if !defined(PYXIS__INTRUSIVE_REFERENCE)
#define PYXIS__INTRUSIVE_REFERENCE

#include <boost/intrusive_ptr.hpp>
#include <memory> // std::auto_ptr
#include <stdexcept> // std::invalid_argument

namespace Pyxis
{
	// An intrusive reference is an intrusive pointer
	// that can never be null.
	template< typename Value > class IntrusiveReference;
}

// TODO: Add casting magic from intrusive_ptr to this.
template< typename Value > class Pyxis::IntrusiveReference
{
	boost::intrusive_ptr< Value > pointer;

public:

	// Throws if the value pointer is null.
	explicit IntrusiveReference(std::auto_ptr< Value > value) :
	pointer(value.release())
	{
		if (!pointer)
		{
			throw std::invalid_argument("The pointer cannot be null.");
		}
	}

	Value const & operator *() const
	{
		return *pointer;
	}

	Value & operator *()
	{
		return *pointer;
	}
	
	Value const * operator ->() const
	{
		return pointer.operator ->();
	}
	
	Value * operator ->() const
	{
		return pointer.operator ->();
	}
};

#endif
