#ifndef PYXIS__UTILITY__ABSTRACT_ITERATOR_H
#define PYXIS__UTILITY__ABSTRACT_ITERATOR_H
/******************************************************************************
abstract_iterator.h

begin		: 2006-09-07
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

//! This is the base class for iterators that iterate over a property of a PYXIS cell,
//! such as indices or directions.
class PYXLIB_DECL PYXAbstractIterator
{
public:

	//! Destructor
	virtual ~PYXAbstractIterator() {}

	//! Simple test "while (myIterator)..."
	operator bool() const
	{
		return !end();
	}

	//! Simple test "if (!myIterator) ..."
	bool operator !() const
	{
		return end();
	}

	//! Simple operator of the form "++myIterator".
	virtual PYXAbstractIterator& operator ++()
	{
		next();
		return *this;
	}

	//! Move to the next item.
	virtual void next() = 0;

	/*!
	See if we have covered all the items.

	\return	true if all items have been covered, otherwise false.
	*/
	virtual bool end() const = 0;
};

//! Helper class write a iterator class
template<typename T,class Parent>
class PYXIteratorHelper 
{
private:
	typedef bool (Parent::*NextFunc)();
	NextFunc m_next;
	T m_current;
	bool m_ended;

protected:
	bool yield(const T & value,NextFunc func)
	{
		m_current = value;
		m_next = func;
		m_ended = false;
		return true;
	}

	bool yield_end()
	{
		m_next = 0;
		m_ended = true;
		return false;
	}

	void start(NextFunc func)
	{
		m_next = func;
		m_ended  = false;
		next();
	}

public:
	bool end() const { return m_ended; } 

	const T & current() const { return m_current; } 

	void next()
	{
		if (m_next != 0)
		{
			(((Parent & )*this).*m_next)();
		}
		else 
		{
			m_ended = true;
		}
	}
};

#endif // guard
