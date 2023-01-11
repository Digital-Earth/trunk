#ifndef PYXIS__PROCS__TOOL_BOX_PROVIDER_H
#define PYXIS__PROCS__TOOL_BOX_PROVIDER_H
/******************************************************************************
tool_box_provider.h

begin		: Sep 04, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyxlib.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/utility/object.h"
#include "pyxis/procs/process_collection_proc.h"

/*!

IToolBoxProvider - interface to allow process to export additional process to the pipeline editor

the pipeline editor will export the additional process in the following way:
1. generate a new catagory with the IToolBoxProvider process name.
2. add all process inside the process list.

*/
//! IToolBoxProvider - interface to allow process to hold embedded resource inside iself.
struct PYXLIB_DECL IToolBoxProvider : public IProcessCollection
{
	PYXCOM_DECLARE_INTERFACE();
};

/*!
*/
//! A process that represents and arbitrary grouping of processes
class PYXLIB_DECL ToolBoxProviderProc : 
	public ProcessImpl<ToolBoxProviderProc>, public IToolBoxProvider
{
	PYXCOM_DECLARE_CLASS();

public:

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IProcessCollection)
		IUNKNOWN_QI_CASE(IToolBoxProvider)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IToolBoxProvider*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IToolBoxProvider*>(this);
	}

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // IProcessCollection

	//! Get the list of processes
	virtual PYXPointer<ProcessList> STDMETHODCALLTYPE getProcessList() const
	{
		return m_spProcList;
	}

	//! Replace the list of processes with a new list.
	virtual void STDMETHODCALLTYPE setProcessList(PYXPointer<ProcessList> spProcList);

public: // IToolBoxProvider


private:

	//! The storage of all of the associated processes.
	PYXPointer<ProcessList> m_spProcList;
};



#endif // guard
