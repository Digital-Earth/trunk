/******************************************************************************
value.cpp

begin		: 2006-02-15
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/value.h"

// local includes
#include "pyxis/utility/tester.h"

// boost includes
#include <boost/static_assert.hpp>
#include <boost/math/special_functions/round.hpp>

// standard includes
#include <iostream>
#include <set>
#include <vector>

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

//! Tester class
Tester<PYXValue> gTester;

// Microsoft Visual Studio .NET 2003 produces this warning:
//		warning C4800: 'const int' : forcing value to bool 'true' or 'false' (performance warning)
// Pragma it away.
#pragma warning(push)
#pragma warning(disable: 4800)


////////////////////////////////////////////////////////
// PYXValue::StringManager
////////////////////////////////////////////////////////
// allocate and deallocate PYXValue(string) can be very
// expensive when happen in high frequency on many threads
// as it look up the heap.
//
// therefore, we generated 16 object pools for strings.
// so every time we create/destory PYXValue(string) we 
// keep the str object in those pools for later use
// so we can avoid many malloc/free calls
////////////////////////////////////////////////////////

#define POOL_COUNT 16
std::vector<std::string*> string_pools[POOL_COUNT];
boost::recursive_mutex string_pools_mutex[POOL_COUNT];

std::string * PYXValue::StringManager::create(const std::string & val)
{
	int pool_index = GetCurrentThreadId() % POOL_COUNT;
	
	std::vector<std::string*> & pool = string_pools[pool_index];
	std::string * str = 0;
	{
		boost::recursive_mutex::scoped_lock lock(string_pools_mutex[pool_index]);
		if (!pool.empty())
		{
			str = pool.back();
			pool.pop_back();			
		}		
	}
	
	if (str==0)
	{
		return new std::string(val);
	}

	*str = val;
	return str;
}

void PYXValue::StringManager::dispose(std::string * str)
{
	if (str == 0)
	{
		return;
	}
	int pool_index = GetCurrentThreadId() % POOL_COUNT;	
	std::vector<std::string*> & pool = string_pools[pool_index];
	{
		boost::recursive_mutex::scoped_lock lock(string_pools_mutex[pool_index]);
		if (pool.size()<1000)
		{
			pool.push_back(str);
			return;
		}
	}
	//if we got here - the pool is full
	delete str;	
}

////////////////////////////////////////////////////////
// PYXValue::ArrayManager
////////////////////////////////////////////////////////
// allocate and deallocate PYXValue(unit8[4] - rgb) can be very
// expensive when happen in high frequency on many threads
// as it look up the heap.
//
// therefore, we generated 16 object pools for strings.
// so every time we create/destory PYXValue(unit8[4]) we 
// keep the buffers in those pools for later use
// so we can avoid many malloc/free calls
////////////////////////////////////////////////////////

#define BUFFER_POOL_COUNT 32
std::vector<char*> buffer4_pools[BUFFER_POOL_COUNT];
std::vector<char*> buffer8_pools[BUFFER_POOL_COUNT];
std::vector<char*> buffer16_pools[BUFFER_POOL_COUNT];
std::vector<char*> buffer32_pools[BUFFER_POOL_COUNT];
boost::recursive_mutex buffer_pools_mutex[BUFFER_POOL_COUNT];

std::vector<char*> & BufferManagerGetBufferPool(int size,int pool)
{
	if (size<=4) return buffer4_pools[pool];
	if (size<=8) return buffer8_pools[pool];
	if (size<=16) return buffer16_pools[pool];
	return buffer32_pools[pool];
}

int BufferManagerNormalizeBufferSize(int size)
{
	if (size<4) return 4;
	if (size<8) return 8;
	if (size<16) return 16;
	return 32;
}

char * PYXValue::BufferManager::create(int size)
{
	if (size>32)
	{
		return new char[size];
	}

	char * item = 0;
	int pool_index = GetCurrentThreadId() % BUFFER_POOL_COUNT;
	
	for(int i=0;i<BUFFER_POOL_COUNT;i++)
	{
		std::vector<char*> & pool = BufferManagerGetBufferPool(size,pool_index % BUFFER_POOL_COUNT);
		{
			boost::recursive_mutex::scoped_try_lock lock(buffer_pools_mutex[pool_index % BUFFER_POOL_COUNT]);
			if (lock)
			{
				if (!pool.empty())
				{
					item = pool.back();
					pool.pop_back();
				}
				break;
			}	
		}
		pool_index++;
	}
	
	if (item==0)
	{
		return new char[BufferManagerNormalizeBufferSize(size)];
	}
	
	return item;
}

void PYXValue::BufferManager::dispose(char * item,int size)
{
	if (item == 0)
	{
		return;
	}

	if (size>32)
	{
		delete[] item;
		return;
	}

	int pool_index = GetCurrentThreadId() % BUFFER_POOL_COUNT;	

	for(int i=0;i<BUFFER_POOL_COUNT;i++)
	{
		std::vector<char*> & pool = BufferManagerGetBufferPool(size,pool_index % BUFFER_POOL_COUNT);
		{
			boost::recursive_mutex::scoped_try_lock lock(buffer_pools_mutex[pool_index % BUFFER_POOL_COUNT]);

			if (lock)
			{
				if (pool.size()<256)
				{
					pool.push_back(item);
					return;
				}
				break;
			}
		}
		pool_index++;
	}

	//if we got here - the pool is full
	delete[] item;	
}



////////////////////////////////////////////////////////
// PYXValue
////////////////////////////////////////////////////////

//! Convert a type to a string for display.
const char* PYXValue::getString(PYXValue::eType nType)
{
	switch (nType)
	{
	case PYXValue::knBool:
		return "bool";
	case PYXValue::knChar:
		return "char";
	case PYXValue::knInt8:
		return "int8_t";
	case PYXValue::knUInt8:
		return "uint8_t";
	case PYXValue::knInt16:
		return "int16_t";
	case PYXValue::knUInt16:
		return "uint16_t";
	case PYXValue::knInt32:
		return "int32_t";
	case PYXValue::knUInt32:
		return "uint32_t";
	case PYXValue::knFloat:
		return "float";
	case PYXValue::knDouble:
		return "double";
	case PYXValue::knString:
		return "string";
	default:
		return "null";
	}
}

//! Convert a string to a type.
PYXValue::eType PYXValue::getType(const std::string& strType)
{
	if (strType == "bool")
	{
		return PYXValue::knBool;
	}
	if (strType == "char")
	{
		return PYXValue::knChar;
	}
	if (strType == "int8_t")
	{
		return PYXValue::knInt8;
	}
	if (strType == "uint8_t")
	{
		return PYXValue::knUInt8;
	}
	if (strType == "int16_t")
	{
		return PYXValue::knInt16;
	}
	if (strType == "uint16_t")
	{
		return PYXValue::knUInt16;
	}
	if (strType == "int32_t")
	{
		return PYXValue::knInt32;
	}
	if (strType == "uint32_t")
	{
		return PYXValue::knUInt32;
	}
	if (strType == "float")
	{
		return PYXValue::knFloat;
	}
	if (strType == "double")
	{
		return PYXValue::knDouble;
	}
	if (strType == "string")
	{
		return PYXValue::knString;
	}
	return PYXValue::knNull;
}

//! Is the specified type a numeric type
bool PYXValue::isNumeric(eType nType)
{
	switch (nType)
	{
	case PYXValue::knBool:
	case PYXValue::knChar:
		return false;
	case PYXValue::knInt8:
	case PYXValue::knUInt8:
	case PYXValue::knInt16:
	case PYXValue::knUInt16:
	case PYXValue::knInt32:
	case PYXValue::knUInt32:
	case PYXValue::knFloat:
	case PYXValue::knDouble:
		return true;
	case PYXValue::knString:
	case PYXValue::knNull:
		return false;
	default:
		assert(false && "Unknown data type.");
		return false;
	}
}

//! return true for numeric types that are signed
bool PYXValue::isSigned(eType type)
{
	switch (type)
	{
	case PYXValue::knBool:
	case PYXValue::knChar:
		return false;
	case PYXValue::knInt8:
	case PYXValue::knInt16:
	case PYXValue::knInt32:
	case PYXValue::knFloat:
	case PYXValue::knDouble:
		return true;
	case PYXValue::knUInt8:
	case PYXValue::knUInt16:
	case PYXValue::knUInt32:
		return false;
	case PYXValue::knString:
	case PYXValue::knNull:
		return false;
	default:
		assert(false && "Unknown data type.");
		return false;
	}
}

//! return true for numeric types that are unsigned
bool PYXValue::isUnsigned(eType type)
{
	switch (type)
	{
	case PYXValue::knBool:
	case PYXValue::knChar:
		return false;
	case PYXValue::knInt8:
	case PYXValue::knInt16:
	case PYXValue::knInt32:
	case PYXValue::knFloat:
	case PYXValue::knDouble:
		return false;
	case PYXValue::knUInt8:
	case PYXValue::knUInt16:
	case PYXValue::knUInt32:
		return true;
	case PYXValue::knString:
	case PYXValue::knNull:
		return false;
	default:
		assert(false && "Unknown data type.");
		return false;
	}
}

template<typename T>
inline int do_compare(const T & a,const T & b)
{
	if (a<b) return -1;
	if (a>b) return 1;
	return 0;
}

int PYXValue::compare(const PYXValue & a,const PYXValue & b,int n)
{
	switch (a.getType())
	{
	case PYXValue::knBool:
		return do_compare(a.getBool(n),b.getBool(n));
	case PYXValue::knChar:
		return do_compare(a.getChar(n),b.getChar(n));
	case PYXValue::knInt8:
		return do_compare(a.getInt8(n),b.getInt8(n));
	case PYXValue::knInt16:
		return do_compare(a.getInt16(n),b.getInt16(n));
	case PYXValue::knInt32:
		return do_compare(a.getInt32(n),b.getInt32(n));
	case PYXValue::knFloat:
		return do_compare(a.getFloat(n),b.getFloat(n));
	case PYXValue::knDouble:
		return do_compare(a.getDouble(n),b.getDouble(n));
	case PYXValue::knUInt8:
		return do_compare(a.getUInt8(n),b.getUInt8(n));
	case PYXValue::knUInt16:
		return do_compare(a.getUInt16(n),b.getUInt16(n));
	case PYXValue::knUInt32:
		return do_compare(a.getUInt32(n),b.getUInt32(n));
	case PYXValue::knString:
		return do_compare(a.getString(n),b.getString(n));
	case PYXValue::knNull:
		if (b.isNull(n)) return 0;
		return -compare(b,a,n); //let b compare it self (and b is not null :D
	default:
		assert(false && "Unknown data type.");
		return 0;
	}
}

int PYXValue::compare(const PYXValue & a,const PYXValue & b)
{
	int aSize = a.getArraySize();
	int bSize = b.getArraySize();
	int minLength = std::min(aSize,bSize);
	for(int i=0;i<minLength;i++)
	{
		int result = compare(a,b,i);
		if (result !=0) return result;
	}
	return do_compare(aSize,bSize);
}

int PYXValue::compare(const PYXValue & other) const
{
	return compare(*this,other);
}


PYXValue PYXValue::cast(PYXValue::eType type) const
{
	if (getType() == type)
	{
		//copy yourself
		return *this;
	}
	switch(type)
	{
	case PYXValue::knBool:
		return PYXValue(getBool());
	case PYXValue::knChar:
		return PYXValue(getChar());
	case PYXValue::knInt8:
		return PYXValue(getInt8());
	case PYXValue::knInt16:
		return PYXValue(getInt16());
	case PYXValue::knInt32:
		return PYXValue(getInt32());
	case PYXValue::knFloat:
		return PYXValue(getFloat());
	case PYXValue::knDouble:
		return PYXValue(getDouble());
	case PYXValue::knUInt8:
		return PYXValue(getUInt8());
	case PYXValue::knUInt16:
		return PYXValue(getUInt16());
	case PYXValue::knUInt32:
		return PYXValue(getUInt32());
	case PYXValue::knString:
		return PYXValue(getString());
	case PYXValue::knNull:
		return PYXValue();
	default:
		PYXTHROW(PYXException,"cast only work for simple types. unsupported type " << getTypeAsString(type));
	}
}

namespace
{
//! Helper for stream operator.
template <typename TD>
void readValues(std::istream& in, PYXValue& value, PYXValue::eType nType, int nSize, bool bNullable, bool bArray)
{
	int nCount = nSize + (bNullable ? 1 : 0);
	boost::scoped_array<TD> spVal(new TD[nCount]);
	TD* pNullVal = bNullable ? &spVal[nSize] : 0;

	for (int n = 0; n != nCount; ++n)
	{
		in >> spVal[n];
	}

	value = PYXValue::create(nType, spVal.get(), nSize, pNullVal, bArray);
}

//! Specialization for char.
template <>
void readValues<char>(std::istream& in, PYXValue& value, PYXValue::eType nType, int nSize, bool bNullable, bool bArray)
{
	typedef char TD;

	int nCount = nSize + (bNullable ? 1 : 0);
	boost::scoped_array<TD> spVal(new TD[nCount]);
	TD* pNullVal = bNullable ? &spVal[nSize] : 0;

	for (int n = 0; n != nCount; ++n)
	{
		int nVal;
		in >> nVal;
		spVal[n] = static_cast<TD>(nVal);
	}

	value = PYXValue::create(nType, spVal.get(), nSize, pNullVal, bArray);
}

//! Specialization for signed char.
template <>
void readValues<signed char>(std::istream& in, PYXValue& value, PYXValue::eType nType, int nSize, bool bNullable, bool bArray)
{
	typedef signed char TD;

	int nCount = nSize + (bNullable ? 1 : 0);
	boost::scoped_array<TD> spVal(new TD[nCount]);
	TD* pNullVal = bNullable ? &spVal[nSize] : 0;

	for (int n = 0; n != nCount; ++n)
	{
		int nVal;
		in >> nVal;
		spVal[n] = static_cast<TD>(nVal);
	}

	value = PYXValue::create(nType, spVal.get(), nSize, pNullVal, bArray);
}

//! Specialization for unsigned char.
template <>
void readValues<unsigned char>(std::istream& in, PYXValue& value, PYXValue::eType nType, int nSize, bool bNullable, bool bArray)
{
	typedef unsigned char TD;

	int nCount = nSize + (bNullable ? 1 : 0);
	boost::scoped_array<TD> spVal(new TD[nCount]);
	TD* pNullVal = bNullable ? &spVal[nSize] : 0;

	for (int n = 0; n != nCount; ++n)
	{
		int nVal;
		in >> nVal;
		spVal[n] = static_cast<TD>(nVal);
	}

	value = PYXValue::create(nType, spVal.get(), nSize, pNullVal, bArray);
}

//! Specialization for string.
template <>
void readValues<std::string>(std::istream& in, PYXValue& value, PYXValue::eType nType, int nSize, bool bNullable, bool bArray)
{
	typedef std::string TD;

	int nCount = nSize + (bNullable ? 1 : 0);
	boost::scoped_array<TD> spVal(new TD[nCount]);
	TD* pNullVal = bNullable ? &spVal[nSize] : 0;

	for (int n = 0; n != nCount; ++n)
	{
		// Read string size.
		int nStrSize;
		in >> nStrSize;
		spVal[n].resize(nStrSize);
		// Throw away space.
		in.get();
		// Read string characters.
		for (int nChar = 0; nChar != nStrSize; ++nChar)
		{
			spVal[n][nChar] = in.get();
		}
	}

	value = PYXValue::create(nType, spVal.get(), nSize, pNullVal, bArray);
}

/*!
Reads in values from the stream as specified.

\param in			The input stream.
\param value		The value to assign.
\param nType		The type of value to assign.
\param nSize		The array size to use. (1 or greater)
\param bNullable	Whether an array has a null value.
\param bArray		Whether to force an array in the case of size 1.
*/
//! Helper for stream operator.
void readValues(std::istream& in, PYXValue& value, PYXValue::eType nType, int nSize, bool bNullable, bool bArray)
{
	switch (nType)
	{
	case PYXValue::knBool:
		readValues<bool>(in, value, nType, nSize, bNullable, bArray);
		break;
	case PYXValue::knChar:
		readValues<char>(in, value, nType, nSize, bNullable, bArray);
		break;
	case PYXValue::knInt8:
		readValues<int8_t>(in, value, nType, nSize, bNullable, bArray);
		break;
	case PYXValue::knUInt8:
		readValues<uint8_t>(in, value, nType, nSize, bNullable, bArray);
		break;
	case PYXValue::knInt16:
		readValues<int16_t>(in, value, nType, nSize, bNullable, bArray);
		break;
	case PYXValue::knUInt16:
		readValues<uint16_t>(in, value, nType, nSize, bNullable, bArray);
		break;
	case PYXValue::knInt32:
		readValues<int32_t>(in, value, nType, nSize, bNullable, bArray);
		break;
	case PYXValue::knUInt32:
		readValues<uint32_t>(in, value, nType, nSize, bNullable, bArray);
		break;
	case PYXValue::knFloat:
		readValues<float>(in, value, nType, nSize, bNullable, bArray);
		break;
	case PYXValue::knDouble:
		readValues<double>(in, value, nType, nSize, bNullable, bArray);
		break;
	case PYXValue::knString:
		readValues<std::string>(in, value, nType, nSize, bNullable, bArray);
		break;
	}
}

