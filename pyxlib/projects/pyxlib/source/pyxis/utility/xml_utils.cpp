/******************************************************************************
xml_utils.cpp

begin		: 2007-02-08
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/xml_utils.h"

#include "pyxis/utility/trace.h"
#include "pyxis/utility/tester.h"

// xerces includes
#pragma warning(push)
#pragma warning(disable: 4244) // warning C4244: 'return' : conversion from '__w64 int' to 'unsigned long', possible loss of data
#pragma warning(disable: 4267) // warning C4267: 'argument' : conversion from 'size_t' to 'const unsigned int', possible loss of data
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/Base64.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#pragma warning(pop)

// boost includes
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string/replace.hpp>

// standard includes
#include <algorithm>
#include <iostream>
#include <sstream>
#include <utility>

XERCES_CPP_NAMESPACE_USE

namespace
{

//! UTF-8 transcoder.
XMLTranscoder* utf8Transcoder = 0;

//! Helper function.
bool isASCIIOnly(const XMLByte * pXMLBytes)
{
	while (*pXMLBytes++)
	{
		if (*pXMLBytes < 0 || CHAR_MAX < *pXMLBytes)
		{
			return false;
		}
	}
	return true;
}

}


//! Tester class
Tester<XMLUtils> gTester;

//! Test method
void XMLUtils::test()
{
	//unicode <--> utf8 coverstion

	//we assume the transcoder know how to convert. let just make sure the strings stay the same.

	std::string testUtf8("Hello world");

	std::wstring testUnicode(L"Hellow world");

	TEST_ASSERT(testUtf8 == XMLUtils::encodeUtf8(XMLUtils::encodeUnicode(testUtf8)));
	TEST_ASSERT(testUnicode == XMLUtils::encodeUnicode(XMLUtils::encodeUtf8(testUnicode)));

	//test empty strings
	TEST_ASSERT("" == XMLUtils::encodeUtf8(XMLUtils::encodeUnicode("")));
	TEST_ASSERT(L"" == XMLUtils::encodeUnicode(XMLUtils::encodeUtf8(L"")));


	// base 64 conversion

	std::string testBin;
	for(int i=1;i<400;i++)
	{
		testBin += (char)i;
	}

	std::string testBinResult = XMLUtils::fromBase64(XMLUtils::toBase64(testBin));
	TEST_ASSERT(testBin == testBinResult && testBin.size() == testBinResult.size());
	TEST_ASSERT("" == XMLUtils::fromBase64(XMLUtils::toBase64("")));
}


/*!
Must be called exactly once near program start.
*/
void XMLUtils::initialize()
{
	XMLPlatformUtils::Initialize(); // may throw

	// Create UTF-8 transcoder.
	// http://www.ibm.com/developerworks/xml/library/x-serial.html
	// http://www.nabble.com/Converting-XMLCh*-to-std::string-with-encoding-td13755245.html
	XMLTransService::Codes resCode;
	utf8Transcoder =
		XMLPlatformUtils::fgTransService->makeNewTranscoderFor("UTF-8", resCode, 16*1024);
}

/*!
Must be called exactly once near program end.
*/
void XMLUtils::uninitialize()
{
	delete utf8Transcoder;
	XMLPlatformUtils::Terminate();
}

