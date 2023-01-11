#ifndef PYXIS__UTILITY__HTTP_UTILS_H
#define PYXIS__UTILITY__HTTP_UTILS_H
/******************************************************************************
http_utils.h

begin		: 2011-09-06
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/procs/path.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/procs/user_credentials_provider.h"

// standard includes
#include <string>
#include <vector>

class PYXLIB_DECL HttpRequestProvider : public PYXObject
{
public:	
	virtual int createRequest(const std::string & url,const std::string & method);

	virtual int createRequest(const std::string & url,const std::string & method ,boost::intrusive_ptr<IUserCredentials> credentials);


	virtual void destroyRequest(const int & requestHandle);

	//return true if response returned succufuly.
	virtual bool getResponse(const int & requestHandle);

	//get the response body.
	virtual std::string getResponseBody(const int & requestHandle);

	//save the response body into a local file.
	virtual bool downloadResponse(const int & requestHandle,const std::string & filename);

	//add body to the request, good for post method.
	virtual void addRequestBody(const int & requestHandle,const std::string & body);

	//add headers to the request.
	virtual void addRequestHeader(const int & requestHandle,const std::string & headerName,const std::string & headerValue);

	// SWIG doesn't know about addRef and release, since they are defined in 
	// the opaque PYXObject.  Add them here so they get director'ed.
public:

	virtual long release() const
	{
		return PYXObject::release();
	}

	virtual long addRef() const
	{
		return PYXObject::addRef();
	}

public:

	//! Virtual destructor.
	virtual ~HttpRequestProvider()
	{}

private:

	static PYXPointer<HttpRequestProvider> m_spProvider;

public:

	static PYXPointer<HttpRequestProvider> getHttpRequestProvider();
	static void setHttpRequestProvider( PYXPointer<HttpRequestProvider> spProvider);
};


/*!

A simple wapper around .Net WebRequest.

you can use this wrapper to make Http requests.

*/
class PYXLIB_DECL HttpRequest : public PYXObject
{
protected:
	HttpRequest(const std::string & url,const std::string & method);
	HttpRequest(const std::string & url,const std::string & method,boost::intrusive_ptr<IUserCredentials> credentials);

public:
	static PYXPointer<HttpRequest> create(const std::string & url,const std::string & method)
	{
		return PYXNEW(HttpRequest,url,method);
	}
	static PYXPointer<HttpRequest> create(const std::string & url,const std::string & method,boost::intrusive_ptr<IUserCredentials> credentials)
	{
		return PYXNEW(HttpRequest,url,method,credentials);
	}

	virtual ~HttpRequest();

public:
	//add body to the request, good for post method.
	void addRequestBody(const std::string & body);

	//add headers to the request.
	void addRequestHeader(const std::string & headerName,const std::string & headerValue);

	//return true if response returned succufuly.
	bool getResponse();

	//get the response body.
	std::string getResponseBody();

	//save the response body into a local file.
	bool downloadResponse(const std::string & filename);

protected:
	int m_handle;

public:
	//! UnitTest Method
	static void test();
};

class PYXLIB_DECL HttpUtils
{
	static PYXPointer<HttpRequest> getRequest(const std::string & uri) {
		return HttpRequest::create(uri,"get");
	}

	static PYXPointer<HttpRequest> postRequest(const std::string & uri) {
		return HttpRequest::create(uri,"post");
	}
};

#endif // guard
