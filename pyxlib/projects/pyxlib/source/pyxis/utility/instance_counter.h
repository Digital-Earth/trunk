#pragma once
/******************************************************************************
instance_counter.h

begin		: 2009-Nov-13
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

// stl
#include <map>
#include <string>

/*!
 * InstanceCounter counts instances of all classes that derive from it.  
 * It manages the count as a static map.  The map can
 * be viewed in a debugger at any time.  
 *
 * TODO: It is possible that the wrong type will be set in the list,
 * since each constructor in the inheritance chain will set its class name.
 * At resolution time, the object might not be fully constructed.
 *
 * TODO: Consider adding reports or logging for remote diagnostics.
 * 
 */
class PYXLIB_DECL InstanceCounter
{
public:

	//! This map is the whole goal of this class.
	typedef std::map<std::string, std::size_t> InstanceCountMap;

public:
	template<typename T>
	class TypeSpecific
	{
	public:
		TypeSpecific()
		{
			InstanceCounter::increment(typeid(T).name());
		}

		TypeSpecific(const TypeSpecific & other)
		{
			InstanceCounter::increment(typeid(T).name());
		}

		virtual ~TypeSpecific()
		{
			InstanceCounter::decrement(typeid(T).name());
		}
	};

private:

	/*! 
	 * A holder for the class name.  This enables us to use RTTI to
	 * remember the real type of the object, since RTTI will always
	 * tell us "InstanceCounter" during a call to the constructor or
	 * destructor.
	 */
	mutable std::string m_className;

private:

	static bool nameIsKnown(InstanceCounter const * const p);

	void increment() const;
	void decrement() const;

	static void increment(const std::string & name);
	static void decrement(const std::string & name);

protected:

	/*!
	 * Constructor.  We want to add this object into s_instanceCount,
	 * but the object is only half-built right now.  (IE: RTTI would
	 * tell us it is an InstanceCounter, regardless of the type it 
	 * will eventually have.)  So we store it in s_untypedObjects for
	 * later resolution.
	 */
	InstanceCounter();

	/*!
	 * Virtual destructor.  This usually reduces the count for this 
	 * object's "real" class (as stored in m_className).  
	 */
	virtual ~InstanceCounter();

public:

	static InstanceCountMap takeSnapShot();

	static void traceObjectCountChange(
		const InstanceCountMap & snapshot1,
		const InstanceCountMap & snapshot2);
};
