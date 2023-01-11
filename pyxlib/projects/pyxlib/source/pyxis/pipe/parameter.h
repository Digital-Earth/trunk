#ifndef PYXIS__PIPE__PARAMETER_H
#define PYXIS__PIPE__PARAMETER_H
/******************************************************************************
parameter.h

begin		: 2006-12-04
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/pipe/parameter_spec.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/notifier.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/pointer.h"

// boost includes
#include <boost/intrusive_ptr.hpp>

// standard includes
#include <cassert>
#include <vector>

// forward declarations
struct IProcess;
class ProcRef;

//! The event type that gets fired whenever a parameter value changes.
class PYXLIB_DECL ParameterEvent : public NotifierEvent
{
public:
	std::vector< boost::intrusive_ptr<IProcess> > addedValues;
	std::vector< boost::intrusive_ptr<IProcess> > removedValues;

	static PYXPointer< ParameterEvent > create()
	{
		return PYXNEW(ParameterEvent);
	}
protected:
	explicit ParameterEvent() {};
};

/*!
*/
//! A parameter for a process.
class PYXLIB_DECL Parameter : public PYXObject
{
public:
	//! Create method
	static PYXPointer<Parameter> create(
		PYXPointer<ParameterSpec> spSpec	)
	{
		return PYXNEW(Parameter,
			spSpec	);
	}

	//! Create method
	static PYXPointer<Parameter> create(
		PYXPointer<ParameterSpec> spSpec,
		const std::vector<boost::intrusive_ptr<IProcess> >& vecValue	)
	{
		return PYXNEW(Parameter,
			spSpec,
			vecValue	);
	}

private:

	Parameter(
		PYXPointer<ParameterSpec> spSpec	)
	:
		m_spSpec(spSpec),
		m_notifier("Parameter change notifier")
	{
	}

	Parameter(
		PYXPointer<ParameterSpec> spSpec,
		const std::vector<boost::intrusive_ptr<IProcess> >& vecValue	)	:
		m_spSpec(spSpec),
		m_vecValue(vecValue)
	{
		// TODO: reasonable assertions
	}

public:

	//! Return the specification that defines the parmater
	PYXPointer<ParameterSpec> getSpec() const
	{
		return m_spSpec;
	}

	//! Return the number of processes that are part of the parameter.
	int getValueCount() const
	{
		return static_cast<int>(m_vecValue.size());
	}

	/*!
		Return a particular process value from the parameter using its zero based
		offset. If the passed offset does not exist then an empty pointer is returned.

		\return The requested process.
	*/
	//! Return a particular process by offset.
	boost::intrusive_ptr<IProcess> getValue(int n) const
	{
		if (n < 0 || static_cast<int>(m_vecValue.size()) <= n)
		{
			return boost::intrusive_ptr<IProcess>();
		}
		return m_vecValue[n];
	}

	//! Return all of the processes associated with the parameter.
	const std::vector<boost::intrusive_ptr<IProcess> > getValues() const
	{
		return m_vecValue;
	}

	/*!
	Finds the location of the given pipeline inside the value list of the parameter.
	if the given value wasn't found, the function return -1.
	*/
	//! Finds the location of the given pipeline inside the value list of the parameter.
	int findValue(boost::intrusive_ptr<IProcess> spProc);
	
	/*!
	Finds the location of the given pipeline inside the value list of the parameter.
	if the given value wasn't found, the function return -1.
	*/
	//! Finds the location of the given pipeline inside the value list of the parameter.
	int findValue(const ProcRef & procRef);

public:

	/*!
	Set all of the values for the parameter overwriting any existing values.
	This method will cause a change notification to be sent.
	*/
	//! Set all of the values for the parameter.
	void setValues(const std::vector<boost::intrusive_ptr<IProcess> >& vecValue)
	{
		PYXPointer<ParameterEvent> eventArgs = ParameterEvent::create();
		eventArgs->removedValues = m_vecValue;
		eventArgs->addedValues = vecValue;

		m_vecValue = vecValue;
		m_notifier.notify(eventArgs);
	}

	/*!
	Set set a single value for the parameter overwriting any existing values.
	This method will cause a change notification to be sent.
	*/
	//! Set a single value for the parameter.
	void setValue(int nIndex, boost::intrusive_ptr<IProcess> spProc)
	{
		assert((0 <= nIndex) && (nIndex < static_cast<int>(m_vecValue.size())) && "Index out of bounds");

		PYXPointer<ParameterEvent> eventArgs = ParameterEvent::create();
		eventArgs->removedValues.push_back(m_vecValue[nIndex]);
		eventArgs->addedValues.push_back(spProc);

		m_vecValue[nIndex] = spProc;

		m_notifier.notify(eventArgs);
	}

	/*!
	Add a new value to the parameter. No verification is made to determine if the new state
	meets specification or not. This method will cause a change notification to be sent.
	*/
	//! Adds a new value to the parameter.
	void addValue(boost::intrusive_ptr<IProcess> spProc)
	{
		m_vecValue.push_back(spProc);

		PYXPointer<ParameterEvent> eventArgs = ParameterEvent::create();
		eventArgs->addedValues.push_back(spProc);
		m_notifier.notify(eventArgs);
	}

	/*!
	Remove an existing value from the parameter. No verification is made to determine if the new state
	meets specification or not. This method will cause a change notification to be sent.
	*/
	//! Remove a value from the parameter.
	void removeValue(int nIndex)
	{
		assert((0 <= nIndex) && (nIndex < static_cast<int>(m_vecValue.size())) && "Index out of bounds");

		PYXPointer<ParameterEvent> eventArgs = ParameterEvent::create();
		eventArgs->removedValues.push_back(m_vecValue[nIndex]);

		m_vecValue.erase(m_vecValue.begin() + nIndex);

		m_notifier.notify(eventArgs);
	}

	/*!
	Swap between two existing values inside the parameter. No verification is made to determine if the new state
	meets specification or not. This method will cause a change notification to be sent.
	*/
	//! Remove a value from the parameter.
	void swapValues(int nIndex1,int nIndex2)
	{
		assert((0 <= nIndex1) && (nIndex1 < static_cast<int>(m_vecValue.size())) && "Index out of bounds");
		assert((0 <= nIndex2) && (nIndex2 < static_cast<int>(m_vecValue.size())) && "Index out of bounds");

		if (nIndex1 != nIndex2)
		{
			std::swap(m_vecValue[nIndex1],m_vecValue[nIndex2]);
			m_notifier.notify(ParameterEvent::create());
		}
	}


	/*!
	Clear all values from the parameter. No verification is made to determine if the new state
	meets specification or not. This method will cause a change notification to be sent.
	*/
	//! Remove all values from the parameter.
	void removeAllValues()
	{
		PYXPointer<ParameterEvent> eventArgs = ParameterEvent::create();
		eventArgs->removedValues = m_vecValue;
		m_vecValue.clear();
		m_notifier.notify(eventArgs);
	}

	//! Return the notifier that indicates when a change is made to the parameter.
	Notifier& getChangeNotifier()
	{
		return m_notifier;
	}

private:

	PYXPointer<ParameterSpec> m_spSpec;
	std::vector<boost::intrusive_ptr<IProcess> > m_vecValue;
	Notifier m_notifier;
};

#endif
