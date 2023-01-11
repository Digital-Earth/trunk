#ifndef PYXIS__UTILITY__VALUE_H
#define PYXIS__UTILITY__VALUE_H
/******************************************************************************
value.h

begin		: 2006-02-15
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/stdint.h"
#include "pyxis/utility/string_utils.h"

// system includes
#include <algorithm>
#include <cassert>
#include <iosfwd>
#include <limits>
#include <sstream>
#include <string>

// boost includes
#include <boost/array.hpp>
#include <boost/scoped_array.hpp>

/*!
This class implements a bounded discriminated union. 

This concrete implementation contains only a pointer to a fake vtable and a
buffer for 64 bits of data. It may use the free store.

TODO write more docs (or reference docs on wiki?)

Some naming conventions:
- PYXValue objects are named "value" while raw values are named "val".
- Template parameter TD represents the underlying data type.
- Template parameter T represents the desired data type.
- Functions named like nullXXX are used by the null vtable.
- Functions named like stringXXX are used by the string vtable.
- Functions named like arrayXXX are used by the array vtable.
*/
//! Represents a value of a type.
class PYXLIB_DECL PYXValue
{
public:

	//! Value type.
	enum eType
	{
		// DO NOT REORDER!

		knNull = 0,				//!< Type represents null.
		knBool,					//!< Type represents bool.
		knChar,					//!< Type represents char.
		knInt8,					//!< Type represents int8_t.
		knUInt8,				//!< Type represents uint8_t.
		knInt16,				//!< Type represents int16_t.
		knUInt16,				//!< Type represents uint16_t.
		knInt32,				//!< Type represents int32_t.
		knUInt32,				//!< Type represents uint32_t.
		knReservedForInt64,		//!< Type represents int64_t.
		knReservedForUInt64,	//!< Type represents uint64_t.
		knFloat,				//!< Type represents float.
		knDouble,				//!< Type represents double.
		knString,				//!< Type represents string.
		knArray,				//!< Type represents an array of elements.

		knTypeCount				//!< For internal use only.

	};

	//! For integer types, we mimic the stl::max function.
	static double getMax(eType type);

	//! For integer types, we mimic the stl::min function.
	static double getMin(eType type);

	//! return true for numeric types that are signed
	static bool isSigned(eType type);

	//! return true for numeric types that are unsigned
	static bool isUnsigned(eType type);

public:

	//! Test method
	static void test();

	//! Test method
	template <typename TD>
	static void testValue(PYXValue& value, const TD& val);

	//! Test method
	template <typename TD>
	static void testValueAndType(PYXValue& value, TD val);

	//! Test method
	template <typename TD>
	static void testSingleValue(TD val);

	//! Test method
	template <typename TD>
	static void testArray(PYXValue& value, TD* pVal, int nSize, const TD& nullVal, bool bNullable);

	//! Generalized factory.
	static PYXValue create(eType nType, const void* pVal, int nSize, const void* pNullVal, bool bForceArray = false);

	//! Get a string representing a type.
	static const std::string& getTypeAsString(eType nType);

public:

	//! Default constructor creates null value.
	PYXValue() : m_pVTable(&VTable::m_vtable[knNull]) {}

	//! Copy constructor copies type and value.
	PYXValue(const PYXValue& other) : m_pVTable(other.m_pVTable) { m_pVTable->constructCopy(*this, other); }

public:

	//! Value constructor determines type from value.
	explicit PYXValue(bool val) : m_pVTable(&Helper::getVTable(val)) { Helper::setData(*this, val); }

	//! Value constructor determines type from value.
	explicit PYXValue(char val) : m_pVTable(&Helper::getVTable(val)) { Helper::setData(*this, val); }

	//! Value constructor determines type from value.
	explicit PYXValue(int8_t val) : m_pVTable(&Helper::getVTable(val)) { Helper::setData(*this, val); }

	//! Value constructor determines type from value.
	explicit PYXValue(uint8_t val) : m_pVTable(&Helper::getVTable(val)) { Helper::setData(*this, val); }

	//! Value constructor determines type from value.
	explicit PYXValue(int16_t val) : m_pVTable(&Helper::getVTable(val)) { Helper::setData(*this, val); }

	//! Value constructor determines type from value.
	explicit PYXValue(uint16_t val) : m_pVTable(&Helper::getVTable(val)) { Helper::setData(*this, val); }

	//! Value constructor determines type from value.
	explicit PYXValue(int32_t val) : m_pVTable(&Helper::getVTable(val)) { Helper::setData(*this, val); }

	//! Value constructor determines type from value.
	explicit PYXValue(uint32_t val) : m_pVTable(&Helper::getVTable(val)) { Helper::setData(*this, val); }

	//! Value constructor determines type from value.
	explicit PYXValue(float val) : m_pVTable(&Helper::getVTable(val)) { Helper::setData(*this, val); }

	//! Value constructor determines type from value.
	explicit PYXValue(double val) : m_pVTable(&Helper::getVTable(val)) { Helper::setData(*this, val); }

	//! Value constructor determines type from value.
	explicit PYXValue(const std::string& val) : m_pVTable(&Helper::getVTable(val)) { Helper::constructString(*this, val); }

	//! Value constructor determines type from value.
	explicit PYXValue(const char* val) : m_pVTable(0)
	{
		std::string const valString(val);
		m_pVTable = &Helper::getVTable(valString);
		Helper::constructString(*this, valString);
	}

public:

	//! Array constructor determines type from value. C-style array pVal must contain nSize elements.
	PYXValue(const bool* pVal, int nSize) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize); }

	//! Array constructor determines type from value. C-style array pVal must contain nSize elements.
	PYXValue(const char* pVal, int nSize) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize); }

	//! Array constructor determines type from value. C-style array pVal must contain nSize elements.
	PYXValue(const int8_t* pVal, int nSize) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize); }

	//! Array constructor determines type from value. C-style array pVal must contain nSize elements.
	PYXValue(const unsigned char* pVal, int nSize) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize); }

	//! Array constructor determines type from value. C-style array pVal must contain nSize elements.
	PYXValue(const int16_t* pVal, int nSize) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize); }

	//! Array constructor determines type from value. C-style array pVal must contain nSize elements.
	PYXValue(const uint16_t* pVal, int nSize) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize); }

	//! Array constructor determines type from value. C-style array pVal must contain nSize elements.
	PYXValue(const int32_t* pVal, int nSize) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize); }

	//! Array constructor determines type from value. C-style array pVal must contain nSize elements.
	PYXValue(const uint32_t* pVal, int nSize) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize); }

	//! Array constructor determines type from value. C-style array pVal must contain nSize elements.
	PYXValue(const float* pVal, int nSize) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize); }

	//! Array constructor determines type from value. C-style array pVal must contain nSize elements.
	PYXValue(const double* pVal, int nSize) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize); }

	//! Array constructor determines type from value. C-style array pVal must contain nSize elements.
	PYXValue(const std::string* pVal, int nSize) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize); }

	//! Array constructor determines type from value. C-style array pVal must contain nSize elements.
	PYXValue(const char** pVal, int nSize) : m_pVTable(&VTable::m_vtable[knArray])
	{
		assert(0 < nSize);
		boost::scoped_array<std::string> strings(new std::string[nSize]);
		std::copy(pVal, pVal + nSize, strings.get());
		Helper::constructArray(*this, strings.get(), nSize);
	}

public:

	//! Array constructor determines type from value, uses specified null value. C-style array pVal must contain nSize elements.
	PYXValue(const bool* pVal, int nSize, bool nullVal) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize, nullVal); }

	//! Array constructor determines type from value, uses specified null value. C-style array pVal must contain nSize elements.
	PYXValue(const char* pVal, int nSize, char nullVal) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize, nullVal); }

	//! Array constructor determines type from value, uses specified null value. C-style array pVal must contain nSize elements.
	PYXValue(const int8_t* pVal, int nSize, int8_t nullVal) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize, nullVal); }

	//! Array constructor determines type from value, uses specified null value. C-style array pVal must contain nSize elements.
	PYXValue(const uint8_t* pVal, int nSize, uint8_t nullVal) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize, nullVal); }

	//! Array constructor determines type from value, uses specified null value. C-style array pVal must contain nSize elements.
	PYXValue(const int16_t* pVal, int nSize, int16_t nullVal) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize, nullVal); }

	//! Array constructor determines type from value, uses specified null value. C-style array pVal must contain nSize elements.
	PYXValue(const uint16_t* pVal, int nSize, uint16_t nullVal) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize, nullVal); }

	//! Array constructor determines type from value, uses specified null value. C-style array pVal must contain nSize elements.
	PYXValue(const int32_t* pVal, int nSize, int32_t nullVal) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize, nullVal); }

	//! Array constructor determines type from value, uses specified null value. C-style array pVal must contain nSize elements.
	PYXValue(const uint32_t* pVal, int nSize, uint32_t nullVal) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize, nullVal); }

	//! Array constructor determines type from value, uses specified null value. C-style array pVal must contain nSize elements.
	PYXValue(const float* pVal, int nSize, float nullVal) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize, nullVal); }

	//! Array constructor determines type from value, uses specified null value. C-style array pVal must contain nSize elements.
	PYXValue(const double* pVal, int nSize, double nullVal) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize, nullVal); }

	//! Array constructor determines type from value, uses specified null value. C-style array pVal must contain nSize elements.
	PYXValue(const std::string* pVal, int nSize, const std::string& nullVal) : m_pVTable(&VTable::m_vtable[knArray]) { Helper::constructArray(*this, pVal, nSize, nullVal); }

	//! Array constructor determines type from value. C-style array pVal must contain nSize elements.
	PYXValue(const char** pVal, int nSize, const std::string& nullVal) : m_pVTable(&VTable::m_vtable[knArray])
	{
		assert(0 < nSize);
		boost::scoped_array<std::string> strings(new std::string[nSize]);
		std::copy(pVal, pVal + nSize, strings.get());
		Helper::constructArray(*this, strings.get(), nSize, nullVal);
	}

