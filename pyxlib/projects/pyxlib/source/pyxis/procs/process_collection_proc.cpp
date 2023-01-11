/******************************************************************************
process_collection_proc.cpp

begin      : 19/06/2008 4:30:20 PM
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"

#include "process_collection_proc.h"

// pyxlib includes
#include "pyxis/utility/string_utils.h"

// {9EE2F5F8-8F64-4b8a-945A-FB55C801E6DB}
PYXCOM_DEFINE_IID(IProcessCollection, 
0x9ee2f5f8, 0x8f64, 0x4b8a, 0x94, 0x5a, 0xfb, 0x55, 0xc8, 0x1, 0xe6, 0xdb);

// {F9C16F11-D897-4ce2-A6EC-70E286F0D194}
PYXCOM_DEFINE_CLSID(ProcessCollectionProc, 
0xf9c16f11, 0xd897, 0x4ce2, 0xa6, 0xec, 0x70, 0xe2, 0x86, 0xf0, 0xd1, 0x94);
PYXCOM_CLASS_INTERFACES(ProcessCollectionProc, IProcess::iid, IProcessCollection::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(ProcessCollectionProc, "Process Collection", "An arbitrary grouping of processes.", "Hidden",
					IProcessCollection::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(PYXCOM_IUnknown::iid, 0, -1, "Input Process(es)", "Any Process to be associated with the collection.")
IPROCESS_SPEC_END


////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

/*!
Pull all of the parameters out and store them as processes in a separate list.
*/
IProcess::eInitStatus ProcessCollectionProc::initImpl()
{
	// create a new list
	m_spProcList = ProcessList::create();

	std::vector<boost::intrusive_ptr<IProcess>> vecProcs = 
		getParameter(0)->getValues();
	
	std::vector<boost::intrusive_ptr<IProcess>>::iterator it =
		vecProcs.begin();
	for (; it != vecProcs.end(); ++it)
	{
		m_spProcList->addProc(*it);
	}

	return IProcess::knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

/*!
Clear out the input process parameters and replace them with the items in the list
*/
void STDMETHODCALLTYPE ProcessCollectionProc::setProcessList(
	PYXPointer<ProcessList> spProcList)
{
	if (!spProcList)
	{
		PYXTHROW(PYXException, "Can't set the process list with a null");
	}
	m_spProcList = spProcList;
	std::vector<boost::intrusive_ptr<IProcess> > vecProcesses;
	m_spProcList->getProcesses(vecProcesses);
	getParameter(0)->setValues(vecProcesses);
}

