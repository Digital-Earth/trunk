#if !defined(PYXIS__FUNCTOR_INTERFACE)
#define PYXIS__FUNCTOR_INTERFACE

#include "pyxis/pointee.hpp"
#include <functional>

namespace Pyxis
{
	// A class fulfilling this interface is a functor that takes a single argument
	// and returns a single value.
	// The return and argument values can be composite types.
	template < typename Return = void, typename Argument = void > struct FunctorInterface;

	// A specialization for a void argument, with the same behaviour.
	template < typename Return > struct FunctorInterface< Return, void >;
}

template < typename Return, typename Argument >
struct Pyxis::FunctorInterface :
virtual std::unary_function< Argument, Return >,
virtual Pointee
{
	virtual Return operator ()(Argument argument) = 0;
};

template < typename Return >
struct Pyxis::FunctorInterface< Return, void > :
virtual std::unary_function< void, Return >,
virtual Pointee
{
	virtual Return operator ()() = 0;
};

#endif