/*!
\param attrs	The xerces attributes object.
\return A map of transcoded name strings to transcoded value strings.
*/
std::map<std::string, std::string> XMLUtils::getAttributes(const Attributes& attrs)
{
	std::map<std::string, std::string> mapAttr;

	std::vector<char> vecUTF8;

	for (int n = 0; n != attrs.getLength(); ++n)
	{
		const XMLCh* const nameUnicodeString = attrs.getLocalName(n);
		unsigned int nameUnicodeStringLength = XMLString::stringLen(nameUnicodeString);

		//should be enough...
		if (vecUTF8.size() < nameUnicodeStringLength*4)
		{
			vecUTF8.resize(nameUnicodeStringLength*4);
		}

		// Finally transcode to UTF-8.
		XMLSize_t nCharsEaten = 0;
		
		XMLSize_t nLen = utf8Transcoder->transcodeTo(
			nameUnicodeString,
			nameUnicodeStringLength,
			reinterpret_cast<XMLByte*>(&vecUTF8[0]),
			static_cast<unsigned int>(vecUTF8.size() - 1),
			nCharsEaten,
			XMLTranscoder::UnRep_Throw);

		assert(nCharsEaten == nameUnicodeStringLength);

		vecUTF8[nLen] = 0;
		std::string name(&vecUTF8[0],nLen);


		const XMLCh* const valueUnicodeString = attrs.getValue(n);
		unsigned int valueUnicodeStringLength  = XMLString::stringLen(valueUnicodeString);
		
		//should be enough...
		if (vecUTF8.size() < valueUnicodeStringLength*4)
		{
			vecUTF8.resize(valueUnicodeStringLength*4);
		}

		// Finally transcode to UTF-8.
		nCharsEaten = 0;
		
		nLen = utf8Transcoder->transcodeTo(
			valueUnicodeString,
			valueUnicodeStringLength,
			reinterpret_cast<XMLByte*>(&vecUTF8[0]),
			static_cast<unsigned int>(vecUTF8.size() - 1),
			nCharsEaten,
			XMLTranscoder::UnRep_Throw);

		assert(nCharsEaten == valueUnicodeStringLength);

		vecUTF8[nLen] = 0;
		std::string value(&vecUTF8[0],nLen);

		mapAttr.insert(std::make_pair(name, value));
	}

	return mapAttr;
}

/*!
\param in		The stream to parse.
\param handler	The handler to use.
\return True if parse completed, false if an exception occurred.
*/
bool XMLUtils::parse(std::istream& in, DefaultHandler& handler)
{
	// TODO xerces doesn't quite seem to support input streams yet so until then...

	// TODO is it better to use ostringstream with ostreambuf_iterator,
	// or a string with back_inserter? Which is faster or better?
	std::ostringstream out;
	std::copy(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>(), std::ostreambuf_iterator<char>(out));

	std::string str;
	str.swap(out.str());
	return parseFromString(str.c_str(), static_cast<int>(str.size()), handler);
}

/*!
\param strURI	The URI to parse.
\param handler	The handler to use.
\return True if parse completed, false if an exception occurred.
*/
bool XMLUtils::parseFromFile(const std::string& strURI, DefaultHandler& handler)
{
	boost::shared_ptr<SAX2XMLReader> spParser(XMLReaderFactory::createXMLReader());
	//spParser->setFeature(XMLUni::fgSAX2CoreValidation, true);   // optional
	//spParser->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);   // optional
	spParser->setContentHandler(&handler);
	spParser->setErrorHandler(&handler);

	try
	{
		spParser->parse(strURI.c_str());
	}
	catch (...)
	{
		TRACE_ERROR("Exception occurred during SAX2 parse of '" << strURI.c_str() << "'");
		return false;
	}

	return true;
}

/*!
\param pStr		The string to parse.
\param nStrSize	The string size.
\param handler	The handler to use.
\return True if parse completed, false if an exception occurred.
*/
bool XMLUtils::parseFromString(const char* pStr, int nStrSize, DefaultHandler& handler)
{
	assert(pStr);

	boost::shared_ptr<SAX2XMLReader> spParser(XMLReaderFactory::createXMLReader());
	//spParser->setFeature(XMLUni::fgSAX2CoreValidation, true);   // optional
	//spParser->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);   // optional
	spParser->setContentHandler(&handler);
	spParser->setErrorHandler(&handler);

	try
	{
		MemBufInputSource input((const XMLByte*)pStr, nStrSize, "character string");
		spParser->parse(input);
	}
	catch (PYXException ex)
	{
		TRACE_ERROR("Exception occurred during SAX2 parse : " << ex.getFullErrorString() );
		return false;
	}
	catch (...)
	{
		TRACE_ERROR("Exception occurred during SAX2 parse.");
		return false;
	}

	return true;
}

