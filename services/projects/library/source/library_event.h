#ifndef LIBRARY_EVENT_H
#define LIBRARY_EVENT_H
/******************************************************************************
library_event.h

begin      : 08/03/2007 5:36:18 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "library_config.h"
#include "pyxlib.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/utility/notifier.h"

//! A library operation occurred.
class LIBRARY_DECL LibraryEvent : public NotifierEvent
{
public:

	//! Event attributes.
	typedef std::map<std::string, std::string> AttributeMap;

public:

	/*!
	Attributes for proc actions:
		- proc_id Process ID
		- proc_ver Process version
		- proc_ref Process reference

	Attributes for procspec actions:
		- procspec_id Process spec ID
	*/
	//! The action that was performed.
	enum eAction
	{
		knProcAdded,
		knProcRemoved,
		knProcHidden,
		knProcUnhidden,
		knProcSpecAdded,
		knProcSpecRemoved,
		knBatchProcAdded,
		knProcGeometryResolved
	};

	//! Create a new instance.
	static PYXPointer< LibraryEvent > create(eAction nAction, const AttributeMap& attrMap)
	{
		return PYXNEW(LibraryEvent, nAction, attrMap);
	}

	//! Return the action that is being reported by the event.
	eAction getAction() {return m_nAction;}

	//! Returns the specified attribute, or empty string if it doesn't exist.
	std::string getAttribute(const std::string& strName)
	{
		std::string str;
		AttributeMap::const_iterator it = m_attrMap.find(strName);
		if (it != m_attrMap.end())
		{
			str = it->second;
		}
		return str;
	}

protected:

	//! Construct a new event.
	explicit LibraryEvent(eAction nAction, const AttributeMap& attrMap) :
		m_nAction(nAction),
		m_attrMap(attrMap)
	{
	}

private:

	//! The action that occurred.
	eAction m_nAction;

	//! Event attributes.
	AttributeMap m_attrMap;
};

#endif
