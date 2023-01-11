#if !defined(PYXIS__POINTEE)
#define PYXIS__POINTEE

#include <boost/detail/atomic_count.hpp>
#include <boost/integer_traits.hpp>
#include <cassert>

namespace Pyxis
{
	// A pointee of an intrusive pointer.
	// This class is thread-safe.
	class Pointee;
}

namespace boost
{
	void intrusive_ptr_add_ref(Pyxis::Pointee const * const p);
	void intrusive_ptr_release(Pyxis::Pointee const * const p);
}

class Pyxis::Pointee
{
	mutable boost::detail::atomic_count referenceCount;

	friend void boost::intrusive_ptr_add_ref(Pointee const * const p);
	friend void boost::intrusive_ptr_release(Pointee const * const p);

protected:

	Pointee() : referenceCount(0) {}
	Pointee(Pointee const &) : referenceCount(0) {}
	Pointee & operator = (Pointee const &)
	{
		return *this;
	}

public:

	virtual ~Pointee()
	{
		assert(0 == referenceCount && "The pointee was deleted with a non-zero reference count.");
	}

	long getReferenceCount() const
	{
		return referenceCount;
	}
};

namespace boost
{
	inline void intrusive_ptr_add_ref(Pyxis::Pointee const * const p)
	{
		assert(p && "The argument cannot be null.");

		// TODO: Use a proper atomics library, change this to an exception, and make the class guarantee strong exception safety.
		assert(p->referenceCount < boost::integer_traits< long >::const_max && "Reference count overflow.");

		++(p->referenceCount);
	}

	inline void intrusive_ptr_release(Pyxis::Pointee const * const p)
	{
		assert(p && "The argument cannot be null.");

		if (0 == --(p->referenceCount))
		{
			delete p;
		}
	}
}

#endif