// TODO: Add unit test for this.
std::string XMLUtils::toBase64(const std::string& strSource)
{
	const size_t inputLength = strSource.length();
	if (UINT_MAX < inputLength)
	{
		return "";
	}
	XMLSize_t outputLength = 0;
	XMLByte * pXMLBytes = Base64::encode(
		reinterpret_cast<const XMLByte * const>(strSource.c_str()),
		static_cast<const unsigned int>(inputLength),
		&outputLength);
	if (!pXMLBytes)
	{
		return "";
	}

	// Assert that there are only ASCII chars in the result (as there should be);
	// otherwise a cast to char is unsafe.
	assert(isASCIIOnly(pXMLBytes));

	std::string strResult = reinterpret_cast<const char *>(pXMLBytes);
	delete[] pXMLBytes;
	return strResult;
}

// TODO: Add unit test for this.
std::string XMLUtils::fromBase64(const std::string& strSource)
{
	const size_t inputLength = strSource.length();
	if (UINT_MAX < inputLength)
	{
		return "";
	}

	// Assert that there are only ASCII chars in the result (as there should be);
	// otherwise a cast to char is unsafe.
	assert(isASCIIOnly(reinterpret_cast<const XMLByte * const>(strSource.c_str())));

	XMLSize_t outputLength = 0;
	XMLByte * pXMLBytes = Base64::decode(
		reinterpret_cast<const XMLByte * const>(strSource.c_str()),		
		&outputLength);
	if (!pXMLBytes)
	{
		return "";
	}

	std::string strResult(reinterpret_cast<const char *>(pXMLBytes),outputLength);
	delete[] pXMLBytes;
	return strResult;
}

std::string XMLUtils::encodeUtf8(const std::wstring & inputString)
{
	std::vector<char> vecUTF8(inputString.size()*4+1);
	
	// Finally transcode to UTF-8.
	XMLSize_t nCharsEaten = 0;
	XMLSize_t nLen = utf8Transcoder->transcodeTo(
		inputString.c_str(),
		static_cast<unsigned int>(inputString.size()), // should be same as XMLString::stringLen(strWide.c_str()) I think
		reinterpret_cast<XMLByte*>(&vecUTF8[0]),
		static_cast<unsigned int>(vecUTF8.size() - 1),
		nCharsEaten,
		XMLTranscoder::UnRep_Throw);
	vecUTF8[nLen] = 0;
	assert(nCharsEaten == inputString.size());
	return std::string(&vecUTF8[0],nLen);	
}

std::wstring XMLUtils::encodeUnicode(const std::string & inputString)
{
	std::vector<XMLCh> vecUni(inputString.size()+1);
	std::vector<unsigned char> vecCharSize(inputString.size()+1);
	
	// Finally transcode from UTF-8.
	XMLSize_t nCharsEaten = 0;
	XMLSize_t nLen = utf8Transcoder->transcodeFrom(
		reinterpret_cast<const XMLByte*>(inputString.c_str()),
		static_cast<unsigned int>(inputString.size()), 
		reinterpret_cast<XMLCh*>(&vecUni[0]),
		static_cast<unsigned int>(vecUni.size() - 1),
		nCharsEaten,
		static_cast<unsigned char*>(&vecCharSize[0]));
	vecUni[nLen] = 0;
	assert(nCharsEaten == inputString.size());
	return std::wstring(&vecUni[0],nLen);
}

std::string XMLUtils::toSafeXMLText(const std::string & inputString, bool bEscQuot)
{
	std::string result = inputString;

	boost::algorithm::replace_all(result, "&", "&amp;"); // do this first!
	boost::algorithm::replace_all(result, "<", "&lt;");
	boost::algorithm::replace_all(result, ">", "&gt;");
	boost::algorithm::replace_all(result, "=", "&#61;");
	if (bEscQuot)
	{
		boost::algorithm::replace_all(result, "\"", "&quot;"); // only needed in attr
	}

	return result;
}