/******************************************************************************
tool_box_provider.cpp

begin		: Sep 04, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "tool_box_provider.h"

// {A6050EFF-4C46-4c84-8B1A-BF7B3C136713}
PYXCOM_DEFINE_IID(IToolBoxProvider, 
0xa6050eff, 0x4c46, 0x4c84, 0x8b, 0x1a, 0xbf, 0x7b, 0x3c, 0x13, 0x67, 0x13);


// {D7D6DDE7-E439-46b9-A075-0A16FF9DC64F}
PYXCOM_DEFINE_CLSID(ToolBoxProviderProc, 
0xd7d6dde7, 0xe439, 0x46b9, 0xa0, 0x75, 0xa, 0x16, 0xff, 0x9d, 0xc6, 0x4f);
PYXCOM_CLASS_INTERFACES(ToolBoxProviderProc, IProcess::iid, IToolBoxProvider::iid, IProcessCollection::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(ToolBoxProviderProc, "Tool Box Provider", "A set of processes to be used at the Pipeline Editor.", "Hidden",
					IToolBoxProvider::iid, IProcessCollection::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(PYXCOM_IUnknown::iid, 0, -1, "Input Process(es)", "Any Process to be associated with the collection.")
IPROCESS_SPEC_END


////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

/*!
Pull all of the parameters out and store them as processes in a separate list.
*/
IProcess::eInitStatus ToolBoxProviderProc::initImpl()
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
void STDMETHODCALLTYPE ToolBoxProviderProc::setProcessList(
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

