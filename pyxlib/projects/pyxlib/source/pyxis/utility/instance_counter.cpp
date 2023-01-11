/******************************************************************************
instance_counter.cpp

begin		: 2009-Nov-13
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/instance_counter.h"

// stl
#include <list>

// boost includes
#include <boost/thread/mutex.hpp>

namespace 
{

//! A list of instance counters.
typedef std::list<InstanceCounter const *> List;

struct State
{
	InstanceCounter::InstanceCountMap instanceCount;
	boost::mutex instanceCountMutex;

	//! We maintain a list of newly created objects.
	List untypedObjects;
	boost::mutex untypedObjectMutex;
};

// TODO: This is not thread-safe, and it may be destructed early.
State & state()
{
	static State s_state;
	return s_state;
}

}

//! Helper function (lets us purge the untyped objects list.)
//! Called within lock(state().untypedObjectMutex).
bool InstanceCounter::nameIsKnown(InstanceCounter const * const p)
{
	return (!(p->m_className.empty()));
}


void InstanceCounter::increment(const std::string & name)
{
	boost::mutex::scoped_lock lock(state().instanceCountMutex);
	++(state().instanceCount[name]);
}

void InstanceCounter::decrement(const std::string & name)
{
	boost::mutex::scoped_lock lock(state().instanceCountMutex);
	--(state().instanceCount[name]);
}

/*!
 * Store the not-fully-constructed instance counter, 
 * process any objects that are in state().untypedObjects, and remove
 * them as they are processed.
 */
void InstanceCounter::increment() const
{
	// Lock the tables for thread safety.
	boost::mutex::scoped_lock lock(state().untypedObjectMutex);

	// Process any objects that were previously constructed....
	for (List::const_iterator i = state().untypedObjects.begin(); 
		i != state().untypedObjects.end();
		++i)
	{
		// Do we know the type of this object yet?
		if (typeid(**i) != typeid(InstanceCounter))
		{
			(*i)->m_className = typeid(**i).name();

			boost::mutex::scoped_lock lock(state().instanceCountMutex);
			++(state().instanceCount[(*i)->m_className]);
		}
	}

	// Now remove any that have been processed.
	state().untypedObjects.remove_if(nameIsKnown);

	// And finally remember this object for future processing.
	state().untypedObjects.push_back(this);
}

/*!
 * Decrement the instance count, or remove the instance counter.
 * Does not throw.
 */
void InstanceCounter::decrement() const
{
	// Catch exceptions (the static data might go away first).
    try
	{
		boost::mutex::scoped_lock lock(state().untypedObjectMutex);

		// Do we know the class name? (This means we've been processed.)
		if (m_className.empty())
		{
			// We neeed to remove this object from state().untypedObjects,
			// since we never got around to processing it.
			state().untypedObjects.remove(this);
		}
		else
		{
			boost::mutex::scoped_lock lock(state().instanceCountMutex);
			--(state().instanceCount[m_className]);
		}
	} 	
    catch (...)
	{}
}

/*!
 * Constructor.  We want to add this object into state().instanceCount,
 * but the object is only half-built right now.  (IE: RTTI would
 * tell us it is an InstanceCounter, regardless of the type it 
 * will eventually have.)  So we store it in state().untypedObjects for
 * later resolution.
 */
InstanceCounter::InstanceCounter() : m_className()
{
	increment();
}

/*!
 * Virtual destructor.  This usually reduces the count for this 
 * object's "real" class (as stored in m_className).  
 */
InstanceCounter::~InstanceCounter()
{
	decrement();
}

InstanceCounter::InstanceCountMap InstanceCounter::takeSnapShot()
{
	boost::mutex::scoped_lock lock(state().instanceCountMutex);
	return state().instanceCount;
}

void InstanceCounter::traceObjectCountChange(
	const InstanceCountMap & snapshot1,
	const InstanceCountMap & snapshot2)
{
	for (
		InstanceCountMap::const_iterator it2 = snapshot2.begin();
		it2 != snapshot2.end();
		++it2)
	{
		InstanceCountMap::const_iterator it1 = snapshot1.find(it2->first);
		std::string className(it2->first);
					
		if (it1 == snapshot1.end())
		{
			if (it2->second > 0)
			{
				TRACE_INFO(className << " : " << it2->second << " new instances");
			}
		}
		else if (it1->second > it2->second)
		{
			TRACE_INFO(className << " : " << it2->second << " instances were deleted");
		}
		else if (it1->second < it2->second)
		{
			TRACE_INFO(className << " : " << (it2->second - it1->second) << " new instances");
		}
	}
}
