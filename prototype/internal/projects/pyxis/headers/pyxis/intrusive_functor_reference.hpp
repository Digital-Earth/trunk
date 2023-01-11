#if !defined(PYXIS__INTRUSIVE_FUNCTOR_REFERENCE)
#define PYXIS__INTRUSIVE_FUNCTOR_REFERENCE

#include "pyxis/functor_interface.hpp"
#include <boost/intrusive_ptr.hpp>
#include <memory> // std::auto_ptr
#include <stdexcept>

namespace Pyxis
{
	// This class is a wrapper for local class functors
	// to allow them to be used as template arguments
	// without violating the standard.
	// Contains an intrusive pointer to the wrapped functor that is shallow-copied;
	// note that if the call operator changes the instance,
	// all copies will be changed.
	// Because the reference is owned, this class is useful
	// for passing a functor across a thread boundary
	// (considering that Pointee is thread-safe).
	template < typename Return = void, typename Argument = void > class IntrusiveFunctorReference;

	// A specialization for a void argument that has the same behaviour.
	template < typename Return > class IntrusiveFunctorReference< Return, void >;
};

template < typename Return, typename Argument > 
class Pyxis::IntrusiveFunctorReference :
public virtual FunctorInterface< Return, Argument >
{
	boost::intrusive_ptr< FunctorInterface< Return, Argument > > functor;

public:

	explicit IntrusiveFunctorReference(
		std::auto_ptr< FunctorInterface< Return, Argument > > functor) :
	functor(functor.release())
	{
		if (!this->functor)
		{
			throw std::invalid_argument("The functor pointer cannot be null.");
		}
	}

	Return operator ()(Argument argument)
	{
		assert(functor);
		return (*functor)(argument);
	}
};

template < typename Return > 
class Pyxis::IntrusiveFunctorReference< Return, void > :
public virtual FunctorInterface< Return >
{
	boost::intrusive_ptr< FunctorInterface< Return > > functor;

public:

	explicit IntrusiveFunctorReference(
		std::auto_ptr< FunctorInterface< Return > > functor) :
	functor(functor.release())
	{
		if (!this->functor)
		{
			throw std::invalid_argument("The functor pointer cannot be null.");
		}
	}

	Return operator ()()
	{
		assert(functor);
		return (*functor)();
	}
};

#endif
