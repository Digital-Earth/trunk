#ifndef PYXIS__PIPE__PROCESS_INIT_ERROR_H
#define PYXIS__PIPE__PROCESS_INIT_ERROR_H
/******************************************************************************
process_init_error.h

begin      : 25/04/2008 10:56:27 AM
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "pyxis/utility/pyxcom.h"

//! Interface for errors that occur during process initialization.
struct PYXLIB_DECL IProcessInitError : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//! Return the description of the error.
	virtual const std::string& STDMETHODCALLTYPE getError() const = 0;

	//! Set the description of the error.
	virtual void STDMETHODCALLTYPE setError(const std::string& strError) = 0;
	
	//! Return the unique id for the error class
	virtual std::string STDMETHODCALLTYPE getErrorID() const = 0;

};

class PYXLIB_DECL ProcInitErrorBase : public IProcessInitError
{
// IProcessInitError
public:

	ProcInitErrorBase() : 
		m_strError("Error string not specified.") {};

	virtual const std::string& STDMETHODCALLTYPE getError() const
	{
		return m_strError;
	}
	virtual void STDMETHODCALLTYPE setError(const std::string& strError) {m_strError = strError;}

protected:

	// The descriptive error string for the error
	std::string m_strError;

};

// macro to define the getErrorID method
#define GET_ERRORID_IMPL(TYPE) \
	inline virtual std::string STDMETHODCALLTYPE getErrorID() const \
	{ \
		return guidToStr(TYPE::clsid); \
	};

//! A general initialization error with a process.
class PYXLIB_DECL GenericProcInitError : public ProcInitErrorBase
{
	PYXCOM_DECLARE_CLASS();

public:
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcessInitError)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();
	GET_ERRORID_IMPL(GenericProcInitError);

	GenericProcInitError()
	{
		m_strError = "Generic initialization error";
	}
};

//! An error that indicates one of the input parameters could not be successfully initialized.
class PYXLIB_DECL InputInitError : public ProcInitErrorBase
{
	PYXCOM_DECLARE_CLASS();

public:
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcessInitError)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();
	GET_ERRORID_IMPL(InputInitError)

	InputInitError()
	{
		m_strError = "One or more input processes could not be initialized.";
	}
};

//! An error that indicates the process does not meet its own specification.
class PYXLIB_DECL ProcSpecFailure : public ProcInitErrorBase
{
	PYXCOM_DECLARE_CLASS();

public:
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcessInitError)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();
	GET_ERRORID_IMPL(ProcSpecFailure)

	ProcSpecFailure()
	{
		m_strError = "This process is missing one or more required input processes.";
	}
};

#endif