public:

	//! Destructor.
	~PYXValue() { m_pVTable->destruct(*this); }

	//! Copy assignment operator copies type and value.
	PYXValue& operator =(const PYXValue& other)
	{
		if (m_pVTable != other.m_pVTable)
		{
			m_pVTable->destruct(*this);
			m_pVTable = other.m_pVTable;
			m_pVTable->constructCopy(*this, other);
		}
		else 
		{
			m_pVTable->assignCopy(*this, other);
		}
		return *this;
	}

	//! Swap.
	void swap(PYXValue& other)
	{
		// Swap vtable.
		std::swap(m_pVTable, other.m_pVTable);
		// Swap buffer.
		std::swap(
			*reinterpret_cast<int32_t*>(&m_buffer[0]),
			*reinterpret_cast<int32_t*>(&other.m_buffer[0]));
		std::swap(
			*reinterpret_cast<int32_t*>(&m_buffer[knArrayNullableIndex]),
			*reinterpret_cast<int32_t*>(&other.m_buffer[knArrayNullableIndex]));
	}

public:

	//! Gets the type.
	eType getType() const { return m_pVTable->getType(); }

	//! Sets the type.
	void setType(eType type) { m_pVTable->setType(*this, type); }

	//! Gets the type as a string
	static const char* getString(eType nType);

	//! Gets the type from a string
	static eType getType(const std::string& strType);

	//! Is the specified type a numeric type
	static bool isNumeric(eType nType);

	//! compare a the "n" element in two values (will do type cast to type "a" if needed)
	static int compare(const PYXValue & a,const PYXValue & b,int n);
	//! compare all elements in two values (will do type cast to type "a" if needed)
	static int compare(const PYXValue & a,const PYXValue & b);

public:

	//! Returns true if the value can be interpreted entirely as a number.
	bool isNumeric() const { return m_pVTable->isNumeric(*this); }

	int compare(const PYXValue & other) const;

public:

	//! Whether the type is knNull.
	bool isNull() const { return m_pVTable->getType() == knNull; }

	//! Whether the type is knBool.
	bool isBool() const { return m_pVTable->getType() == knBool; }

	//! Whether the type is knChar.
	bool isChar() const { return m_pVTable->getType() == knChar; }

	//! Whether the type is knInt8.
	bool isInt8() const { return m_pVTable->getType() == knInt8; }

	//! Whether the type is knUInt8.
	bool isUInt8() const { return m_pVTable->getType() == knUInt8; }

	//! Whether the type is knInt16.
	bool isInt16() const { return m_pVTable->getType() == knInt16; }

	//! Whether the type is knUInt8.
	bool isUInt16() const { return m_pVTable->getType() == knUInt16; }

	//! Whether the type is knInt32.
	bool isInt32() const { return m_pVTable->getType() == knInt32; }

	//! Whether the type is knUInt32.
	bool isUInt32() const { return m_pVTable->getType() == knUInt32; }

	//! Whether the type is knFloat.
	bool isFloat() const { return m_pVTable->getType() == knFloat; }

	//! Whether the type is knDouble.
	bool isDouble() const { return m_pVTable->getType() == knDouble; }

	//! Whether the type is knString.
	bool isString() const { return m_pVTable->getType() == knString; }

	//! Whether the type is knArray.
	bool isArray() const { return m_pVTable->getType() == knArray; }

public:

	//! Whether the type is equivalent to signed char (compiler dependent).
	bool isSChar() const { return isInt8(); }

	//! Whether the type is equivalent to unsigned char (compiler dependent).
	bool isUChar() const { return isUInt8(); }

	//! Whether the type is equivalent to signed short (compiler dependent).
	bool isShort() const { return isInt16(); }

	//! Whether the type is equivalent to unsigned short (compiler dependent).
	bool isUShort() const { return isUInt16(); }

	//! Whether the type is equivalent to signed int (compiler dependent).
	bool isInt() const { return isInt32(); }

	//! Whether the type is equivalent to unsigned int (compiler dependent).
	bool isUInt() const { return isUInt32(); }

	//! Whether the type is equivalent to signed long (compiler dependent).
	bool isLong() const { return isInt32(); }

	//! Whether the type is equivalent to unsigned long (compiler dependent).
	bool isULong() const { return isUInt32(); }

public:

	//! Get the value as bool.
	bool getBool() const { return m_pVTable->getBool(*this); }

	//! Get the value as char.
	char getChar() const { return m_pVTable->getChar(*this); }

	//! Get the value as int8_t.
	int8_t getInt8() const { return m_pVTable->getInt8(*this); }

	//! Get the value as uint8_t.
	uint8_t getUInt8() const { return m_pVTable->getUInt8(*this); }

	//! Get the value as int16_t.
	int16_t getInt16() const { return m_pVTable->getInt16(*this); }

	//! Get the value as uint16_t.
	uint16_t getUInt16() const { return m_pVTable->getUInt16(*this); }

	//! Get the value as int32_t.
	int32_t getInt32() const { return m_pVTable->getInt32(*this); }

	//! Get the value as uint32_t.
	uint32_t getUInt32() const { return m_pVTable->getUInt32(*this); }

	//! Get the value as float.
	float getFloat() const { return m_pVTable->getFloat(*this); }

	//! Get the value as double.
	double getDouble() const { return m_pVTable->getDouble(*this); }

	//! Get the value as string.
	std::string getString() const { return m_pVTable->getString(*this); }

public:

	//! Get the value as signed char (compiler dependent).
	signed char getSChar() const { return getInt8(); }

	//! Get the value as unsigned char (compiler dependent).
	unsigned char getUChar() const { return getUInt8(); }

	//! Get the value as signed short (compiler dependent).
	short getShort() const { return getInt16(); }

	//! Get the value as unsigned short (compiler dependent).
	unsigned short getUShort() const { return getUInt16(); }

	//! Get the value as signed int (compiler dependent).
	int getInt() const { return getInt32(); }

	//! Get the value as unsigned int (compiler dependent).
	unsigned int getUInt() const { return getUInt32(); }

	//! Get the value as signed long (compiler dependent).
	long getLong() const { return getInt32(); }

	//! Get the value as unsigned long (compiler dependent).
	unsigned long getULong() const { return getUInt32(); }

public:

	//! Get the value as bool.
	void get(bool& val) const { val = getBool(); }

	//! Get the value as char.
	void get(char& val) const { val = getChar(); }

	//! Get the value as int8_t.
	void get(int8_t& val) const { val = getInt8(); }

	//! Get the value as uint8_t.
	void get(uint8_t& val) const { val = getUInt8(); }

	//! Get the value as int16_t.
	void get(int16_t& val) const { val = getInt16(); }

	//! Get the value as uint16_t.
	void get(uint16_t& val) const { val = getUInt16(); }

	//! Get the value as int32_t.
	void get(int32_t& val) const { val = getInt32(); }

	//! Get the value as uint32_t.
	void get(uint32_t& val) const { val = getUInt32(); }

	//! Get the value as float.
	void get(float& val) const { val = getFloat(); }

	//! Get the value as double.
	void get(double& val) const { val = getDouble(); }

	//! Get the value as string.
	void get(std::string& val) const { val = getString(); }

public:

	//! Set the value as bool. Type is unaffected.
	void setBool(bool val) { m_pVTable->setBool(*this, val); }

	//! Set the value as char. Type is unaffected.
	void setChar(char val) { m_pVTable->setChar(*this, val); }

	//! Set the value as int8_t. Type is unaffected.
	void setInt8(int8_t val) { m_pVTable->setInt8(*this, val); }

	//! Set the value as uint8_t. Type is unaffected.
	void setUInt8(uint8_t val) { m_pVTable->setUInt8(*this, val); }

	//! Set the value as int16_t. Type is unaffected.
	void setInt16(int16_t val) { m_pVTable->setInt16(*this, val); }

	//! Set the value as uint16_t. Type is unaffected.
	void setUInt16(uint16_t val) { m_pVTable->setUInt16(*this, val); }

	//! Set the value as int32_t. Type is unaffected.
	void setInt32(int32_t val) { m_pVTable->setInt32(*this, val); }

	//! Set the value as uint32_t. Type is unaffected.
	void setUInt32(uint32_t val) { m_pVTable->setUInt32(*this, val); }

	//! Set the value as float. Type is unaffected.
	void setFloat(float val) { m_pVTable->setFloat(*this, val); }

	//! Set the value as double. Type is unaffected.
	void setDouble(double val) { m_pVTable->setDouble(*this, val); }

	//! Set the value as string. Type is unaffected.
	void setString(const std::string& val) { m_pVTable->setString(*this, val); }

