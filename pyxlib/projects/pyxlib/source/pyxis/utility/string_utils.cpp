/******************************************************************************
string_utils.cpp

begin		: 2003-12-08
copyright	: (C) 2003 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/string_utils.h"

// local includes
#include "pyxis/utility/math_utils.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <fstream>
#include <ios>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <vector>
#include <regex>

//! Tester class
Tester<StringUtils> gTester;

//! Test method
void StringUtils::test()
{

	//Test tryParseFormattedNumber Method
	std::string result;
	TEST_ASSERT(tryParseFormattedNumber("1230.01",result));
	TEST_ASSERT("1230.01" == result);

	TEST_ASSERT(tryParseFormattedNumber("+1230.01",result));
	TEST_ASSERT("+1230.01" == result);

	TEST_ASSERT(tryParseFormattedNumber("-1230.01",result));
	TEST_ASSERT("-1230.01" == result);

	TEST_ASSERT(tryParseFormattedNumber(" $ 10",result));
	TEST_ASSERT("10" == result);

	TEST_ASSERT(tryParseFormattedNumber("0",result));
	TEST_ASSERT("0" == result);

	TEST_ASSERT(tryParseFormattedNumber("35.5 %",result));
	TEST_ASSERT("35.5" == result);

	TEST_ASSERT(tryParseFormattedNumber(" $-0.01",result));
	TEST_ASSERT("-0.01" == result);

	TEST_ASSERT(tryParseFormattedNumber("  3,210.01",result));
	TEST_ASSERT("3210.01" == result);

	TEST_ASSERT(tryParseFormattedNumber("3,210.01",result));
	TEST_ASSERT("3210.01" == result);

	TEST_ASSERT(tryParseFormattedNumber("3,000,210.01",result));
	TEST_ASSERT("3000210.01" == result);

	TEST_ASSERT(!tryParseFormattedNumber("3000,210.01",result));
	TEST_ASSERT(!tryParseFormattedNumber("$10%",result));
	TEST_ASSERT(!tryParseFormattedNumber(".1",result));
	TEST_ASSERT(!tryParseFormattedNumber("1.",result));

	TEST_ASSERT(!tryParseFormattedNumber("1 2 ",result));
	TEST_ASSERT(!tryParseFormattedNumber("a12",result));
	TEST_ASSERT(!tryParseFormattedNumber("--12",result));

	TEST_ASSERT(!tryParseFormattedNumber("122.31Dr",result));
	TEST_ASSERT(!tryParseFormattedNumber("Number 123",result));
	TEST_ASSERT(!tryParseFormattedNumber("D.1.2",result));
	TEST_ASSERT(!tryParseFormattedNumber("DDSAD$10",result));

	// test the to string method with double
	double fDouble = 12345.6789;
	std::string strDouble = toString(fDouble);
	TEST_ASSERT(strDouble == "12345.7");

	// test the doubleToString method with precision
	int nPrecision = 9;
	std::string strDoublePrec = doubleToString(fDouble, nPrecision);
	TEST_ASSERT(strDoublePrec == "12345.6789");

	// test the rounding
	nPrecision = 8;
	strDoublePrec = doubleToString(fDouble, nPrecision);
	TEST_ASSERT(strDoublePrec == "12345.679");

	// Test buffering of 0's on the end of a float
	nPrecision = 10;
	strDoublePrec = doubleToString(fDouble, nPrecision);
	TEST_ASSERT(strDoublePrec == "12345.67890");

	// Test buffering of 0's when the decimal portion is all 0's
	nPrecision = 10;
	strDoublePrec = doubleToString(nPrecision, nPrecision);
	TEST_ASSERT(strDoublePrec == "10.00000000");

	// test for size_t to strings
	nPrecision = 10;
	strDoublePrec = "12345.6789";
	std::string strSizet = toString(strDoublePrec.length());
	TEST_ASSERT(strSizet == "10");

	// test format functionality.
	fDouble = 12345.6789;
	strDoublePrec = "12345.6789";
	int nValue = 12345;
	nPrecision = 10;
	std::string strFormat = StringUtils::format
		(	"%1 %2 %3 %4", 
		toString(fDouble), 
		toString(strDoublePrec.length()),
		doubleToString(nPrecision, nPrecision),
		doubleToString(fDouble, nPrecision)	);

	TEST_ASSERT(strFormat == "12345.7 10 10.00000000 12345.67890");

	// test decimal count functionality
	strDoublePrec = "12345.6789";
	int nDecimals = StringUtils::countDecimalPlaces(strDoublePrec);
	TEST_ASSERT(nDecimals == 4);

	// test the intToString with precision
	nValue = 1;
	nPrecision = 10;
	std::string strIntPrec = intToString(nValue, nPrecision);
	TEST_ASSERT(strIntPrec == "0000000001");

	// test the decimal string extraction
	std::string strValue = "12345.6789";
	std::string strDecimal = StringUtils::getDecimalString(strValue);
	TEST_ASSERT(strDecimal == "6789");

	// test the trim method when the string is made up of all whitespace
	std::string strSpaces = "  \t \xA0   ";
	std::string strTrimmed = StringUtils::trim(strSpaces);
	TEST_ASSERT(strTrimmed == "");

	// test trimRight functionality
	std::string strTestRight = "size               \t \xA0                                                     ";
	std::string strTrimRight = StringUtils::trim(strTestRight);
	TEST_ASSERT(strTrimRight == "size");
	strTrimRight = StringUtils::trimRight(strTestRight);
	TEST_ASSERT(strTrimRight == "size");

	// test trimLeft functionality
	std::string strTestLeft = "               \t \xA0                       size";
	std::string strTrimLeft = StringUtils::trim(strTestLeft);
	TEST_ASSERT(strTrimLeft == "size");
	strTrimLeft = StringUtils::trimLeft(strTestLeft);
	TEST_ASSERT(strTrimLeft == "size");

	const char kszTestHex[10] = {1,2,'0','1','2','A','B','C','\n',0};

	// test writeHex method
	std::ostringstream outStreamHex;
	writeHex(outStreamHex, kszTestHex, 10);
	std::string strNewHex = outStreamHex.str();
	TEST_ASSERT(strNewHex == "01023031324142430a00");

	// test readHex method
	std::string strReadHex = "01023031324142430a00";
	char szReadHex[10];
	std::istringstream inStreamHex(strReadHex);
	readHex(inStreamHex, szReadHex, 10);
	TEST_ASSERT(memcmp(szReadHex, kszTestHex, 10) == 0);

	// test writeHex method for float
	const float kfTest = 231.345f;
	const void* pFloat = static_cast<const void*>(&kfTest);
	const char* pszFloatInput = static_cast<const char*>(pFloat);
	std::ostringstream outStreamHexFloat;
	writeHex(outStreamHexFloat, pszFloatInput, sizeof(float));
	std::string strNewHexFloat = outStreamHexFloat.str();
	TEST_ASSERT(strNewHexFloat == "52586743");

	// test readHex method for float
	std::string strReadHexFloat = "52586743";
	char szReadHexFloat[4];
	std::istringstream inStreamHexFloat(strReadHexFloat);
	readHex(inStreamHexFloat, szReadHexFloat, sizeof(float));
	float kfReadFloat = *(reinterpret_cast<float*>(szReadHexFloat));
	TEST_ASSERT(kfReadFloat == 231.345f);

	// test writeHex method for double
	const double kfTestDouble = 231.34532423f;
	const void* pDouble = static_cast<const void*>(&kfTestDouble);
	const char* pszDoubleInput = static_cast<const char*>(pDouble);
	std::ostringstream outStreamHexDouble;
	writeHex(outStreamHexDouble, pszDoubleInput, sizeof(double));
	std::string strNewHexDouble = outStreamHexDouble.str();
	TEST_ASSERT(strNewHexDouble == "000000e00ceb6c40");

	// test readHex method for double
	std::string strReadHexDouble = "000000e00ceb6c40";
	char szReadHexDouble[8];
	std::istringstream inStreamHexDouble(strReadHexDouble);
	readHex(inStreamHexDouble, szReadHexDouble, sizeof(double));
	const void* pReadDouble = static_cast<const void*>(szReadHexDouble);
	double kfReadDouble = *(static_cast<const double*>(pReadDouble));
	TEST_ASSERT(kfReadDouble == 231.34532423f);

	// test toString -> fromString
	{
		double f1 = 10.876548;
		double f2;
		std::string strValue = toString(f1);
		fromString(strValue, &f2);
		TEST_ASSERT(!MathUtils::equal(f1, f2));

		strValue = doubleToString(f1);
		fromString(strValue, &f2);
		TEST_ASSERT(MathUtils::equal(f1, f2));

		f1 = 1.056;
		strValue = toString(f1);
		fromString(strValue, &f2);
		TEST_ASSERT(MathUtils::equal(f1, f2));


		f1 = 0;
		strValue = "0";
		fromString(strValue, &f2);
		TEST_ASSERT(MathUtils::equal(f1, f2));

		int n1 = 919497;
		int n2;
		strValue = toString(n1);
		fromString(strValue, &n2);
		TEST_ASSERT(n1 == n2);
	}
}

/*!
Global method for converting a double to a formatted string.  The
string created will be in a 'fixed' format (i.e., integer.fraction).  

\param fValue		The value;
\param nPrecision	Total number of significant digits (default is 10)
\return	A string containing the value.
*/
std::string doubleToString(double fValue, int nPrecision)
{
	std::ostringstream ost;
	ost << std::setprecision(nPrecision) << fValue;

	// if there are less digits than the precision pad with 0's
	bool bHasDecimal = (ost.str().find('.') < ost.str().length());
	int nDifference = nPrecision - static_cast<int>(ost.str().length());
	if (bHasDecimal)
	{
		nDifference++;
	}

	// if there are too few digits
	if (nDifference > 0)
	{
		if (!bHasDecimal)
		{
			ost << '.';
		}
		while (nDifference > 0)
		{
			ost << '0';
			nDifference--;
		}
	}

	return ost.str();
}

