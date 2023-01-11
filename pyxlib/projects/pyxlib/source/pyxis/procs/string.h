#ifndef PYXIS__PROCS__STRING_H
#define PYXIS__PROCS__STRING_H
/******************************************************************************
string.h

begin		: 2007-04-13
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxlib.h"

// pyxlib includes
#include "pyxis/pipe/process.h"

// standard includes
#include <string>

//! Interface for strings.
struct PYXLIB_DECL IString : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	virtual const std::string& STDMETHODCALLTYPE str() = 0;
	virtual void STDMETHODCALLTYPE str(const std::string& str) = 0;
};

/*!
*/
//! A process for strings.
class PYXLIB_DECL StringProc : public ProcessImpl<StringProc>, public IString
{
	PYXCOM_DECLARE_CLASS();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IString)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IString*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IString*>(this);
	}

public: // IString

	virtual void STDMETHODCALLTYPE str(const std::string& str)
	{
		setData(str);
	}

	virtual const std::string& STDMETHODCALLTYPE str()
	{
		return m_strData;
	}
};

#endif // guard
