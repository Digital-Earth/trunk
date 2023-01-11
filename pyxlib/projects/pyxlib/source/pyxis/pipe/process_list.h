#ifndef PYXIS__PIPE__PROCESS_LIST_H
#define PYXIS__PIPE__PROCESS_LIST_H
/******************************************************************************
process_list.h

begin      : 09/10/2007 4:37:36 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/pipe/process.h"
#include "pyxis/utility/notifier.h"

// Standard includes
#include <string.h>

/*!
This object holds a list of unique instantiated processes that are managed and 
reference counted. The list notifies when objects are added or removed.
*/
//! A managed list of unique pipeline objects
class PYXLIB_DECL ProcessList : public PYXObject
{
public:

	//! Create a new data visualization manager
	static inline PYXPointer<ProcessList> create()
	{
		return PYXNEW(ProcessList);	
	}

	//! Destructor.
	~ProcessList() {}


	//! Indicate if the list is assoicated with any pipelines.
	bool hasProcs() 
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		return m_procs.size() != 0;
	}

	//! Determine if a particular process is referenced in the list.
	bool contains(const ProcRef& procref) const;

	//! Add a process to the list by reference.
	void addProc(const ProcRef& procref);

	//! Add a process to the list by pointer.
	void addProc(boost::intrusive_ptr<IProcess> proc);

	//! Remove a process from the list.
	void removeProc(const ProcRef& procref);

	//! Remove all processes from the list. Notify of the change.
	void clear();

	//! Return the size of the list
	int count(); 

	//! Return all of the individual processes in the list.
	void getProcessRefs(std::vector< ProcRef > & vecProcs);

	//! Return all of the individual processes in the list.
	void getProcesses(std::vector< boost::intrusive_ptr<IProcess> > & vecProcs);

	//! Replace the current list of processes with a new one.
	void setProcesses(const std::vector<boost::intrusive_ptr<IProcess> >& vecProcs);

	//! Return the list changed notifier
	Notifier& getListChangedNotifier()
	{
		return m_listChangedNotifier;
	}
	
private:

	//! Default constructor.
	ProcessList();

	//! Load processes from the pipeline into the procs map.
	void registerProcesses(boost::intrusive_ptr<IProcess> spProc);

private:

	//! Add a unique process to the internal data structure and do not notify.
	bool ProcessList::addProcessToMap(boost::intrusive_ptr<IProcess> spProc);

	//! A mutex to protect the file from multithreaded access.
	mutable boost::recursive_mutex m_mutex;

	//! The list of unique data processes being displayed.
	std::map< ProcRef, boost::intrusive_ptr<IProcess> > m_procs;

	//! Notifier for the process list.
	Notifier m_listChangedNotifier;
};

//! A change has been made to the list of data sources being rendered on the screen.
class PYXLIB_DECL ProcessListEvent : public NotifierEvent
{
public:

	//! The action that was performed on the specified guid
	enum eAction
	{
		knAdded,		//<! A single source was added to the list
		knRemoved,		//<! A single source was removed from the list
		knChanged		//<! The list was fundamentally changed.
	};

	//! Create a new instance of the event for a particular process and type.
	static PYXPointer< ProcessListEvent > create(ProcRef procRef, eAction nAction)
	{
		return PYXNEW(ProcessListEvent, procRef, nAction);
	}

	//! Create an event that indicates the entire list is invaidated. 
	static PYXPointer< ProcessListEvent > createChangedEvent()
	{
		return PYXNEW(ProcessListEvent, knChanged);
	}

	//! Default constructor.
	~ProcessListEvent() {}

	//! Return the unique identifier for the data process in question. 
	ProcRef getProcRef() {return m_procRef;}

	//! Return the action that is being performed on the process
	eAction getAction() {return m_nAction;}

private:

	//! Hide constructor from general use.
	explicit ProcessListEvent(ProcRef procRef, eAction nAction) :
		m_procRef(procRef),
		m_nAction(nAction) {}

	//! Hide constructor from general use.
	explicit ProcessListEvent(eAction nAction) :
		m_nAction(nAction) {}

	//! The unique identifier for the data source in question.
	ProcRef m_procRef;

	//! The action being taken on the data source in question.
	eAction m_nAction;
};

#endif