/*!
Global method for converting an int to a formatted string.  The
string created will '0's padded in the front for required 
precision.  

\param nValue		The value.
\param nPrecision	Total number of digits.

\return	A string containing the value.
*/
std::string intToString(int nValue, int nPrecision)
{
	std::ostringstream ost;
	ost << std::setprecision(nPrecision) 
		<< std::setw(nPrecision) 
		<< std::setfill('0') << nValue;

	return ost.str();
}

/*!
Calculates a colour for a string. It can be cast to a Windows COLORREF.
\param str	The string.
\return A colour value in 0x00BBGGRR format.
*/
int StringUtils::toColour(const std::string& str)
{
	// Colour palette.
	const int array[] =
	{
		0x000000ff, 0x0000ff00, 0x00ff0000,
		0x00ffff00, 0x00ff00ff, 0x0000ffff,
		0x000080ff, 0x008000ff,
		0x0000ff80, 0x0080ff00,
		0x00ff0080, 0x00ff0080,
		0x00808000, 0x00800080, 0x00008080
	};

	// Simple hash on string.
	int nAccum = 0;
	for (int n = 0; n != str.size(); ++n)
	{
		nAccum ^= str[n];
		int nSignBit = nAccum < 0 ? 1 : 0;
		nAccum <<= 1;
		nAccum |= nSignBit;
	}
	int nIndex = nAccum % (sizeof(array) / sizeof(array[0]));

	return array[nIndex];
}