public:

	//! Set the value as signed char (compiler dependent). Type is unaffected.
	void setSChar(signed char val) { setInt8(val); }

	//! Set the value as unsigned char (compiler dependent). Type is unaffected.
	void setUChar(unsigned char val) { setUInt8(val); }

	//! Set the value as signed short (compiler dependent). Type is unaffected.
	void setShort(short val) { setInt16(val); }

	//! Set the value as unsigned short (compiler dependent). Type is unaffected.
	void setUShort(unsigned short val) { setUInt16(val); }

	//! Set the value as signed int (compiler dependent). Type is unaffected.
	void setInt(int val) { setInt32(val); }

	//! Set the value as unsigned int (compiler dependent). Type is unaffected.
	void setUInt(unsigned int val) { setUInt32(val); }

	//! Set the value as signed long (compiler dependent). Type is unaffected.
	void setLong(long val) { setInt32(val); }

	//! Set the value as unsigned long (compiler dependent). Type is unaffected.
	void setULong(unsigned long val) { setUInt32(val); }

public:

	//! Set the value as bool. Type is unaffected.
	void set(bool val) { setBool(val); }

	//! Set the value as char. Type is unaffected.
	void set(char val) { setChar(val); }

	//! Set the value as int8_t. Type is unaffected.
	void set(int8_t val) { setInt8(val); }

	//! Set the value as uint8_t. Type is unaffected.
	void set(uint8_t val) { setUInt8(val); }

	//! Set the value as int16_t. Type is unaffected.
	void set(int16_t val) { setInt16(val); }

	//! Set the value as uint16_t. Type is unaffected.
	void set(uint16_t val) { setUInt16(val); }

	//! Set the value as int32_t. Type is unaffected.
	void set(int32_t val) { setInt32(val); }

	//! Set the value as uint32_t. Type is unaffected.
	void set(uint32_t val) { setUInt32(val); }

	//! Set the value as float. Type is unaffected.
	void set(float val) { setFloat(val); }

	//! Set the value as double. Type is unaffected.
	void set(double val) { setDouble(val); }

	//! Set the value as string. Type is unaffected.
	void set(const std::string& val) { setString(val); }

	//! Set the value as string. Type is unaffected.
	void set(const char* val) { setString(val); }

public:

#if 0
	// TODO disabled for now, do we really want them? Document if so.

	// Convenience overloads.
	PYXValue& operator =(bool val) { setBool(val); return *this; }
	PYXValue& operator =(char val) { setChar(val); return *this; }
	PYXValue& operator =(int8_t val) { setInt8(val); return *this; }
	PYXValue& operator =(uint8_t val) { setUInt8(val); return *this; }
	PYXValue& operator =(int16_t val) { setInt16(val); return *this; }
	PYXValue& operator =(uint16_t val) { setUInt16(val); return *this; }
	PYXValue& operator =(int32_t val) { setInt32(val); return *this; }
	PYXValue& operator =(uint32_t val) { setUInt32(val); return *this; }
	PYXValue& operator =(float val) { setFloat(val); return *this; }
	PYXValue& operator =(double val) { setDouble(val); return *this; }
	PYXValue& operator =(const std::string& val) { setString(val); return *this; }
	PYXValue& operator =(const char* val) { setString(val); return *this; }
#endif

public:

	//! Returns the element type; if not an array, returns type.
	eType getArrayType() const
	{
		if (!isArray())
		{
			return getType();
		}
		const ArrayTypeType& nAType = *reinterpret_cast<const ArrayTypeType*>(&m_buffer[knArrayTypeIndex]);
		return static_cast<eType>(nAType);
	}

	//! Returns the number of elements; if not an array, returns 1 (0 if null).
	int getArraySize() const
	{
		if (!isArray())
		{
			return isNull() ? 0 : 1;
		}
		const ArraySizeType& nASize = *reinterpret_cast<const ArraySizeType*>(&m_buffer[knArraySizeIndex]);
		return nASize;
	}

	//! Returns whether array elements are nullable; if not an array, returns false.
	bool isArrayNullable() const
	{
		if (!isArray())
		{
			return false;
		}
		const ArrayNullableType& nANullable = *reinterpret_cast<const ArrayNullableType*>(&m_buffer[knArrayNullableIndex]);
		return nANullable != 0;
	}

	//! Whether the specified array element is null.
	bool isNull(int n) const
	{
		// TODO this can be made more efficient
		return getValue(n).isNull();
	}

	//! Get the specified array element.
	PYXValue getValue(int n) const
	{
		return m_pVTable->getValue(*this, n);
	}

	//! Get the null array element; if not nullable, returns type null.
	PYXValue getNullValue() const
	{
		// TODO this can be made more efficient
		if (!isArrayNullable())
		{
			return PYXValue();
		}
		return m_pVTable->getValue(*this, getArraySize());
	}

public:

	//! Get the specified array element as bool.
	bool getBool(int n) const { return m_pVTable->arrayGetBool(*this, n); }

	//! Get the specified array element as char.
	char getChar(int n) const { return m_pVTable->arrayGetChar(*this, n); }

	//! Get the specified array element as int8_t.
	int8_t getInt8(int n) const { return m_pVTable->arrayGetInt8(*this, n); }

	//! Get the specified array element as uint8_t.
	uint8_t getUInt8(int n) const { return m_pVTable->arrayGetUInt8(*this, n); }

	//! Get the specified array element as int16_t.
	int16_t getInt16(int n) const { return m_pVTable->arrayGetInt16(*this, n); }

	//! Get the specified array element as uint16_t.
	uint16_t getUInt16(int n) const { return m_pVTable->arrayGetUInt16(*this, n); }

	//! Get the specified array element as int32_t.
	int32_t getInt32(int n) const { return m_pVTable->arrayGetInt32(*this, n); }

	//! Get the specified array element as uint32_t.
	uint32_t getUInt32(int n) const { return m_pVTable->arrayGetUInt32(*this, n); }

	//! Get the specified array element as float.
	float getFloat(int n) const { return m_pVTable->arrayGetFloat(*this, n); }

	//! Get the specified array element as double.
	double getDouble(int n) const { return m_pVTable->arrayGetDouble(*this, n); }

	//! Get the specified array element as string.
	std::string getString(int n) const { return m_pVTable->arrayGetString(*this, n); }

public:

	//! Get the specified array element as signed char (compiler dependent).
	signed char getSChar(int n) const { return getInt8(n); }

	//! Get the specified array element as unsigned char (compiler dependent).
	unsigned char getUChar(int n) const { return getUInt8(n); }

	//! Get the specified array element as signed short (compiler dependent).
	short getShort(int n) const { return getInt16(n); }

	//! Get the specified array element as unsigned short (compiler dependent).
	unsigned short getUShort(int n) const { return getUInt16(n); }

	//! Get the specified array element as signed int (compiler dependent).
	int getInt(int n) const { return getInt32(n); }

	//! Get the specified array element as unsigned int (compiler dependent).
	unsigned int getUInt(int n) const { return getUInt32(n); }

	//! Get the specified array element as signed long (compiler dependent).
	long getLong(int n) const { return getInt32(n); }

	//! Get the specified array element as unsigned long (compiler dependent).
	unsigned long getULong(int n) const { return getUInt32(n); }

public:

	//! Set the specified array element as bool. Type is unaffected.
	void setBool(int n, bool val) { m_pVTable->arraySetBool(*this, n, val); }

	//! Set the specified array element as char. Type is unaffected.
	void setChar(int n, char val) { m_pVTable->arraySetChar(*this, n, val); }

	//! Set the specified array element as int8_t. Type is unaffected.
	void setInt8(int n, int8_t val) { m_pVTable->arraySetInt8(*this, n, val); }

	//! Set the specified array element as uint8_t. Type is unaffected.
	void setUInt8(int n, uint8_t val) { m_pVTable->arraySetUInt8(*this, n, val); }

	//! Set the specified array element as int16_t. Type is unaffected.
	void setInt16(int n, int16_t val) { m_pVTable->arraySetInt16(*this, n, val); }

	//! Set the specified array element as uint16_t. Type is unaffected.
	void setUInt16(int n, uint16_t val) { m_pVTable->arraySetUInt16(*this, n, val); }

	//! Set the specified array element as int32_t. Type is unaffected.
	void setInt32(int n, int32_t val) { m_pVTable->arraySetInt32(*this, n, val); }

	//! Set the specified array element as uint32_t. Type is unaffected.
	void setUInt32(int n, uint32_t val) { m_pVTable->arraySetUInt32(*this, n, val); }

	//! Set the specified array element as float. Type is unaffected.
	void setFloat(int n, float val) { m_pVTable->arraySetFloat(*this, n, val); }

	//! Set the specified array element as double. Type is unaffected.
	void setDouble(int n, double val) { m_pVTable->arraySetDouble(*this, n, val); }

	//! Set the specified array element as string. Type is unaffected.
	void setString(int n, const std::string& val) { m_pVTable->arraySetString(*this, n, val); }

public:

	//! Set the specified array element as signed char (compiler dependent). Type is unaffected.
	void setSChar(int n, signed char val) { setInt8(n, val); }

	//! Set the specified array element as unsigned char (compiler dependent). Type is unaffected.
	void setUChar(int n, unsigned char val) { setUInt8(n, val); }

	//! Set the specified array element as signed short (compiler dependent). Type is unaffected.
	void setShort(int n, short val) { setInt16(n, val); }

	//! Set the specified array element as unsigned short (compiler dependent). Type is unaffected.
	void setUShort(int n, unsigned short val) { setUInt16(n, val); }

	//! Set the specified array element as signed int (compiler dependent). Type is unaffected.
	void setInt(int n, int val) { setInt32(n, val); }

	//! Set the specified array element as unsigned int (compiler dependent). Type is unaffected.
	void setUInt(int n, unsigned int val) { setUInt32(n, val); }

	//! Set the specified array element as signed long (compiler dependent). Type is unaffected.
	void setLong(int n, long val) { setInt32(n, val); }

	//! Set the specified array element as unsigned long (compiler dependent). Type is unaffected.
	void setULong(int n, unsigned long val) { setUInt32(n, val); }