//! Helper function for compareEqualTo.
static bool nullCompareEqualTo(const PYXValue& value, const PYXValue& other)
{
	// Null equals null only.
	return other.isNull();
}

//! Helper function for compareLess.
static bool nullCompareLess(const PYXValue& value, const PYXValue& other)
{
	// Null is less than all but null.
	return !other.isNull();
}

//! Helper function for getValue.
static PYXValue nullGetValue(const PYXValue& value, int n)
{
	return PYXValue();
}

//! Helper function for getValue.
PYXValue singleGetValue(const PYXValue& value, int n)
{
	return PYXValue(value);
}

//! Helper function for array comparison.
bool compareArrayEqualTo(const PYXValue& value, const PYXValue& other)
{
	// TODO this function can be more efficient
	if (!other.isArray()
		|| value.getArrayType() != other.getArrayType()
		|| value.getArraySize() != other.getArraySize()
		|| value.getNullValue() != other.getNullValue())
	{
		return false;
	}
	for (int n = 0; n != value.getArraySize(); ++n)
	{
		if (value.getValue(n) != other.getValue(n))
		{
			return false;
		}
	}
	return true;
}

//! Helper function for array comparison.
bool compareArrayLess(const PYXValue& value, const PYXValue& other)
{
	// TODO this function can be more efficient
	if (!other.isArray())
	{
		return false;
	}

	if (value.getArrayType() != other.getArrayType())
	{
		return value.getArrayType() < other.getArrayType();
	}

	int nMinSize = std::min(value.getArraySize(), other.getArraySize());

	for (int n = 0; n != nMinSize; ++n)
	{
		PYXValue valueElem = value.getValue(n);
		PYXValue otherElem = other.getValue(n);

		if (valueElem != otherElem)
		{
			if (valueElem.isNull())
			{
				return true;
			}
			else if (otherElem.isNull())
			{
				return false;
			}
			else
			{
				return valueElem < otherElem;
			}
		}
	}

	{
		int nValueSize = value.getArraySize();
		int nOtherSize = other.getArraySize();
		if (nValueSize != nOtherSize)
		{
			return nValueSize < nOtherSize;
		}
	}

	if (value.isArrayNullable())
	{
		if (other.isArrayNullable())
		{
			return value.getNullValue() < other.getNullValue();
		}
		else
		{
			return true;
		}
	}

	return false;
}

//! Tests that the type is as specified.
void testType(PYXValue& value, PYXValue::eType type)
{
	TEST_ASSERT(value == value);
	TEST_ASSERT(value.getType() == type);
	TEST_ASSERT(value.isNull() == (type == PYXValue::knNull));
	TEST_ASSERT(value.isBool() == (type == PYXValue::knBool));
	TEST_ASSERT(value.isChar() == (type == PYXValue::knChar));
	TEST_ASSERT(value.isInt8() == (type == PYXValue::knInt8));
	TEST_ASSERT(value.isUInt8() == (type == PYXValue::knUInt8));
	TEST_ASSERT(value.isInt16() == (type == PYXValue::knInt16));
	TEST_ASSERT(value.isUInt16() == (type == PYXValue::knUInt16));
	TEST_ASSERT(value.isInt32() == (type == PYXValue::knInt32));
	TEST_ASSERT(value.isUInt32() == (type == PYXValue::knUInt32));
	TEST_ASSERT(value.isFloat() == (type == PYXValue::knFloat));
	TEST_ASSERT(value.isDouble() == (type == PYXValue::knDouble));
	TEST_ASSERT(value.isString() == (type == PYXValue::knString));
	TEST_ASSERT(value.isArray() == (type == PYXValue::knArray));
}

}

//! Tests that the value and type is as specified.
template <typename TD>
void PYXValue::testValue(PYXValue& value, const TD& val)
{
	TEST_ASSERT(value == value);
	TEST_ASSERT(value.getBool() == static_cast<bool>(val));
	TEST_ASSERT(value.getChar() == static_cast<char>(val));
	TEST_ASSERT(value.getInt8() == static_cast<int8_t>(val));
	TEST_ASSERT(value.getUInt8() == static_cast<uint8_t>(val));
	TEST_ASSERT(value.getInt16() == static_cast<int16_t>(val));
	TEST_ASSERT(value.getUInt16() == static_cast<uint16_t>(val));
	TEST_ASSERT(value.getInt32() == static_cast<int32_t>(val));
	TEST_ASSERT(value.getUInt32() == static_cast<uint32_t>(val));
#if 0
	// Code generation issues with float comparison.
	TEST_ASSERT(value.getFloat() == static_cast<float>(val));
#else
	{
		float f1 = value.getFloat();
		float f2 = static_cast<float>(val);
		TEST_ASSERT(f1 == f2);
	}
#endif
	TEST_ASSERT(value.getDouble() == static_cast<double>(val));
	if (value.isNull())
	{
		TEST_ASSERT(value.getString().empty());
	}
	else
	{
		TEST_ASSERT(value.getString() == Helper::toString(val));
	}

	TEST_ASSERT(value.getBool(0) == static_cast<bool>(val));
	TEST_ASSERT(value.getChar(0) == static_cast<char>(val));
	TEST_ASSERT(value.getInt8(0) == static_cast<int8_t>(val));
	TEST_ASSERT(value.getUInt8(0) == static_cast<uint8_t>(val));
	TEST_ASSERT(value.getInt16(0) == static_cast<int16_t>(val));
	TEST_ASSERT(value.getUInt16(0) == static_cast<uint16_t>(val));
	TEST_ASSERT(value.getInt32(0) == static_cast<int32_t>(val));
	TEST_ASSERT(value.getUInt32(0) == static_cast<uint32_t>(val));
#if 0
	// Code generation issues with float comparison.
	TEST_ASSERT(value.getFloat(0) == static_cast<float>(val));
#else
	{
		float f1 = value.getFloat(0);
		float f2 = static_cast<float>(val);
		TEST_ASSERT(f1 == f2);
	}
#endif
	TEST_ASSERT(value.getDouble(0) == static_cast<double>(val));
	if (value.isNull())
	{
		TEST_ASSERT(value.getString(0).empty());
	}
	else
	{
		TEST_ASSERT(value.getString(0) == Helper::toString(val));
	}
	TEST_ASSERT(value == value.getValue(0));
	if (!(value == value.getValue(0)))
	{
		PYXValue::eType nType = value.getType();
		PYXValue::eType nType2 = value.getValue(0).getType();
		PYXValue value2 = value.getValue(0);
	}
}

// Specialization for std::string
template <>
void PYXValue::testValue(PYXValue& value, const std::string& val)
{
	TEST_ASSERT(value == value);
	TEST_ASSERT(value.getBool() == Helper::fromString<bool>(val));
	TEST_ASSERT(value.getChar() == Helper::fromString<char>(val));
	TEST_ASSERT(value.getInt8() == Helper::fromString<int8_t>(val));
	TEST_ASSERT(value.getUInt8() == Helper::fromString<uint8_t>(val));
	TEST_ASSERT(value.getInt16() == Helper::fromString<int16_t>(val));
	TEST_ASSERT(value.getUInt16() == Helper::fromString<uint16_t>(val));
	TEST_ASSERT(value.getInt32() == Helper::fromString<int32_t>(val));
	TEST_ASSERT(value.getUInt32() == Helper::fromString<uint32_t>(val));
#if 0
	// Code generation issues with float comparison.
	TEST_ASSERT(value.getFloat() == Helper::fromString<float>(val));
#else
	{
		float f1 = value.getFloat();
		float f2 = Helper::fromString<float>(val);
		TEST_ASSERT(f1 == f2);
	}
#endif
	TEST_ASSERT(value.getDouble() == Helper::fromString<double>(val));
	TEST_ASSERT(value.getString() == val);

	TEST_ASSERT(value.getBool(0) == Helper::fromString<bool>(val));
	TEST_ASSERT(value.getChar(0) == Helper::fromString<char>(val));
	TEST_ASSERT(value.getInt8(0) == Helper::fromString<int8_t>(val));
	TEST_ASSERT(value.getUInt8(0) == Helper::fromString<uint8_t>(val));
	TEST_ASSERT(value.getInt16(0) == Helper::fromString<int16_t>(val));
	TEST_ASSERT(value.getUInt16(0) == Helper::fromString<uint16_t>(val));
	TEST_ASSERT(value.getInt32(0) == Helper::fromString<int32_t>(val));
	TEST_ASSERT(value.getUInt32(0) == Helper::fromString<uint32_t>(val));
#if 0
	// Code generation issues with float comparison.
	TEST_ASSERT(value.getFloat(0) == Helper::fromString<float>(val));
#else
	{
		float f1 = value.getFloat(0);
		float f2 = Helper::fromString<float>(val);
		TEST_ASSERT(f1 == f2);
	}
#endif
	TEST_ASSERT(value.getDouble(0) == Helper::fromString<double>(val));
	TEST_ASSERT(value.getString(0) == val);
	TEST_ASSERT(value == value.getValue(0));
}

//! Tests that the value and type is as specified.
template <typename TD>
void PYXValue::testValueAndType(PYXValue& value, TD val)
{
	testType(value, Helper::getType(&val)); // infer type from value
	testValue(value, val);
}

//! Tests that the value and type is as specified.
template <typename TD>
void PYXValue::testSingleValue(TD val)
{
	PYXValue::eType type = Helper::getType(&val);
	PYXValue value(val);
	testValueAndType(value, val);
	{
		PYXValue copy(value);
		testValueAndType(copy, val);

		TD newval;
		std::string strVal;

		newval = 0;
		copy.set(newval);
		testValueAndType(copy, newval);
		testValueAndType(value, val);

		newval = 1;
		copy.set(newval);
		testValueAndType(copy, newval);
		testValueAndType(value, val);

		strVal = Helper::toString(val);
		copy.set(strVal);
		testType(copy, type);
		testValue(copy, strVal);

		newval = 0;
		copy.set(0, newval);
		testValueAndType(copy, newval);
		testValueAndType(value, val);

		strVal = Helper::toString(val);
		copy.set(0, strVal);
		testType(copy, type);
		testValue(copy, strVal);
	}
	const TD* ptr = reinterpret_cast<const TD*>(value.getPtr(0));
	TEST_ASSERT(*ptr == val);
}

//! Tests that the array is as specified.
template <typename TD>
void PYXValue::testArray(PYXValue& value, TD* pVal, int nSize, const TD& nullVal, bool bNullable)
{
	TEST_ASSERT(value == value);
	testType(value, PYXValue::knArray);
	eType AType = Helper::getType(pVal); // infer type from value
	TEST_ASSERT(value.getArrayType() == AType);
	TEST_ASSERT(value.getArraySize() == nSize);
	TEST_ASSERT(value.isArrayNullable() == bNullable);

	if (bNullable)
	{
		PYXValue nv = value.getNullValue();
		TEST_ASSERT(value.getNullValue() == PYXValue(nullVal));
	}

	for (int n = 0; n != nSize; ++n)
	{
		PYXValue elem = value.getValue(n);
		if (bNullable && pVal[n] == nullVal)
		{
			TEST_ASSERT(value.isNull(n) == true);
			testType(elem, knNull);
			testValue(elem, 0);
		}
		else
		{
			TEST_ASSERT(value.isNull(n) == false);
			testValueAndType(elem, pVal[n]);

			TEST_ASSERT(elem.getBool() == value.getBool(n));
			TEST_ASSERT(elem.getChar() == value.getChar(n));
			TEST_ASSERT(elem.getInt8() == value.getInt8(n));
			TEST_ASSERT(elem.getUInt8() == value.getUInt8(n));
			TEST_ASSERT(elem.getInt16() == value.getInt16(n));
			TEST_ASSERT(elem.getUInt16() == value.getUInt16(n));
			TEST_ASSERT(elem.getInt32() == value.getInt32(n));
			TEST_ASSERT(elem.getUInt32() == value.getUInt32(n));
#if 1
			// More float comparison woes, only in release builds this time.
			{
				float fVal = elem.getFloat();
				float fValue = value.getFloat(n);
				int n = memcmp(&fVal, &fValue, sizeof(float));
				TEST_ASSERT(n == 0);
			}
#else
			TEST_ASSERT(elem.getFloat() == value.getFloat(n));
#endif
			TEST_ASSERT(elem.getDouble() == value.getDouble(n));
		}

		const TD* ptr = reinterpret_cast<const TD*>(value.getPtr(n));
		TEST_ASSERT(*ptr == pVal[n]);
	}

	{
		TEST_ASSERT(value.getBool() == value.getBool(0));
		TEST_ASSERT(value.getChar() == value.getChar(0));
		TEST_ASSERT(value.getInt8() == value.getInt8(0));
		TEST_ASSERT(value.getUInt8() == value.getUInt8(0));
		TEST_ASSERT(value.getInt16() == value.getInt16(0));
		TEST_ASSERT(value.getUInt16() == value.getUInt16(0));
		TEST_ASSERT(value.getInt32() == value.getInt32(0));
		TEST_ASSERT(value.getUInt32() == value.getUInt32(0));
		TEST_ASSERT(value.getFloat() == value.getFloat(0));
		TEST_ASSERT(value.getDouble() == value.getDouble(0));
	}
}