/*!
Static method that returns current time with local adjustment to
a string  output.
*/
std::string StringUtils::ascTime(time_t time)
{
	const int buffersize = 256;
	char timeAscBuffer[buffersize];
	ctime_s(timeAscBuffer,buffersize,&time);
	std::string nowString(timeAscBuffer);
	return nowString;
}

/*!
Static method for converting a time_t instance to
a string  output.
*/
std::string StringUtils::now()
{
	time_t now;
	time(&now);
	return ascTime(now);
}

//regular expression of a decimal number with 3 digit separators
static const std::string s_valueRegexStr("([\\+-]\\s*)?(\\d{1,3}((,\\d{3})+)|\\d+)(\\.\\d+)?");
static const std::regex s_valueRegex(s_valueRegexStr);
//regular expression of a decimal number preceeded by $ sign or followed by %
static const std::regex s_formattedRegex("(\\$?\\s*" + s_valueRegexStr + ")|(" + s_valueRegexStr + "\\s*\\%?)");


bool StringUtils::tryParseFormattedNumber(const std::string& strValue, std::string &trimmedValue)
{
	std::string trimmedStr= trim(strValue);

	if(!std::regex_match(trimmedStr,s_formattedRegex))
	{
		return false;
	}

	//find the single number that exists in the input string
	std::regex_iterator<std::string::iterator> rit (trimmedStr.begin(),trimmedStr.end(), s_valueRegex);
	std::regex_iterator<std::string::iterator> rend;

	while (rit!=rend && rit->str().empty()) 
	{
		++rit;
	}

	std::string foundNumber = rit->str();
	trimmedValue.clear();
	for (unsigned int i = 0; i < rit->str().length(); i++)
	{
		char token = foundNumber[i];
		if(token!=',')
		{
			trimmedValue.push_back(token);
		}
	}
	return true;
}

/*!
Global method for converting a char array to
a string hexadecimal output.

\param out		The output stream.
\param p		The char array.
\param n		The size of the array.

\return			The out stream.
*/
std::ostream& writeHex(std::ostream& out, const char* p, size_t n)
{
	// store the current flags
	std::ios_base::fmtflags  currentFlags = out.flags();

	// set the stream to hex and two character output
	out << std::hex << std::setfill('0');

	for (; n != 0; --n)
	{
		unsigned char nValue = static_cast<const unsigned char>(*p);
		int nIntValue = static_cast<const int>(nValue);

		out << std::setw(2) << nIntValue;
		++p;
	}

	//restore the flags
	out.flags(currentFlags);

	return out;
}

/*!
Read in hex string into an array

\param in				The stream to read from.
\param pszByteArray		The char array to store binary.
\param nValueCount		The length of the array.

\return					The stream input.
*/
std::istream& readHex(std::istream& in,	char* pszByteArray,	int nValueCount)
{
	const int knPair = 2;

	char cByte[knPair + 1];

	long nValue;
	for (int nIndex = 0; nIndex < nValueCount; ++nIndex)
	{
		in.read(cByte, knPair);
		cByte[knPair] = 0;
		nValue = strtol(cByte, NULL, 16);
		*pszByteArray++ = static_cast<char>(nValue);
	}

	return in;
}

/*!
Read in hex string into a ascii standard string.

\param hexStr the string as hex to decode into a standard string.

\return a ascii representation of the hex string.
*/
std::string readHex(const std::string& hexStr)
{
	std::istringstream strIn(hexStr);
	const int knNumChars = static_cast<int>(hexStr.length()) / 2;
	std::vector<char> vec(knNumChars);
	readHex(strIn, &vec[0], knNumChars);
	return std::string(vec.begin(), vec.end());
}

