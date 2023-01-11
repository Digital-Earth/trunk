/******************************************************************************
http_utils.cpp

begin		: 2011-09-06
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/http_utils.h"

#include "pyxis/utility/trace.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"



PYXPointer<HttpRequestProvider> HttpRequestProvider::m_spProvider;

PYXPointer<HttpRequestProvider> HttpRequestProvider::getHttpRequestProvider()
{
	return m_spProvider;
}

void HttpRequestProvider::setHttpRequestProvider( PYXPointer<HttpRequestProvider> spProvider)
{
	m_spProvider = spProvider;
}

int HttpRequestProvider::createRequest(const std::string & url,const std::string & method)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setHttpRequestProvider()");
	return 0;
}

int HttpRequestProvider::createRequest(const std::string & url,const std::string & method,boost::intrusive_ptr<IUserCredentials> credentials)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setHttpRequestProvider()");
	return 0;
}


void HttpRequestProvider::destroyRequest(const int & requestHandle)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setHttpRequestProvider()");
}

//add body to the request, good for post method.
void HttpRequestProvider::addRequestBody(const int & requestHandle, const std::string & body)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setHttpRequestProvider()");
}

//add headers to the request.
void HttpRequestProvider::addRequestHeader(const int & requestHandle, const std::string & headerName,const std::string & headerValue)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setHttpRequestProvider()");
}

//return true if response returned succufuly.
bool HttpRequestProvider::getResponse(const int & requestHandle)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setHttpRequestProvider()");
}

//get the response body.
std::string HttpRequestProvider::getResponseBody(const int & requestHandle)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setHttpRequestProvider()");
}

//save the response body into a local file.
bool HttpRequestProvider::downloadResponse(const int & requestHandle,const std::string & filename)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setHttpRequestProvider()");
}




//! Tester Class
Tester<HttpRequest> gTester;

//! Test Method 
void HttpRequest::test()
{
	PYXPointer<HttpRequest> request = HttpRequest::create("http://www.google.com","get");

	TEST_ASSERT(request->getResponse());

	//request = HttpRequest::create("http://www.pyxisinnovation.com/images/pyxis-logo-tm.gif","get");
	request = HttpRequest::create("http://www.google.ca/images/srpr/logo3w.png","get");

	boost::filesystem::path tempFile = AppServices::makeTempFile(".gif");

	TEST_ASSERT(request->downloadResponse(FileUtils::pathToString(tempFile)));
}


HttpRequest::HttpRequest(const std::string & url,const std::string & method)
{
	m_handle = HttpRequestProvider::getHttpRequestProvider()->createRequest(url,method);
}

HttpRequest::HttpRequest(const std::string & url,const std::string & method,boost::intrusive_ptr<IUserCredentials> credentials)
{
	m_handle = HttpRequestProvider::getHttpRequestProvider()->createRequest(url,method,credentials);
}

HttpRequest::~HttpRequest()
{
	HttpRequestProvider::getHttpRequestProvider()->destroyRequest(m_handle);
}


//add body to the request, good for post method.
void HttpRequest::addRequestBody(const std::string & body)
{
	HttpRequestProvider::getHttpRequestProvider()->addRequestBody(m_handle,body);
}

//add headers to the request.
void HttpRequest::addRequestHeader(const std::string & headerName,const std::string & headerValue)
{
	HttpRequestProvider::getHttpRequestProvider()->addRequestHeader(m_handle,headerName,headerValue);
}

//return true if response returned succufuly.
bool HttpRequest::getResponse()
{
	return HttpRequestProvider::getHttpRequestProvider()->getResponse(m_handle);
}

//get the response body.
std::string HttpRequest::getResponseBody()
{
	return HttpRequestProvider::getHttpRequestProvider()->getResponseBody(m_handle);
}

//save the response body into a local file.
bool HttpRequest::downloadResponse(const std::string & filename)
{
	return HttpRequestProvider::getHttpRequestProvider()->downloadResponse(m_handle,filename);
}