public:

	//! Set the specified array element as bool. Type is unaffected.
	void set(int n, bool val) { setBool(n, val); }

	//! Set the specified array element as char. Type is unaffected.
	void set(int n, char val) { setChar(n, val); }

	//! Set the specified array element as int8_t. Type is unaffected.
	void set(int n, int8_t val) { setInt8(n, val); }

	//! Set the specified array element as uint8_t. Type is unaffected.
	void set(int n, uint8_t val) { setUInt8(n, val); }

	//! Set the specified array element as int16_t. Type is unaffected.
	void set(int n, int16_t val) { setInt16(n, val); }

	//! Set the specified array element as uint16_t. Type is unaffected.
	void set(int n, uint16_t val) { setUInt16(n, val); }

	//! Set the specified array element as int32_t. Type is unaffected.
	void set(int n, int32_t val) { setInt32(n, val); }

	//! Set the specified array element as uint32_t. Type is unaffected.
	void set(int n, uint32_t val) { setUInt32(n, val); }

	//! Set the specified array element as float. Type is unaffected.
	void set(int n, float val) { setFloat(n, val); }

	//! Set the specified array element as double. Type is unaffected.
	void set(int n, double val) { setDouble(n, val); }

	//! Set the specified array element as string. Type is unaffected.
	void set(int n, const std::string& val) { setString(n, val); }

	//! Set the specified array element as string. Type is unaffected.
	void set(int n, const char* val) { setString(n, val); }

public:

	//! Get a pointer to the specified array element.
	const void* getPtr(int n) const { return m_pVTable->getPtr(*this, n); }

	//! Get a pointer to the specified array element as bool.
	const bool* getBoolPtr(int n) const { return reinterpret_cast<const bool*>(getPtr(n)); }

	//! Get a pointer to the specified array element as char.
	const char* getCharPtr(int n) const { return reinterpret_cast<const char*>(getPtr(n)); }

	//! Get a pointer to the specified array element as int8_t.
	const int8_t* getInt8Ptr(int n) const { return reinterpret_cast<const int8_t*>(getPtr(n)); }

	//! Get a pointer to the specified array element as uint8_t.
	const uint8_t* getUInt8Ptr(int n) const { return reinterpret_cast<const uint8_t*>(getPtr(n)); }

	//! Get a pointer to the specified array element as int16_t.
	const int16_t* getInt16Ptr(int n) const { return reinterpret_cast<const int16_t*>(getPtr(n)); }

	//! Get a pointer to the specified array element as uint16_t.
	const uint16_t* getUInt16Ptr(int n) const { return reinterpret_cast<const uint16_t*>(getPtr(n)); }

	//! Get a pointer to the specified array element as int32_t.
	const int32_t* getInt32Ptr(int n) const { return reinterpret_cast<const int32_t*>(getPtr(n)); }

	//! Get a pointer to the specified array element as uint32_t.
	const uint32_t* getUInt32Ptr(int n) const { return reinterpret_cast<const uint32_t*>(getPtr(n)); }

	//! Get a pointer to the specified array element as float.
	const float* getFloatPtr(int n) const { return reinterpret_cast<const float*>(getPtr(n)); }

	//! Get a pointer to the specified array element as double.
	const double* getDoublePtr(int n) const { return reinterpret_cast<const double*>(getPtr(n)); }

	//! Get a pointer to the specified array element as string.
	const std::string* getStringPtr(int n) const { return reinterpret_cast<const std::string*>(getPtr(n)); }

public:

	//! Get a pointer to the specified array element.
	void* getPtr(int n) { return const_cast<void*>(m_pVTable->getPtr(*this, n)); }

	//! Get a pointer to the specified array element as bool.
	bool* getBoolPtr(int n) { return reinterpret_cast<bool*>(getPtr(n)); }

	//! Get a pointer to the specified array element as char.
	char* getCharPtr(int n) { return reinterpret_cast<char*>(getPtr(n)); }

	//! Get a pointer to the specified array element as int8_t.
	int8_t* getInt8Ptr(int n) { return reinterpret_cast<int8_t*>(getPtr(n)); }

	//! Get a pointer to the specified array element as uint8_t.
	uint8_t* getUInt8Ptr(int n) { return reinterpret_cast<uint8_t*>(getPtr(n)); }

	//! Get a pointer to the specified array element as int16_t.
	int16_t* getInt16Ptr(int n) { return reinterpret_cast<int16_t*>(getPtr(n)); }

	//! Get a pointer to the specified array element as uint16_t.
	uint16_t* getUInt16Ptr(int n) { return reinterpret_cast<uint16_t*>(getPtr(n)); }

	//! Get a pointer to the specified array element as int32_t.
	int32_t* getInt32Ptr(int n) { return reinterpret_cast<int32_t*>(getPtr(n)); }

	//! Get a pointer to the specified array element as uint32_t.
	uint32_t* getUInt32Ptr(int n) { return reinterpret_cast<uint32_t*>(getPtr(n)); }

	//! Get a pointer to the specified array element as float.
	float* getFloatPtr(int n) { return reinterpret_cast<float*>(getPtr(n)); }

	//! Get a pointer to the specified array element as double.
	double* getDoublePtr(int n) { return reinterpret_cast<double*>(getPtr(n)); }

	//! Get a pointer to the specified array element as string.
	std::string* getStringPtr(int n) { return reinterpret_cast<std::string*>(getPtr(n)); }
	
	PYXValue cast(eType type) const;

private:

#ifndef DOXYGEN_IGNORE // Don't document this internal class.

	//! Fake vtable idiom. (See Alexandrescu article in CUJ June 2002.)
	struct VTable
	{
		// DO NOT REORDER!

		// Destruct
		void		(*destruct)			(PYXValue& value);

		// Types
		eType		(*getType)			();
		void		(*setType)			(PYXValue& value, eType);

		// Comparisons
		bool		(*compareEqualTo)	(const PYXValue& value, const PYXValue& other);
		bool		(*compareLess)		(const PYXValue& value, const PYXValue& other);

		// Copying
		void		(*constructCopy)	(PYXValue& value, const PYXValue& other);
		void		(*assignCopy)		(PYXValue& value, const PYXValue& other);

		// Properties
		bool		(*isNumeric)		(const PYXValue& value);

		// Getters
		bool		(*getBool)			(const PYXValue& value);
		char		(*getChar)			(const PYXValue& value);
		int8_t		(*getInt8)			(const PYXValue& value);
		uint8_t		(*getUInt8)			(const PYXValue& value);
		int16_t		(*getInt16)			(const PYXValue& value);
		uint16_t	(*getUInt16)		(const PYXValue& value);
		int32_t		(*getInt32)			(const PYXValue& value);
		uint32_t	(*getUInt32)		(const PYXValue& value);
		float		(*getFloat)			(const PYXValue& value);
		double		(*getDouble)		(const PYXValue& value);
		std::string	(*getString)		(const PYXValue& value);

		// Setters
		void		(*setBool)			(PYXValue& value, bool val);
		void		(*setChar)			(PYXValue& value, char val);
		void		(*setInt8)			(PYXValue& value, int8_t val);
		void		(*setUInt8)			(PYXValue& value, uint8_t val);
		void		(*setInt16)			(PYXValue& value, int16_t val);
		void		(*setUInt16)		(PYXValue& value, uint16_t val);
		void		(*setInt32)			(PYXValue& value, int32_t val);
		void		(*setUInt32)		(PYXValue& value, uint32_t val);
		void		(*setFloat)			(PYXValue& value, float val);
		void		(*setDouble)		(PYXValue& value, double val);
		void		(*setString)		(PYXValue& value, const std::string& val);

		// Arrays
		PYXValue	(*getValue)			(const PYXValue& value, int n);

		// Array getters
		bool		(*arrayGetBool)		(const PYXValue& value, int n);
		char		(*arrayGetChar)		(const PYXValue& value, int n);
		int8_t		(*arrayGetInt8)		(const PYXValue& value, int n);
		uint8_t		(*arrayGetUInt8)	(const PYXValue& value, int n);
		int16_t		(*arrayGetInt16)	(const PYXValue& value, int n);
		uint16_t	(*arrayGetUInt16)	(const PYXValue& value, int n);
		int32_t		(*arrayGetInt32)	(const PYXValue& value, int n);
		uint32_t	(*arrayGetUInt32)	(const PYXValue& value, int n);
		float		(*arrayGetFloat)	(const PYXValue& value, int n);
		double		(*arrayGetDouble)	(const PYXValue& value, int n);
		std::string	(*arrayGetString)	(const PYXValue& value, int n);

		// Array setters
		void		(*arraySetBool)		(PYXValue& value, int n, bool val);
		void		(*arraySetChar)		(PYXValue& value, int n, char val);
		void		(*arraySetInt8)		(PYXValue& value, int n, int8_t val);
		void		(*arraySetUInt8)	(PYXValue& value, int n, uint8_t val);
		void		(*arraySetInt16)	(PYXValue& value, int n, int16_t val);
		void		(*arraySetUInt16)	(PYXValue& value, int n, uint16_t val);
		void		(*arraySetInt32)	(PYXValue& value, int n, int32_t val);
		void		(*arraySetUInt32)	(PYXValue& value, int n, uint32_t val);
		void		(*arraySetFloat)	(PYXValue& value, int n, float val);
		void		(*arraySetDouble)	(PYXValue& value, int n, double val);
		void		(*arraySetString)	(PYXValue& value, int n, const std::string& val);

		// Pointer access
		const void*	(*getPtr)			(const PYXValue& value, int n);

		//! VTables.
		static const VTable m_vtable[];
	};