//! Test method
void PYXValue::test()
{
	// The buffer must have enough room to store a data pointer along with the array metadata.
	BOOST_STATIC_ASSERT((sizeof (void *)) <= (knBufferSize - knArrayNullableIndex));

	{
		// Test null
		PYXValue::eType type = PYXValue::knNull;
		typedef int TD;
		TD val = 0;
		PYXValue value;
		testType(value, type);
		testValue(value, val);
		{
			PYXValue copy(value);
			testType(copy, type);
			testValue(copy, val);
			TD newval = 0;
			copy.set(newval);
			testType(copy, type);
			testValue(copy, newval);
			testType(value, type);
			testValue(value, val);
			std::string strval("");
			copy.set(strval);
			testType(copy, type);
			testValue(copy, strval);
		}
	}

	{
		// Test bool
		testSingleValue<bool>(false);
		testSingleValue<bool>(true);
	}

	{
		// Test char
		typedef char TD;
		{
			// Test range
			TD nStep = 4; // 256/64
			TD val = std::numeric_limits<TD>::min();
			for (int n = 0; n != 64; ++n, val += nStep)
			{
				testSingleValue<TD>(val);
			}
			testSingleValue<TD>(std::numeric_limits<TD>::max());
		}
	}

	{
		// Test int8_t
		typedef int8_t TD;
		{
			// Test range
			TD nStep = 4; // 256/64
			TD val = std::numeric_limits<TD>::min();
			for (int n = 0; n != 64; ++n, val += nStep)
			{
				testSingleValue<TD>(val);
			}
			testSingleValue<TD>(std::numeric_limits<TD>::max());
		}
	}

	{
		// Test uint8_t
		typedef uint8_t TD;
		{
			// Test range
			TD nStep = 4; // 256/64
			TD val = std::numeric_limits<TD>::min();
			for (int n = 0; n != 64; ++n, val += nStep)
			{
				testSingleValue<TD>(val);
			}
			testSingleValue<TD>(std::numeric_limits<TD>::max());
		}
	}

	{
		// Test int16_t
		typedef int16_t TD;
		{
			// Test range
			TD nStep = 1024; // 2^16/64
			TD val = std::numeric_limits<TD>::min();
			for (int n = 0; n != 64; ++n, val += nStep)
			{
				testSingleValue<TD>(val);
			}
			testSingleValue<TD>(std::numeric_limits<TD>::max());
		}
		// Test primes
		testSingleValue<TD>(127);
		testSingleValue<TD>(251);
		testSingleValue<TD>(32749);
	}

	{
		// Test uint16_t
		typedef uint16_t TD;
		{
			// Test range
			TD nStep = 1024; // 2^16/64
			TD val = std::numeric_limits<TD>::min();
			for (int n = 0; n != 64; ++n, val += nStep)
			{
				testSingleValue<TD>(val);
			}
			testSingleValue<TD>(std::numeric_limits<TD>::max());
		}
		// Test primes
		testSingleValue<TD>(127);
		testSingleValue<TD>(251);
		testSingleValue<TD>(32749);
		testSingleValue<TD>(65521);
	}

	{
		// Test int32_t
		typedef int32_t TD;
		{
			// Test range
			TD nStep = 67108864; // 2^32/64
			TD val = std::numeric_limits<TD>::min();
			for (int n = 0; n != 64; ++n, val += nStep)
			{
				testSingleValue<TD>(val);
			}
			testSingleValue<TD>(std::numeric_limits<TD>::max());
		}
		// Test primes
		testSingleValue<TD>(127);
		testSingleValue<TD>(251);
		testSingleValue<TD>(32749);
		testSingleValue<TD>(65521);
		testSingleValue<TD>(275604541); // would be nice to have larger prime
	}

	{
		// Test uint32_t
		typedef uint32_t TD;
		{
			// Test range
			TD nStep = 67108864; // 2^32/64
			TD val = std::numeric_limits<TD>::min();
			for (int n = 0; n != 64; ++n, val += nStep)
			{
				testSingleValue<TD>(val);
			}
			testSingleValue<TD>(std::numeric_limits<TD>::max());
		}
		// Test primes
		testSingleValue<TD>(127);
		testSingleValue<TD>(251);
		testSingleValue<TD>(32749);
		testSingleValue<TD>(65521);
		testSingleValue<TD>(275604541); // would be nice to have larger prime
	}

	{
		// Test float
		typedef float TD;
#if 0 // Having problems testing with float/int interaction
		TD nStep = 65536.123f; // 2^24/256 + 0.123
		for (TD val = std::numeric_limits<TD>::min(); ; val += nStep)
		{
			testSingleValue<TD>(val);
			if (std::numeric_limits<TD>::max() - val < nStep)
			{
				testSingleValue<TD>(std::numeric_limits<TD>::max());
				break;
			}
		}
#else
		testSingleValue<TD>(-12345.6789f);
		testSingleValue<TD>(12345.6789f);
#endif
		// Test primes
		testSingleValue<TD>(127);
		testSingleValue<TD>(251);
		testSingleValue<TD>(32749);
		testSingleValue<TD>(65521);
	}

	{
		// Test double
		typedef double TD;
#if 0 // Having problems testing with float/int interaction
		TD nStep = 3.51843721e13; // 2^53/256
		for (TD val = std::numeric_limits<TD>::min(); ; val += nStep)
		{
			testSingleValue<TD>(val);
			if (std::numeric_limits<TD>::max() - val < nStep)
			{
				testSingleValue<TD>(std::numeric_limits<TD>::max());
				break;
			}
		}
#else
		testSingleValue<TD>(-12345.6789);
		testSingleValue<TD>(12345.6789);
#endif
		// Test primes
		testSingleValue<TD>(127);
		testSingleValue<TD>(251);
		testSingleValue<TD>(32749);
		testSingleValue<TD>(65521);
		testSingleValue<TD>(275604541); // would be nice to have larger prime
	}

	{
		// Test swap
		PYXValue value(123);
		PYXValue other(3.14);
		testValueAndType(value, 123);
		testValueAndType(other, 3.14);
		::swap(value, other);
		testValueAndType(value, 3.14);
		testValueAndType(other, 123);
	}

	{
		// Test copy assignment
		PYXValue value;
		testType(value, PYXValue::knNull);
		testValue(value, static_cast<int>(0));
		{
			bool val = true;
			value = PYXValue(val);
			testValueAndType(value, val);
		}
		{
			char val = 2;
			value = PYXValue(val);
			testValueAndType(value, val);
		}
		{
			int8_t val = 7;
			value = PYXValue(val);
			testValueAndType(value, val);
		}
		{
			uint8_t val = 8;
			value = PYXValue(val);
			testValueAndType(value, val);
		}
		{
			int16_t val = 15;
			value = PYXValue(val);
			testValueAndType(value, val);
		}
		{
			uint16_t val = 16;
			value = PYXValue(val);
			testValueAndType(value, val);
		}
		{
			int32_t val = 31;
			value = PYXValue(val);
			testValueAndType(value, val);
		}
		{
			uint32_t val = 32;
			value = PYXValue(val);
			testValueAndType(value, val);
		}
		{
			float val = 8.23f;
			value = PYXValue(val);
			testValueAndType(value, val);
		}
		{
			double val = 11.52;
			value = PYXValue(val);
			testValueAndType(value, val);
		}
	}

	{
		// Test setType
		PYXValue value;
		testType(value, PYXValue::knNull);
		testValue(value, 0);
		value.setType(PYXValue::knBool);
		testType(value, PYXValue::knBool);
		testValue(value, 0);
		value.set(1);
		testValue(value, 1);
		value.setType(PYXValue::knChar);
		testType(value, PYXValue::knChar);
		testValue(value, 1);
		value.set(127);
		testValue(value, 127);
		value.setType(PYXValue::knInt8);
		testType(value, PYXValue::knInt8);
		testValue(value, 127);
		value.setType(PYXValue::knUInt8);
		testType(value, PYXValue::knUInt8);
		testValue(value, 127);
		value.setType(PYXValue::knInt16);
		testType(value, PYXValue::knInt16);
		testValue(value, 127);
		value.setType(PYXValue::knUInt16);
		testType(value, PYXValue::knUInt16);
		testValue(value, 127);
		value.setType(PYXValue::knInt32);
		testType(value, PYXValue::knInt32);
		testValue(value, 127);
		value.setType(PYXValue::knUInt32);
		testType(value, PYXValue::knUInt32);
		testValue(value, 127);
		value.setType(PYXValue::knFloat);
		testType(value, PYXValue::knFloat);
		testValue(value, 127);
		value.setType(PYXValue::knDouble);
		testType(value, PYXValue::knDouble);
		testValue(value, 127);
		value.setType(PYXValue::knFloat);
		testType(value, PYXValue::knFloat);
		testValue(value, 127);
		value.setType(PYXValue::knUInt32);
		testType(value, PYXValue::knUInt32);
		testValue(value, 127);
		value.setType(PYXValue::knInt32);
		testType(value, PYXValue::knInt32);
		testValue(value, 127);
		value.setType(PYXValue::knUInt16);
		testType(value, PYXValue::knUInt16);
		testValue(value, 127);
		value.setType(PYXValue::knInt16);
		testType(value, PYXValue::knInt16);
		testValue(value, 127);
		value.setType(PYXValue::knUInt8);
		testType(value, PYXValue::knUInt8);
		testValue(value, 127);
		value.setType(PYXValue::knInt8);
		testType(value, PYXValue::knInt8);
		testValue(value, 127);
		value.setType(PYXValue::knChar);
		testType(value, PYXValue::knChar);
		testValue(value, 127);
		value.setType(PYXValue::knBool);
		testType(value, PYXValue::knBool);
		testValue(value, 1);
		value.setType(PYXValue::knNull);
		testType(value, PYXValue::knNull);
		testValue(value, 0);
	}
	{
		// Test setType when converting from string to POD.
		double pi = 3.14159265358979;
		std::string strPi("3.14159265358979");
		PYXValue value(strPi);
		TEST_ASSERT(abs(value.getDouble() - pi) <= std::numeric_limits<double>::epsilon());
		value.setType(knDouble);
		TEST_ASSERT(abs(value.getDouble() - pi) <= std::numeric_limits<double>::epsilon());
		TEST_ASSERT(value.getString() == strPi);
	}
	{
		// Test setType when converting from integer POD to string.
		PYXValue value(42);
		value.setType(knString);
	}
	{
		// Test setType when converting from floating-point POD to string.
		double pi = 3.14159265358979;
		PYXValue value(pi);
		TEST_ASSERT(abs(value.getDouble() - pi) <= std::numeric_limits<double>::epsilon());
		value.setType(knString);
		TEST_ASSERT(abs(value.getDouble() - pi) <= std::numeric_limits<double>::epsilon());
	}
	{
		// Test setType when converting from POD to array.
		int data = 144000;
		PYXValue value(data);
		TEST_ASSERT(value.getInt32() == data);
		value.setType(knArray);
		TEST_ASSERT(value.getType() == knArray);
		TEST_ASSERT(value.getArrayType() == knInt32);
		TEST_ASSERT(value.getArraySize() == 1);
		TEST_ASSERT(value.getValue(0).getInt32() == data);
	}
	{
		// Test setType when converting from string to array.
		std::string data("Convert me into an array!");
		PYXValue value(data);
		TEST_ASSERT(value.getString() == data);
		value.setType(knArray);
		TEST_ASSERT(value.getType() == knArray);
		TEST_ASSERT(value.getArrayType() == knString);
		TEST_ASSERT(value.getArraySize() == 1);
		TEST_ASSERT(value.getValue(0).getString() == data);
	}
	{
		// Test setType when converting from array to POD type.
		std::string data[] = {"10", "20"};
		PYXValue value(data, 2);
		TEST_ASSERT(value.getType() == knArray);
		TEST_ASSERT(value.getArrayType() == knString);
		TEST_ASSERT(value.getArraySize() == 2);
		value.setType(knUInt8);
		TEST_ASSERT(value.getType() == knUInt8);
		TEST_ASSERT(value.getUInt8() == 10);
		TEST_ASSERT(value.getString() == data[0]);
	}

	{
		// Test construction from a string literal.
		char const * data = "Am I resolved to the wrong type?";
		PYXValue value(data);
		TEST_ASSERT(value.getType() == knString);
		TEST_ASSERT(value.getString() == data);
	}
	{
		// Test construction from an array of string literals.
		char const * data[] = {
			"Am I resolved to the wrong type?",
			"And am I?"};
		PYXValue value(data, 2);
		TEST_ASSERT(value.getType() == knArray);
		TEST_ASSERT(value.getArrayType() == knString);
		TEST_ASSERT(value.getString(0) == std::string(data[0]));
		TEST_ASSERT(value.getString(1) == std::string(data[1]));
	}
	{
		// Test construction from an array of string literals with a null value.
		char const * data[] = {
			"Am I resolved to the wrong type?",
			"And am I?"};
		PYXValue value(data, 2, data[0]);
		TEST_ASSERT(value.getType() == knArray);
		TEST_ASSERT(value.getArrayType() == knString);
		TEST_ASSERT(value.getString(0) == std::string(data[0]));
		TEST_ASSERT(value.getString(1) == std::string(data[1]));
		TEST_ASSERT(value.getValue(0).getString().empty());
		TEST_ASSERT(value.getValue(1).getString() == std::string(data[1]));
	}

	{
		// Test array construction, from a null, with a string literal null value specified.
		std::string * data = 0;
		char const * nullString = "Null string.";
		PYXValue value(data, 2, nullString);
		TEST_ASSERT(value.getType() == knArray);
		TEST_ASSERT(value.getArrayType() == knString);
		TEST_ASSERT(value.getArraySize() == 2);
		TEST_ASSERT(value.getValue(0) == PYXValue());
		TEST_ASSERT(value.getValue(1) == PYXValue());
	}
	{
		// Test array construction, from a non-null, with a null value specified.
		std::string data[3];
		std::string nonNullString = "Non-null string";
		std::string nullString = "Null string";
		data[0] = nonNullString;
		data[1] = nullString;
		data[2] = nonNullString;
		PYXValue value(data, 3, nullString);
		TEST_ASSERT(value.getType() == knArray);
		TEST_ASSERT(value.getArrayType() == knString);
		TEST_ASSERT(value.getArraySize() == 3);
		TEST_ASSERT(value.getString(0) == std::string(data[0]));
		TEST_ASSERT(value.getString(1) == std::string(data[1]));
		TEST_ASSERT(value.getString(2) == std::string(data[2]));
		TEST_ASSERT(value.getValue(0).getString() == nonNullString);
		TEST_ASSERT(value.getValue(1) == PYXValue());
		TEST_ASSERT(value.getValue(2).getString() == nonNullString);
	}

	{
		// Test array
		int32_t pVal[] = { 0, 1, 2, 3, -1, 5, -1, 7 };
		int nSize = sizeof(pVal)/sizeof(pVal[0]);
		int32_t nullVal = -1;
		PYXValue value(pVal, nSize);
		testArray(value, pVal, nSize, nullVal, false);
		{
			PYXValue other(value);
			testArray(other, pVal, nSize, nullVal, false);
			TEST_ASSERT((value == other) && (other == value));
			TEST_ASSERT(!(value < other) && !(other < value));
		}
		{
			PYXValue other;
			other = value;
			testArray(other, pVal, nSize, nullVal, false);
			TEST_ASSERT((value == other) && (other == value));
			TEST_ASSERT(!(value < other) && !(other < value));
		}
		{
			PYXValue other(pVal, nSize);
			other = value;
			testArray(other, pVal, nSize, nullVal, false);;
			TEST_ASSERT((value == other) && (other == value));
			TEST_ASSERT(!(value < other) && !(other < value));
		}
		{
			PYXValue other(pVal, nSize, nullVal);
			testArray(other, pVal, nSize, nullVal, true);
			TEST_ASSERT(!(value == other) && !(other == value));
			TEST_ASSERT(!(value < other) && (other < value));
		}
		{
			int32_t pVal2[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
			PYXValue other(pVal2, nSize);
			testArray(other, pVal2, nSize, nullVal, false);
			TEST_ASSERT(!(value == other) && !(other == value));
			TEST_ASSERT((value < other) && !(other < value));
		}
	}

	{
		// Test array setters
		typedef int32_t TD;
		TD data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
		int nSize = sizeof(data)/sizeof(data[0]);
		PYXValue value(data, nSize);
		int n = 0;
		{
			{
				bool val = true;
				data[n] = static_cast<TD>(val);
				value.set(val);
				testArray(value, data, nSize, 0, false);
			}
			{
				char val = 2;
				data[n] = static_cast<TD>(val);
				value.set(val);
				testArray(value, data, nSize, 0, false);
			}
			{
				int8_t val = 7;
				data[n] = static_cast<TD>(val);
				value.set(val);
				testArray(value, data, nSize, 0, false);
			}
			{
				uint8_t val = 8;
				data[n] = static_cast<TD>(val);
				value.set(val);
				testArray(value, data, nSize, 0, false);
			}
			{
				int16_t val = 15;
				data[n] = static_cast<TD>(val);
				value.set(val);
				testArray(value, data, nSize, 0, false);
			}
			{
				uint16_t val = 16;
				data[n] = static_cast<TD>(val);
				value.set(val);
				testArray(value, data, nSize, 0, false);
			}
			{
				int32_t val = 31;
				data[n] = static_cast<TD>(val);
				value.set(val);
				testArray(value, data, nSize, 0, false);
			}
			{
				uint32_t val = 32;
				data[n] = static_cast<TD>(val);
				value.set(val);
				testArray(value, data, nSize, 0, false);
			}
			{
				float val = 8.23f;
				data[n] = static_cast<TD>(val);
				value.set(val);
				testArray(value, data, nSize, 0, false);
			}
			{
				double val = 11.52;
				data[n] = static_cast<TD>(val);
				value.set(val);
				testArray(value, data, nSize, 0, false);
			}
			{
				std::string val = "123";
				data[n] = Helper::fromString<TD>(val);
				value.set(val);
				testArray(value, data, nSize, 0, false);
			}
		}

		{
			bool val = ++n;
			data[n] = static_cast<TD>(val);
			value.set(n, static_cast<TD>(val));
			testArray(value, data, nSize, 0, false);
		}
		{
			char val = 100 + ++n;
			data[n] = static_cast<TD>(val);
			value.set(n, static_cast<TD>(val));
			testArray(value, data, nSize, 0, false);
		}
		{
			int8_t val = 100 + ++n;
			data[n] = static_cast<TD>(val);
			value.set(n, static_cast<TD>(val));
			testArray(value, data, nSize, 0, false);
		}
		{
			uint8_t val = 100 + ++n;
			data[n] = static_cast<TD>(val);
			value.set(n, static_cast<TD>(val));
			testArray(value, data, nSize, 0, false);
		}
		{
			int16_t val = 100 + ++n;
			data[n] = static_cast<TD>(val);
			value.set(n, static_cast<TD>(val));
			testArray(value, data, nSize, 0, false);
		}
		{
			uint16_t val = 100 + ++n;
			data[n] = static_cast<TD>(val);
			value.set(n, static_cast<TD>(val));
			testArray(value, data, nSize, 0, false);
		}
		{
			int32_t val = 100 + ++n;
			data[n] = static_cast<TD>(val);
			value.set(n, static_cast<TD>(val));
			testArray(value, data, nSize, 0, false);
		}
		{
			uint32_t val = 100 + ++n;
			data[n] = static_cast<TD>(val);
			value.set(n, static_cast<TD>(val));
			testArray(value, data, nSize, 0, false);
		}
		{
			float val = 100.5f + ++n;
			data[n] = static_cast<TD>(val);
			value.set(n, static_cast<TD>(val));
			testArray(value, data, nSize, 0, false);
		}
		{
			double val = 100.5 + ++n;
			data[n] = static_cast<TD>(val);
			value.set(n, static_cast<TD>(val));
			testArray(value, data, nSize, 0, false);
		}
		{
			std::string val = "123.5";
			data[++n] = Helper::fromString<TD>(val);
			value.set(n, val);
			testArray(value, data, nSize, 0, false);
		}
	}

	{
		// Test string
		PYXValue::eType type = PYXValue::knString;
		std::string val("string");
		PYXValue value(val);
		testValueAndType(value, val);
		{
			PYXValue other(val);
			PYXValue other2(other);
			double newval = 3.14159265358979323846264338327;
			other.set(newval);
			testType(other, type);
			testValue(other, newval);
			other.setString("1.5");
			testValue(other, 1.5);
			std::string str("föo");
			other.setString(str);
			testValue(other, str);
			testValueAndType(other2, val);
		}
	}

	{
		// Test array of strings
		std::string nullval("");
		std::vector<std::string> vec;
		vec.push_back("zero");
		vec.push_back("one");
		vec.push_back("two");
		vec.push_back("three");
		vec.push_back("four");
		vec.push_back("five");
		vec.push_back("six");
		vec.push_back("seven");
		PYXValue value(&vec[0], static_cast<int>(vec.size()));
		testArray(value, &vec[0], static_cast<int>(vec.size()), nullval, false);
	}
	{
		// Test array of strings
		std::string nullval("null");
		std::vector<std::string> vec;
		vec.push_back("zero");
		vec.push_back("one");
		vec.push_back("two");
		vec.push_back("three");
		vec.push_back(nullval);
		vec.push_back("five");
		vec.push_back(nullval);
		vec.push_back("seven");
		PYXValue value(&vec[0], static_cast<int>(vec.size()), nullval);
		testArray(value, &vec[0], static_cast<int>(vec.size()), nullval, true);
	}
	{
		// Test array of strings
		std::string nullval("");
		std::vector<std::string> vec;
		vec.push_back("zero");
		vec.push_back("one");
		vec.push_back("two");
		vec.push_back("three");
		vec.push_back(nullval);
		vec.push_back("five");
		vec.push_back(nullval);
		vec.push_back("seven");
		PYXValue value(&vec[0], static_cast<int>(vec.size()), nullval);
		testArray(value, &vec[0], static_cast<int>(vec.size()), nullval, true);
	}

	{
		// Test non-nullable array of double: setting an element as a double or as a string should work
		typedef double TD;
		TD data[] = { 0.1, 1.2, 2.3, 3.4 };
		int nSize = sizeof(data)/sizeof(data[0]);
		TD nullval(data[1]);
		bool bNullable = false;
		PYXValue value1(data, nSize);
		PYXValue value2(data, nSize);
		int n = 0;
		{
			TD val = -0.123;
			data[n] = val;
			value1.set(val);
			value2.set(Helper::toString(val));
			testArray(value1, data, nSize, nullval, bNullable);
			testArray(value2, data, nSize, nullval, bNullable);
		}
		{
			TD val = -1.234;
			data[++n] = val;
			value1.set(n, val);
			value2.set(n, Helper::toString(val));
			testArray(value1, data, nSize, nullval, bNullable);
			testArray(value2, data, nSize, nullval, bNullable);
		}
		{
			TD val = -2.345;
			data[++n] = val;
			value1.set(n, val);
			value2.set(n, Helper::toString(val));
			testArray(value1, data, nSize, nullval, bNullable);
			testArray(value2, data, nSize, nullval, bNullable);
		}
		{
			TD val = nullval;
			data[++n] = val;
			value1.set(n, val);
			value2.set(n, Helper::toString(val));
			testArray(value1, data, nSize, nullval, bNullable);
			testArray(value2, data, nSize, nullval, bNullable);
		}
	}
	{
		// Test nullable array of double: setting an element as a double or as a string should work
		typedef double TD;
		TD data[] = { 0.1, 1.2, 2.3, 3.4 };
		int nSize = sizeof(data)/sizeof(data[0]);
		TD nullval(data[1]);
		bool bNullable = true;
		PYXValue value1(data, nSize, nullval);
		PYXValue value2(data, nSize, nullval);
		int n = 0;
		{
			TD val = -0.123;
			data[n] = val;
			value1.set(val);
			value2.set(Helper::toString(val));
			testArray(value1, data, nSize, nullval, bNullable);
			testArray(value2, data, nSize, nullval, bNullable);
		}
		{
			TD val = -1.234;
			data[++n] = val;
			value1.set(n, val);
			value2.set(n, Helper::toString(val));
			testArray(value1, data, nSize, nullval, bNullable);
			testArray(value2, data, nSize, nullval, bNullable);
		}
		{
			TD val = -2.345;
			data[++n] = val;
			value1.set(n, val);
			value2.set(n, Helper::toString(val));
			testArray(value1, data, nSize, nullval, bNullable);
			testArray(value2, data, nSize, nullval, bNullable);
		}
		{
			TD val = nullval;
			data[++n] = val;
			value1.set(n, val);
			value2.set(n, Helper::toString(val));
			testArray(value1, data, nSize, nullval, bNullable);
			testArray(value2, data, nSize, nullval, bNullable);
		}
	}
	{
		// Test non-nullable array of string: setting an element as a double or as a string should work
		typedef std::string TD;
		TD data[] = { Helper::toString("0.1"), Helper::toString("1.2"), Helper::toString("2.3"), Helper::toString("3.4") };
		int nSize = sizeof(data)/sizeof(data[0]);
		std::string nullval(data[1]);
		bool bNullable = false;
		PYXValue value1(data, nSize);
		PYXValue value2(data, nSize);
		int n = 0;
		{
			TD val = Helper::toString(-0.123);
			data[n] = val;
			value1.set(val);
			value2.set(Helper::toString(val));
			testArray(value1, data, nSize, nullval, bNullable);
			testArray(value2, data, nSize, nullval, bNullable);
		}
		{
			TD val = Helper::toString(-1.234);
			data[++n] = val;
			value1.set(n, val);
			value2.set(n, Helper::toString(val));
			testArray(value1, data, nSize, nullval, bNullable);
			testArray(value2, data, nSize, nullval, bNullable);
		}
		{
			TD val = Helper::toString(-2.345);
			data[++n] = val;
			value1.set(n, val);
			value2.set(n, Helper::toString(val));
			testArray(value1, data, nSize, nullval, bNullable);
			testArray(value2, data, nSize, nullval, bNullable);
		}
		{
			TD val = nullval;
			data[++n] = val;
			value1.set(n, val);
			value2.set(n, Helper::toString(val));
			testArray(value1, data, nSize, nullval, bNullable);
			testArray(value2, data, nSize, nullval, bNullable);
		}
	}
	{
		// Test nullable array of string: setting an element as a double or as a string should work
		typedef std::string TD;
		TD data[] = { Helper::toString("0.1"), Helper::toString("1.2"), Helper::toString("2.3"), Helper::toString("3.4") };
		int nSize = sizeof(data)/sizeof(data[0]);
		std::string nullval(data[1]);
		bool bNullable = true;
		PYXValue value1(data, nSize, nullval);
		PYXValue value2(data, nSize, nullval);
		int n = 0;
		{
			TD val = Helper::toString(-0.123);
			data[n] = val; value1.set(val);
			value2.set(Helper::toString(val));
			testArray(value1, data, nSize, nullval, bNullable);
			testArray(value2, data, nSize, nullval, bNullable);
		}
		{
			TD val = Helper::toString(-1.234);
			data[++n] = val;
			value1.set(n, val);
			value2.set(n, Helper::toString(val));
			testArray(value1, data, nSize, nullval, bNullable);
			testArray(value2, data, nSize, nullval, bNullable);
		}
		{
			TD val = Helper::toString(-2.345);
			data[++n] = val;
			value1.set(n, val);
			value2.set(n, Helper::toString(val));
			testArray(value1, data, nSize, nullval, bNullable);
			testArray(value2, data, nSize, nullval, bNullable);
		}
		{
			TD val = nullval;
			data[++n] = val;
			value1.set(n, val);
			value2.set(n, Helper::toString(val));
			testArray(value1, data, nSize, nullval, bNullable);
			testArray(value2, data, nSize, nullval, bNullable);
		}
	}

	{
		// Test compareLess.
		int arrayInt[] = { 0, 1, 2 };
		int arrayInt2[] = { 0, 2, 2 };
		double arrayDouble[] = { 1.0, 1.1, 1.2 };
		double arrayDouble2[] = { 1.0, 1.2, 1.2 };
		std::string arrayString[] = { "a", "abc", "ac" };
		std::string arrayString2[] = { "a", "ac", "ac" };

		// Vector has values in correct order.
		std::vector<PYXValue> vecValue;

		vecValue.push_back(PYXValue());

		vecValue.push_back(PYXValue(false));
		vecValue.push_back(PYXValue(true));

		vecValue.push_back(PYXValue('\0'));
		vecValue.push_back(PYXValue('A'));
		vecValue.push_back(PYXValue('B'));
		vecValue.push_back(PYXValue('C'));

		vecValue.push_back(PYXValue(-100));
		vecValue.push_back(PYXValue(-1));
		vecValue.push_back(PYXValue(0));
		vecValue.push_back(PYXValue(1));
		vecValue.push_back(PYXValue(100));

		vecValue.push_back(PYXValue(-3.14f));
		vecValue.push_back(PYXValue(0.0f));
		vecValue.push_back(PYXValue(3.14f));

		vecValue.push_back(PYXValue(-3.14));
		vecValue.push_back(PYXValue(0.0));
		vecValue.push_back(PYXValue(3.14));

		vecValue.push_back(PYXValue(std::string()));
		vecValue.push_back(PYXValue(std::string("-1")));
		vecValue.push_back(PYXValue(std::string("0")));
		vecValue.push_back(PYXValue(std::string("1")));
		vecValue.push_back(PYXValue(std::string("12")));
		vecValue.push_back(PYXValue(std::string("123")));
		vecValue.push_back(PYXValue(std::string("13")));
		vecValue.push_back(PYXValue(std::string("23")));
		vecValue.push_back(PYXValue(std::string("3.14")));
		vecValue.push_back(PYXValue(std::string("a")));
		vecValue.push_back(PYXValue(std::string("ab")));
		vecValue.push_back(PYXValue(std::string("abc")));
		vecValue.push_back(PYXValue(std::string("ac")));
		vecValue.push_back(PYXValue(std::string("bc")));

		vecValue.push_back(PYXValue(arrayInt, 3, arrayInt[0]));
		vecValue.push_back(PYXValue(arrayInt2, 3, arrayInt2[0]));
		vecValue.push_back(PYXValue(arrayInt, 1));
		vecValue.push_back(PYXValue(arrayInt2, 3, arrayInt2[2]));
		vecValue.push_back(PYXValue(arrayInt, 3, arrayInt[1]));
		vecValue.push_back(PYXValue(arrayInt, 2));
		vecValue.push_back(PYXValue(arrayInt, 3, arrayInt[2]));
		vecValue.push_back(PYXValue(arrayInt, 3, 100));
		vecValue.push_back(PYXValue(arrayInt, 3));
		vecValue.push_back(PYXValue(arrayInt2, 3, 100));
		vecValue.push_back(PYXValue(arrayInt2, 3));

		vecValue.push_back(PYXValue(arrayDouble, 3, arrayDouble[0]));
		vecValue.push_back(PYXValue(arrayDouble2, 3, arrayDouble2[0]));
		vecValue.push_back(PYXValue(arrayDouble, 1));
		vecValue.push_back(PYXValue(arrayDouble2, 3, arrayDouble2[2]));
		vecValue.push_back(PYXValue(arrayDouble, 3, arrayDouble[1]));
		vecValue.push_back(PYXValue(arrayDouble, 2));
		vecValue.push_back(PYXValue(arrayDouble, 3, arrayDouble[2]));
		vecValue.push_back(PYXValue(arrayDouble, 3, 100.0));
		vecValue.push_back(PYXValue(arrayDouble, 3));
		vecValue.push_back(PYXValue(arrayDouble2, 3, 100.0));
		vecValue.push_back(PYXValue(arrayDouble2, 3));

		vecValue.push_back(PYXValue(arrayString, 3, arrayString[0]));
		vecValue.push_back(PYXValue(arrayString2, 3, arrayString2[0]));
		vecValue.push_back(PYXValue(arrayString, 1));
		vecValue.push_back(PYXValue(arrayString2, 3, arrayString2[2]));
		vecValue.push_back(PYXValue(arrayString, 3, arrayString[1]));
		vecValue.push_back(PYXValue(arrayString, 2));
		vecValue.push_back(PYXValue(arrayString, 3, arrayString[2]));
		vecValue.push_back(PYXValue(arrayString, 3, std::string("dummy")));
		vecValue.push_back(PYXValue(arrayString, 3));
		vecValue.push_back(PYXValue(arrayString2, 3, std::string("dummy")));
		vecValue.push_back(PYXValue(arrayString2, 3));

		// Fill set from vector.
		std::set<PYXValue> setValue;
		for (int n = 0; n < static_cast<int>(vecValue.size()) * 10
			|| setValue.size() < vecValue.size(); ++n)
		{
			int nPos = static_cast<int>(static_cast<double>(rand())
				/ RAND_MAX * vecValue.size());
			if (nPos == vecValue.size())
			{
				--nPos;
			}
			setValue.insert(vecValue[nPos]);
		}

		// Check set against vector.
		TEST_ASSERT(vecValue.size() == setValue.size());
		std::set<PYXValue>::const_iterator it = setValue.begin();
		for (int n = 0; n != vecValue.size(); ++n, ++it)
		{
			TEST_ASSERT(vecValue[n] == *it);
		}

		// Test stream operators.
		std::ostringstream out;
		for (int n = 0; n != vecValue.size(); ++n)
		{
			out << vecValue[n] << ' ';
		}
		out << std::ends;
		std::istringstream in(out.str());
		for (int n = 0; n != vecValue.size(); ++n)
		{
			PYXValue value;
			in >> value;
			TEST_ASSERT(value == vecValue[n]);
		}
	}

	{
		// Test polymorphic isNumeric.
		{
			PYXValue partialNumericString("42 is a number.");
			TEST_ASSERT(partialNumericString.isString());
			TEST_ASSERT(!partialNumericString.isNumeric());
		}
		{
			PYXValue numericString("3.14159265358979");
			TEST_ASSERT(numericString.isString());
			TEST_ASSERT(numericString.isNumeric());
		}
		{
			PYXValue emptyString("");
			TEST_ASSERT(emptyString.isString());
			TEST_ASSERT(!emptyString.isNumeric());
		}
		{
			PYXValue trueBool(true);
			TEST_ASSERT(trueBool.isBool());
			TEST_ASSERT(trueBool.isNumeric());
		}
		{
			PYXValue falseBool(false);
			TEST_ASSERT(falseBool.isBool());
			TEST_ASSERT(falseBool.isNumeric());
		}
		{
			PYXValue nonNumericChar('x');
			TEST_ASSERT(nonNumericChar.isChar());
			TEST_ASSERT(!nonNumericChar.isNumeric());
		}
		{
			PYXValue numericChar('4');
			TEST_ASSERT(numericChar.isChar());
			TEST_ASSERT(numericChar.isNumeric());
		}
		{
			PYXValue numeric(42.0);
			TEST_ASSERT(numeric.isDouble());
			TEST_ASSERT(numeric.isNumeric());
		}
		{
			PYXValue nullValue;
			TEST_ASSERT(nullValue.isNull());
			TEST_ASSERT(!nullValue.isNumeric());
		}
		{
			std::string stringsStartingWithNumeric[] = {"1.001", "two"};
			PYXValue numericStringArray(stringsStartingWithNumeric, 2);
			TEST_ASSERT(numericStringArray.isArray());
			TEST_ASSERT(numericStringArray.getArrayType() == knString);
			TEST_ASSERT(numericStringArray.isNumeric());
		}
		{
			std::string stringsStartingWithNonNumeric[] = {"one", "2"};
			PYXValue nonNumericStringArray(stringsStartingWithNonNumeric, 2);
			TEST_ASSERT(nonNumericStringArray.isArray());
			TEST_ASSERT(nonNumericStringArray.getArrayType() == knString);
			TEST_ASSERT(!nonNumericStringArray.isNumeric());
		}
	}

	//test floats round to integer assingments
	{
		PYXValue intValue(0);

		intValue.setDouble(0.1);
		TEST_ASSERT(intValue.getInt32() == 0);

		intValue.setDouble(-0.1);
		TEST_ASSERT(intValue.getInt32() == 0);

		intValue.setDouble(0.6);
		TEST_ASSERT(intValue.getInt32() == 1);

		intValue.setDouble(-0.6);
		TEST_ASSERT(intValue.getInt32() == -1);
	}

	//test boolean conversion
	{
		PYXValue boolValue(false);

		boolValue.setDouble(0.1);
		TEST_ASSERT(boolValue.getBool() == true);

		boolValue.setDouble(0);
		TEST_ASSERT(boolValue.getBool() == false);

		
		boolValue.setInt32(1);
		TEST_ASSERT(boolValue.getBool() == true);

		boolValue.setInt32(0);
		TEST_ASSERT(boolValue.getBool() == false);
	}
}

#pragma warning(pop)

#ifndef DOXYGEN_IGNORE // Don't document this internal class.

const PYXValue::VTable PYXValue::VTable::m_vtable[] =
{

	// VTABLE(knNull) ---------------------------------------------------------
	{
		// Destruct
		&Helper::doNothing,							// destruct

		// Types
		&Helper::getConstant<eType, knNull>,		// getType
		&Helper::nullSetType,						// setType

		// Comparisons
		&nullCompareEqualTo,						// compareEqualTo
		&nullCompareLess,							// compareLess

		// Copying
		&Helper::doNothing,							// constructCopy
		&Helper::doNothing,							// assignCopy

		// Properties
		&Helper::nullIsNumeric,						// isNumeric

		// Getters
		&Helper::getConstant<bool, 0>,				// getBool
		&Helper::getConstant<char, 0>,				// getChar
		&Helper::getConstant<int8_t, 0>,			// getInt8
		&Helper::getConstant<uint8_t, 0>,			// getUInt8
		&Helper::getConstant<int16_t, 0>,			// getInt16
		&Helper::getConstant<uint16_t, 0>,			// getUInt16
		&Helper::getConstant<int32_t, 0>,			// getInt32
		&Helper::getConstant<uint32_t, 0>,			// getUInt32
		&Helper::getConstantZero<float>,			// getFloat
		&Helper::getConstantZero<double>,			// getDouble
		&Helper::nullGetString,						// getString

		// Setters
		&Helper::doNothing<bool>,					// setBool
		&Helper::doNothing<char>,					// setChar
		&Helper::doNothing<int8_t>,					// setInt8
		&Helper::doNothing<uint8_t>,				// setUInt8
		&Helper::doNothing<int16_t>,				// setInt16
		&Helper::doNothing<uint16_t>,				// setUInt16
		&Helper::doNothing<int32_t>,				// setInt32
		&Helper::doNothing<uint32_t>,				// setUInt32
		&Helper::doNothing<float>,					// setFloat
		&Helper::doNothing<double>,					// setDouble
		&Helper::doNothing,							// setString

		// Arrays
		&nullGetValue,					// getValue

		&Helper::getConstant<bool, 0>,				// arrayGetBool
		&Helper::getConstant<char, 0>,				// arrayGetChar
		&Helper::getConstant<int8_t, 0>,			// arrayGetInt8
		&Helper::getConstant<uint8_t, 0>,			// arrayGetUInt8
		&Helper::getConstant<int16_t, 0>,			// arrayGetInt16
		&Helper::getConstant<uint16_t, 0>,			// arrayGetUInt16
		&Helper::getConstant<int32_t, 0>,			// arrayGetInt32
		&Helper::getConstant<uint32_t, 0>,			// arrayGetUInt32
		&Helper::getConstantZero<float>,				// arrayGetFloat
		&Helper::getConstantZero<double>,			// arrayGetDouble
		&Helper::nullGetString,						// arrayGetString

		&Helper::doNothing<bool>,					// arraySetBool
		&Helper::doNothing<char>,					// arraySetChar
		&Helper::doNothing<int8_t>,					// arraySetInt8
		&Helper::doNothing<uint8_t>,				// arraySetUInt8
		&Helper::doNothing<int16_t>,				// arraySetInt16
		&Helper::doNothing<uint16_t>,				// arraySetUInt16
		&Helper::doNothing<int32_t>,				// arraySetInt32
		&Helper::doNothing<uint32_t>,				// arraySetUInt32
		&Helper::doNothing<float>,					// arraySetFloat
		&Helper::doNothing<double>,					// arraySetDouble
		&Helper::doNothing<const std::string&>,		// arraySetString

		&Helper::getConstant<const void*, 0>,		// getPtr
	},

	// VTABLE(knBool) ---------------------------------------------------------
	{
		#define CPPTYPE bool

		// Destruct
		&Helper::doNothing,							// destruct

		// Types
		&Helper::getConstant<eType, knBool>,		// getType
		&Helper::setType<CPPTYPE>,					// setType

		// Comparisons
		&Helper::compareEqualTo<CPPTYPE>,			// compareEqualTo
		&Helper::compareLess<CPPTYPE>,				// compareLess

		// Copying
		&Helper::assignCopy<CPPTYPE>,				// constructCopy
		&Helper::assignCopy<CPPTYPE>,				// assignCopy

		// Properties
		&Helper::isNumeric<CPPTYPE>,				// isNumeric

		// Getters
		&Helper::getDataAs<CPPTYPE, bool>,			// getBool
		&Helper::getDataAs<CPPTYPE, char>,			// getChar
		&Helper::getDataAs<CPPTYPE, int8_t>,		// getInt8
		&Helper::getDataAs<CPPTYPE, uint8_t>,		// getUInt8
		&Helper::getDataAs<CPPTYPE, int16_t>,		// getInt16
		&Helper::getDataAs<CPPTYPE, uint16_t>,		// getUInt16
		&Helper::getDataAs<CPPTYPE, int32_t>,		// getInt32
		&Helper::getDataAs<CPPTYPE, uint32_t>,		// getUInt32
		&Helper::getDataAs<CPPTYPE, float>,			// getFloat
		&Helper::getDataAs<CPPTYPE, double>,		// getDouble
		&Helper::getDataAsString<CPPTYPE>,			// getString

		// Setters
		&Helper::setDataAs<CPPTYPE, bool>,			// setBool
		&Helper::setDataAs<CPPTYPE, char>,			// setChar
		&Helper::setDataAs<CPPTYPE, int8_t>,		// setInt8
		&Helper::setDataAs<CPPTYPE, uint8_t>,		// setUInt8
		&Helper::setDataAs<CPPTYPE, int16_t>,		// setInt16
		&Helper::setDataAs<CPPTYPE, uint16_t>,		// setUInt16
		&Helper::setDataAs<CPPTYPE, int32_t>,		// setInt32
		&Helper::setDataAs<CPPTYPE, uint32_t>,		// setUInt32
		&Helper::setDataAs<CPPTYPE, float>,			// setFloat
		&Helper::setDataAs<CPPTYPE, double>,		// setDouble
		&Helper::setDataAsString<CPPTYPE>,			// setString

		// Arrays
		&singleGetValue,				// getValue

		&Helper::getDataAs<CPPTYPE, bool>,			// arrayGetBool
		&Helper::getDataAs<CPPTYPE, char>,			// arrayGetChar
		&Helper::getDataAs<CPPTYPE, int8_t>,		// arrayGetInt8
		&Helper::getDataAs<CPPTYPE, uint8_t>,		// arrayGetUInt8
		&Helper::getDataAs<CPPTYPE, int16_t>,		// arrayGetInt16
		&Helper::getDataAs<CPPTYPE, uint16_t>,		// arrayGetUInt16
		&Helper::getDataAs<CPPTYPE, int32_t>,		// arrayGetInt32
		&Helper::getDataAs<CPPTYPE, uint32_t>,		// arrayGetUInt32
		&Helper::getDataAs<CPPTYPE, float>,			// arrayGetFloat
		&Helper::getDataAs<CPPTYPE, double>,		// arrayGetDouble
		&Helper::getDataAsString<CPPTYPE>,			// arrayGetString

		&Helper::setDataAs<CPPTYPE, bool>,			// arraySetBool
		&Helper::setDataAs<CPPTYPE, char>,			// arraySetChar
		&Helper::setDataAs<CPPTYPE, int8_t>,		// arraySetInt8
		&Helper::setDataAs<CPPTYPE, uint8_t>,		// arraySetUInt8
		&Helper::setDataAs<CPPTYPE, int16_t>,		// arraySetInt16
		&Helper::setDataAs<CPPTYPE, uint16_t>,		// arraySetUInt16
		&Helper::setDataAs<CPPTYPE, int32_t>,		// arraySetInt32
		&Helper::setDataAs<CPPTYPE, uint32_t>,		// arraySetUInt32
		&Helper::setDataAs<CPPTYPE, float>,			// arraySetFloat
		&Helper::setDataAs<CPPTYPE, double>,		// arraySetDouble
		&Helper::setDataAsString<CPPTYPE>,			// arraySetString

		&Helper::getPtr<CPPTYPE>,					// getPtr

		#undef CPPTYPE
	},

	// VTABLE(knChar) ---------------------------------------------------------
	{
		#define CPPTYPE char

		// Destruct
		&Helper::doNothing,							// destruct

		// Types
		&Helper::getConstant<eType, knChar>,		// getType
		&Helper::setType<CPPTYPE>,					// setType

		// Comparisons
		&Helper::compareEqualTo<CPPTYPE>,			// compareEqualTo
		&Helper::compareLess<CPPTYPE>,				// compareLess

		// Copying
		&Helper::assignCopy<CPPTYPE>,				// constructCopy
		&Helper::assignCopy<CPPTYPE>,				// assignCopy

		// Properties
		&Helper::isNumeric<CPPTYPE>,				// isNumeric

		// Getters
		&Helper::getDataAs<CPPTYPE, bool>,			// getBool
		&Helper::getDataAs<CPPTYPE, char>,			// getChar
		&Helper::getDataAs<CPPTYPE, int8_t>,		// getInt8
		&Helper::getDataAs<CPPTYPE, uint8_t>,		// getUInt8
		&Helper::getDataAs<CPPTYPE, int16_t>,		// getInt16
		&Helper::getDataAs<CPPTYPE, uint16_t>,		// getUInt16
		&Helper::getDataAs<CPPTYPE, int32_t>,		// getInt32
		&Helper::getDataAs<CPPTYPE, uint32_t>,		// getUInt32
		&Helper::getDataAs<CPPTYPE, float>,			// getFloat
		&Helper::getDataAs<CPPTYPE, double>,		// getDouble
		&Helper::getDataAsString<CPPTYPE>,			// getString

		// Setters
		&Helper::setDataAs<CPPTYPE, bool>,			// setBool
		&Helper::setDataAs<CPPTYPE, char>,			// setChar
		&Helper::setDataAs<CPPTYPE, int8_t>,		// setInt8
		&Helper::setDataAs<CPPTYPE, uint8_t>,		// setUInt8
		&Helper::setDataAs<CPPTYPE, int16_t>,		// setInt16
		&Helper::setDataAs<CPPTYPE, uint16_t>,		// setUInt16
		&Helper::setDataAs<CPPTYPE, int32_t>,		// setInt32
		&Helper::setDataAs<CPPTYPE, uint32_t>,		// setUInt32
		&Helper::setDataAsRound<CPPTYPE, float>,	// setFloat
		&Helper::setDataAsRound<CPPTYPE, double>,	// setDouble
		&Helper::setDataAsString<CPPTYPE>,			// setString

		// Arrays
		&singleGetValue,				// getValue

		&Helper::getDataAs<CPPTYPE, bool>,			// arrayGetBool
		&Helper::getDataAs<CPPTYPE, char>,			// arrayGetChar
		&Helper::getDataAs<CPPTYPE, int8_t>,		// arrayGetInt8
		&Helper::getDataAs<CPPTYPE, uint8_t>,		// arrayGetUInt8
		&Helper::getDataAs<CPPTYPE, int16_t>,		// arrayGetInt16
		&Helper::getDataAs<CPPTYPE, uint16_t>,		// arrayGetUInt16
		&Helper::getDataAs<CPPTYPE, int32_t>,		// arrayGetInt32
		&Helper::getDataAs<CPPTYPE, uint32_t>,		// arrayGetUInt32
		&Helper::getDataAs<CPPTYPE, float>,			// arrayGetFloat
		&Helper::getDataAs<CPPTYPE, double>,		// arrayGetDouble
		&Helper::getDataAsString<CPPTYPE>,			// arrayGetString

		&Helper::setDataAs<CPPTYPE, bool>,			// arraySetBool
		&Helper::setDataAs<CPPTYPE, char>,			// arraySetChar
		&Helper::setDataAs<CPPTYPE, int8_t>,		// arraySetInt8
		&Helper::setDataAs<CPPTYPE, uint8_t>,		// arraySetUInt8
		&Helper::setDataAs<CPPTYPE, int16_t>,		// arraySetInt16
		&Helper::setDataAs<CPPTYPE, uint16_t>,		// arraySetUInt16
		&Helper::setDataAs<CPPTYPE, int32_t>,		// arraySetInt32
		&Helper::setDataAs<CPPTYPE, uint32_t>,		// arraySetUInt32
		&Helper::setDataAsRound<CPPTYPE, float>,	// arraySetFloat
		&Helper::setDataAsRound<CPPTYPE, double>,	// arraySetDouble
		&Helper::setDataAsString<CPPTYPE>,			// arraySetString

		&Helper::getPtr<CPPTYPE>,					// getPtr

		#undef CPPTYPE
	},

	// VTABLE(knInt8) ---------------------------------------------------------
	{
		#define CPPTYPE int8_t

		// Destruct
		&Helper::doNothing,							// destruct

		// Types
		&Helper::getConstant<eType, knInt8>,		// getType
		&Helper::setType<CPPTYPE>,					// setType

		// Comparisons
		&Helper::compareEqualTo<CPPTYPE>,			// compareEqualTo
		&Helper::compareLess<CPPTYPE>,				// compareLess

		// Copying
		&Helper::assignCopy<CPPTYPE>,				// constructCopy
		&Helper::assignCopy<CPPTYPE>,				// assignCopy

		// Properties
		&Helper::isNumeric<CPPTYPE>,				// isNumeric

		// Getters
		&Helper::getDataAs<CPPTYPE, bool>,			// getBool
		&Helper::getDataAs<CPPTYPE, char>,			// getChar
		&Helper::getDataAs<CPPTYPE, int8_t>,		// getInt8
		&Helper::getDataAs<CPPTYPE, uint8_t>,		// getUInt8
		&Helper::getDataAs<CPPTYPE, int16_t>,		// getInt16
		&Helper::getDataAs<CPPTYPE, uint16_t>,		// getUInt16
		&Helper::getDataAs<CPPTYPE, int32_t>,		// getInt32
		&Helper::getDataAs<CPPTYPE, uint32_t>,		// getUInt32
		&Helper::getDataAs<CPPTYPE, float>,			// getFloat
		&Helper::getDataAs<CPPTYPE, double>,		// getDouble
		&Helper::getDataAsString<CPPTYPE>,			// getString

		// Setters
		&Helper::setDataAs<CPPTYPE, bool>,			// setBool
		&Helper::setDataAs<CPPTYPE, char>,			// setChar
		&Helper::setDataAs<CPPTYPE, int8_t>,		// setInt8
		&Helper::setDataAs<CPPTYPE, uint8_t>,		// setUInt8
		&Helper::setDataAs<CPPTYPE, int16_t>,		// setInt16
		&Helper::setDataAs<CPPTYPE, uint16_t>,		// setUInt16
		&Helper::setDataAs<CPPTYPE, int32_t>,		// setInt32
		&Helper::setDataAs<CPPTYPE, uint32_t>,		// setUInt32
		&Helper::setDataAsRound<CPPTYPE, float>,	// setFloat
		&Helper::setDataAsRound<CPPTYPE, double>,	// setDouble
		&Helper::setDataAsString<CPPTYPE>,			// setString

		// Arrays
		&singleGetValue,				// getValue

		&Helper::getDataAs<CPPTYPE, bool>,			// arrayGetBool
		&Helper::getDataAs<CPPTYPE, char>,			// arrayGetChar
		&Helper::getDataAs<CPPTYPE, int8_t>,		// arrayGetInt8
		&Helper::getDataAs<CPPTYPE, uint8_t>,		// arrayGetUInt8
		&Helper::getDataAs<CPPTYPE, int16_t>,		// arrayGetInt16
		&Helper::getDataAs<CPPTYPE, uint16_t>,		// arrayGetUInt16
		&Helper::getDataAs<CPPTYPE, int32_t>,		// arrayGetInt32
		&Helper::getDataAs<CPPTYPE, uint32_t>,		// arrayGetUInt32
		&Helper::getDataAs<CPPTYPE, float>,			// arrayGetFloat
		&Helper::getDataAs<CPPTYPE, double>,		// arrayGetDouble
		&Helper::getDataAsString<CPPTYPE>,			// arrayGetString

		&Helper::setDataAs<CPPTYPE, bool>,			// arraySetBool
		&Helper::setDataAs<CPPTYPE, char>,			// arraySetChar
		&Helper::setDataAs<CPPTYPE, int8_t>,		// arraySetInt8
		&Helper::setDataAs<CPPTYPE, uint8_t>,		// arraySetUInt8
		&Helper::setDataAs<CPPTYPE, int16_t>,		// arraySetInt16
		&Helper::setDataAs<CPPTYPE, uint16_t>,		// arraySetUInt16
		&Helper::setDataAs<CPPTYPE, int32_t>,		// arraySetInt32
		&Helper::setDataAs<CPPTYPE, uint32_t>,		// arraySetUInt32
		&Helper::setDataAsRound<CPPTYPE, float>,	// arraySetFloat
		&Helper::setDataAsRound<CPPTYPE, double>,	// arraySetDouble
		&Helper::setDataAsString<CPPTYPE>,			// arraySetString

		&Helper::getPtr<CPPTYPE>,					// getPtr

		#undef CPPTYPE
	},

	// VTABLE(knUInt8) --------------------------------------------------------
	{
		#define CPPTYPE uint8_t

		// Destruct
		&Helper::doNothing,							// destruct

		// Types
		&Helper::getConstant<eType, knUInt8>,		// getType
		&Helper::setType<CPPTYPE>,					// setType

		// Comparisons
		&Helper::compareEqualTo<CPPTYPE>,			// compareEqualTo
		&Helper::compareLess<CPPTYPE>,				// compareLess

		// Copying
		&Helper::assignCopy<CPPTYPE>,				// constructCopy
		&Helper::assignCopy<CPPTYPE>,				// assignCopy

		// Properties
		&Helper::isNumeric<CPPTYPE>,				// isNumeric

		// Getters
		&Helper::getDataAs<CPPTYPE, bool>,			// getBool
		&Helper::getDataAs<CPPTYPE, char>,			// getChar
		&Helper::getDataAs<CPPTYPE, int8_t>,		// getInt8
		&Helper::getDataAs<CPPTYPE, uint8_t>,		// getUInt8
		&Helper::getDataAs<CPPTYPE, int16_t>,		// getInt16
		&Helper::getDataAs<CPPTYPE, uint16_t>,		// getUInt16
		&Helper::getDataAs<CPPTYPE, int32_t>,		// getInt32
		&Helper::getDataAs<CPPTYPE, uint32_t>,		// getUInt32
		&Helper::getDataAs<CPPTYPE, float>,			// getFloat
		&Helper::getDataAs<CPPTYPE, double>,		// getDouble
		&Helper::getDataAsString<CPPTYPE>,			// getString

		// Setters
		&Helper::setDataAs<CPPTYPE, bool>,			// setBool
		&Helper::setDataAs<CPPTYPE, char>,			// setChar
		&Helper::setDataAs<CPPTYPE, int8_t>,		// setInt8
		&Helper::setDataAs<CPPTYPE, uint8_t>,		// setUInt8
		&Helper::setDataAs<CPPTYPE, int16_t>,		// setInt16
		&Helper::setDataAs<CPPTYPE, uint16_t>,		// setUInt16
		&Helper::setDataAs<CPPTYPE, int32_t>,		// setInt32
		&Helper::setDataAs<CPPTYPE, uint32_t>,		// setUInt32
		&Helper::setDataAsRound<CPPTYPE, float>,	// setFloat
		&Helper::setDataAsRound<CPPTYPE, double>,	// setDouble
		&Helper::setDataAsString<CPPTYPE>,			// setString

		// Arrays
		&singleGetValue,				// getValue

		&Helper::getDataAs<CPPTYPE, bool>,			// arrayGetBool
		&Helper::getDataAs<CPPTYPE, char>,			// arrayGetChar
		&Helper::getDataAs<CPPTYPE, int8_t>,		// arrayGetInt8
		&Helper::getDataAs<CPPTYPE, uint8_t>,		// arrayGetUInt8
		&Helper::getDataAs<CPPTYPE, int16_t>,		// arrayGetInt16
		&Helper::getDataAs<CPPTYPE, uint16_t>,		// arrayGetUInt16
		&Helper::getDataAs<CPPTYPE, int32_t>,		// arrayGetInt32
		&Helper::getDataAs<CPPTYPE, uint32_t>,		// arrayGetUInt32
		&Helper::getDataAs<CPPTYPE, float>,			// arrayGetFloat
		&Helper::getDataAs<CPPTYPE, double>,		// arrayGetDouble
		&Helper::getDataAsString<CPPTYPE>,			// arrayGetString

		&Helper::setDataAs<CPPTYPE, bool>,			// arraySetBool
		&Helper::setDataAs<CPPTYPE, char>,			// arraySetChar
		&Helper::setDataAs<CPPTYPE, int8_t>,		// arraySetInt8
		&Helper::setDataAs<CPPTYPE, uint8_t>,		// arraySetUInt8
		&Helper::setDataAs<CPPTYPE, int16_t>,		// arraySetInt16
		&Helper::setDataAs<CPPTYPE, uint16_t>,		// arraySetUInt16
		&Helper::setDataAs<CPPTYPE, int32_t>,		// arraySetInt32
		&Helper::setDataAs<CPPTYPE, uint32_t>,		// arraySetUInt32
		&Helper::setDataAsRound<CPPTYPE, float>,	// arraySetFloat
		&Helper::setDataAsRound<CPPTYPE, double>,	// arraySetDouble
		&Helper::setDataAsString<CPPTYPE>,			// arraySetString

		&Helper::getPtr<CPPTYPE>,					// getPtr

		#undef CPPTYPE
	},

	// VTABLE(knInt16) --------------------------------------------------------
	{
		#define CPPTYPE int16_t

		// Destruct
		&Helper::doNothing,							// destruct

		// Types
		&Helper::getConstant<eType, knInt16>,		// getType
		&Helper::setType<CPPTYPE>,					// setType

		// Comparisons
		&Helper::compareEqualTo<CPPTYPE>,			// compareEqualTo
		&Helper::compareLess<CPPTYPE>,				// compareLess

		// Copying
		&Helper::assignCopy<CPPTYPE>,				// constructCopy
		&Helper::assignCopy<CPPTYPE>,				// assignCopy

		// Properties
		&Helper::isNumeric<CPPTYPE>,				// isNumeric

		// Getters
		&Helper::getDataAs<CPPTYPE, bool>,			// getBool
		&Helper::getDataAs<CPPTYPE, char>,			// getChar
		&Helper::getDataAs<CPPTYPE, int8_t>,		// getInt8
		&Helper::getDataAs<CPPTYPE, uint8_t>,		// getUInt8
		&Helper::getDataAs<CPPTYPE, int16_t>,		// getInt16
		&Helper::getDataAs<CPPTYPE, uint16_t>,		// getUInt16
		&Helper::getDataAs<CPPTYPE, int32_t>,		// getInt32
		&Helper::getDataAs<CPPTYPE, uint32_t>,		// getUInt32
		&Helper::getDataAs<CPPTYPE, float>,			// getFloat
		&Helper::getDataAs<CPPTYPE, double>,		// getDouble
		&Helper::getDataAsString<CPPTYPE>,			// getString

		// Setters
		&Helper::setDataAs<CPPTYPE, bool>,			// setBool
		&Helper::setDataAs<CPPTYPE, char>,			// setChar
		&Helper::setDataAs<CPPTYPE, int8_t>,		// setInt8
		&Helper::setDataAs<CPPTYPE, uint8_t>,		// setUInt8
		&Helper::setDataAs<CPPTYPE, int16_t>,		// setInt16
		&Helper::setDataAs<CPPTYPE, uint16_t>,		// setUInt16
		&Helper::setDataAs<CPPTYPE, int32_t>,		// setInt32
		&Helper::setDataAs<CPPTYPE, uint32_t>,		// setUInt32
		&Helper::setDataAsRound<CPPTYPE, float>,	// setFloat
		&Helper::setDataAsRound<CPPTYPE, double>,	// setDouble
		&Helper::setDataAsString<CPPTYPE>,			// setString

		// Arrays
		&singleGetValue,				// getValue

		&Helper::getDataAs<CPPTYPE, bool>,			// arrayGetBool
		&Helper::getDataAs<CPPTYPE, char>,			// arrayGetChar
		&Helper::getDataAs<CPPTYPE, int8_t>,		// arrayGetInt8
		&Helper::getDataAs<CPPTYPE, uint8_t>,		// arrayGetUInt8
		&Helper::getDataAs<CPPTYPE, int16_t>,		// arrayGetInt16
		&Helper::getDataAs<CPPTYPE, uint16_t>,		// arrayGetUInt16
		&Helper::getDataAs<CPPTYPE, int32_t>,		// arrayGetInt32
		&Helper::getDataAs<CPPTYPE, uint32_t>,		// arrayGetUInt32
		&Helper::getDataAs<CPPTYPE, float>,			// arrayGetFloat
		&Helper::getDataAs<CPPTYPE, double>,		// arrayGetDouble
		&Helper::getDataAsString<CPPTYPE>,			// arrayGetString

		&Helper::setDataAs<CPPTYPE, bool>,			// arraySetBool
		&Helper::setDataAs<CPPTYPE, char>,			// arraySetChar
		&Helper::setDataAs<CPPTYPE, int8_t>,		// arraySetInt8
		&Helper::setDataAs<CPPTYPE, uint8_t>,		// arraySetUInt8
		&Helper::setDataAs<CPPTYPE, int16_t>,		// arraySetInt16
		&Helper::setDataAs<CPPTYPE, uint16_t>,		// arraySetUInt16
		&Helper::setDataAs<CPPTYPE, int32_t>,		// arraySetInt32
		&Helper::setDataAs<CPPTYPE, uint32_t>,		// arraySetUInt32
		&Helper::setDataAsRound<CPPTYPE, float>,	// arraySetFloat
		&Helper::setDataAsRound<CPPTYPE, double>,	// arraySetDouble
		&Helper::setDataAsString<CPPTYPE>,			// arraySetString

		&Helper::getPtr<CPPTYPE>,					// getPtr

		#undef CPPTYPE
	},

	// VTABLE(knUInt16) -------------------------------------------------------
	{
		#define CPPTYPE uint16_t

		// Destruct
		&Helper::doNothing,							// destruct

		// Types
		&Helper::getConstant<eType, knUInt16>,		// getType
		&Helper::setType<CPPTYPE>,					// setType

		// Comparisons
		&Helper::compareEqualTo<CPPTYPE>,			// compareEqualTo
		&Helper::compareLess<CPPTYPE>,				// compareLess

		// Copying
		&Helper::assignCopy<CPPTYPE>,				// constructCopy
		&Helper::assignCopy<CPPTYPE>,				// assignCopy

		// Properties
		&Helper::isNumeric<CPPTYPE>,				// isNumeric

		// Getters
		&Helper::getDataAs<CPPTYPE, bool>,			// getBool
		&Helper::getDataAs<CPPTYPE, char>,			// getChar
		&Helper::getDataAs<CPPTYPE, int8_t>,		// getInt8
		&Helper::getDataAs<CPPTYPE, uint8_t>,		// getUInt8
		&Helper::getDataAs<CPPTYPE, int16_t>,		// getInt16
		&Helper::getDataAs<CPPTYPE, uint16_t>,		// getUInt16
		&Helper::getDataAs<CPPTYPE, int32_t>,		// getInt32
		&Helper::getDataAs<CPPTYPE, uint32_t>,		// getUInt32
		&Helper::getDataAs<CPPTYPE, float>,			// getFloat
		&Helper::getDataAs<CPPTYPE, double>,		// getDouble
		&Helper::getDataAsString<CPPTYPE>,			// getString

		// Setters
		&Helper::setDataAs<CPPTYPE, bool>,			// setBool
		&Helper::setDataAs<CPPTYPE, char>,			// setChar
		&Helper::setDataAs<CPPTYPE, int8_t>,		// setInt8
		&Helper::setDataAs<CPPTYPE, uint8_t>,		// setUInt8
		&Helper::setDataAs<CPPTYPE, int16_t>,		// setInt16
		&Helper::setDataAs<CPPTYPE, uint16_t>,		// setUInt16
		&Helper::setDataAs<CPPTYPE, int32_t>,		// setInt32
		&Helper::setDataAs<CPPTYPE, uint32_t>,		// setUInt32
		&Helper::setDataAsRound<CPPTYPE, float>,	// setFloat
		&Helper::setDataAsRound<CPPTYPE, double>,	// setDouble
		&Helper::setDataAsString<CPPTYPE>,			// setString

		// Arrays
		&singleGetValue,				// getValue

		&Helper::getDataAs<CPPTYPE, bool>,			// arrayGetBool
		&Helper::getDataAs<CPPTYPE, char>,			// arrayGetChar
		&Helper::getDataAs<CPPTYPE, int8_t>,		// arrayGetInt8
		&Helper::getDataAs<CPPTYPE, uint8_t>,		// arrayGetUInt8
		&Helper::getDataAs<CPPTYPE, int16_t>,		// arrayGetInt16
		&Helper::getDataAs<CPPTYPE, uint16_t>,		// arrayGetUInt16
		&Helper::getDataAs<CPPTYPE, int32_t>,		// arrayGetInt32
		&Helper::getDataAs<CPPTYPE, uint32_t>,		// arrayGetUInt32
		&Helper::getDataAs<CPPTYPE, float>,			// arrayGetFloat
		&Helper::getDataAs<CPPTYPE, double>,		// arrayGetDouble
		&Helper::getDataAsString<CPPTYPE>,			// arrayGetString

		&Helper::setDataAs<CPPTYPE, bool>,			// arraySetBool
		&Helper::setDataAs<CPPTYPE, char>,			// arraySetChar
		&Helper::setDataAs<CPPTYPE, int8_t>,		// arraySetInt8
		&Helper::setDataAs<CPPTYPE, uint8_t>,		// arraySetUInt8
		&Helper::setDataAs<CPPTYPE, int16_t>,		// arraySetInt16
		&Helper::setDataAs<CPPTYPE, uint16_t>,		// arraySetUInt16
		&Helper::setDataAs<CPPTYPE, int32_t>,		// arraySetInt32
		&Helper::setDataAs<CPPTYPE, uint32_t>,		// arraySetUInt32
		&Helper::setDataAsRound<CPPTYPE, float>,	// arraySetFloat
		&Helper::setDataAsRound<CPPTYPE, double>,	// arraySetDouble
		&Helper::setDataAsString<CPPTYPE>,			// arraySetString

		&Helper::getPtr<CPPTYPE>,					// getPtr

		#undef CPPTYPE
	},

	// VTABLE(knInt32) --------------------------------------------------------
	{
		#define CPPTYPE int32_t

		// Destruct
		&Helper::doNothing,							// destruct

		// Types
		&Helper::getConstant<eType, knInt32>,		// getType
		&Helper::setType<CPPTYPE>,					// setType

		// Comparisons
		&Helper::compareEqualTo<CPPTYPE>,			// compareEqualTo
		&Helper::compareLess<CPPTYPE>,				// compareLess

		// Copying
		&Helper::assignCopy<CPPTYPE>,				// constructCopy
		&Helper::assignCopy<CPPTYPE>,				// assignCopy

		// Properties
		&Helper::isNumeric<CPPTYPE>,				// isNumeric

		// Getters
		&Helper::getDataAs<CPPTYPE, bool>,			// getBool
		&Helper::getDataAs<CPPTYPE, char>,			// getChar
		&Helper::getDataAs<CPPTYPE, int8_t>,		// getInt8
		&Helper::getDataAs<CPPTYPE, uint8_t>,		// getUInt8
		&Helper::getDataAs<CPPTYPE, int16_t>,		// getInt16
		&Helper::getDataAs<CPPTYPE, uint16_t>,		// getUInt16
		&Helper::getDataAs<CPPTYPE, int32_t>,		// getInt32
		&Helper::getDataAs<CPPTYPE, uint32_t>,		// getUInt32
		&Helper::getDataAs<CPPTYPE, float>,			// getFloat
		&Helper::getDataAs<CPPTYPE, double>,		// getDouble
		&Helper::getDataAsString<CPPTYPE>,			// getString

		// Setters
		&Helper::setDataAs<CPPTYPE, bool>,			// setBool
		&Helper::setDataAs<CPPTYPE, char>,			// setChar
		&Helper::setDataAs<CPPTYPE, int8_t>,		// setInt8
		&Helper::setDataAs<CPPTYPE, uint8_t>,		// setUInt8
		&Helper::setDataAs<CPPTYPE, int16_t>,		// setInt16
		&Helper::setDataAs<CPPTYPE, uint16_t>,		// setUInt16
		&Helper::setDataAs<CPPTYPE, int32_t>,		// setInt32
		&Helper::setDataAs<CPPTYPE, uint32_t>,		// setUInt32
		&Helper::setDataAsRound<CPPTYPE, float>,	// setFloat
		&Helper::setDataAsRound<CPPTYPE, double>,	// setDouble
		&Helper::setDataAsString<CPPTYPE>,			// setString

		// Arrays
		&singleGetValue,				// getValue

		&Helper::getDataAs<CPPTYPE, bool>,			// arrayGetBool
		&Helper::getDataAs<CPPTYPE, char>,			// arrayGetChar
		&Helper::getDataAs<CPPTYPE, int8_t>,		// arrayGetInt8
		&Helper::getDataAs<CPPTYPE, uint8_t>,		// arrayGetUInt8
		&Helper::getDataAs<CPPTYPE, int16_t>,		// arrayGetInt16
		&Helper::getDataAs<CPPTYPE, uint16_t>,		// arrayGetUInt16
		&Helper::getDataAs<CPPTYPE, int32_t>,		// arrayGetInt32
		&Helper::getDataAs<CPPTYPE, uint32_t>,		// arrayGetUInt32
		&Helper::getDataAs<CPPTYPE, float>,			// arrayGetFloat
		&Helper::getDataAs<CPPTYPE, double>,		// arrayGetDouble
		&Helper::getDataAsString<CPPTYPE>,			// arrayGetString

		&Helper::setDataAs<CPPTYPE, bool>,			// arraySetBool
		&Helper::setDataAs<CPPTYPE, char>,			// arraySetChar
		&Helper::setDataAs<CPPTYPE, int8_t>,		// arraySetInt8
		&Helper::setDataAs<CPPTYPE, uint8_t>,		// arraySetUInt8
		&Helper::setDataAs<CPPTYPE, int16_t>,		// arraySetInt16
		&Helper::setDataAs<CPPTYPE, uint16_t>,		// arraySetUInt16
		&Helper::setDataAs<CPPTYPE, int32_t>,		// arraySetInt32
		&Helper::setDataAs<CPPTYPE, uint32_t>,		// arraySetUInt32
		&Helper::setDataAsRound<CPPTYPE, float>,	// arraySetFloat
		&Helper::setDataAsRound<CPPTYPE, double>,	// arraySetDouble
		&Helper::setDataAsString<CPPTYPE>,			// arraySetString

		&Helper::getPtr<CPPTYPE>,					// getPtr

		#undef CPPTYPE
	},

	// VTABLE(knUInt32) -------------------------------------------------------
	{
		#define CPPTYPE uint32_t

		// Destruct
		&Helper::doNothing,							// destruct

		// Types
		&Helper::getConstant<eType, knUInt32>,		// getType
		&Helper::setType<CPPTYPE>,					// setType

		// Comparisons
		&Helper::compareEqualTo<CPPTYPE>,			// compareEqualTo
		&Helper::compareLess<CPPTYPE>,				// compareLess

		// Copying
		&Helper::assignCopy<CPPTYPE>,				// constructCopy
		&Helper::assignCopy<CPPTYPE>,				// assignCopy

		// Properties
		&Helper::isNumeric<CPPTYPE>,				// isNumeric

		// Getters
		&Helper::getDataAs<CPPTYPE, bool>,			// getBool
		&Helper::getDataAs<CPPTYPE, char>,			// getChar
		&Helper::getDataAs<CPPTYPE, int8_t>,		// getInt8
		&Helper::getDataAs<CPPTYPE, uint8_t>,		// getUInt8
		&Helper::getDataAs<CPPTYPE, int16_t>,		// getInt16
		&Helper::getDataAs<CPPTYPE, uint16_t>,		// getUInt16
		&Helper::getDataAs<CPPTYPE, int32_t>,		// getInt32
		&Helper::getDataAs<CPPTYPE, uint32_t>,		// getUInt32
		&Helper::getDataAs<CPPTYPE, float>,			// getFloat
		&Helper::getDataAs<CPPTYPE, double>,		// getDouble
		&Helper::getDataAsString<CPPTYPE>,			// getString

		// Setters
		&Helper::setDataAs<CPPTYPE, bool>,			// setBool
		&Helper::setDataAs<CPPTYPE, char>,			// setChar
		&Helper::setDataAs<CPPTYPE, int8_t>,		// setInt8
		&Helper::setDataAs<CPPTYPE, uint8_t>,		// setUInt8
		&Helper::setDataAs<CPPTYPE, int16_t>,		// setInt16
		&Helper::setDataAs<CPPTYPE, uint16_t>,		// setUInt16
		&Helper::setDataAs<CPPTYPE, int32_t>,		// setInt32
		&Helper::setDataAs<CPPTYPE, uint32_t>,		// setUInt32
		&Helper::setDataAsRound<CPPTYPE, float>,	// setFloat
		&Helper::setDataAsRound<CPPTYPE, double>,	// setDouble
		&Helper::setDataAsString<CPPTYPE>,			// setString

		// Arrays
		&singleGetValue,				// getValue

		&Helper::getDataAs<CPPTYPE, bool>,			// arrayGetBool
		&Helper::getDataAs<CPPTYPE, char>,			// arrayGetChar
		&Helper::getDataAs<CPPTYPE, int8_t>,		// arrayGetInt8
		&Helper::getDataAs<CPPTYPE, uint8_t>,		// arrayGetUInt8
		&Helper::getDataAs<CPPTYPE, int16_t>,		// arrayGetInt16
		&Helper::getDataAs<CPPTYPE, uint16_t>,		// arrayGetUInt16
		&Helper::getDataAs<CPPTYPE, int32_t>,		// arrayGetInt32
		&Helper::getDataAs<CPPTYPE, uint32_t>,		// arrayGetUInt32
		&Helper::getDataAs<CPPTYPE, float>,			// arrayGetFloat
		&Helper::getDataAs<CPPTYPE, double>,		// arrayGetDouble
		&Helper::getDataAsString<CPPTYPE>,			// arrayGetString

		&Helper::setDataAs<CPPTYPE, bool>,			// arraySetBool
		&Helper::setDataAs<CPPTYPE, char>,			// arraySetChar
		&Helper::setDataAs<CPPTYPE, int8_t>,		// arraySetInt8
		&Helper::setDataAs<CPPTYPE, uint8_t>,		// arraySetUInt8
		&Helper::setDataAs<CPPTYPE, int16_t>,		// arraySetInt16
		&Helper::setDataAs<CPPTYPE, uint16_t>,		// arraySetUInt16
		&Helper::setDataAs<CPPTYPE, int32_t>,		// arraySetInt32
		&Helper::setDataAs<CPPTYPE, uint32_t>,		// arraySetUInt32
		&Helper::setDataAsRound<CPPTYPE, float>,	// arraySetFloat
		&Helper::setDataAsRound<CPPTYPE, double>,	// arraySetDouble
		&Helper::setDataAsString<CPPTYPE>,			// arraySetString

		&Helper::getPtr<CPPTYPE>,					// getPtr

		#undef CPPTYPE
	},

	// VTABLE(knInt64) --------------------------------------------------------
	{ 0 },

	// VTABLE(knUInt64) -------------------------------------------------------
	{ 0 },

	// VTABLE(knFloat) --------------------------------------------------------
	{
		#define CPPTYPE float

		// Destruct
		&Helper::doNothing,							// destruct

		// Types
		&Helper::getConstant<eType, knFloat>,		// getType
		&Helper::setType<CPPTYPE>,					// setType

		// Comparisons
		&Helper::compareEqualTo<CPPTYPE>,			// compareEqualTo
		&Helper::compareLess<CPPTYPE>,				// compareLess

		// Copying
		&Helper::assignCopy<CPPTYPE>,				// constructCopy
		&Helper::assignCopy<CPPTYPE>,				// assignCopy

		// Properties
		&Helper::isNumeric<CPPTYPE>,				// isNumeric

		// Getters
		&Helper::getDataAs<CPPTYPE, bool>,			// getBool
		&Helper::getDataAs<CPPTYPE, char>,			// getChar
		&Helper::getDataAs<CPPTYPE, int8_t>,		// getInt8
		&Helper::getDataAs<CPPTYPE, uint8_t>,		// getUInt8
		&Helper::getDataAs<CPPTYPE, int16_t>,		// getInt16
		&Helper::getDataAs<CPPTYPE, uint16_t>,		// getUInt16
		&Helper::getDataAs<CPPTYPE, int32_t>,		// getInt32
		&Helper::getDataAs<CPPTYPE, uint32_t>,		// getUInt32
		&Helper::getDataAs<CPPTYPE, float>,			// getFloat
		&Helper::getDataAs<CPPTYPE, double>,		// getDouble
		&Helper::getDataAsString<CPPTYPE>,			// getString

		// Setters
		&Helper::setDataAs<CPPTYPE, bool>,			// setBool
		&Helper::setDataAs<CPPTYPE, char>,			// setChar
		&Helper::setDataAs<CPPTYPE, int8_t>,		// setInt8
		&Helper::setDataAs<CPPTYPE, uint8_t>,		// setUInt8
		&Helper::setDataAs<CPPTYPE, int16_t>,		// setInt16
		&Helper::setDataAs<CPPTYPE, uint16_t>,		// setUInt16
		&Helper::setDataAs<CPPTYPE, int32_t>,		// setInt32
		&Helper::setDataAs<CPPTYPE, uint32_t>,		// setUInt32
		&Helper::setDataAs<CPPTYPE, float>,			// setFloat
		&Helper::setDataAs<CPPTYPE, double>,		// setDouble
		&Helper::setDataAsString<CPPTYPE>,			// setString

		// Arrays
		&singleGetValue,				// getValue

		&Helper::getDataAs<CPPTYPE, bool>,			// arrayGetBool
		&Helper::getDataAs<CPPTYPE, char>,			// arrayGetChar
		&Helper::getDataAs<CPPTYPE, int8_t>,		// arrayGetInt8
		&Helper::getDataAs<CPPTYPE, uint8_t>,		// arrayGetUInt8
		&Helper::getDataAs<CPPTYPE, int16_t>,		// arrayGetInt16
		&Helper::getDataAs<CPPTYPE, uint16_t>,		// arrayGetUInt16
		&Helper::getDataAs<CPPTYPE, int32_t>,		// arrayGetInt32
		&Helper::getDataAs<CPPTYPE, uint32_t>,		// arrayGetUInt32
		&Helper::getDataAs<CPPTYPE, float>,			// arrayGetFloat
		&Helper::getDataAs<CPPTYPE, double>,		// arrayGetDouble
		&Helper::getDataAsString<CPPTYPE>,			// arrayGetString

		&Helper::setDataAs<CPPTYPE, bool>,			// arraySetBool
		&Helper::setDataAs<CPPTYPE, char>,			// arraySetChar
		&Helper::setDataAs<CPPTYPE, int8_t>,		// arraySetInt8
		&Helper::setDataAs<CPPTYPE, uint8_t>,		// arraySetUInt8
		&Helper::setDataAs<CPPTYPE, int16_t>,		// arraySetInt16
		&Helper::setDataAs<CPPTYPE, uint16_t>,		// arraySetUInt16
		&Helper::setDataAs<CPPTYPE, int32_t>,		// arraySetInt32
		&Helper::setDataAs<CPPTYPE, uint32_t>,		// arraySetUInt32
		&Helper::setDataAs<CPPTYPE, float>,			// arraySetFloat
		&Helper::setDataAs<CPPTYPE, double>,		// arraySetDouble
		&Helper::setDataAsString<CPPTYPE>,			// arraySetString

		&Helper::getPtr<CPPTYPE>,					// getPtr

		#undef CPPTYPE
	},

	// VTABLE(knDouble) -------------------------------------------------------
	{
		#define CPPTYPE double

		// Destruct
		&Helper::doNothing,							// destruct

		// Types
		&Helper::getConstant<eType, knDouble>,		// getType
		&Helper::setType<CPPTYPE>,					// setType

		// Comparisons
		&Helper::compareEqualTo<CPPTYPE>,			// compareEqualTo
		&Helper::compareLess<CPPTYPE>,				// compareLess

		// Copying
		&Helper::assignCopy<CPPTYPE>,				// constrcutCopy
		&Helper::assignCopy<CPPTYPE>,				// assignCopy

		// Properties
		&Helper::isNumeric<CPPTYPE>,				// isNumeric

		// Getters
		&Helper::getDataAs<CPPTYPE, bool>,			// getBool
		&Helper::getDataAs<CPPTYPE, char>,			// getChar
		&Helper::getDataAs<CPPTYPE, int8_t>,		// getInt8
		&Helper::getDataAs<CPPTYPE, uint8_t>,		// getUInt8
		&Helper::getDataAs<CPPTYPE, int16_t>,		// getInt16
		&Helper::getDataAs<CPPTYPE, uint16_t>,		// getUInt16
		&Helper::getDataAs<CPPTYPE, int32_t>,		// getInt32
		&Helper::getDataAs<CPPTYPE, uint32_t>,		// getUInt32
		&Helper::getDataAs<CPPTYPE, float>,			// getFloat
		&Helper::getDataAs<CPPTYPE, double>,		// getDouble
		&Helper::getDataAsString<CPPTYPE>,			// getString

		// Setters
		&Helper::setDataAs<CPPTYPE, bool>,			// setBool
		&Helper::setDataAs<CPPTYPE, char>,			// setChar
		&Helper::setDataAs<CPPTYPE, int8_t>,		// setInt8
		&Helper::setDataAs<CPPTYPE, uint8_t>,		// setUInt8
		&Helper::setDataAs<CPPTYPE, int16_t>,		// setInt16
		&Helper::setDataAs<CPPTYPE, uint16_t>,		// setUInt16
		&Helper::setDataAs<CPPTYPE, int32_t>,		// setInt32
		&Helper::setDataAs<CPPTYPE, uint32_t>,		// setUInt32
		&Helper::setDataAs<CPPTYPE, float>,			// setFloat
		&Helper::setDataAs<CPPTYPE, double>,		// setDouble
		&Helper::setDataAsString<CPPTYPE>,			// setString

		// Arrays
		&singleGetValue,				// getValue

		&Helper::getDataAs<CPPTYPE, bool>,			// arrayGetBool
		&Helper::getDataAs<CPPTYPE, char>,			// arrayGetChar
		&Helper::getDataAs<CPPTYPE, int8_t>,		// arrayGetInt8
		&Helper::getDataAs<CPPTYPE, uint8_t>,		// arrayGetUInt8
		&Helper::getDataAs<CPPTYPE, int16_t>,		// arrayGetInt16
		&Helper::getDataAs<CPPTYPE, uint16_t>,		// arrayGetUInt16
		&Helper::getDataAs<CPPTYPE, int32_t>,		// arrayGetInt32
		&Helper::getDataAs<CPPTYPE, uint32_t>,		// arrayGetUInt32
		&Helper::getDataAs<CPPTYPE, float>,			// arrayGetFloat
		&Helper::getDataAs<CPPTYPE, double>,		// arrayGetDouble
		&Helper::getDataAsString<CPPTYPE>,			// arrayGetString

		&Helper::setDataAs<CPPTYPE, bool>,			// arraySetBool
		&Helper::setDataAs<CPPTYPE, char>,			// arraySetChar
		&Helper::setDataAs<CPPTYPE, int8_t>,		// arraySetInt8
		&Helper::setDataAs<CPPTYPE, uint8_t>,		// arraySetUInt8
		&Helper::setDataAs<CPPTYPE, int16_t>,		// arraySetInt16
		&Helper::setDataAs<CPPTYPE, uint16_t>,		// arraySetUInt16
		&Helper::setDataAs<CPPTYPE, int32_t>,		// arraySetInt32
		&Helper::setDataAs<CPPTYPE, uint32_t>,		// arraySetUInt32
		&Helper::setDataAs<CPPTYPE, float>,			// arraySetFloat
		&Helper::setDataAs<CPPTYPE, double>,		// arraySetDouble
		&Helper::setDataAsString<CPPTYPE>,			// arraySetString

		&Helper::getPtr<CPPTYPE>,					// getPtr

		#undef CPPTYPE
	},

	// VTABLE(knString) -------------------------------------------------------
	{
		#define CPPTYPE std::string

		// Destruct
		&Helper::destructString,					// destruct

		// Types
		&Helper::getConstant<eType, knString>,		// getType
		&Helper::setType<CPPTYPE>,					// setType

		// Comparisons
		&Helper::stringCompareEqualTo,				// compareEqualTo
		&Helper::stringCompareLess,					// compareLess

		// Copying
		&Helper::constructString,					// constrcutCopy
		&Helper::assignString,						// assignCopy

		// Properties
		&Helper::isNumeric<CPPTYPE>,				// isNumeric

		// Getters
		&Helper::stringGetDataAs<bool>,				// getBool
		&Helper::stringGetDataAs<char>,				// getChar
		&Helper::stringGetDataAs<int8_t>,			// getInt8
		&Helper::stringGetDataAs<uint8_t>,			// getUInt8
		&Helper::stringGetDataAs<int16_t>,			// getInt16
		&Helper::stringGetDataAs<uint16_t>,			// getUInt16
		&Helper::stringGetDataAs<int32_t>,			// getInt32
		&Helper::stringGetDataAs<uint32_t>,			// getUInt32
		&Helper::stringGetDataAs<float>,			// getFloat
		&Helper::stringGetDataAs<double>,			// getDouble
		&Helper::stringGetString,					// getString

		// Setters
		&Helper::stringSetDataAs<bool>,				// setBool
		&Helper::stringSetDataAs<char>,				// setChar
		&Helper::stringSetDataAs<int8_t>,			// setInt8
		&Helper::stringSetDataAs<uint8_t>,			// setUInt8
		&Helper::stringSetDataAs<int16_t>,			// setInt16
		&Helper::stringSetDataAs<uint16_t>,			// setUInt16
		&Helper::stringSetDataAs<int32_t>,			// setInt32
		&Helper::stringSetDataAs<uint32_t>,			// setUInt32
		&Helper::stringSetDataAs<float>,			// setFloat
		&Helper::stringSetDataAs<double>,			// setDouble
		&Helper::stringSetString,					// setString

		// Arrays
		&singleGetValue,				// getValue

		&Helper::stringGetDataAs<bool>,				// arrayGetBool
		&Helper::stringGetDataAs<char>,				// arrayGetChar
		&Helper::stringGetDataAs<int8_t>,			// arrayGetInt8
		&Helper::stringGetDataAs<uint8_t>,			// arrayGetUInt8
		&Helper::stringGetDataAs<int16_t>,			// arrayGetInt16
		&Helper::stringGetDataAs<uint16_t>,			// arrayGetUInt16
		&Helper::stringGetDataAs<int32_t>,			// arrayGetInt32
		&Helper::stringGetDataAs<uint32_t>,			// arrayGetUInt32
		&Helper::stringGetDataAs<float>,			// arrayGetFloat
		&Helper::stringGetDataAs<double>,			// arrayGetDouble
		&Helper::stringGetString,					// arrayGetString

		&Helper::stringSetDataAs<bool>,				// arraySetBool
		&Helper::stringSetDataAs<char>,				// arraySetChar
		&Helper::stringSetDataAs<int8_t>,			// arraySetInt8
		&Helper::stringSetDataAs<uint8_t>,			// arraySetUInt8
		&Helper::stringSetDataAs<int16_t>,			// arraySetInt16
		&Helper::stringSetDataAs<uint16_t>,			// arraySetUInt16
		&Helper::stringSetDataAs<int32_t>,			// arraySetInt32
		&Helper::stringSetDataAs<uint32_t>,			// arraySetUInt32
		&Helper::stringSetDataAs<float>,			// arraySetFloat
		&Helper::stringSetDataAs<double>,			// arraySetDouble
		&Helper::stringSetString,					// arraySetString

		&Helper::stringGetPtr,						// getPtr

		#undef CPPTYPE
	},

	// VTABLE(knArray) --------------------------------------------------------
	{
		// Destruct
		&Helper::destructArray,						// destruct

		// Types
		&Helper::getConstant<eType, knArray>,		// getType
		&Helper::arraySetType,						// setType

		// Comparisons
		&compareArrayEqualTo,						// compareEqualTo
		&compareArrayLess,							// compareLess

		// Copying
		&Helper::constructArray,					// constrcutCopy
		&Helper::assignArray,						// assignCopy

		// Properties
		&Helper::arrayIsNumeric,					// isNumeric

		// Getters
		&Helper::arrayGetDataAs<bool>,				// getBool
		&Helper::arrayGetDataAs<char>,				// getChar
		&Helper::arrayGetDataAs<int8_t>,			// getInt8
		&Helper::arrayGetDataAs<uint8_t>,			// getUInt8
		&Helper::arrayGetDataAs<int16_t>,			// getInt16
		&Helper::arrayGetDataAs<uint16_t>,			// getUInt16
		&Helper::arrayGetDataAs<int32_t>,			// getInt32
		&Helper::arrayGetDataAs<uint32_t>,			// getUInt32
		&Helper::arrayGetDataAs<float>,				// getFloat
		&Helper::arrayGetDataAs<double>,			// getDouble
		&Helper::arrayGetDataAs<std::string>,		// getString

		// Setters
		&Helper::arraySetDataAs<bool>,				// setBool
		&Helper::arraySetDataAs<char>,				// setChar
		&Helper::arraySetDataAs<int8_t>,			// setInt8
		&Helper::arraySetDataAs<uint8_t>,			// setUInt8
		&Helper::arraySetDataAs<int16_t>,			// setInt16
		&Helper::arraySetDataAs<uint16_t>,			// setUInt16
		&Helper::arraySetDataAs<int32_t>,			// setInt32
		&Helper::arraySetDataAs<uint32_t>,			// setUInt32
		&Helper::arraySetDataAs<float>,				// setFloat
		&Helper::arraySetDataAs<double>,			// setDouble
		&Helper::arraySetDataAsString,				// setString

		// Arrays
		&Helper::arrayGetValue,						// getValue

		&Helper::arrayGetDataAs<bool>,				// arrayGetBool
		&Helper::arrayGetDataAs<char>,				// arrayGetChar
		&Helper::arrayGetDataAs<int8_t>,			// arrayGetInt8
		&Helper::arrayGetDataAs<uint8_t>,			// arrayGetUInt8
		&Helper::arrayGetDataAs<int16_t>,			// arrayGetInt16
		&Helper::arrayGetDataAs<uint16_t>,			// arrayGetUInt16
		&Helper::arrayGetDataAs<int32_t>,			// arrayGetInt32
		&Helper::arrayGetDataAs<uint32_t>,			// arrayGetUInt32
		&Helper::arrayGetDataAs<float>,				// arrayGetFloat
		&Helper::arrayGetDataAs<double>,			// arrayGetDouble
		&Helper::arrayGetDataAs<std::string>,		// arrayGetString

		&Helper::arraySetDataAs<bool>,				// arraySetBool
		&Helper::arraySetDataAs<char>,				// arraySetChar
		&Helper::arraySetDataAs<int8_t>,			// arraySetInt8
		&Helper::arraySetDataAs<uint8_t>,			// arraySetUInt8
		&Helper::arraySetDataAs<int16_t>,			// arraySetInt16
		&Helper::arraySetDataAs<uint16_t>,			// arraySetUInt16
		&Helper::arraySetDataAs<int32_t>,			// arraySetInt32
		&Helper::arraySetDataAs<uint32_t>,			// arraySetUInt32
		&Helper::arraySetDataAs<float>,				// arraySetFloat
		&Helper::arraySetDataAs<double>,			// arraySetDouble
		&Helper::arraySetDataAsString,				// arraySetString

		&Helper::arrayGetPtr,						// getPtr
	}
};

/*!
Helper function for getValue.
\param value	The value (must be array type).
\param n		The index of the element to get.
\return The element as a value.
*/
template <typename TD>
static PYXValue PYXValue::Helper::getValue(const PYXValue& value, int n)
{
	const TD* const& pAData = *reinterpret_cast<const TD* const *>(&value.m_buffer[0]);
	const int8_t& nANullable = *reinterpret_cast<const int8_t*>(&value.m_buffer[4]);
	const int8_t& nAType = *reinterpret_cast<const int8_t*>(&value.m_buffer[5]);
	const int16_t& nASize = *reinterpret_cast<const int16_t*>(&value.m_buffer[6]);

	// Null value handling. We do return the actual null value, however.
	if (nANullable != 0 && n != nASize && pAData[n] == pAData[nASize])
	{
		return PYXValue();
	}

	return PYXValue(pAData[n]);
}

/*!
Function table for getValue.
Index by type. Entries for the null type are undefined.
*/
PYXValue (*PYXValue::Helper::m_funcTableGetValue[knTypeCount])
	(const PYXValue& value, int n) =
{
	// DO NOT REORDER!

	0, // knNull
	&Helper::getValue<bool>,
	&Helper::getValue<char>,
	&Helper::getValue<int8_t>,
	&Helper::getValue<uint8_t>,
	&Helper::getValue<int16_t>,
	&Helper::getValue<uint16_t>,
	&Helper::getValue<int32_t>,
	&Helper::getValue<uint32_t>,
	0, // knInt64
	0, // knUInt64
	&Helper::getValue<float>,
	&Helper::getValue<double>,
	&Helper::getValue<std::string>,
	0 // knArray
};

/*!
Function table for arrayCopy.
Index by destination type, then source type. Entries for the null type are undefined.
*/
void (*PYXValue::Helper::m_funcTableArrayCopy[knTypeCount][knTypeCount])
	(char* pDest, const char* pSrc, int nSize) =
{
	// DO NOT REORDER!
	#define LINE(TDEST) \
		{ \
			0, /* null */ \
			&Helper::arrayCopy<TDEST, bool>, \
			&Helper::arrayCopy<TDEST, char>, \
			&Helper::arrayCopy<TDEST, int8_t>, \
			&Helper::arrayCopy<TDEST, uint8_t>, \
			&Helper::arrayCopy<TDEST, int16_t>, \
			&Helper::arrayCopy<TDEST, uint16_t>, \
			&Helper::arrayCopy<TDEST, int32_t>, \
			&Helper::arrayCopy<TDEST, uint32_t>, \
			0, /* int64_t */ \
			0, /* uint64_t */ \
			&Helper::arrayCopy<TDEST, float>, \
			&Helper::arrayCopy<TDEST, double>, \
			&Helper::arrayCopyFromString<TDEST>, \
		},
	{ 0 }, // null line
	LINE(bool)
	LINE(char)
	LINE(int8_t)
	LINE(uint8_t)
	LINE(int16_t)
	LINE(uint16_t)
	LINE(int32_t)
	LINE(uint32_t)
	{ 0 }, // int64_t line
	{ 0 }, // uint64_t line
	LINE(float)
	LINE(double)
	// string line
	{
		0, /* null */ \
		&Helper::arrayCopyToString<bool>,
		&Helper::arrayCopyToString<char>,
		&Helper::arrayCopyToString<int8_t>,
		&Helper::arrayCopyToString<uint8_t>,
		&Helper::arrayCopyToString<int16_t>,
		&Helper::arrayCopyToString<uint16_t>,
		&Helper::arrayCopyToString<int32_t>,
		&Helper::arrayCopyToString<uint32_t>,
		0, /* int64_t */
		0, /* uint64_t */
		&Helper::arrayCopyToString<float>,
		&Helper::arrayCopyToString<double>,
		&Helper::arrayCopy<std::string, std::string>,
	}
#undef LINE
};

/*!
Size table. Index by value type. Entries for the null type are undefined.
*/
int PYXValue::Helper::m_typeSize[] =
{
	// DO NOT REORDER!

	0, // null
	sizeof(bool),
	sizeof(char),
	sizeof(int8_t),
	sizeof(uint8_t),
	sizeof(int16_t),
	sizeof(uint16_t),
	sizeof(int32_t),
	sizeof(uint32_t),
	0, // int64_t
	0, // uint64_t
	sizeof(float),
	sizeof(double),
	sizeof(std::string),
	0 // array
};

float PYXValue::Helper::round(float value)
{
	return boost::math::round(value);
}


double PYXValue::Helper::round(double value)
{
	return boost::math::round(value);
}

#endif // DOXYGEN_IGNORE

namespace
{

/*!
Create a value from general arguments.

The type of values stored in pVal and pNullVal must match the specified
type parameter.

If a null pointer is passed for pVal then the newly created PYXValue will
be initialized to zero.

If array size is one, a single value (possibly null) is created; otherwise
an array value is created (using the null value if specified).

\param pVal			The value(s) to use (can be null except for string types).
\param nSize		The array size to use. (1 or greater)
\param pNullVal		The null value to use. (optional)
\param bForceArray	Whether to force an array in the case of size 1.
\return The created value.
*/
template <typename TD>
PYXValue create(const void* pVal, int nSize, const void* pNullVal, bool bForceArray)
{
	assert(1 <= nSize);

	const TD* pValCast = static_cast<const TD*>(pVal);
	const TD* pNullValCast = static_cast<const TD*>(pNullVal);

	if (nSize == 1 && !bForceArray)
	{
		if (pVal != 0)
		{
			if (pNullVal != 0 && *pValCast == *pNullValCast)
			{
				// Null value.
				return PYXValue();
			}
			else
			{
				// Single value.
				return PYXValue(*pValCast);
			}
		}
		else
		{
			TD defaultValue = 0;
			return PYXValue(defaultValue);
		}
	}
	else
	{
		if (pNullVal != 0)
		{
			// Nullable array value.
			return PYXValue(pValCast, nSize, *pNullValCast);
		}
		else
		{
			// Non-nullable array value.
			return PYXValue(pValCast, nSize);
		}
	}
}

}

/*!
Create a value from general arguments.

The type of values stored in pVal and pNullVal must match the specified
type parameter if they are not null pointers.

If nType is knNull a null PYXValue is created.

If pVal is a null pointer, then the data will be set to zero.  When
a knString type is requested and nSize is 1, pVal must be a valid pointer.

If array size is one, a single value is created; otherwise an array value is
created (using the null value if specified).

\param nType		The type of value to create.
\param pVal			The value(s) to use. (optional)
\param nSize		The array size to use. (1 or greater)
\param pNullVal		The array null value to use. (optional)
\param bForceArray	Whether to force an array in the case of size 1.
\return The created value.
*/
PYXValue PYXValue::create(eType nType, const void* pVal, int nSize, const void* pNullVal, bool bForceArray)
{
	assert(((nType != knString) || (nSize > 1) || (pVal != 0)) && "can't have type of knString with size of 1 and pVal null.");
	switch (nType)
	{
	case knBool:
		return ::create<bool>(pVal, nSize, pNullVal, bForceArray);
	case knChar:
		return ::create<char>(pVal, nSize, pNullVal, bForceArray);
	case knInt8:
		return ::create<int8_t>(pVal, nSize, pNullVal, bForceArray);
	case knUInt8:
		return ::create<uint8_t>(pVal, nSize, pNullVal, bForceArray);
	case knInt16:
		return ::create<int16_t>(pVal, nSize, pNullVal, bForceArray);
	case knUInt16:
		return ::create<uint16_t>(pVal, nSize, pNullVal, bForceArray);
	case knInt32:
		return ::create<int32_t>(pVal, nSize, pNullVal, bForceArray);
	case knUInt32:
		return ::create<uint32_t>(pVal, nSize, pNullVal, bForceArray);
	case knFloat:
		return ::create<float>(pVal, nSize, pNullVal, bForceArray);
	case knDouble:
		return ::create<double>(pVal, nSize, pNullVal, bForceArray);
	case knString:
		return ::create<std::string>(pVal, nSize, pNullVal, bForceArray);
	default:
		return PYXValue();
	}
}

/*!
\param	nType	The type.
*/
const std::string& PYXValue::getTypeAsString(PYXValue::eType nType)
{
	static std::string table[] =
	{
		"null",
		"bool",
		"char",
		"int8_t",
		"uint8_t",
		"int16_t",
		"uint16_t",
		"int32_t",
		"uint32_t",
		"int64_t",
		"uint64_t",
		"float",
		"double",
		"string",
		"array"
	};

	assert(knNull <= nType && nType < knTypeCount);
	return table[nType];
}

//! Stream operator.
std::ostream& operator <<(std::ostream& out, const PYXValue& value)
{
	if (value.isArray())
	{
		int nSize = value.getArraySize();
		int nCount = nSize + (value.isArrayNullable() ? 1 : 0);
		out << PYXValue::getTypeAsString(value.getArrayType())
			<< '[' << (value.isArrayNullable() ? -nSize : nSize) << ']';
		if (value.getArrayType() == PYXValue::knString)
		{
			for (int n = 0; n != nCount; ++n)
			{
				// String needs size prefix.
				std::string s = value.getString(n);
				out << ' ' << static_cast<int>(s.size()) << ' ' << s;
			}
		}
		else
		{
			for (int n = 0; n != nCount; ++n)
			{
				out << ' ' << value.getString(n);
			}
		}
	}
	else
	{
		out << PYXValue::getTypeAsString(value.getType());
		if (!value.isNull())
		{
			out << ' ';
			if (value.isString())
			{
				// String needs size prefix.
				std::string s = value.getString();
				out << static_cast<int>(s.size()) << ' ' << s;
			}
			else
			{
				out << value.getString();
			}
		}
	}

	return out;
}

//! Stream operator.
std::istream& operator >>(std::istream& in, PYXValue& value)
{
	std::string strType;
	in >> strType;

	PYXValue::eType nType;
	int nSize = 1;
	bool bNullable = false;
	bool bArray = false;

	std::string::size_type nBracketPos = strType.find('[');
	if (nBracketPos != std::string::npos)
	{
		// Array.
		bArray = true;
		nType = PYXValue::getType(strType.substr(0, nBracketPos));
		nSize = PYXValue::Helper::fromString<int>(strType.substr(nBracketPos + 1));
		if (nSize < 0)
		{
			nSize = -nSize;
			bNullable = true;
		}
	}
	else
	{
		// Single value.
		nType = PYXValue::getType(strType);

		if (nType == PYXValue::knNull)
		{
			value = PYXValue();
			return in;
		}
	}

	readValues(in, value, nType, nSize, bNullable, bArray);

	return in;
}

//! vector Stream operator.
std::ostream& operator <<(std::ostream& out, const std::vector<PYXValue> & value)
{
	out << value.size();
	for(unsigned int i=0;i<value.size();i++)
	{
		out << value[i];
	}
	return out;
}

//! vector Stream operator.
std::istream& operator >>(std::istream& in, std::vector<PYXValue> & value)
{
	unsigned int size;
	in >> size;

	value.resize(size);
	for(unsigned int i=0;i<value.size();i++)
	{
		in >> value[i];
	}
	return in;
}

double PYXValue::getMax(eType nType)
{
	switch (nType) 
	{ 
	case PYXValue::knInt8:
		return std::numeric_limits<char>::max();
	case PYXValue::knChar:
		return std::numeric_limits<char>::max(); 
	case PYXValue::knInt16:
		return std::numeric_limits<short>::max(); 
	case PYXValue::knInt32:
		return std::numeric_limits<int>::max(); 
	case PYXValue::knDouble:
		return std::numeric_limits<double>::max(); 
	case PYXValue::knFloat:
		return std::numeric_limits<float>::max(); 
	case PYXValue::knUInt8:
		return std::numeric_limits<unsigned char>::max();
	case PYXValue::knUInt16:
		return std::numeric_limits<unsigned short>::max();  
	case PYXValue::knUInt32:
		return std::numeric_limits<unsigned int>::max();
	default:
		assert(false && "Invalid type."); 
		return -1;
	}
}

double PYXValue::getMin(eType nType)
{
	switch (nType) 
	{
	case PYXValue::knInt8:
		return std::numeric_limits<char>::min();
	case PYXValue::knChar:
		return std::numeric_limits<char>::min(); 
	case PYXValue::knInt16:
		return std::numeric_limits<short>::min(); 
	case PYXValue::knInt32:
		return std::numeric_limits<int>::min(); 
	case PYXValue::knDouble:
		return - std::numeric_limits<double>::max();  // min return 2.22507e-308 ~= 0.
	case PYXValue::knFloat:
		return - std::numeric_limits<float>::max(); //min return 1.17549e-038 ~= 0.
	case PYXValue::knUInt8:
		return std::numeric_limits<unsigned char>::min();
	case PYXValue::knUInt16:
		return std::numeric_limits<unsigned short>::min();  
	case PYXValue::knUInt32:
		return std::numeric_limits<unsigned int>::min();
	default:
		assert(false && "Invalid type.");
		return -1;
	}
}
