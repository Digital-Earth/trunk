#if !defined(PYXIS__IS_SINGLETON)
#define PYXIS__IS_SINGLETON

#include <boost/config.hpp> // BOOST_STATIC_CONSTANT

namespace Pyxis
{
	// Tests for Singleton type (for which every instance is equivalent)
	// at compile time.
	// Any classes for which this is true
	// should provide a specialization on the class type in which
	// the compile-time "value" constant is true.
	// This is only necessary if the class is to be used as an 
	// argument for a template in which IsSingleton is checked.
	template < typename Type > struct IsSingleton
	{
		BOOST_STATIC_CONSTANT(bool, value = false);
	};
}

#endif
