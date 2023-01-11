/******************************************************************************
url.cpp

begin      : 15/02/2008 10:01:38 AM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/procs/url.h"

// pyxlib includes
#include "pyxis/procs/exceptions.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/xml_transform.h"

// boost includes
#include <boost/regex.hpp>

// {6C95205F-3E8E-4849-AC44-3FAA7B3E3246}
PYXCOM_DEFINE_IID(IUrl, 
0x6c95205f, 0x3e8e, 0x4849, 0xac, 0x44, 0x3f, 0xaa, 0x7b, 0x3e, 0x32, 0x46);

// {1E452AC4-2E27-455b-A156-D9C83F9115F3}
PYXCOM_DEFINE_CLSID(URLInitError, 
0x1e452ac4, 0x2e27, 0x455b, 0xa1, 0x56, 0xd9, 0xc8, 0x3f, 0x91, 0x15, 0xf3);
PYXCOM_CLASS_INTERFACES(URLInitError, IProcessInitError::iid, PYXCOM_IUnknown::iid);

// {BB6C61B0-67E2-4072-8A6C-D24AE68F5B0E}
PYXCOM_DEFINE_CLSID(UrlProcess,
0xbb6c61b0, 0x67e2, 0x4072, 0x8a, 0x6c, 0xd2, 0x4a, 0xe6, 0x8f, 0x5b, 0xe);

PYXCOM_CLASS_INTERFACES(UrlProcess, IUrl::iid, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(UrlProcess, "File Link", "A uniform resource locator (URL) for web locations.", "Reader",
					IUrl::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END

const std::string UrlProcess::kStrUrlKey = "urlString";

//! Tester Class
Tester<UrlProcess> gTester;
void UrlProcess::test()
{
	{ // Test an empty URL string
		boost::intrusive_ptr<UrlProcess> spProc(new UrlProcess);
		std::map<std::string, std::string> mapAttr;
		mapAttr.clear();
		mapAttr[kStrUrlKey] = "";
		spProc->setAttributes(mapAttr);
		TEST_ASSERT(spProc->initProc() == IProcess::knFailedToInit);
	}

	{ // Test a bad URL
		boost::intrusive_ptr<UrlProcess> spProc(new UrlProcess);
		std::map<std::string, std::string> mapAttr;
		mapAttr.clear();
		mapAttr[kStrUrlKey] = "http://www. pyxisinnovation.com";
		spProc->setAttributes(mapAttr);
		TEST_ASSERT(spProc->initProc() == IProcess::knFailedToInit);
	}

	{// a good url
		boost::intrusive_ptr<UrlProcess> spProc(new UrlProcess);
		std::map<std::string, std::string> mapAttr;
		mapAttr.clear();
		mapAttr[kStrUrlKey] = "http://www.pyxisinnovation";
		spProc->setAttributes(mapAttr);
		TEST_ASSERT(spProc->initProc() == IProcess::knInitialized);
	}

	{ //Test a good url
		bool bExceptionOccured = false;
		boost::intrusive_ptr<UrlProcess> spProc(new UrlProcess);
		std::map<std::string, std::string> mapAttr;
		mapAttr.clear();
		mapAttr[kStrUrlKey] = "http://www.pyxisinnovation.com";
		try
		{
			spProc->setAttributes(mapAttr);
			TEST_ASSERT(spProc->initProc() == IProcess::knInitialized);
		}
		catch(PYXException&)
		{
			bExceptionOccured = true;
		}

		TEST_ASSERT(!bExceptionOccured);
	}

	{	//Test when initProc called without setting attributes.
		boost::intrusive_ptr<UrlProcess> spProc(new UrlProcess);
		TEST_ASSERT(spProc->initProc() == IProcess::knFailedToInit);
	}

	{ //Test a FTP URL
		bool bExceptionOccured = false;
		boost::intrusive_ptr<UrlProcess> spProc(new UrlProcess);
		std::map<std::string, std::string> mapAttr;
		mapAttr.clear();
		mapAttr[kStrUrlKey] = "ftp://www.pyxisinnovation.com";
		try
		{
			spProc->setAttributes(mapAttr);
			spProc->initProc();
		}
		catch(PYXException&)
		{
			bExceptionOccured = true;
		}

		TEST_ASSERT(!bExceptionOccured);
	}

	{ //Test a URL prefixed with https
		bool bExceptionOccured = false;
		boost::intrusive_ptr<UrlProcess> spProc(new UrlProcess);
		std::map<std::string, std::string> mapAttr;
		mapAttr.clear();
		mapAttr[kStrUrlKey] = "https://www.pyxisinnovation.com/pyxinternalwiki/index.php?title=Main_Page";
		try
		{
			spProc->setAttributes(mapAttr);
			spProc->initProc();
		}
		catch(PYXException&)
		{
			bExceptionOccured = true;
		}

		TEST_ASSERT(!bExceptionOccured);
	}

	{ //Test that getURL returns what is expected
		std::string strUrl = "www.pyxisinnovation.com";
		boost::intrusive_ptr<UrlProcess> spProc(new UrlProcess);
		std::map<std::string, std::string> mapAttr;
		mapAttr.clear();
		mapAttr[kStrUrlKey] = strUrl;
		spProc->setAttributes(mapAttr);
		std::string strExpected = "www.pyxisinnovation.com";
		std::string strRt = spProc->getUrl();
		TEST_ASSERT(spProc->getUrl().compare(strExpected) == 0);
	}

	{ //Test a good url
		bool bExceptionOccured = false;
		boost::intrusive_ptr<UrlProcess> spProc(new UrlProcess);
		std::map<std::string, std::string> mapAttr;
		mapAttr.clear();
		mapAttr[kStrUrlKey] = "http://www.ctv.ca/servlet/ArticleNews/story/CTVNews/20080225/south_korea_080225/20080225?hub=World";;
		try
		{
			spProc->setAttributes(mapAttr);
			spProc->initProc();
		}
		catch(PYXException&)
		{
			bExceptionOccured = true;
		}

		TEST_ASSERT(!bExceptionOccured);
	}


#if 0
	{ //Test a URL that connects on ports
		bool bExceptionOccured = false;
		boost::intrusive_ptr<UrlProcess> spProc(new UrlProcess);
		std::map<std::string, std::string> mapAttr;
		mapAttr.clear();
		mapAttr["UrlString"] = "http://Euclid:9000/WorldView";
		try
		{
			spProc->setAttributes(mapAttr);
		}
		catch(PYXException&)
		{
			bExceptionOccured = true;
		}

		TEST_ASSERT(!bExceptionOccured);
	}
#endif
}

/*!
Default Constructor, initalizes the url to an empty string.
*/
UrlProcess::UrlProcess()
{
}

