#ifndef URL_H
#define URL_H
/******************************************************************************
url.h

begin      : 15/02/2008 10:01:38 AM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "pyxlib.h"

// pyxlib includes
#include "pyxis/pipe/process.h"

//! Interface to Urls.
struct PYXLIB_DECL IUrl : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//! Gets the URL.
	virtual const std::string STDMETHODCALLTYPE getUrl() const = 0;

	//! Sets the URL
	virtual bool STDMETHODCALLTYPE setUrl(const std::string& url) = 0;

	/*! Return the serialized manifest of the files for this process.
	this call is only valid for local files and will only return a result
	if the checksum of all the files has already been calculated.
	*/
	virtual std::string STDMETHODCALLTYPE getManifest() const = 0;

	//! Return true if the URL refers to a local file.
	virtual bool STDMETHODCALLTYPE isLocalFile() const = 0;
};

/*
Url process, which implements IUrl, similar to the path process, used for plugging into processes
which take a url instead of a local file or directory. This process validates that the url it
represents is a well formed url.
*/
//! A process wich represents a url.
class PYXLIB_DECL UrlProcess : public ProcessImpl<UrlProcess>, public IUrl
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constants
	static const std::string kStrUrlKey;
	
	//! Test method.
	static void test();
	
	//! Default Constructor.
	UrlProcess();

	//! Destructor.
	~UrlProcess(){;}

public:

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IUrl)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST(UrlProcess, IProcess);

public: //IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IUrl*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IUrl*>(this);
	}

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: //IUrl

	virtual const std::string STDMETHODCALLTYPE getUrl() const
	{
		return m_strUrl;
	}

	virtual bool STDMETHODCALLTYPE setUrl(const std::string& url)
	{
		if (isValidUrl(url))
		{
			m_strUrl = url;
			return true;
		}
		return false;
	}

	virtual std::string STDMETHODCALLTYPE getManifest() const;

	//! Return true if the URL refers to a local file.
	virtual bool STDMETHODCALLTYPE isLocalFile() const;

private:

	//! Validates that the url received as an attribute is a well formed url.
	bool isValidUrl(const std::string& strUrl) const;

	//! The url string.
	mutable std::string m_strUrl;

};

//! An error that indicates one of the input parameters could not be successfully initialized.
class PYXLIB_DECL URLInitError : public GenericProcInitError
{
	PYXCOM_DECLARE_CLASS();

public:
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcessInitError)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

	URLInitError()
	{
		m_strError = "URL initialization error";
	}
};

#endif