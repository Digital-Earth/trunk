/******************************************************************************
process_list.cpp

begin      : 09/10/2007 4:35:32 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "process_list.h"

#include "exceptions.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/utility/exception.h"

// boost includes
#include <boost/thread/recursive_mutex.hpp>

/*!
Constructor. Creates an empty default process list.
*/
ProcessList::ProcessList() :
	m_mutex(),
	m_procs(),
	m_listChangedNotifier("Process List Changed")
{
}

/*!
Determine if a particular process is being visualized.

\param procref	The unique identifier for a process.

\return true if the process is contained in the manager.
*/
bool ProcessList::contains(const ProcRef& procref) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	return m_procs.find(procref) != m_procs.end();
}

/*!
Add a process to the list by procref. When this method is called the procref will
first be resolved into an actual process and then added to the list. The process
will only be added to the list if it is unique.

\param procref	The process reference for the process to add to the list.
*/
void ProcessList::addProc(const ProcRef& procref)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	// verify the process does not already exist.
	if (contains(procref))
	{
		PYXTHROW(PYXException, 
			"Can't add process '" << procref << "'. Process already exists in the list.");
	}

	// get the process
	boost::intrusive_ptr<IProcess> spProc = PipeManager::getProcess(procref);
	if (!spProc)
	{
		PYXTHROW(ProcessListException,
			"Can not add process " << procRefToStr(procref) << " to process list");
	}

	addProc(spProc);
}

/*!
Add a unique process to the list of processes.
*/
void ProcessList::addProc(boost::intrusive_ptr<IProcess> spProc)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	if (!addProcessToMap(spProc))
	{
		PYXTHROW(ProcessListException, "Can not add process " << spProc->getProcName() << " to process list");
	}

	PYXPointer<ProcessListEvent> spEvent = 
		ProcessListEvent::create(
			ProcRef(spProc), 
			ProcessListEvent::knAdded);
	m_listChangedNotifier.notify(spEvent);
}

/*!
Internal method to perform the work of adding the process to the map. This
method is not locked against multi threaded access and does not perform
any notifications. It is simply a worker method.

\pre This must be called within a lock on m_mutex.

\param spProc	The process to add to the list of data sources. Must be unique.

\return true if the process was added otherwise false.
*/
bool ProcessList::addProcessToMap(boost::intrusive_ptr<IProcess> spProc)
{
	try
	{
		ProcRef procref(spProc);

		// verify the process does not already exist.
		if (contains(procref))
		{
			PYXTHROW(PYXException, 
				"Can't add process '" << procref << "'. Process already in the process list.");
		}

		m_procs[procref] = spProc;

		return true;
	}
	catch(...)
	{
		TRACE_ERROR("An error occurred while adding proc '" <<
					spProc->getProcID() << 
					"' to the process list.");
	}
	return false;
}

/*!
Remove an existing process from the list. A notification of the change is
sent to all observers. Throws an exception if the process could not be removed.

\param procref	The process reference for the process to remove from the manager.
*/
void ProcessList::removeProc(const ProcRef& procref) 
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	if (!contains(procref))
	{
		PYXTHROW(ProcessListException, "Can not remove process " << 
			procRefToStr(procref) << " from process list because it could not "
			"be found in the list.");
	}
	m_procs.erase(procref);

	PYXPointer<ProcessListEvent> spEvent = 
		ProcessListEvent::create(
			procref, 
			ProcessListEvent::knRemoved);
	m_listChangedNotifier.notify(spEvent);
}

void ProcessList::clear()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	m_procs.clear();

	// notify the change
	PYXPointer<ProcessListEvent> spEvent = 
		ProcessListEvent::createChangedEvent();
	m_listChangedNotifier.notify(spEvent);
}

int ProcessList::count() 
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return static_cast<int>(m_procs.size());
}

void ProcessList::getProcesses(std::vector< boost::intrusive_ptr<IProcess> > & vecProcs)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	for (std::map< ProcRef, boost::intrusive_ptr<IProcess> >::const_iterator it = 
			m_procs.begin();
		it != m_procs.end();
		++it)
	{
		vecProcs.push_back(it->second);
	}
}

/*!
Return a copy of the the list of processes.

\return The copy of the list of processes.
*/
void ProcessList::getProcessRefs(std::vector<ProcRef> & vecProcs)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	for (std::map< ProcRef, boost::intrusive_ptr<IProcess> >::const_iterator it = 
			m_procs.begin();
		it != m_procs.end();
		++it)
	{
		vecProcs.push_back(it->first);
	}
}

/*!
Replace the entire list of processes in the list with a new one. A single 
notification is sent after the list is replaced.

*/
void ProcessList::setProcesses(const std::vector<boost::intrusive_ptr<IProcess> >& vecProcs)
{
	// lock must be kept until after the notification is complete.
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	// clear the current process map.
	m_procs.clear();

	// insert each of the new processes into the map
	for (std::vector<boost::intrusive_ptr<IProcess> >::const_iterator it =
			vecProcs.begin();
		it != vecProcs.end();
		++it)
	{
		addProcessToMap(*it);
	}

	// notify the change
	PYXPointer<ProcessListEvent> spEvent = 
		ProcessListEvent::createChangedEvent();
	m_listChangedNotifier.notify(spEvent);
}
