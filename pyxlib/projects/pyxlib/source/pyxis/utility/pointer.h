#ifndef PYXIS__UTILITY__POINTER_H
#define PYXIS__UTILITY__POINTER_H
/******************************************************************************
pointer.h

begin		: 2006-10-06
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

// Define this to enable the debug implementation of PYXPointer.
// TODO: Fix our code so that this can be enabled without errors.
//#define PYXPOINTER_DEBUG 1

// Define this to enable strong debugging of PYXPointer.
// TODO: Fix our code so that this can be enabled without errors.
//#define PYXPOINTER_DEBUG_STRONG 1

#if PYXPOINTER_DEBUG

// boost includes
#include <boost/detail/sp_convertible.hpp>

// standard includes
#include <cassert>

// TODO: Tidy this up; possibly privately inherit from intrusive_ptr<T>.
//! The debug implementation of a smart pointer to a PYXObject, based on boost::intrusive_ptr.
template<typename T> class PYXPointer
{
	friend T;
	friend class PYXObject;

private:

	typedef PYXPointer this_type;

public:

	typedef T element_type;

	PYXPointer(): p_(0)
	{
	}

#if PYXPOINTER_DEBUG_STRONG
private:
#endif

	PYXPointer(T * p): p_(p)
	{
		if (p_ != 0) intrusive_ptr_add_ref(p_);
	}

	PYXPointer(T * p, bool add_ref): p_(p)
	{
		if (p_ != 0 && add_ref) intrusive_ptr_add_ref(p_);
	}

public:

#if !defined(BOOST_NO_MEMBER_TEMPLATES) || defined(BOOST_MSVC6_MEMBER_TEMPLATES)

	template<class U>
#if !defined( BOOST_SP_NO_SP_CONVERTIBLE )
	PYXPointer( PYXPointer<U> const & rhs, typename boost::detail::sp_enable_if_convertible<U,T>::type = boost::detail::sp_empty() )
#else
	PYXPointer( PYXPointer<U> const & rhs )
#endif
	: p_( rhs.get() )
	{
		if( p_ != 0 ) intrusive_ptr_add_ref( p_ );
	}

#endif

	PYXPointer(PYXPointer const & rhs): p_(rhs.p_)
	{
		if(p_ != 0) intrusive_ptr_add_ref(p_);
	}

	~PYXPointer()
	{
		if(p_ != 0) intrusive_ptr_release(p_);
	}

#if !defined(BOOST_NO_MEMBER_TEMPLATES) || defined(BOOST_MSVC6_MEMBER_TEMPLATES)
	template<class U> PYXPointer & operator=(PYXPointer<U> const & rhs)
	{
		this_type(rhs).swap(*this);
		return *this;
	}
#endif

	PYXPointer & operator=(PYXPointer const & rhs)
	{
		this_type(rhs).swap(*this);
		return *this;
	}

#if PYXPOINTER_DEBUG_STRONG
private:
#endif

	PYXPointer & operator=(T * rhs)
	{
		this_type(rhs).swap(*this);
		return *this;
	}

public:

	void reset()
	{
		this_type().swap( *this );
	}

#if PYXPOINTER_DEBUG_STRONG
private:
#endif

	void reset( T * rhs )
	{
		this_type( rhs ).swap( *this );
	}

public:

	T * get() const
	{
		return p_;
	}

	T & operator*() const
	{
		assert(p_ != 0);
		return *p_;
	}

	T * operator->() const
	{
		assert(p_ != 0);
		return p_;
	}

	template<class U> static PYXPointer<T> static_pointer_cast(PYXPointer<U> const & p)
	{
		return static_cast<T *>(p.get());
	}

	template<class U> static PYXPointer<T> const_pointer_cast(PYXPointer<U> const & p)
	{
		return const_cast<T *>(p.get());
	}

	template<class U> static PYXPointer<T> dynamic_pointer_cast(PYXPointer<U> const & p)
	{
		return dynamic_cast<T *>(p.get());
	}

	typedef T * this_type::*unspecified_bool_type;

	operator unspecified_bool_type () const
	{
		return p_ == 0? 0: &this_type::p_;
	}

	// operator! is a Borland-specific workaround
	bool operator! () const
	{
		return p_ == 0;
	}

	void swap(PYXPointer & rhs)
	{
		T * tmp = p_;
		p_ = rhs.p_;
		rhs.p_ = tmp;
	}

private:

	T * p_;
};

template<class T, class U> inline bool operator==(PYXPointer<T> const & a, PYXPointer<U> const & b)
{
	return a.get() == b.get();
}

template<class T, class U> inline bool operator!=(PYXPointer<T> const & a, PYXPointer<U> const & b)
{
	return a.get() != b.get();
}

template<class T, class U> inline bool operator==(PYXPointer<T> const & a, U * b)
{
	return a.get() == b;
}

template<class T, class U> inline bool operator!=(PYXPointer<T> const & a, U * b)
{
	return a.get() != b;
}

template<class T, class U> inline bool operator==(T * a, PYXPointer<U> const & b)
{
	return a == b.get();
}

template<class T, class U> inline bool operator!=(T * a, PYXPointer<U> const & b)
{
	return a != b.get();
}

template<class T> inline bool operator<(PYXPointer<T> const & a, PYXPointer<T> const & b)
{
	return std::less<T *>()(a.get(), b.get());
}

template<class T> void swap(PYXPointer<T> & lhs, PYXPointer<T> & rhs)
{
	lhs.swap(rhs);
}

namespace boost
{

template<class T> T * get_pointer(PYXPointer<T> const & p)
{
	return p.get();
}

template<class T, class U> PYXPointer<T> static_pointer_cast(PYXPointer<U> const & p)
{
	return PYXPointer<T>::static_pointer_cast(p);
}

template<class T, class U> PYXPointer<T> const_pointer_cast(PYXPointer<U> const & p)
{
	return PYXPointer<T>::const_pointer_cast(p);
}

template<class T, class U> PYXPointer<T> dynamic_pointer_cast(PYXPointer<U> const & p)
{
	return PYXPointer<T>::dynamic_pointer_cast(p);
}

}

#if !defined(BOOST_NO_IOSTREAM)

#if defined(BOOST_NO_TEMPLATED_IOSTREAMS) || ( defined(__GNUC__) &&  (__GNUC__ < 3) )

template<class Y> std::ostream & operator<< (std::ostream & os, PYXPointer<Y> const & p)
{
	os << p.get();
	return os;
}

#else

// in STLport's no-iostreams mode no iostream symbols can be used
#ifndef _STLP_NO_IOSTREAMS

# if defined(BOOST_MSVC) && BOOST_WORKAROUND(BOOST_MSVC, < 1300 && __SGI_STL_PORT)
// MSVC6 has problems finding std::basic_ostream through the using declaration in namespace _STL
using std::basic_ostream;
template<class E, class T, class Y> basic_ostream<E, T> & operator<< (basic_ostream<E, T> & os, PYXPointer<Y> const & p)
# else
template<class E, class T, class Y> std::basic_ostream<E, T> & operator<< (std::basic_ostream<E, T> & os, PYXPointer<Y> const & p)
# endif 
{
	os << p.get();
	return os;
}

#endif // _STLP_NO_IOSTREAMS

#endif // __GNUC__ < 3

#endif // !defined(BOOST_NO_IOSTREAM)

#else // !PYXPOINTER_DEBUG

// boost includes
#include <boost/intrusive_ptr.hpp>

//! A smart pointer to a PYXObject.
#define PYXPointer boost::intrusive_ptr

#endif // if PYXPOINTER_DEBUG

#endif