#endif // DOXYGEN_IGNORE

private:

// Microsoft Visual Studio .NET 2003 produces this warning:
//		warning C4800: 'const int' : forcing value to bool 'true' or 'false' (performance warning)
// We can't eliminate it using partial template specialization on bool (illegal for
// functions) so pragma it away.
#pragma warning(push)
#pragma warning(disable: 4800)

#ifndef DOXYGEN_IGNORE // Don't document this internal class.

	struct StringManager
	{				
		static std::string * create(const std::string & value);
		static void dispose(std::string * item);
	};

	struct BufferManager
	{				
		static char * create(int size);
		static void dispose(char * item,int size);
	};

	//! Helper class for namespace management.
	struct Helper
	{
		//! Helper function returns a PYXValue::eType for a builtin type.
		template <typename TD>
		static eType getType(const TD*)
		{
			return knNull;
		}

		// Specializations for the above template
		template <> static eType getType(const bool*) { return knBool; }
		template <> static eType getType(const char*) { return knChar; }
		template <> static eType getType(const int8_t*) { return knInt8; }
		template <> static eType getType(const uint8_t*) { return knUInt8; }
		template <> static eType getType(const int16_t*) { return knInt16; }
		template <> static eType getType(const uint16_t*) { return knUInt16; }
		template <> static eType getType(const int32_t*) { return knInt32; }
		template <> static eType getType(const uint32_t*) { return knUInt32; }
		template <> static eType getType(const float*) { return knFloat; }
		template <> static eType getType(const double*) { return knDouble; }
		template <> static eType getType(const std::string*) { return knString; }

		//! Helper function returns a vtable for a builtin type.
		template <typename TD>
		static const VTable& getVTable(const TD& val)
		{
			return VTable::m_vtable[getType(&val)];
		}

		//! Helper function returns a vtable for a PYXValue::eType.
		static const VTable& getVTable(eType type)
		{
			return VTable::m_vtable[type];
		}

		//! Helper function returns a size for a PYXValue::eType.
		static int getSize(eType type)
		{
			return m_typeSize[type];
		}

		//! Helper function does nothing.
		static void doNothing(PYXValue& value)
		{
		}

		//! Helper function does nothing.
		template <typename T>
		static void doNothing(PYXValue& value, T)
		{
		}

		//! Helper function does nothing.
		template <typename T>
		static void doNothing(PYXValue& value, int n, T val)
		{
		}

		static float round(float value);

		static double round(double value);

		//! Helper function returns a ptr to the nth element. (n ignored.)
		template <typename T>
		static const void* getPtr(const PYXValue& value, int n)
		{
			return &value.m_buffer;
		}

		static const void* stringGetPtr(const PYXValue& value, int n)
		{
			const std::string* const& pStr = *reinterpret_cast<const std::string* const *>(&value.m_buffer[0]);
			return pStr;
		}

		//! Helper function returns a ptr to the nth element.
		static const void* arrayGetPtr(const PYXValue& value, int n)
		{
			const char* const& pAData = *reinterpret_cast<const char* const*>(&value.m_buffer[0]);
			const ArrayTypeType& nAType = *reinterpret_cast<const ArrayTypeType*>(&value.m_buffer[knArrayTypeIndex]);

			int nElementSize = getSize(static_cast<eType>(nAType));

			return pAData + (nElementSize * n);
		}

		//! Helper function returns a constant of a specified type.
		template <typename T, T V>
		static T getConstant()
		{
			return V;
		}

		//! Helper function returns a constant of a specified type.
		template <typename T, T V>
		static T getConstant(const PYXValue& value)
		{
			return V;
		}

		//! Helper function returns a constant of a specified type.
		template <typename T, T V>
		static T getConstant(const PYXValue& value, const PYXValue& other)
		{
			return V;
		}

		//! Helper function returns a constant of a specified type.
		template <typename T, T V>
		static T getConstant(const PYXValue& value, int n)
		{
			return V;
		}

		//! Helper function returns a constant of a specified type.
		template <typename T>
		static T getConstantZero(const PYXValue& value)
		{
			return T(0);
		}
		
		//! Helper function returns a constant of a specified type.
		template <typename T>
		static T getConstantZero(const PYXValue& value, int n)
		{
			return T(0);
		}

		//! Helper function for setType; changes type from 'TD' to 'type'.
		template <typename TD>
		static void setType(PYXValue& value, eType type)
		{
			if (type != value.getType())
			{
				// Get the source data in the source type.
				TD data;
				value.get(data);

				if (type == knArray)
				{
					// Swap with a temporary value that is the same type, but in array form.
					value.swap(PYXValue(&data, 1));
				}
				else
				{
					// Create a temporary value of the desired type, set the data, and swap.
					PYXValue temporary;
					temporary.setType(type); // Will call 'nullSetType'.
					temporary.set(data);
					value.swap(temporary);
				}
			}
		}

		//! Helper function for null setType.
		static void nullSetType(PYXValue& value, eType type)
		{
			assert(value.getType() == knNull);

			switch (type)
			{
			case knNull:
				break;
			case knString:
				// Construct a PYXValue so that the buffer is allocated.
				value.swap(PYXValue(std::string()));
				break;
			case knArray:
				// Construct a PYXValue so that the buffer is allocated.
				{
					int32_t const int32Val = value.getInt32();
					value.swap(PYXValue(&int32Val, 1));
				}
				break;
			default:
				// Set the vtable for the proper type,
				// and set the value to 0.
				{
					int32_t const int32Val = value.getInt32();
					value.m_pVTable = &getVTable(type);
					value.setInt32(int32Val);
				}
				break;
			}
		}

		//! Helper function for array setType.
		static void arraySetType(PYXValue& value, eType type)
		{
			assert(value.getType() == knArray);

			if (type != knArray)
			{
				value.swap(value.getValue(0));
				value.setType(type);
			}
		}

		//! Helper function for isNumeric.
		static bool nullIsNumeric(const PYXValue& value)
		{
			assert(value.isNull());
			return false;
		}

		//! Helper function for isNumeric.
		template <typename TD>
		static bool isNumeric(const PYXValue& value)
		{
			return PYXValue::isNumeric(value.getType());
		}

		//! Helper function for isNumeric.
		template <>
		static bool isNumeric<bool>(const PYXValue& value)
		{
			assert(value.isBool());
			return true;
		}

		//! Helper function for isNumeric.
		template <>
		static bool isNumeric<char>(const PYXValue& value)
		{
			assert(value.isChar());
			return isdigit(value.getChar());
		}

		//! Helper function for isNumeric.
		template <>
		static bool isNumeric<std::string>(const PYXValue& value)
		{
			assert(value.isString());
			return StringUtils::isNumeric(value.getString());
		}

		//! Helper function for isNumeric.
		static bool arrayIsNumeric(const PYXValue& value)
		{
			assert(value.isArray());
			assert(0 < value.getArraySize());
			return value.getValue(0).isNumeric();
		}

		//! Helper function for null getString.
		static std::string nullGetString(const PYXValue& value)
		{
			return std::string();
		}

		//! Helper function for null getString.
		static std::string nullGetString(const PYXValue& value, int n)
		{
			return std::string();
		}

		//! Helper function for getting data as a specified type.
		template <typename TD, typename T>
		static T getDataAs(const PYXValue& value)
		{
			const TD& data = *reinterpret_cast<const TD*>(&value.m_buffer[0]);
			return static_cast<T>(data);
		}

		//! Helper function for getting data as a specified type. (n ignored.)
		template <typename TD, typename T>
		static T getDataAs(const PYXValue& value, int n)
		{
			const TD& data = *reinterpret_cast<const TD*>(&value.m_buffer[0]);
			return static_cast<T>(data);
		}

		//! Helper function for getString. (Can't partially specialize function template.)
		template <typename TD>
		static std::string getDataAsString(const PYXValue& value)
		{
			const TD& data = *reinterpret_cast<const TD*>(&value.m_buffer[0]);
			return toString(data);
		}

		//! Helper function for getString. (Can't partially specialize function template. n ignored.)
		template <typename TD>
		static std::string getDataAsString(const PYXValue& value, int n)
		{
			const TD& data = *reinterpret_cast<const TD*>(&value.m_buffer[0]);
			return toString(data);
		}

		//! Helper function for setting data as a specified type.
		template <typename TD, typename T>
		static void setDataAs(PYXValue& value, T val)
		{
			TD& data = *reinterpret_cast<TD*>(&value.m_buffer[0]);
			data = static_cast<TD>(val);
		}

		//! Helper function for setting data as a specified type. (n ignored.)
		template <typename TD, typename T>
		static void setDataAs(PYXValue& value, int n, T val)
		{
			TD& data = *reinterpret_cast<TD*>(&value.m_buffer[0]);
			data = static_cast<TD>(val);
		}

		//! Helper function for setting data as a specified type.
		template <typename TD, typename T>
		static void setDataAsRound(PYXValue& value, T val)
		{
			TD& data = *reinterpret_cast<TD*>(&value.m_buffer[0]);
			data = static_cast<TD>(Helper::round(val));
		}

		//! Helper function for setting data as a specified type. (n ignored.)
		template <typename TD, typename T>
		static void setDataAsRound(PYXValue& value, int n, T val)
		{
			TD& data = *reinterpret_cast<TD*>(&value.m_buffer[0]);
			data = static_cast<TD>(Helper::round(val));
		}

		//! Helper function for setString. (Can't partially specialize function template.)
		template <typename TD>
		static void setDataAsString(PYXValue& value, const std::string& val)
		{
			TD& data = *reinterpret_cast<TD*>(&value.m_buffer[0]);
			data = fromString<TD>(val);
		}

		//! Helper function for setString. (Can't partially specialize function template. n ignored.)
		template <typename TD>
		static void setDataAsString(PYXValue& value, int n, const std::string& val)
		{
			TD& data = *reinterpret_cast<TD*>(&value.m_buffer[0]);
			data = fromString<TD>(val);
		}

		//! Helper function for setting data.
		template <typename TD>
		static void setData(PYXValue& value, TD val)
		{
			TD& data = *reinterpret_cast<TD*>(&value.m_buffer[0]);
			data = val;
		}

		//! Helper function for non-array copy assignment.
		template <typename TD>
		static void assignCopy(PYXValue& value, const PYXValue& other)
		{
			TD& data = *reinterpret_cast<TD*>(&value.m_buffer[0]);
			const TD& otherdata = *reinterpret_cast<const TD*>(&other.m_buffer[0]);
			data = otherdata;
		}

		//! Helper function for non-array comparison.
		template <typename TD>
		static bool compareEqualTo(const PYXValue& value, const PYXValue& other)
		{
			const TD& data = *reinterpret_cast<const TD*>(&value.m_buffer[0]);
			const TD& otherdata = *reinterpret_cast<const TD*>(&other.m_buffer[0]);
			return value.m_pVTable == other.m_pVTable && data == otherdata;
		}

		//! Helper function for string comparison.
		static bool stringCompareEqualTo(const PYXValue& value, const PYXValue& other)
		{
			const std::string* const& pStr = *reinterpret_cast<const std::string* const *>(&value.m_buffer[0]);
			const std::string* const& pOtherStr = *reinterpret_cast<const std::string* const *>(&other.m_buffer[0]);
			return value.m_pVTable == other.m_pVTable && *pStr == *pOtherStr;
		}

		//! Helper function for non-array comparison.
		template <typename TD>
		static bool compareLess(const PYXValue& value, const PYXValue& other)
		{
			const TD& data = *reinterpret_cast<const TD*>(&value.m_buffer[0]);
			const TD& otherdata = *reinterpret_cast<const TD*>(&other.m_buffer[0]);
			return value.getType() < other.getType() ||
				value.m_pVTable == other.m_pVTable && data < otherdata;
		}

		//! Helper function for string comparison.
		static bool stringCompareLess(const PYXValue& value, const PYXValue& other)
		{
			const std::string* const& pStr = *reinterpret_cast<const std::string* const *>(&value.m_buffer[0]);
			const std::string* const& pOtherStr = *reinterpret_cast<const std::string* const *>(&other.m_buffer[0]);
			return value.getType() < other.getType() ||
				value.m_pVTable == other.m_pVTable && *pStr < *pOtherStr;
		}

		//! Helper function for array constructor.
		template <typename TD>
		static void constructArray(PYXValue& value, const TD* pVal, int nSize)
		{
			assert(0 < nSize && "Array size must be greater than zero.");
			assert(nSize <= std::numeric_limits<ArraySizeType>::max() && "Array size cannot overflow.");

			TD*& pAData = *reinterpret_cast<TD**>(&value.m_buffer[0]);
			ArrayNullableType& nANullable = *reinterpret_cast<ArrayNullableType*>(&value.m_buffer[knArrayNullableIndex]);
			ArrayTypeType& nAType = *reinterpret_cast<ArrayTypeType*>(&value.m_buffer[knArrayTypeIndex]);
			ArraySizeType& nASize = *reinterpret_cast<ArraySizeType*>(&value.m_buffer[knArraySizeIndex]);

			pAData = reinterpret_cast<TD*>(BufferManager::create(sizeof(TD)*(nSize)));//new TD[nSize];
			nANullable = 0;
			nAType = static_cast<ArrayTypeType>(getType(pVal));
			nASize = nSize;

			if (pVal != 0)
			{
				// Use memcpy for performance, which is safe for POD types.
				memcpy(pAData, pVal, sizeof(TD) * nSize);
			}
			else
			{
				// Initialize to zero.
				// Use memset for performance, which is safe for POD types.
				memset(pAData, 0, sizeof(TD) * nSize);
			}
		}

		//! Helper function for array constructor. (Specialized for string.)
		template <>
		static void constructArray(PYXValue& value, const std::string* pVal, int nSize)
		{
			assert(0 < nSize && "Array size must be greater than zero.");
			assert(nSize <= std::numeric_limits<ArraySizeType>::max() && "Array size cannot overflow.");

			typedef std::string TD;
			TD*& pAData = *reinterpret_cast<TD**>(&value.m_buffer[0]);
			ArrayNullableType& nANullable = *reinterpret_cast<ArrayNullableType*>(&value.m_buffer[knArrayNullableIndex]);
			ArrayTypeType& nAType = *reinterpret_cast<ArrayTypeType*>(&value.m_buffer[knArrayTypeIndex]);
			ArraySizeType& nASize = *reinterpret_cast<ArraySizeType*>(&value.m_buffer[knArraySizeIndex]);

			pAData = new TD[nSize];
			nANullable = 0;
			nAType = static_cast<ArrayTypeType>(getType(pVal));
			nASize = nSize;

			// The destination string array contains empty strings.
			// If the source array is not null, copy them.
			if (pVal != 0)
			{
				// Use std::copy for correctness.
				std::copy(pVal, pVal + nSize, pAData);
			}
		}

		/*!
		Stores a copy of the null value past the end, for use by getNullValue.
		The null value acts as a "filter", replacing the given value with the default PYXValue.
		*/
		//! Helper function for array constructor.
		template <typename TD>
		static void constructArray(PYXValue& value, const TD* pVal, int nSize, const TD& nullVal)
		{
			assert(0 < nSize && "Array size must be greater than zero.");
			assert(nSize <= std::numeric_limits<ArraySizeType>::max() && "Array size cannot overflow.");

			TD*& pAData = *reinterpret_cast<TD**>(&value.m_buffer[0]);
			ArrayNullableType& nANullable = *reinterpret_cast<ArrayNullableType*>(&value.m_buffer[knArrayNullableIndex]);
			ArrayTypeType& nAType = *reinterpret_cast<ArrayTypeType*>(&value.m_buffer[knArrayTypeIndex]);
			ArraySizeType& nASize = *reinterpret_cast<ArraySizeType*>(&value.m_buffer[knArraySizeIndex]);

			pAData = reinterpret_cast<TD*>(BufferManager::create(sizeof(TD)*(nSize+1))); //new TD[nSize + 1];
			nANullable = 1;
			nAType = static_cast<ArrayTypeType>(getType(pVal));
			nASize = nSize;

			if (pVal != 0)
			{
				// Use memcpy for performance, which is safe for POD types.
				memcpy(pAData, pVal, sizeof(TD) * nSize);
			}
			else
			{
				// Initialize to zero.
				// Use memset for performance, which is safe for POD types.
				memset(pAData, 0, sizeof(TD) * nSize);
			}
			pAData[nSize] = nullVal;
		}

		/*!
		Stores a copy of the null value past the end, for use by getNullValue.
		The null value acts as a "filter", replacing the given value with the default PYXValue.
		*/
		//! Helper function for array constructor. (Specialized for string.)
		template <>
		static void constructArray(PYXValue& value, const std::string* pVal, int nSize, const std::string& nullVal)
		{
			assert(0 < nSize && "Array size must be greater than zero.");
			assert(nSize <= std::numeric_limits<ArraySizeType>::max() && "Array size cannot overflow.");

			typedef std::string TD;
			TD*& pAData = *reinterpret_cast<TD**>(&value.m_buffer[0]);
			ArrayNullableType& nANullable = *reinterpret_cast<ArrayNullableType*>(&value.m_buffer[knArrayNullableIndex]);
			ArrayTypeType& nAType = *reinterpret_cast<ArrayTypeType*>(&value.m_buffer[knArrayTypeIndex]);
			ArraySizeType& nASize = *reinterpret_cast<ArraySizeType*>(&value.m_buffer[knArraySizeIndex]);

			pAData = new TD[nSize + 1];
			nANullable = 1;
			nAType = static_cast<ArrayTypeType>(getType(pVal));
			nASize = nSize;

			// If the value is null, we can leave the strings empty.
			if (pVal != 0)
			{
				// Use std::copy for correctness.
				std::copy(pVal, pVal + nSize, pAData);
				pAData[nSize] = nullVal;
			}
		}

		//! Helper function for array destructor.
		static void destructArray(PYXValue& value)
		{
			ArrayTypeType& nAType = *reinterpret_cast<ArrayTypeType*>(&value.m_buffer[knArrayTypeIndex]);
			if (nAType == knString)
			{
				// Cast to string so destructors are called.
				std::string*& pAData = *reinterpret_cast<std::string**>(&value.m_buffer[0]);
				delete[] pAData;
				pAData = 0;
			}
			else
			{
				// No need to call destructors here.
				char*& pAData = *reinterpret_cast<char**>(&value.m_buffer[0]);

				ArrayNullableType& nANullable = *reinterpret_cast<ArrayNullableType*>(&value.m_buffer[knArrayNullableIndex]);
				ArrayTypeType& nAType = *reinterpret_cast<ArrayTypeType*>(&value.m_buffer[knArrayTypeIndex]);
				ArraySizeType& nASize = *reinterpret_cast<ArraySizeType*>(&value.m_buffer[knArraySizeIndex]);

				int nElementSize = getSize(static_cast<eType>(nAType));
				int nElementCountPlusNull = nASize + nANullable;
				int nTotalSize = nElementSize * nElementCountPlusNull;

				BufferManager::dispose(pAData,nTotalSize); //delete[] pAData;
				pAData = 0;
			}
		}

		//! Helper function for array constructor assignment.
		static void constructArray(PYXValue& value, const PYXValue& other)
		{
			char*& pAData = *reinterpret_cast<char**>(&value.m_buffer[0]);
			ArrayNullableType& nANullable = *reinterpret_cast<ArrayNullableType*>(&value.m_buffer[knArrayNullableIndex]);
			ArrayTypeType& nAType = *reinterpret_cast<ArrayTypeType*>(&value.m_buffer[knArrayTypeIndex]);
			ArraySizeType& nASize = *reinterpret_cast<ArraySizeType*>(&value.m_buffer[knArraySizeIndex]);

			const char* const& pOtherAData = *reinterpret_cast<const char* const *>(&other.m_buffer[0]);
			const ArrayNullableType& nOtherANullable = *reinterpret_cast<const ArrayNullableType*>(&other.m_buffer[knArrayNullableIndex]);
			const ArrayTypeType& nOtherAType = *reinterpret_cast<const ArrayTypeType*>(&other.m_buffer[knArrayTypeIndex]);
			const ArraySizeType& nOtherASize = *reinterpret_cast<const ArraySizeType*>(&other.m_buffer[knArraySizeIndex]);

			int nElementSize = getSize(static_cast<eType>(nOtherAType));
			int nElementCountPlusNull = nOtherASize + nOtherANullable;
			int nTotalSize = nElementSize * nElementCountPlusNull;

			nANullable = nOtherANullable;
			nAType = nOtherAType;
			nASize = nOtherASize;

			if (nAType == knString)
			{
				std::string*& pStringData = reinterpret_cast<std::string*&>(pAData);
				pStringData = new std::string[nElementCountPlusNull];
				const std::string* pOtherStringData = reinterpret_cast<const std::string*>(pOtherAData);
				// Use std::copy for correctness.
				std::copy(pOtherStringData, pOtherStringData + nElementCountPlusNull, pStringData);
			}
			else
			{
				pAData = BufferManager::create(nTotalSize);// new char[nTotalSize];
				// Use memcpy for performance.
				memcpy(pAData, pOtherAData, nTotalSize);
			}
		}

		//! Helper function for array copy assignment.
		static void assignArray(PYXValue& value, const PYXValue& other)
		{
			char*& pAData = *reinterpret_cast<char**>(&value.m_buffer[0]);
			ArrayNullableType& nANullable = *reinterpret_cast<ArrayNullableType*>(&value.m_buffer[knArrayNullableIndex]);
			ArrayTypeType& nAType = *reinterpret_cast<ArrayTypeType*>(&value.m_buffer[knArrayTypeIndex]);
			ArraySizeType& nASize = *reinterpret_cast<ArraySizeType*>(&value.m_buffer[knArraySizeIndex]);

			const char* const& pOtherAData = *reinterpret_cast<const char* const *>(&other.m_buffer[0]);
			const ArrayNullableType& nOtherANullable = *reinterpret_cast<const ArrayNullableType*>(&other.m_buffer[knArrayNullableIndex]);
			const ArrayTypeType& nOtherAType = *reinterpret_cast<const ArrayTypeType*>(&other.m_buffer[knArrayTypeIndex]);
			const ArraySizeType& nOtherASize = *reinterpret_cast<const ArraySizeType*>(&other.m_buffer[knArraySizeIndex]);

			int nElementSize = getSize(static_cast<eType>(nOtherAType));
			int nElementCountPlusNull = nOtherASize + nOtherANullable;
			int nTotalSize = nElementSize * nElementCountPlusNull;

			if (nANullable == nOtherANullable && 
				nAType == nOtherAType &&
				nASize == nOtherASize)
			{
				if (nAType == knString)
				{
					std::string*& pStringData = reinterpret_cast<std::string*&>(pAData);
					const std::string* pOtherStringData = reinterpret_cast<const std::string*>(pOtherAData);
					// Use std::copy for correctness.
					std::copy(pOtherStringData, pOtherStringData + nElementCountPlusNull, pStringData);
				}
				else 
				{
					// Use memcpy for performance.
					memcpy(pAData, pOtherAData, nTotalSize);
				}
			}
			else
			{
				destructArray(value);

				nANullable = nOtherANullable;
				nAType = nOtherAType;
				nASize = nOtherASize;

				if (nAType == knString)
				{
					std::string*& pStringData = reinterpret_cast<std::string*&>(pAData);
					pStringData = new std::string[nElementCountPlusNull];
					const std::string* pOtherStringData = reinterpret_cast<const std::string*>(pOtherAData);
					// Use std::copy for correctness.
					std::copy(pOtherStringData, pOtherStringData + nElementCountPlusNull, pStringData);
				}
				else
				{
					pAData = BufferManager::create(nTotalSize);// new char[nTotalSize];
					// Use memcpy for performance.
					memcpy(pAData, pOtherAData, nTotalSize);
				}
			}
		}

		//! Helper function for getValue.
		template <typename TD>
		static PYXValue getValue(const PYXValue& value, int n);

		//! Helper function for getValue.
		static PYXValue arrayGetValue(const PYXValue& value, int n)
		{
			const ArrayTypeType& nAType = *reinterpret_cast<const ArrayTypeType*>(&value.m_buffer[knArrayTypeIndex]);
			return m_funcTableGetValue[nAType](value, n);
		}

		//! Helper function for getting the first array value.
		template <typename T>
		static T arrayGetDataAs(const PYXValue& value)
		{
			return arrayGetDataAs<T>(value, 0);
		}

		//! Helper function for getting an array value.
		template <typename T>
		static T arrayGetDataAs(const PYXValue& value, int n)
		{
			const char* const& pAData = *reinterpret_cast<const char* const*>(&value.m_buffer[0]);
			const ArrayTypeType& nAType = *reinterpret_cast<const ArrayTypeType*>(&value.m_buffer[knArrayTypeIndex]);

			T val;
			char* pDest = reinterpret_cast<char*>(&val);
			int nOtherAType = getType(&val);

			int nElementSize = getSize(static_cast<eType>(nAType));

			m_funcTableArrayCopy[nOtherAType][nAType](pDest, pAData + (nElementSize * n), 1);

			return val;
		}

		//! Helper function for setting the first array value.
		template <typename T>
		static void arraySetDataAs(PYXValue& value, T val)
		{
			arraySetDataAs(value, 0, val);
		}

		//! Helper function for setting the first array value as string.
		static void arraySetDataAsString(PYXValue& value, const std::string& val)
		{
			arraySetDataAsString(value, 0, val);
		}

		//! Helper function for setting an array value.
		template <typename T>
		static void arraySetDataAs(PYXValue& value, int n, T val)
		{
			char*& pAData = *reinterpret_cast<char**>(&value.m_buffer[0]);
			ArrayTypeType& nAType = *reinterpret_cast<ArrayTypeType*>(&value.m_buffer[knArrayTypeIndex]);

			const char* pSrc = reinterpret_cast<const char*>(&val);
			int nOtherAType = getType(&val);

			int nElementSize = getSize(static_cast<eType>(nAType));

			m_funcTableArrayCopy[nAType][nOtherAType](pAData + (nElementSize * n), pSrc, 1);
		}

		//! Helper function for setting an array value as string.
		static void arraySetDataAsString(PYXValue& value, int n, const std::string& val)
		{
			char*& pAData = *reinterpret_cast<char**>(&value.m_buffer[0]);
			ArrayTypeType& nAType = *reinterpret_cast<ArrayTypeType*>(&value.m_buffer[knArrayTypeIndex]);

			const char* pSrc = reinterpret_cast<const char*>(&val);
			int nOtherAType = getType(&val);

			int nElementSize = getSize(static_cast<eType>(nAType));

			m_funcTableArrayCopy[nAType][nOtherAType](pAData + (nElementSize * n), pSrc, 1);
		}

		/*!
		Helper function copies elements from one C-style array to another, performing
		a static cast on each element as it copies.
		\param pDest	Pointer to the first destination element (as raw memory).
		\param pSrc		Pointer to the first source element (as raw memory).
		\param nSize	The number of elements to copy.
		*/
		//! Helper function copies (and casts) array elements.
		template <typename TDEST, typename TSRC>
		static void arrayCopy(char* pDest, const char* pSrc, int nSize)
		{
			TDEST* pDestCast = reinterpret_cast<TDEST*>(pDest);
			const TSRC* pSrcCast = reinterpret_cast<const TSRC*>(pSrc);
			for (int n = 0; n != nSize; ++n)
			{
				pDestCast[n] = static_cast<TDEST>(pSrcCast[n]);
			}
		}

		/*!
		Helper function copies elements from one C-style array to another, performing
		a string conversion on each element as it copies.
		\param pDest	Pointer to the first destination element (as raw memory).
		\param pSrc		Pointer to the first source element (as raw memory).
		\param nSize	The number of elements to copy.
		*/
		//! Helper function copies (and casts) array elements.
		template <typename TSRC>
		static void arrayCopyToString(char* pDest, const char* pSrc, int nSize)
		{
			typedef std::string TDEST;
			TDEST* pDestCast = reinterpret_cast<TDEST*>(pDest);
			const TSRC* pSrcCast = reinterpret_cast<const TSRC*>(pSrc);
			for (int n = 0; n != nSize; ++n)
			{
				pDestCast[n] = toString(pSrcCast[n]);
			}
		}

		/*!
		Helper function copies elements from one C-style array to another, performing
		a string conversion on each element as it copies.
		\param pDest	Pointer to the first destination element (as raw memory).
		\param pSrc		Pointer to the first source element (as raw memory).
		\param nSize	The number of elements to copy.
		*/
		//! Helper function copies (and casts) array elements.
		template <typename TDEST>
		static void arrayCopyFromString(char* pDest, const char* pSrc, int nSize)
		{
			typedef std::string TSRC;
			TDEST* pDestCast = reinterpret_cast<TDEST*>(pDest);
			const TSRC* pSrcCast = reinterpret_cast<const TSRC*>(pSrc);
			for (int n = 0; n != nSize; ++n)
			{
				pDestCast[n] = fromString<TDEST>(pSrcCast[n]);
			}
		}

		//! Helper function for string constructor.
		static void constructString(PYXValue& value, const std::string& val)
		{
			std::string*& pStr = *reinterpret_cast<std::string**>(&value.m_buffer[0]);

			//pStr =  new std::string(val);
			pStr = StringManager::create(val);
		}

		//! Helper function for string copy assignment.
		static void constructString(PYXValue& value, const PYXValue& other)
		{
			std::string*& pStr = *reinterpret_cast<std::string**>(&value.m_buffer[0]);

			const std::string* const& pOtherStr = *reinterpret_cast<const std::string* const*>(&other.m_buffer[0]);

			//pStr = new std::string(*pOtherStr);
			pStr = StringManager::create(*pOtherStr);
		}

		//! Helper function for string destructor.
		static void destructString(PYXValue& value)
		{
			std::string*& pStr = *reinterpret_cast<std::string**>(&value.m_buffer[0]);

			StringManager::dispose(pStr);
			//delete pStr;
			pStr = 0;
		}

		//! Helper function for string copy assignment.
		static void assignString(PYXValue& value, const PYXValue& other)
		{
			std::string*& pStr = *reinterpret_cast<std::string**>(&value.m_buffer[0]);

			const std::string* const& pOtherStr = *reinterpret_cast<const std::string* const*>(&other.m_buffer[0]);

			//pStr = new std::string(*pOtherStr);
			*pStr = *pOtherStr;
		}

		//! Helper function for string getString.
		static std::string stringGetString(const PYXValue& value)
		{
			const std::string* const& pStr = *reinterpret_cast<const std::string* const *>(&value.m_buffer[0]);
			return std::string(*pStr);
		}

		//! Helper function for string getString. (n ignored)
		static std::string stringGetString(const PYXValue& value, int n)
		{
			const std::string* const& pStr = *reinterpret_cast<const std::string* const *>(&value.m_buffer[0]);
			return std::string(*pStr);
		}

		//! Helper function for string setString.
		static void stringSetString(PYXValue& value, const std::string& val)
		{
			std::string*& pStr = *reinterpret_cast<std::string**>(&value.m_buffer[0]);
			*pStr = val;
		}

		//! Helper function for string setString. (n ignored.)
		static void stringSetString(PYXValue& value, int n, const std::string& val)
		{
			std::string*& pStr = *reinterpret_cast<std::string**>(&value.m_buffer[0]);
			*pStr = val;
		}

		//! Helper function for getting the string value.
		template <typename T>
		static T stringGetDataAs(const PYXValue& value)
		{
			const std::string* const& pStr = *reinterpret_cast<const std::string* const *>(&value.m_buffer[0]);
			return fromString<T>(*pStr);
		}

		//! Helper function for getting the string value. (n ignored.)
		template <typename T>
		static T stringGetDataAs(const PYXValue& value, int n)
		{
			const std::string* const& pStr = *reinterpret_cast<const std::string* const *>(&value.m_buffer[0]);
			return fromString<T>(*pStr);
		}

		//! Helper function for setting the string value.
		template <typename T>
		static void stringSetDataAs(PYXValue& value, T val)
		{
			std::string*& pStr = *reinterpret_cast<std::string**>(&value.m_buffer[0]);
			*pStr = toString(val);
		}

		//! Helper function for setting the string value. (n ignored.)
		template <typename T>
		static void stringSetDataAs(PYXValue& value, int n, T val)
		{
			std::string*& pStr = *reinterpret_cast<std::string**>(&value.m_buffer[0]);
			*pStr = toString(val);
		}

		/*!
		Converts a numeric value to a string. Uses the maximum precision
		possible. Converts boolean and character types as numbers.
		*/
		//! Helper function for string conversion.
		template <typename T>
		static std::string toString(T val)
		{
			// Set the stream's precision.
			// std::numeric_limits<>::digits10 is inadequate, we need to use a
			// rather complicated formula as noted here:
			// http://lists.boost.org/Archives/boost/2003/08/51210.php
			std::ostringstream out;
			const int nDigits = 2 + std::numeric_limits<long double>::digits * 301/1000;
			out.precision(nDigits);
			out << val;
			return out.str();
		}

		// Specializations for the above template
		// (so characters stream numerically not textually)
		template <> static std::string toString(char val) { return toString(static_cast<int>(val)); }
		template <> static std::string toString(signed char val) { return toString(static_cast<int>(val)); }
		template <> static std::string toString(unsigned char val) { return toString(static_cast<int>(val)); }

		/*!
		Converts a string to a numeric value. Uses the maximum precision
		possible. Converts boolean and character types as numbers. Casts as
		needed (so a real number converted to an integer will lose the
		fraction).
		*/
		//! Helper function for string conversion.
		template <typename T>
		static T fromString(const std::string& str)
		{
			// Use unsigned int for greatest precision.
			unsigned int val(0);
			std::istringstream in(str);
			in >> val;
			return static_cast<T>(val);
		}

		// Specializations for the above template
		// (so floating point numbers deserialize as expected)
		template <>
		static float fromString<float>(const std::string& str)
		{
			// Use double for greatest precision.
			double val(0);
			std::istringstream in(str);
			in >> val;
			return static_cast<float>(val);
		}
		template <>
		static double fromString<double>(const std::string& str)
		{
			// Use double for greatest precision.
			double val(0);
			std::istringstream in(str);
			in >> val;
			return static_cast<double>(val);
		}

		//! Function table for getValue, indexed by type.
		static PYXValue (*m_funcTableGetValue[])
			(const PYXValue& value, int n);

		//! Function table for arrayCopy, indexed by dest type then src type.
		static void (*m_funcTableArrayCopy[knTypeCount][knTypeCount])
			(char* pDest, const char* pSrc, int nSize);

		//! Function table for size, indexed by type.
		static int m_typeSize[];
	};

