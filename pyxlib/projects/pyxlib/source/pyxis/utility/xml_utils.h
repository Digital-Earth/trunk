#ifndef PYXIS__UTILITY__XML_UTILS_H
#define PYXIS__UTILITY__XML_UTILS_H
/******************************************************************************
xml_utils.h

begin		: 2007-02-08
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

// xerces includes
#include <xercesc/util/xercesversion.hpp>
#include <xercesc/util/xercesdefs.hpp>

// standard includes
#include <iosfwd>
#include <map>
#include <string>
#include <vector>

namespace XERCES_CPP_NAMESPACE
{
	class Attributes;
	class DefaultHandler;
}

class PYXLIB_DECL XMLUtils
{
public:
	static void test();

public:

	//! Call exactly once to initialize.
	static void initialize();

	//! Call exactly once to uninitialize.
	static void uninitialize();

	//! Converts xerces attributes to a map of transcoded strings (names and values).
	static std::map<std::string, std::string> getAttributes(const XERCES_CPP_NAMESPACE::Attributes& attrs);

	//! Parses using the specified handler.
	static bool parse(std::istream& in, XERCES_CPP_NAMESPACE::DefaultHandler& handler);

	//! Parses from a file using the specified handler.
	static bool parseFromFile(const std::string& strURI, XERCES_CPP_NAMESPACE::DefaultHandler& handler);

	//! Parses from a string using the specified handler.
	static bool parseFromString(const char* pStr, int nStrSize, XERCES_CPP_NAMESPACE::DefaultHandler& handler);
	
	//! Transcodes to UTF-8 
	static std::string encodeUtf8(const std::wstring & inputString);

	//! Transcodes to Unicode 
	static std::wstring encodeUnicode(const std::string & inputString);


	//! Escape few special characters
	static std::string toSafeXMLText(const std::string & inputString, bool bEscQuot = false);

	//! Convert the string to base 64.  If unsuccessful, returns an empty string.
	static std::string toBase64(const std::string& strSource);

	//! Convert the string from base 64.  If unsuccessful, returns an empty string.
	static std::string fromBase64(const std::string& strSource);
};

#endif // guard