/*!
Intializes the process, if the url is a malformed or otherwise invalid url string then a
proc intialization exception is thrown.
*/
IProcess::eInitStatus UrlProcess::initImpl()
{
	if (!isValidUrl(m_strUrl))
	{
		setInitProcError<URLInitError>("The url: " + m_strUrl + " is malformed");
		return knFailedToInit;
	}
	return knInitialized;
}

std::string UrlProcess::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"UrlProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"" + kStrUrlKey + "\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>URL String</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

/*!
Gets a map mapping the url string attribute to a well known key.

\return a std map, mappin the urlKey to urlString.
*/
std::map<std::string, std::string> UrlProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	mapAttr.clear();
	mapAttr[kStrUrlKey] = m_strUrl;
	return mapAttr;
}

/*!
Sets the attribute for this process. When the attribute is set we attempt to validate
if the url is a well formed url via isValidUrl.

\param mapAttr	A standard map containing the attributes to set.
*/
void UrlProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::map<std::string, std::string>::const_iterator it = mapAttr.find(kStrUrlKey);

	if (it != mapAttr.end())
	{
		m_strUrl = it->second;
	}
}

/*!
Attempts to validate if the url is a well formed url by first checking to see if the 
url string that is being validated is empty. If it is not empty then try to validate 
by using the C# validator.

\param strUrl  The url string to attempt to validate if it is a well formed url.

\return a boolean flag indicating the validity of a well formed url string or not.
*/
bool UrlProcess::isValidUrl(const std::string& strUrl) const
{
	if (strUrl.empty())
	{
		return false;
	}
	return CSharpFunctionProvider::getCSharpFunctionProvider()->isWellFormedURI(strUrl);
}

//! Return the mainifest if we can.
std::string UrlProcess::getManifest() const
{
	if (isLocalFile())
	{
		std::string filespec = m_strUrl;
		if (filespec.find("file:///") == 0)
			filespec = filespec.substr( 8);
		return CSharpFunctionProvider::getCSharpFunctionProvider()->getSerializedManifestForFile(filespec);
	}
	return "";
}

//! Return true if the URL refers to a local file.
bool UrlProcess::isLocalFile() const
{
	// If the URL starts with the local file reference qualifier, then
	// we are indeed a local file.
	return (m_strUrl.find("file:///") == 0);
}
