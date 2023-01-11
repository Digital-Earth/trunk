/******************************************************************************
process_collection_proc.h

begin      : 19/06/2008 4:30:18 PM
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#ifndef PROCESS_COLLECTION_PROC_H
#define PROCESS_COLLECTION_PROC_H

// local includes 
#include "pyxlib.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/pipe/process_list.h"
#include "pyxis/utility/pyxcom.h"

// boost includes

//! A process that represents an arbitrary group of processes
struct PYXLIB_DECL IProcessCollection : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//! Get the list of processes
	virtual PYXPointer<ProcessList> STDMETHODCALLTYPE getProcessList() const = 0;

	//! Replace the list of processes with a new list.
	virtual void STDMETHODCALLTYPE setProcessList(PYXPointer<ProcessList> spProcList) = 0;
};

/*!
*/
//! A process that represents and arbitrary grouping of processes
class PYXLIB_DECL ProcessCollectionProc : 
	public ProcessImpl<ProcessCollectionProc>, public IProcessCollection
{
	PYXCOM_DECLARE_CLASS();

public:

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IProcessCollection)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IProcessCollection*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IProcessCollection*>(this);
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

private:

	//! The storage of all of the associated processes.
	PYXPointer<ProcessList> m_spProcList;
};

#endif
