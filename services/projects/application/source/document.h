#pragma once
#ifndef DOCUMENT_H
#define DOCUMENT_H
/******************************************************************************
document.h

begin      : 4/10/2007 10:49:39 AM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "application.h"

//pyxlib includes.
#include "pyxis/pipe/process.h"
#include "pyxis/utility/pyxcom.h"
#include "pyxis/utility/pyxcom_private.h"

// boost inclues.
#include <boost/intrusive_ptr.hpp>

//! Encapsulates all of the data that the app needs to run.  Mostly wraps up the visualization and camera.
struct APPLICATION_API IDocument: public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public: // IDocumentProcess

	//! Set the ViewPointProcess that the document owns.
	virtual void STDMETHODCALLTYPE setViewPointProcess(boost::intrusive_ptr<IProcess> spViewPointProcess) = 0;

	//! Get the ViewPointProcess that the document owns.
	virtual boost::intrusive_ptr<IProcess> STDMETHODCALLTYPE getViewPointProcess() = 0;

	//! Get the document's camera definition. If none is set an empty string is returned.
	virtual std::string STDMETHODCALLTYPE getCameraCookieString() const = 0;

	//! Sets a document's camera.
	virtual void STDMETHODCALLTYPE setCameraCookieString(const std::string& str) = 0;

	//! Sets a document's dirty flag.
	virtual void STDMETHODCALLTYPE setDirtyFlag(bool bValue) = 0;

	//! Save the document.
	virtual void STDMETHODCALLTYPE save(const std::string& path) = 0;

	//! Determine if there are unsaved changes in the document.
	virtual bool STDMETHODCALLTYPE isOutOfDate() const = 0;
};

#endif // end guard