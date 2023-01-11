#ifndef PYXIS__UTILITY__OBJECT_H
#define PYXIS__UTILITY__OBJECT_H
/******************************************************************************
object.h

begin		: 2006-09-12
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/pointer.h"

// boost includes
#include <boost/detail/atomic_count.hpp>

// standard includes
#include <cassert>
#include <limits>

// If defined, initial reference count is 0; else, initial reference count is 1.
// The former is preferred, as additional assertions can be done.
// The code should be robust enough to work with either without memory issues.
// TODO: Fix code so that this can be defined without errors.
//#define INITIAL_ZERO_REF_COUNT 1

#ifdef INSTANCE_COUNTING
#include "instance_counter.h"
#endif

/*!
Returns a PYXPointer to a new instance of a descendant class of PYXObject.
The variadic arguments, signified by the ellipsis, are passed through to
a constructor of PYXObjectDescendantClass.
*/
//! macro to return a new instance of a PYXObject based class.
#if !defined(NDEBUG) && defined(_WINDOWS)
#define PYXNEW(PYXObjectDescendantClass, ...) \
	(PYXObject::pointerTo(new(_NORMAL_BLOCK, __FILE__, __LINE__) PYXObjectDescendantClass(__VA_ARGS__)))
#else
#define PYXNEW(PYXObjectDescendantClass, ...) \
	(PYXObject::pointerTo(new PYXObjectDescendantClass(__VA_ARGS__)))
#endif

/*!
PYXObject is the base class for all objects that require reference counting.
This includes objects that are passed across DLL boundaries.
*/
//! PYXObject provides reference counting for PYXIS objects.
class PYXLIB_DECL PYXObject
#ifdef INSTANCE_COUNTING
	: protected InstanceCounter
#endif
{
public:

	//! Destructor.
	virtual ~PYXObject()
	{
#if INITIAL_ZERO_REF_COUNT
		assert((0 == m_nRefCount) && "The object is being destroyed while there are still references.");
#endif
	}

	//! Get the object's reference count.
	inline long getRefCount() const { return m_nRefCount; }

protected:

#ifndef SWIG_INTERNAL // SWIG wrapper cxx needs access to these operators to wrap destructors etc. (better way?)

	/*!
	This is protected to ensure that only subclasses can dynamically 
	instantiate an object.
	*/
	//! New operator.
	static inline void * operator new(size_t size)
	{
		return ::operator new(size);
	}

	//! Delete operator.
	//! This is protected to ensure that only subclasses can delete an object.
	static inline void operator delete(void * ptr)
	{
		::operator delete(ptr);
	}

	/*! 
	This is protected to ensure that only subclasses can dynamically 
	instantiate an array of objects.
	*/
	//! Array new operator.
	static inline void * operator new[](size_t size)
	{
		return ::operator new[](size);
	}

	/*! 
	This is protected to ensure that only subclasses can delete an array of 
	objects.
	*/
	//! Array delete operator.
	static inline void operator delete[](void * ptr)
	{
		::operator delete[](ptr);
	}

#if !defined(NDEBUG) && defined(_WINDOWS)
	/*! 
	This is protected to ensure that only subclasses can dynamically 
	instantiate an array of objects.
	*/
	//! Custom new operator.
	static inline void * operator new(size_t size, int block, char const * file, size_t line)
	{
		return ::operator new(size, block, file, line);
	}

	/*! 
	This is protected to ensure that only subclasses can dynamically 
	instantiate an array of objects.
	*/
	//! Custom delete operator.
	static inline void operator delete(void * ptr, int block, char const * file, size_t line)
	{
		return ::operator delete(ptr);
	}
#endif

#endif

	/*! 
	Takes a non-null bald pointer that owns the pointee, which must be an 
	instance of PYXObject descendant class, and returns a PYXPointer to the 
	instance. 
	*/
	//! Create a PYXPointer to a new instance of an object.
	template <typename PYXObjectDescendantClass>
	static PYXPointer<PYXObjectDescendantClass> pointerTo(PYXObjectDescendantClass * const p)
	{
		assert(0 != p);

		// This forces a compile failure if the type is not a subclass of PYXObject.
		PYXObject const * const pObject = p;

#if INITIAL_ZERO_REF_COUNT
#if PYXPOINTER_DEBUG_STRONG
		assert(0 == pObject->getRefCount() && "Pointer can only be created from a new PYXObject.");
#endif

		// Create the intrusive pointer, incrementing the reference count to 1.
		return PYXPointer<PYXObjectDescendantClass>(p);
#else
#if PYXPOINTER_DEBUG_STRONG
		assert(1 == pObject->getRefCount() && "Pointer can only be created from a new PYXObject.");
#endif

		// Create the intrusive pointer without incrementing the reference count.
		return PYXPointer<PYXObjectDescendantClass>(p, false);
#endif
	}

#if INITIAL_ZERO_REF_COUNT

	//! Constructor.
	PYXObject() : m_nRefCount(0) {}

	//! Copy constructor.
	PYXObject(const PYXObject& rhs) : m_nRefCount(0) {}

#else

	//! Constructor.
	PYXObject() : m_nRefCount(1) {}

	//! Copy constructor.
	PYXObject(const PYXObject& rhs) : m_nRefCount(1) {}

#endif

	//! Assignment operator.
	PYXObject& operator =(const PYXObject& rhs) { return *this; }

private:

	// This function must have access to the static "addRef" method.
	friend inline void intrusive_ptr_add_ref(PYXObject const * const p);

	// This function must have access to the static "release" method.
	friend inline void intrusive_ptr_release(PYXObject const * const p);

	//! Increment the object's reference count.
	static void addRef(PYXObject const * const pObject)
	{
		assert(0 != pObject);

		pObject->addRef();
	}

	/*! 
	Decrement the object's reference count, and delete the object if the count 
	is less than 1. The pointer must be a non-null pointer to a heap-allocated, owned instance of a 
	PYXObject derivative.

	\param pObject Pointer to the object to clean up.
	*/
	//! Clean up a reference to a PYXObject.
	static void release(PYXObject const * const pObject)
	{
		assert(0 != pObject);

		if (pObject->release() == 0)
		{
			delete pObject;
		}
	}

protected:

	//! Increment the object's reference count.
	virtual long addRef() const
	{
#if !INITIAL_ZERO_REF_COUNT
		assert(0 < m_nRefCount);
#endif

		// TODO: Throw an exception, with a thread-safe check.
		assert(m_nRefCount < std::numeric_limits<long>::max());

		return ++m_nRefCount;
	}

	//! Decrement the object's reference count.
	virtual long release() const
	{
		assert(0 < m_nRefCount);

		return --m_nRefCount;
	}

private:

	//! The reference count that is capable of atomic increment and decrement.
	mutable boost::detail::atomic_count m_nRefCount;
};

//! Helper function for boost::intrusive_ptr which adds a reference to an object.
inline void intrusive_ptr_add_ref(PYXObject const * const p)
{
	PYXObject::addRef(p);
}

//! Helper function for boost::intrusive_ptr which releases an object.
inline void intrusive_ptr_release(PYXObject const * const p)
{
	PYXObject::release(p);
}


//! Utility class for adding a PYXObject interface for and class
template<typename T>
class PYXObjectWrapper : public PYXObject
{
public:
	T value;

	static PYXPointer<PYXObjectWrapper> create(const T & value) 
	{
		return PYXNEW(PYXObjectWrapper,value);
	}

	PYXObjectWrapper(const T & aValue) : value(aValue) {}
};

#endif // guard