#endif // DOXYGEN_IGNORE

#pragma warning(pop)

private:

	friend PYXLIB_DECL bool operator ==(const PYXValue&, const PYXValue&);
	friend PYXLIB_DECL bool operator !=(const PYXValue&, const PYXValue&);

	//order operator - please NOTE:
	//PYXValue v1(100);
	//PYXValue v2(3.14);
	//v1 < v2 == true; why? becaause the type of v1 (int) is "smaller" then v2 (float)
	//PYXVAlue(100) < PYXVAlue(200) == true - if its the same type - it compare the values.
	//
	//If you are looking for comparing PYXValues by the value, check the "compare" functions.
	friend PYXLIB_DECL bool operator <(const PYXValue&, const PYXValue&);


	friend PYXLIB_DECL std::istream& operator >>(std::istream&, PYXValue&);

private:

#ifndef DOXYGEN_IGNORE

	enum
	{
		knBufferSize = 8,			//!< Store data in 64 bit buffer.

		knArrayNullableIndex = 4,	//!< Index of array nullability as encoded in buffer.
		knArrayTypeIndex = 5,		//!< Index of array type as encoded in buffer.
		knArraySizeIndex = 6,		//!< Index of array size as encoded in buffer.
	};

	//! Type of array nullable as encoded in buffer.
	typedef int8_t ArrayNullableType;

	//! Type of array type as encoded in buffer.
	typedef int8_t ArrayTypeType;

	//! Type of array size as encoded in buffer.
	typedef int16_t ArraySizeType;

