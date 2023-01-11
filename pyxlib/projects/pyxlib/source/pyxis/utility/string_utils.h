#ifndef PYXIS__UTILITY__STRING_UTILS_H
#define PYXIS__UTILITY__STRING_UTILS_H
/******************************************************************************
string_utils.h

begin		: 2003-12-08
copyright	: (C) 2003 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

// standard includes
#include <cassert>
#include <sstream>
#include <string>

//! Various string utilities
class PYXLIB_DECL StringUtils
{
public:

	//! Test.
	static void test();

	/*!
	Trim whitespace (space, tab and non-breaking space) from both ends of a string.

	\param		str		The input string.

	\return		The trimmed string.
	*/
	//! Trim whitespace from both ends of a string
	static std::string trim(const std::string& str)
	{
		if (str.empty())
		{
			return str;
		}

		std::string::size_type nBegin;
		std::string::size_type nEnd;

		nBegin = str.find_first_not_of(" \t\xA0");

		if (nBegin == std::string::npos)
		{
			// entire string is whitespace
			return "";
		}

		nEnd = str.find_last_not_of(" \t\xA0");

		std::string strValue = std::string(str, nBegin, nEnd - nBegin + 1);
		return strValue;
	}

	/*!
	Trim whitespace (space, tab, carriage return, linefeed, non-breaking space) from the right side of a string.

	\param str		The input string.

	\return	The trimmed string.
	*/
	//! Trim the right side of the string of whitespace characters
	static std::string trimRight(const std::string& str)
	{
		std::string strDelimiters = std::string(" \t\r\n\xA0");
		std::string strReturn = str;

		size_t idx = strReturn.find_last_not_of(strDelimiters);
		if (idx != std::string::npos)
		{
			strReturn.erase(++idx);
		}
		return strReturn;
	}

	/*!
	Trim whitespace (space, tab, carriage return, linefeed, non-breaking space) from the left side of a string.

	\param str		The input string.

	\return	The trimmed string.
	*/
	//! Trim the left side of the string of whitespace characters
	static std::string trimLeft(const std::string& str)
	{
		std::string strDelimiters = std::string(" \t\r\n\xA0");

		std::string strReturn = str;

		size_t idx = strReturn.find_first_not_of(strDelimiters);
		if (idx != std::string::npos)
		{
			strReturn.erase(0,idx);
		}
		else
		{
			strReturn.erase();
		}

		return strReturn;
	}

	/*!
	Replaces the tags in the string with the args passed in.

	\return	The new string.
	*/
	//! Formats the string
	static std::string format(	const std::string& strString,
		const std::string& strArg1 = "",
		const std::string& strArg2 = "",
		const std::string& strArg3 = "",
		const std::string& strArg4 = ""	)
	{
		std::string strNew = strString;

		// replace first tag
		if (!strArg1.empty() && 0 < strArg1.length())
		{
			std::string strTag = "%1";
			std::string::size_type nIndex = strNew.find(strTag);
			while (std::string::npos != nIndex)
			{
				strNew.replace(nIndex, strTag.length(), strArg1);
				nIndex = strNew.find(strTag, nIndex);
			}
		}

		// replace second tag
		if (0 < strArg2.length())
		{
			std::string strTag = "%2";
			std::string::size_type nIndex = strNew.find(strTag);
			while (std::string::npos != nIndex)
			{
				strNew.replace(nIndex, strTag.length(), strArg2);
				nIndex = strNew.find(strTag, nIndex);
			}
		}

		// replace third tag
		if (0 < strArg3.length())
		{
			std::string strTag = "%3";
			std::string::size_type nIndex = strNew.find(strTag);
			while (std::string::npos != nIndex)
			{
				strNew.replace(nIndex, strTag.length(), strArg3);
				nIndex = strNew.find(strTag, nIndex);
			}
		}

		// replace fourth tag
		if (0 < strArg4.length())
		{
			std::string strTag = "%4";
			std::string::size_type nIndex = strNew.find(strTag);
			while (std::string::npos != nIndex)
			{
				strNew.replace(nIndex, strTag.length(), strArg4);
				nIndex = strNew.find(strTag, nIndex);
			}
		}
		return strNew;
	}

	/*
	Counts the number of decimal places in a number.

	\param strNumber The number in string format.

	\return The number of decimal places
	*/
	static int countDecimalPlaces(const std::string& strNumber)
	{
		const char* pszValue = strNumber.c_str();

		// count the number of decimal places in a value
		int nIndex;
		for (nIndex = 0; pszValue[nIndex] != 0; ++nIndex)
		{
			if ('.' == pszValue[nIndex])
			{
				++nIndex;
				break;
			}
		}

		int nDecimalPlaces = 0;
		for (; pszValue[nIndex] != 0; ++nIndex)
		{
			nDecimalPlaces++;
		}

		return nDecimalPlaces;
	}

	/*
	Get the fraction as a string from a number.  It
	does not return the decimal point in the string.

	\param strNumber The number in string format.

	\return The decimal in string format
	*/
	static std::string getDecimalString(const std::string& strNumber)
	{
		// deal with the easting
		size_t nLength = strNumber.length();

		// how many decimals are there
		int nDecimals = countDecimalPlaces(strNumber);

		std::string strDecimals;

		if (nDecimals > 0)
		{
			// get the decimal string
			strDecimals = strNumber.substr(	nLength-nDecimals,
				nDecimals			);
		}

		return strDecimals;
	}

	static std::string now();

	static std::string ascTime(time_t time);

	//! Calculates a colour for a string.
	static int toColour(const std::string& str);

	/*!
	Global template method for converting a value to a string.

	\param value		The value to turn into a string.
	\param nPrecision	The number of digits to use to represent the value.
	\return	A string containing the value.
	*/
	template <class T>
	static std::string toString(const T& value, int nPrecision = 0)
	{
		std::ostringstream ost;
		if (nPrecision != 0)
		{
			ost.precision(nPrecision);
		}
		ost << value;
		return ost.str();
	}

	/*!
	Global template method for converting string representation of a value.

	\param strValue	The string to convert into a value.
	\param pValue	The value to fill with data from the string.
	*/
	template <class T>
	static void fromString(const std::string& strValue, T* pValue)
	{
		assert(pValue != 0);
		std::istringstream in(strValue);
		in >> *pValue;
	}

	/*!
	Global template method for converting string representation of a value.

	\param strValue	The string to convert into a value.
	\return The value converted from the string.
	*/
	template <class T>
	static T fromString(const std::string& strValue)
	{
		std::istringstream in(strValue);
		T t;
		in >> t;
		return t;
	}

	/*!
	Global method for determining whether a string contains only base 10 numeric data.

	\param strValue	The string to be checked for numeric data.
	\return True if the string contains only numeric data; false if not (including empty).
	*/
	static bool isNumeric(const std::string& strValue)
	{
		if (strValue.empty()) return false;

		std::istringstream in(strValue);
		double dTestSink;
		in >> dTestSink;

		// Return true only if all the input was successfully consumed/converted.
		return (in && in.rdbuf()->in_avail() == 0);
	}

	/*!
	Global method for determining whether a string contains numeric data.

	\param strValue	The string to be checked for numeric data.
	\param value	The value of the numeric string if the function returns true
	\return True if the string contains numeric data; false if not (including empty).
	*/
	static bool tryParseFormattedNumber(const std::string& strValue, std::string &trimmedValue);

	/*!
	Global method for determining whether a string contains only base 8 numeric data.

	\param strValue	The string to be checked for numeric data.
	\return True if the string contains only numeric data; false if not (including empty).
	*/
	static bool isNumericOctal(const std::string& strValue)
	{
		if (strValue.empty()) return false;

		std::istringstream in(strValue);
		int nTestSink;
		in >> std::oct >> nTestSink;

		// Return true only if all the input was successfully consumed/converted.
		return (in && in.rdbuf()->in_avail() == 0);
	}

	/*!
	Global method for determining whether a string contains only base 16 numeric data.

	\param strValue	The string to be checked for numeric data.
	\return True if the string contains only numeric data; false if not (including empty).
	*/
	static bool isNumericHexadecimal(const std::string& strValue)
	{
		if (strValue.empty()) return false;

		std::istringstream in(strValue);
		int nTestSink;
		in >> std::hex >> nTestSink;

		// Return true only if all the input was successfully consumed/converted.
		return (in && in.rdbuf()->in_avail() == 0);
	}
};
//! Convert a float to fixed digit formatted string
PYXLIB_DECL std::string doubleToString(double fValue, int nPrecision = 10);

//! Convert an int to a padded string.
PYXLIB_DECL std::string intToString(int nValue, int nPrecision);

//! Convert a char array to a string hexadecimal representation.
PYXLIB_DECL std::ostream& writeHex(std::ostream& out, const char* p, size_t n);

//! Read in hex stream into an array
PYXLIB_DECL std::istream& readHex(std::istream& in,	char* pszByteArray,	int nValueCount);

PYXLIB_DECL std::string readHex(const std::string& hexStr);

#endif
