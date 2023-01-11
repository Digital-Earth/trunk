#ifndef DOCUMENT_PROCESS_H
#define DOCUMENT_PROCESS_H
/******************************************************************************
document_process.h

begin      : 3/1/2007 12:00:00 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#pragma once

// local includes
#include "StdAfx.h"
#include "application.h"
#include "document.h"

// pyxlib includes
#include "pyxis/data/coverage.h"
#include "pyxis/pipe/pipe_manager.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/notifier.h"
#include "pyxis/utility/pyxcom.h"
#include "pyxis/utility/pyxcom_private.h"

//! Encapsulates all of the data that the app needs to run. 
class APPLICATION_API DocumentProcess : public ProcessImpl<DocumentProcess>, public IDocument
{
	PYXCOM_DECLARE_CLASS();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IDocument)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST( DocumentProcess, IProcess);

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	//! Get the output of this process.
	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IDocument*>(this);
	}

	//! Get the output of this process.
	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IDocument*>(this);
	}

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // IDocumentProcess

	//! Determine if there are unsaved changes in the document.
	virtual bool STDMETHODCALLTYPE isOutOfDate() const 
	{
		return m_bDirtyFlag;
	}

	//! Get the camera.
	virtual std::string STDMETHODCALLTYPE getCameraCookieString() const
	{
		return m_strCamCookie;
	}

	//! Sets the camera.
	virtual void STDMETHODCALLTYPE setCameraCookieString(const std::string& str)
	{
		m_strCamCookie = str;
		m_bDirtyFlag = true;
	}

	//! Save the document.
	virtual void STDMETHODCALLTYPE save(const std::string& path);

	//! Set the ViewPointProcess to be saved.
	virtual void STDMETHODCALLTYPE setViewPointProcess(boost::intrusive_ptr<IProcess> spViewPointProcess);
	
	//! Get the current the ViewPointProcess.
	virtual boost::intrusive_ptr<IProcess> STDMETHODCALLTYPE getViewPointProcess();

	//! Set the document's dirty flag.
	virtual	void STDMETHODCALLTYPE setDirtyFlag(bool bValue)
	{
		m_bDirtyFlag = bValue;
	}

public:

	//! Constants.
	static const std::string kstrDefaultDescription;
	static const std::string kstrDefaultName;
	static const std::string kstrDocExt;
	static const std::string kstrProcsDir;
	static const std::string kstrCameraAttrib;
	static const std::string kstrVisualPipeAttrib;

	//! Test method.
	static void test();
	
	//! Default constructor.
	DocumentProcess(void);

	//! Destructor.
    ~DocumentProcess(void);

private:
	
	//! Receive a notification when an observed object sends out a notification.
	void observedChanged(PYXPointer<NotifierEvent> spEvent)
	{
		m_bDirtyFlag = true;
	}

	//! The camera cookie string.
	std::string m_strCamCookie;

	//! Indicates if the document has been changed since last save.
	bool m_bDirtyFlag;

private:

	//! Copy constructor.
	DocumentProcess::DocumentProcess(const DocumentProcess &c)
	{}
};

#endif /* DOCUMENT_PROCESS_H */