#endif // DOXYGEN_IGNORE

private:

	//! Buffer to store data.  Either stores inline data or a pointer with additional information.
	boost::array<unsigned char, knBufferSize> m_buffer;

	//! Fake vtable idiom. (See Alexandrescu article in CUJ June 2002.)
	VTable const * m_pVTable;
};

//! Overload of swap algorithm.
PYXLIB_DECL inline
void swap(PYXValue& lhs, PYXValue& rhs)
{
	lhs.swap(rhs);
}

//! Comparison operator.
PYXLIB_DECL inline
bool operator ==(const PYXValue& lhs, const PYXValue& rhs)
{
	return lhs.m_pVTable->compareEqualTo(lhs, rhs);
}

//! Comparison operator.
PYXLIB_DECL inline
bool operator !=(const PYXValue& lhs, const PYXValue& rhs)
{
	// TODO consider having compareNotEqualTo function in fake vtable
	return !lhs.m_pVTable->compareEqualTo(lhs, rhs);
}

//! order operator.
PYXLIB_DECL inline
bool operator <(const PYXValue& lhs, const PYXValue& rhs)
{
	return lhs.m_pVTable->compareLess(lhs, rhs);
}

//! Stream operator.
PYXLIB_DECL std::ostream& operator <<(std::ostream& out, const PYXValue& value);
PYXLIB_DECL std::ostream& operator <<(std::ostream& out, const std::vector<PYXValue> & value);

//! Stream operator.
PYXLIB_DECL std::istream& operator >>(std::istream& in, PYXValue& value);
PYXLIB_DECL std::istream& operator >>(std::istream& in, std::vector<PYXValue> & value);

#endif // guard
