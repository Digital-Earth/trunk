/******************************************************************************
value_column.cpp

begin		: 2006-05-10
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/value_column.h"

// pyxlib includes
#include "pyxis/utility/exceptions.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <sstream>

//! The unit test class
Tester<PYXValueColumn> gTester;

//! Guess average string length will be 50 \sa estimateHeapBytes()
static const int knStringSizeGuess = 50;

//! Macros for binary stream I/O, to simplify the syntax of read() and write() methods
#define WRITE(var) write((char*)(&var), sizeof(var))
#define READ(var) read((char*)(&var), sizeof(var))

/*!
The unit test method for the class.
*/
void PYXValueColumn::test()
{
	{ // TEST 1: NULLABLE ARRAY OF int16_t

		// allocate a nullable PYXValueColumn of 10 int16's and verify basic characteristics
		PYXValueColumn va(PYXValue::knInt16, 10);
		TEST_ASSERT(va.getHeight() == 10);
		TEST_ASSERT(va.getWidth() == 1);
		TEST_ASSERT(va.getType() == PYXValue::knInt16);

		// heap size should be 10x2=20 bytes for data plus 2 bytes for the bit vector
		int nHeapBytes = va.getHeapBytes();
		TEST_ASSERT(nHeapBytes == 22);

		// all 10 values should initially be uninitialized-null
		for (int nElement = 0; nElement < 10; ++nElement)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement, &bInitialized).isNull());
			TEST_ASSERT(bInitialized == false);
		}

		// set and verify even-indexed elements, and explicitly set last one to null
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			va.setValue(nElement, PYXValue((int16_t)nElement));
		}
		va.setValue(9, PYXValue());
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement) == PYXValue((int16_t)nElement));
			TEST_ASSERT(va.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

		// uncomment to verify that out-of-range addressing causes assert() failure
		//va.getValue(-1);
		//va.getValue(10);

		// uncomment to verify that setValue() with wrong type causes assert() failure
		//va.setValue(0, PYXValue((char)50));

		// uncomment to print this column to trace output
		//TRACE_INFO("PYXValueColumn diagnostic printout" << std::endl << va);

		// serialize/deserialize and verify that it comes back the same
		std::ostringstream out;
		va.serialize(out);
		PYXValueColumn va2;
		std::istringstream in(out.str());	
		va2.deserialize(in);
		TEST_ASSERT(va2.getHeight() == 10);
		TEST_ASSERT(va2.getWidth() == 1);
		TEST_ASSERT(va2.getType() == PYXValue::knInt16);
		TEST_ASSERT(va2.getHeapBytes() == 22);
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va2.getValue(nElement) == PYXValue((int16_t)nElement));
			TEST_ASSERT(va2.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

	} // END OF TEST 1

	{ // TEST 2: NON-NULLABLE ARRAY
		std::string tempStr;
		// allocate a non-nullable PYXValueColumn of 10 int16's and verify basic characteristics
		PYXValueColumn va(PYXValue::knInt16, 10, 1, false);
		TEST_ASSERT(va.getHeight() == 10);
		TEST_ASSERT(va.getWidth() == 1);
		TEST_ASSERT(va.getType() == PYXValue::knInt16);

		// heap size should be 10x2=20 bytes
		int nHeapBytes = va.getHeapBytes();
		TEST_ASSERT(nHeapBytes == 20);

		// all 10 values should initially be the int16 equivalent of null
		PYXValue null;
		int16_t nullVal = null.getInt16();
		for (int nElement = 0; nElement < 10; ++nElement)
		{
			TEST_ASSERT(va.getValue(nElement) == PYXValue(nullVal));
		}

		// uncomment to verify that attempting to put a null value in will assert
		//va.setValue(0, PYXValue());

		// set and verify even-indexed elements
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			va.setValue(nElement, PYXValue((int16_t)nElement));
		}
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			TEST_ASSERT(va.getValue(nElement) == PYXValue((int16_t)nElement));
			TEST_ASSERT(va.getValue(nElement + 1) == PYXValue(nullVal));
		}

		// uncomment to print this column to trace output
		//TRACE_INFO("PYXValueColumn diagnostic printout" << std::endl << va);

		// serialize/deserialize and verify that it comes back the same
		std::ostringstream out;
		va.serialize(out);
		PYXValueColumn va2;
		std::istringstream in(out.str());	
		va2.deserialize(in);
		TEST_ASSERT(va2.getHeight() == 10);
		TEST_ASSERT(va2.getWidth() == 1);
		TEST_ASSERT(va2.getType() == PYXValue::knInt16);
		TEST_ASSERT(va2.getHeapBytes() == 20);
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			TEST_ASSERT(va2.getValue(nElement) == PYXValue((int16_t)nElement));
			TEST_ASSERT(va2.getValue(nElement + 1) == PYXValue(nullVal));
		}

	} // END OF TEST 2

	{ // TEST 3: type bool, size multiple of 2
		std::string tempStr;
		// allocate a nullable PYXValueColumn of 8 bool's and verify basic characteristics
		PYXValueColumn va(PYXValue::knBool, 8);
		TEST_ASSERT(va.getHeight() == 8);
		TEST_ASSERT(va.getWidth() == 1);
		TEST_ASSERT(va.getType() == PYXValue::knBool);

		// heap size should be 1 byte for data plus 1 byte for the bit vector
		int nHeapBytes = va.getHeapBytes();
		TEST_ASSERT(nHeapBytes == 2);

		// all 8 values should initially be uninitialized-null
		for (int nElement = 0; nElement < 8; ++nElement)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement, &bInitialized).isNull());
			TEST_ASSERT(bInitialized == false);
		}

		// set and verify even-indexed elements, and explicitly set last one to null
		for (int nElement = 0; nElement < 8; nElement += 2)
		{
			va.setValue(nElement, PYXValue((nElement & 2) != 0));	// alternating true/false 
		}
		va.setValue(7, PYXValue());
		for (int nElement = 0; nElement < 8; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement) == PYXValue((nElement & 2) != 0));
			TEST_ASSERT(va.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 7)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

		// uncomment to print this column to trace output
		//TRACE_INFO("PYXValueColumn diagnostic printout" << std::endl << va);

		// serialize/deserialize and verify that it comes back the same
		std::ostringstream out;
		va.serialize(out);
		PYXValueColumn va2;
		std::istringstream in(out.str());	
		va2.deserialize(in);
		TEST_ASSERT(va2.getHeight() == 8);
		TEST_ASSERT(va2.getWidth() == 1);
		TEST_ASSERT(va2.getType() == PYXValue::knBool);
		TEST_ASSERT(va2.getHeapBytes() == 2);
		for (int nElement = 0; nElement < 8; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va2.getValue(nElement) == PYXValue((nElement & 2) != 0));
			TEST_ASSERT(va2.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 7)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

	} // END OF TEST 3

	{ // TEST 4: type char
		std::string tempStr;
		// allocate a nullable PYXValueColumn of 10 char's and verify basic characteristics
		PYXValueColumn va(PYXValue::knChar, 10);
		TEST_ASSERT(va.getHeight() == 10);
		TEST_ASSERT(va.getWidth() == 1);
		TEST_ASSERT(va.getType() == PYXValue::knChar);

		// heap size should be 10 bytes for data plus 2 bytes for the bit vector
		int nHeapBytes = va.getHeapBytes();
		TEST_ASSERT(nHeapBytes == 12);

		// all 10 values should initially be uninitialized-null
		for (int nElement = 0; nElement < 10; ++nElement)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement, &bInitialized).isNull());
			TEST_ASSERT(bInitialized == false);
		}

		// set and verify even-indexed elements, and explicitly set last one to null
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			va.setValue(nElement, PYXValue((char)('A' + nElement)));
		}
		va.setValue(9, PYXValue());
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement) == PYXValue((char)('A' + nElement)));
			TEST_ASSERT(va.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

		// uncomment to print this column to trace output
		//TRACE_INFO("PYXValueColumn diagnostic printout" << std::endl << va);

		// serialize/deserialize and verify that it comes back the same		
		std::ostringstream out;
		va.serialize(out);
		PYXValueColumn va2;
		std::istringstream in(out.str());	
		va2.deserialize(in);
		TEST_ASSERT(va2.getHeight() == 10);
		TEST_ASSERT(va2.getWidth() == 1);
		TEST_ASSERT(va2.getType() == PYXValue::knChar);
		TEST_ASSERT(va2.getHeapBytes() == 12);
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va2.getValue(nElement) == PYXValue((char)('A' + nElement)));
			TEST_ASSERT(va2.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

	} // END OF TEST 4

	{ // TEST 5: type int8_t
		std::string tempStr;
		// allocate a nullable PYXValueColumn of 10 int8's and verify basic characteristics
		PYXValueColumn va(PYXValue::knInt8, 10);
		TEST_ASSERT(va.getHeight() == 10);
		TEST_ASSERT(va.getWidth() == 1);
		TEST_ASSERT(va.getType() == PYXValue::knInt8);

		// heap size should be 10 bytes for data plus 2 bytes for the bit vector
		int nHeapBytes = va.getHeapBytes();
		TEST_ASSERT(nHeapBytes == 12);

		// all 10 values should initially be uninitialized-null
		for (int nElement = 0; nElement < 10; ++nElement)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement, &bInitialized).isNull());
			TEST_ASSERT(bInitialized == false);
		}

		// set and verify even-indexed elements, and explicitly set last one to null
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			va.setValue(nElement, PYXValue((int8_t)nElement));
		}
		va.setValue(9, PYXValue());
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement) == PYXValue((int8_t)nElement));
			TEST_ASSERT(va.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

		// uncomment to print this column to trace output
		//TRACE_INFO("PYXValueColumn diagnostic printout" << std::endl << va);

		// serialize/deserialize and verify that it comes back the same
		std::ostringstream out;
		va.serialize(out);
		PYXValueColumn va2;
		std::istringstream in(out.str());	
		va2.deserialize(in);
		TEST_ASSERT(va2.getHeight() == 10);
		TEST_ASSERT(va2.getWidth() == 1);
		TEST_ASSERT(va2.getType() == PYXValue::knInt8);
		TEST_ASSERT(va2.getHeapBytes() == 12);
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va2.getValue(nElement) == PYXValue((int8_t)nElement));
			TEST_ASSERT(va2.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

	} // END OF TEST 5

	{ // TEST 6: type uint8_t
		std::string tempStr;
		// allocate a nullable PYXValueColumn of 10 uint8's and verify basic characteristics
		PYXValueColumn va(PYXValue::knUInt8, 10);
		TEST_ASSERT(va.getHeight() == 10);
		TEST_ASSERT(va.getWidth() == 1);
		TEST_ASSERT(va.getType() == PYXValue::knUInt8);

		// heap size should be 10 bytes for data plus 2 bytes for the bit vector
		int nHeapBytes = va.getHeapBytes();
		TEST_ASSERT(nHeapBytes == 12);

		// all 10 values should initially be uninitialized-null
		for (int nElement = 0; nElement < 10; ++nElement)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement, &bInitialized).isNull());
			TEST_ASSERT(bInitialized == false);
		}

		// set and verify even-indexed elements, and explicitly set last one to null
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			va.setValue(nElement, PYXValue((uint8_t)nElement));
		}
		va.setValue(9, PYXValue());
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement) == PYXValue((uint8_t)nElement));
			TEST_ASSERT(va.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

		// uncomment to print this column to trace output
		//TRACE_INFO("PYXValueColumn diagnostic printout" << std::endl << va);

		// serialize/deserialize and verify that it comes back the same
		std::ostringstream out;
		va.serialize(out);
		PYXValueColumn va2;
		std::istringstream in(out.str());	
		va2.deserialize(in);
		TEST_ASSERT(va2.getHeight() == 10);
		TEST_ASSERT(va2.getWidth() == 1);
		TEST_ASSERT(va2.getType() == PYXValue::knUInt8);
		TEST_ASSERT(va2.getHeapBytes() == 12);
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va2.getValue(nElement) == PYXValue((uint8_t)nElement));
			TEST_ASSERT(va2.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

	} // END OF TEST 6

	{ // TEST 7: type uint16_t
		std::string tempStr;
		// allocate a nullable PYXValueColumn of 10 uint16's and verify basic characteristics
		PYXValueColumn va(PYXValue::knUInt16, 10);
		TEST_ASSERT(va.getHeight() == 10);
		TEST_ASSERT(va.getWidth() == 1);
		TEST_ASSERT(va.getType() == PYXValue::knUInt16);

		// heap size should be 10x2=20 bytes for data plus 2 bytes for the bit vector
		int nHeapBytes = va.getHeapBytes();
		TEST_ASSERT(nHeapBytes == 22);

		// all 10 values should initially be uninitialized-null
		for (int nElement = 0; nElement < 10; ++nElement)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement, &bInitialized).isNull());
			TEST_ASSERT(bInitialized == false);
		}

		// set and verify even-indexed elements, and explicitly set last one to null
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			va.setValue(nElement, PYXValue((uint16_t)nElement));
		}
		va.setValue(9, PYXValue());
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement) == PYXValue((uint16_t)nElement));
			TEST_ASSERT(va.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

		// uncomment to print this column to trace output
		//TRACE_INFO("PYXValueColumn diagnostic printout" << std::endl << va);

		// serialize/deserialize and verify that it comes back the same
		std::ostringstream out;
		va.serialize(out);
		PYXValueColumn va2;
		std::istringstream in(out.str());	
		va2.deserialize(in);
		TEST_ASSERT(va2.getHeight() == 10);
		TEST_ASSERT(va2.getWidth() == 1);
		TEST_ASSERT(va2.getType() == PYXValue::knUInt16);
		TEST_ASSERT(va2.getHeapBytes() == 22);
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va2.getValue(nElement) == PYXValue((uint16_t)nElement));
			TEST_ASSERT(va2.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

	} // END OF TEST 7

	{ // TEST 8: type int32_t
		std::string tempStr;
		// allocate a nullable PYXValueColumn of 10 int32's and verify basic characteristics
		PYXValueColumn va(PYXValue::knInt32, 10);
		TEST_ASSERT(va.getHeight() == 10);
		TEST_ASSERT(va.getWidth() == 1);
		TEST_ASSERT(va.getType() == PYXValue::knInt32);

		// heap size should be 10x4=40 bytes for data plus 2 bytes for the bit vector
		int nHeapBytes = va.getHeapBytes();
		TEST_ASSERT(nHeapBytes == 42);

		// all 10 values should initially be uninitialized-null
		for (int nElement = 0; nElement < 10; ++nElement)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement, &bInitialized).isNull());
			TEST_ASSERT(bInitialized == false);
		}

		// set and verify even-indexed elements, and explicitly set last one to null
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			va.setValue(nElement, PYXValue((int32_t)nElement));
		}
		va.setValue(9, PYXValue());
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement) == PYXValue((int32_t)nElement));
			TEST_ASSERT(va.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

		// uncomment to print this column to trace output
		//TRACE_INFO("PYXValueColumn diagnostic printout" << std::endl << va);

		// serialize/deserialize and verify that it comes back the same
		std::ostringstream out;
		va.serialize(out);
		PYXValueColumn va2;
		std::istringstream in(out.str());	
		va2.deserialize(in);
		TEST_ASSERT(va2.getHeight() == 10);
		TEST_ASSERT(va2.getWidth() == 1);
		TEST_ASSERT(va2.getType() == PYXValue::knInt32);
		TEST_ASSERT(va2.getHeapBytes() == 42);
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va2.getValue(nElement) == PYXValue((int32_t)nElement));
			TEST_ASSERT(va2.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

	} // END OF TEST 8

	{ // TEST 9: type uint32_t
		std::string tempStr;
		// uncomment to print this column to trace output
		//TRACE_INFO("PYXValueColumn diagnostic printout" << std::endl << va);

		// allocate a nullable PYXValueColumn of 10 uint32's and verify basic characteristics
		PYXValueColumn va(PYXValue::knUInt32, 10);
		TEST_ASSERT(va.getHeight() == 10);
		TEST_ASSERT(va.getWidth() == 1);
		TEST_ASSERT(va.getType() == PYXValue::knUInt32);

		// heap size should be 10x4=40 bytes for data plus 2 bytes for the bit vector
		int nHeapBytes = va.getHeapBytes();
		TEST_ASSERT(nHeapBytes == 42);

		// all 10 values should initially be uninitialized-null
		for (int nElement = 0; nElement < 10; ++nElement)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement, &bInitialized).isNull());
			TEST_ASSERT(bInitialized == false);
		}

		// set and verify even-indexed elements, and explicitly set last one to null
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			va.setValue(nElement, PYXValue((uint32_t)nElement));
		}
		va.setValue(9, PYXValue());
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement) == PYXValue((uint32_t)nElement));
			TEST_ASSERT(va.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

		// serialize/deserialize and verify that it comes back the same
		std::ostringstream out;
		va.serialize(out);
		PYXValueColumn va2;
		std::istringstream in(out.str());	
		va2.deserialize(in);
		TEST_ASSERT(va2.getHeight() == 10);
		TEST_ASSERT(va2.getWidth() == 1);
		TEST_ASSERT(va2.getType() == PYXValue::knUInt32);
		TEST_ASSERT(va2.getHeapBytes() == 42);
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va2.getValue(nElement) == PYXValue((uint32_t)nElement));
			TEST_ASSERT(va2.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

	} // END OF TEST 9

	{ // TEST 10: type float
		std::string tempStr;
		// allocate a nullable PYXValueColumn of 10 floats and verify basic characteristics
		PYXValueColumn va(PYXValue::knFloat, 10);
		TEST_ASSERT(va.getHeight() == 10);
		TEST_ASSERT(va.getWidth() == 1);
		TEST_ASSERT(va.getType() == PYXValue::knFloat);

		// heap size should be 10x4=40 bytes for data plus 2 bytes for the bit vector
		int nHeapBytes = va.getHeapBytes();
		TEST_ASSERT(nHeapBytes == 42);

		// all 10 values should initially be uninitialized-null
		for (int nElement = 0; nElement < 10; ++nElement)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement, &bInitialized).isNull());
			TEST_ASSERT(bInitialized == false);
		}

		// set and verify even-indexed elements, and explicitly set last one to null
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			va.setValue(nElement, PYXValue((float)nElement));
		}
		va.setValue(9, PYXValue());
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement) == PYXValue((float)nElement));
			TEST_ASSERT(va.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

		// uncomment to print this column to trace output
		//TRACE_INFO("PYXValueColumn diagnostic printout" << std::endl << va);

		// serialize/deserialize and verify that it comes back the same
		std::ostringstream out;
		va.serialize(out);
		PYXValueColumn va2;
		std::istringstream in(out.str());	
		va2.deserialize(in);
		TEST_ASSERT(va2.getHeight() == 10);
		TEST_ASSERT(va2.getWidth() == 1);
		TEST_ASSERT(va2.getType() == PYXValue::knFloat);
		TEST_ASSERT(va2.getHeapBytes() == 42);
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va2.getValue(nElement) == PYXValue((float)nElement));
			TEST_ASSERT(va2.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

	} // END OF TEST 10

	{ // TEST 11: type double
		std::string tempStr;
		// allocate a nullable PYXValueColumn of 10 doubles and verify basic characteristics
		PYXValueColumn va(PYXValue::knDouble, 10);
		TEST_ASSERT(va.getHeight() == 10);
		TEST_ASSERT(va.getWidth() == 1);
		TEST_ASSERT(va.getType() == PYXValue::knDouble);

		// heap size should be 10x8=80 bytes for data plus 2 bytes for the bit vector
		int nHeapBytes = va.getHeapBytes();
		TEST_ASSERT(nHeapBytes == 82);

		// all 10 values should initially be uninitialized-null
		for (int nElement = 0; nElement < 10; ++nElement)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement, &bInitialized).isNull());
			TEST_ASSERT(bInitialized == false);
		}

		// set and verify even-indexed elements, and explicitly set last one to null
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			va.setValue(nElement, PYXValue((double)nElement));
		}
		va.setValue(9, PYXValue());
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement) == PYXValue((double)nElement));
			TEST_ASSERT(va.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

		// uncomment to print this column to trace output
		//TRACE_INFO("PYXValueColumn diagnostic printout" << std::endl << va);

		// serialize/deserialize and verify that it comes back the same
		std::ostringstream out;
		va.serialize(out);
		PYXValueColumn va2;
		std::istringstream in(out.str());	
		va2.deserialize(in);
		TEST_ASSERT(va2.getHeight() == 10);
		TEST_ASSERT(va2.getWidth() == 1);
		TEST_ASSERT(va2.getType() == PYXValue::knDouble);
		TEST_ASSERT(va2.getHeapBytes() == 82);
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va2.getValue(nElement) == PYXValue((double)nElement));
			TEST_ASSERT(va2.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

	} // END OF TEST 11

	{ // TEST 12: type string
		std::string tempStr;
		// allocate a nullable PYXValueColumn of 10 strings and verify basic characteristics
		PYXValueColumn va(PYXValue::knString, 10);
		TEST_ASSERT(va.getHeight() == 10);
		TEST_ASSERT(va.getWidth() == 1);
		TEST_ASSERT(va.getType() == PYXValue::knString);

		// initial heap size should be 10*sizeof(std::string*) for the empty strings
		// plus 2 bytes for the bit vector
		int nHeapBytes = va.getHeapBytes();
		TEST_ASSERT(nHeapBytes == (10 * sizeof(std::string*) + 2));

		// all 10 values should initially be uninitialized-null
		for (int nElement = 0; nElement < 10; ++nElement)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement, &bInitialized).isNull());
			TEST_ASSERT(bInitialized == false);
		}

		// set and verify even-indexed elements, and explicitly set last one to null
		std::string poem[5];
		poem[0] = "T'was Brillig, and the slithy toves";
		poem[1] = "Did gyre and gimble in the wabe";
		poem[2] = "All mimsy were the borogroves";
		poem[3] = "And the mome raths outgrabe";
		poem[4] = "- Lewis Carroll, 1871";
		for (int nElement = 0; nElement < 9; nElement += 2)
		{
			va.setValue(nElement, PYXValue(poem[nElement/2]));
		}
		va.setValue(9, PYXValue());
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement) == PYXValue(poem[nElement/2]));
			TEST_ASSERT(va.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

		// heap size should have increased by length of all our strings
		int nStringLength = 0;
		for (int nElement = 0; nElement < 5; ++nElement)
		{
			nStringLength += static_cast<int>(poem[nElement].size());
		}
		int nHeapBytes2 = va.getHeapBytes();

		// uncomment to print this column to trace output
		//TRACE_INFO("PYXValueColumn diagnostic printout" << std::endl << va);

		// serialize/deserialize and verify that it comes back the same
		std::ostringstream out;
		va.serialize(out);
		PYXValueColumn va2;
		std::istringstream in(out.str());	
		va2.deserialize(in);
		TEST_ASSERT(va2.getHeight() == 10);
		TEST_ASSERT(va2.getWidth() == 1);
		TEST_ASSERT(va2.getType() == PYXValue::knString);
		int nh1 = va.getHeapBytes();
		int nh2 = va2.getHeapBytes();
		TEST_ASSERT(va2.getHeapBytes() == va.getHeapBytes());
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement) == PYXValue(poem[nElement/2]));
			TEST_ASSERT(va.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

	} // END OF TEST 12

	{ // TEST 13: type float with element count greater than 1
		std::string tempStr;
		// allocate a nullable PYXValueColumn of 10 float[3]'s and verify basic characteristics
		PYXValueColumn va(PYXValue::knFloat, 10, 3);
		TEST_ASSERT(va.getHeight() == 10);
		TEST_ASSERT(va.getWidth() == 3);
		TEST_ASSERT(va.getType() == PYXValue::knFloat);

		// heap size should be 10x3x4=120 bytes for data plus 2 bytes for the bit vector
		int nHeapBytes = va.getHeapBytes();
		TEST_ASSERT(nHeapBytes == 122);

		// all 10 values should initially be uninitialized-null
		for (int nElement = 0; nElement < 10; ++nElement)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement, &bInitialized).isNull());
			TEST_ASSERT(bInitialized == false);
		}

		// set and verify even-indexed elements, and explicitly set last one to null
		float rgb[3];
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			rgb[0] = (float)(nElement*10.0);
			rgb[1] = (float)(nElement*20.0);
			rgb[2] = (float)(nElement*50.0);
			va.setValue(nElement, PYXValue(rgb,3));
		}
		va.setValue(9, PYXValue());
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			rgb[0] = (float)(nElement*10.0);
			rgb[1] = (float)(nElement*20.0);
			rgb[2] = (float)(nElement*50.0);
			TEST_ASSERT(va.getValue(nElement) == PYXValue(rgb,3));
			TEST_ASSERT(va.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

		// uncomment to print this column to trace output
		//TRACE_INFO("PYXValueColumn diagnostic printout" << std::endl << va);

		// serialize/deserialize and verify that it comes back the same
		std::ostringstream out;
		va.serialize(out);
		PYXValueColumn va2;
		std::istringstream in(out.str());
		va2.deserialize(in);
		TEST_ASSERT(va2.getHeight() == 10);
		TEST_ASSERT(va2.getWidth() == 3);
		TEST_ASSERT(va2.getType() == PYXValue::knFloat);
		TEST_ASSERT(va2.getHeapBytes() == 122);
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			rgb[0] = (float)(nElement*10.0);
			rgb[1] = (float)(nElement*20.0);
			rgb[2] = (float)(nElement*50.0);
			TEST_ASSERT(va2.getValue(nElement) == PYXValue(rgb,3));
			TEST_ASSERT(va2.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}

	} // END OF TEST 13

	{ // TEST 14: type string with element count greater than 1
		std::string tempStr;
		// allocate a nullable PYXValueColumn of 10 string[3]'s and verify basic characteristics
		PYXValueColumn va(PYXValue::knString, 10, 3);
		TEST_ASSERT(va.getHeight() == 10);
		TEST_ASSERT(va.getWidth() == 3);
		TEST_ASSERT(va.getType() == PYXValue::knString);

		// initial heap size should be 10*3*sizeof(std::string*) for the empty strings
		// plus 2 bytes for the bit vector
		int nHeapBytes = va.getHeapBytes();
		TEST_ASSERT(nHeapBytes == (10 * 3 * sizeof(std::string*) + 2));

		// all 10 values should initially be uninitialized-null
		for (int nElement = 0; nElement < 10; ++nElement)
		{
			bool bInitialized = true;
			TEST_ASSERT(va.getValue(nElement, &bInitialized).isNull());
			TEST_ASSERT(bInitialized == false);
		}
		//TRACE_INFO("initial\n" << va);

		// set and verify even-indexed elements (and count up the lengths of all
		// the strings as we add them); explicitly set last entry to null
		std::string stringArray[3];
		std::string poem[5];
		poem[0] = "T'was Brillig, and the slithy toves";
		poem[1] = "Did gyre and gimble in the wabe";
		poem[2] = "All mimsy were the borogroves";
		poem[3] = "And the mome raths outgrabe";
		poem[4] = "- Lewis Carroll, 1871";
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			stringArray[0] = poem[nElement/2];
			nHeapBytes += sizeof(std::string) + static_cast<int>(stringArray[0].size());
			stringArray[1] = poem[(nElement/2 + 1)%5];
			nHeapBytes += sizeof(std::string) + static_cast<int>(stringArray[1].size());
			stringArray[2] = poem[(nElement/2 + 2)%5];
			nHeapBytes += sizeof(std::string) + static_cast<int>(stringArray[2].size());
			va.setValue(nElement, PYXValue(stringArray, 3));
		}
		va.setValue(9, PYXValue());
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			stringArray[0] = poem[nElement/2];
			stringArray[1] = poem[(nElement/2 + 1)%5];
			stringArray[2] = poem[(nElement/2 + 2)%5];
			TEST_ASSERT(va.getValue(nElement) == PYXValue(stringArray, 3));
			TEST_ASSERT(va.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}
		//TRACE_INFO("before\n" << va);

		// heap size should have increased by length of all our strings
		TEST_ASSERT(va.getHeapBytes() == nHeapBytes);

		// uncomment to print this column to trace output
		//TRACE_INFO("PYXValueColumn diagnostic printout" << std::endl << va);

		// serialize/deserialize and verify that it comes back the same
		std::ostringstream out;
		va.serialize(out);
		PYXValueColumn va2;
		std::istringstream in(out.str());
		va2.deserialize(in);
		TEST_ASSERT(va2.getHeight() == 10);
		TEST_ASSERT(va2.getWidth() == 3);
		TEST_ASSERT(va2.getType() == PYXValue::knString);
		TEST_ASSERT(va2.getHeapBytes() == va.getHeapBytes());
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			bool bInitialized = true;
			stringArray[0] = poem[nElement/2];
			stringArray[1] = poem[(nElement/2 + 1)%5];
			stringArray[2] = poem[(nElement/2 + 2)%5];
			TEST_ASSERT(va2.getValue(nElement) == PYXValue(stringArray, 3));
			TEST_ASSERT(va2.getValue(nElement + 1, &bInitialized).isNull());
			if (nElement == 9)
			{
				TEST_ASSERT(bInitialized == true);
			}
			else if (nElement%2 == 1)
			{
				TEST_ASSERT(bInitialized == false);
			}
		}
		//TRACE_INFO("after\n" << va);

		TRACE_TEST("va is " << &va);
		TRACE_TEST("va2 is " << &va2);
	} // END OF TEST 14

	{ // TEST 15: as previous, but non-nullable array
		std::string tempStr;
		// allocate a non-nullable PYXValueColumn of 10 string[3]'s and verify basic characteristics
		PYXValueColumn va(PYXValue::knString, 10, 3, false);
		TEST_ASSERT(va.getHeight() == 10);
		TEST_ASSERT(va.getWidth() == 3);
		TEST_ASSERT(va.getType() == PYXValue::knString);

		// initial heap size should be 10*3*sizeof(std::string*) for the empty strings
		int nHeapBytes = va.getHeapBytes();
		TEST_ASSERT(nHeapBytes == (10 * 3 * sizeof(std::string*)));

		// all 10 values should initially be the null string
		PYXValue null;
		std::string nullVal = null.getString();
		std::string stringArray[3];
		stringArray[0] = nullVal;
		stringArray[1] = nullVal;
		stringArray[2] = nullVal;
		for (int nElement = 0; nElement < 10; ++nElement)
		{
			TEST_ASSERT(va.getValue(nElement) == PYXValue(stringArray, 3));
		}

		// set and verify even-indexed elements (and count up the lengths of all
		// the strings as we add them)
		std::string poem[5];
		poem[0] = "T'was Brillig, and the slithy toves";
		poem[1] = "Did gyre and gimble in the wabe";
		poem[2] = "All mimsy were the borogroves";
		poem[3] = "And the mome raths outgrabe";
		poem[4] = "- Lewis Carroll, 1871";
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			stringArray[0] = poem[nElement/2];
			nHeapBytes += sizeof(std::string) + static_cast<int>(stringArray[0].size());
			stringArray[1] = poem[(nElement/2 + 1)%5];
			nHeapBytes += sizeof(std::string) + static_cast<int>(stringArray[1].size());
			stringArray[2] = poem[(nElement/2 + 2)%5];
			nHeapBytes += sizeof(std::string) + static_cast<int>(stringArray[2].size());
			va.setValue(nElement, PYXValue(stringArray, 3));
		}
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			stringArray[0] = poem[nElement/2];
			stringArray[1] = poem[(nElement/2 + 1)%5];
			stringArray[2] = poem[(nElement/2 + 2)%5];
			TEST_ASSERT(va.getValue(nElement) == PYXValue(stringArray, 3));
			stringArray[0] = nullVal;
			stringArray[1] = nullVal;
			stringArray[2] = nullVal;
			TEST_ASSERT(va.getValue(nElement + 1) == PYXValue(stringArray, 3));
		}

		// heap size should have increased by length of all our strings
		TEST_ASSERT(va.getHeapBytes() == nHeapBytes);

		// uncomment to print this column to trace output
		//TRACE_INFO("PYXValueColumn diagnostic printout" << std::endl << va);

		// serialize/deserialize and verify that it comes back the same
		std::ostringstream out;
		va.serialize(out);
		PYXValueColumn va2;
		std::istringstream in(out.str());
		va2.deserialize(in);
		TEST_ASSERT(va2.getHeight() == 10);
		TEST_ASSERT(va2.getWidth() == 3);
		TEST_ASSERT(va2.getType() == PYXValue::knString);
		int hb = va.getHeapBytes();
		int hb2 = va2.getHeapBytes();
		TEST_ASSERT(va2.getHeapBytes() == va.getHeapBytes());
		for (int nElement = 0; nElement < 10; nElement += 2)
		{
			stringArray[0] = poem[nElement/2];
			stringArray[1] = poem[(nElement/2 + 1)%5];
			stringArray[2] = poem[(nElement/2 + 2)%5];
			TEST_ASSERT(va2.getValue(nElement) == PYXValue(stringArray, 3));
			stringArray[0] = nullVal;
			stringArray[1] = nullVal;
			stringArray[2] = nullVal;
			TEST_ASSERT(va2.getValue(nElement + 1) == PYXValue(stringArray, 3));
		}

	} // END OF TEST 15
}

/*!
Helper: set the indexed bit in a packed bit array.  For convenience, the array
pointer argument may be null, in which case the function does nothing.
*/
static void setBit(char* ptr, const int nIndex, bool bit)
{
	if (ptr != 0)
	{
		ptr += nIndex / 8;
		char mask = (1 << (nIndex % 8));
		if (bit)
		{
			// set a bit to 1
			*ptr |= mask;
		}
		else
		{
			// clear a bit to 0
			*ptr &= ~mask;
		}
	}
}

/*!
Helper: get the indexed bit in a packed bit array
*/
static bool bitSet(char* ptr, const int nIndex)
{
	char byte = ptr[nIndex / 8];
	return ((byte & (1 << (nIndex % 8))) != 0);
}

/*!
Helper: return true only if all bytes in a block are 0xFF.
*/
static bool allOnes(	const void* startAddress,
						const int nCount	)
{
	const uint8_t* ptr = (uint8_t*)startAddress;
	for (int nByte=0; nByte < nCount; nByte++)
	{
		if (ptr[nByte] != 0xFF)
		{
			return false;
		}
	}
	return true;
}

/*!
Helper: compute size (in bytes) of data slot and entire data block (not including extra space
required for not-null bit vector), given column specifications
as follows:

\param	type			Data type.
\param	nColumnHeight	Used to set m_nColumnHeight; number of array elements.
\param	nColumnWidth	Used to set m_nColumnWidth; number of values within each element.
\param	pnSlotBytes		Address of variable to receive Bytes per slot
\param	pnBlockBytes		Address of variable to receive Bytes required for data block
*/
void PYXValueColumn::computeBasicDataBlockSize(
	const PYXValue::eType type,
	const int nColumnHeight,
	const int nColumnWidth,
	int* pnSlotBytes,
	int* pnBlockBytes
	)
{
	switch (type)
	{
	case PYXValue::knBool:
		*pnSlotBytes = 0;
		*pnBlockBytes = nColumnWidth * ((nColumnHeight + 7) / 8);
		break;
	case PYXValue::knChar:
		*pnSlotBytes = sizeof(char) * nColumnWidth;
		*pnBlockBytes = *pnSlotBytes * nColumnHeight;
		break;
	case PYXValue::knInt8:
		*pnSlotBytes = sizeof(int8_t) * nColumnWidth;
		*pnBlockBytes = *pnSlotBytes * nColumnHeight;
		break;
	case PYXValue::knUInt8:
		*pnSlotBytes = sizeof(uint8_t) * nColumnWidth;
		*pnBlockBytes = *pnSlotBytes * nColumnHeight;
		break;
	case PYXValue::knInt16:
		*pnSlotBytes = sizeof(int16_t) * nColumnWidth;
		*pnBlockBytes = *pnSlotBytes * nColumnHeight;
		break;
	case PYXValue::knUInt16:
		*pnSlotBytes = sizeof(uint16_t) * nColumnWidth;
		*pnBlockBytes = *pnSlotBytes * nColumnHeight;
		break;
	case PYXValue::knInt32:
		*pnSlotBytes = sizeof(int32_t) * nColumnWidth;
		*pnBlockBytes = *pnSlotBytes * nColumnHeight;
		break;
	case PYXValue::knUInt32:
		*pnSlotBytes = sizeof(uint32_t) * nColumnWidth;
		*pnBlockBytes = *pnSlotBytes * nColumnHeight;
		break;
	case PYXValue::knFloat:
		*pnSlotBytes = sizeof(float) * nColumnWidth;
		*pnBlockBytes = *pnSlotBytes * nColumnHeight;
		break;
	case PYXValue::knDouble:
		*pnSlotBytes = sizeof(double) * nColumnWidth;
		*pnBlockBytes = *pnSlotBytes * nColumnHeight;
		break;
	case PYXValue::knString:
		*pnSlotBytes = sizeof(std::string*) * nColumnWidth;
		*pnBlockBytes = *pnSlotBytes * nColumnHeight;
		break;
	default:
		throw PYXValueColumnException("Invalid type");
	}
}

/*!
Helper: Because both the standard constructor and the deserialize() method need to
allocate a new data block, the common code is extracted here.

\param	type			Data type.
\param	nColumnHeight	Used to set m_nColumnHeight; number of array elements.
\param	nColumnWidth	Used to set m_nColumnWidth; number of values within each element.
\param	bNullable		True if data slots can be null; false otherwise.

\sa PYXValueColumn::PYXValueColumn()
\sa PYXValueColumn::deserialize()
*/
void PYXValueColumn::allocateDataBlock(	const PYXValue::eType type,
										const int nColumnHeight,
										const int nColumnWidth,
										const bool bNullable	)
{
	m_type = type;
	m_nColumnHeight = nColumnHeight;
	m_nColumnWidth = nColumnWidth;

	computeBasicDataBlockSize(type, nColumnHeight, nColumnWidth, &m_nSlotBytes, &m_nBlockBytes);

	// allocate data block
	if (bNullable)
	{
		// we allocate our "not-null" bit vector at the end of the block,
		// so it won't affect byte alignment of the main part of the block
		m_pValues = new char[m_nBlockBytes + (m_nColumnHeight + 7) / 8];
		m_pNotNull = m_pValues + m_nBlockBytes;
		m_nBlockBytes += (m_nColumnHeight + 7)/8;
	}
	else
	{
		// no "not-null" vector
		m_pValues = new char[m_nBlockBytes];
		m_pNotNull = 0;
	}
	// initialize whole data block to all zeroes ("not-null" vector thus
	// becomes all false, i.e., all data values become null
	memset(m_pValues, 0, m_nBlockBytes);

	// initially, this is how much heap space we're using
	m_nHeapBytes = m_nBlockBytes;

	consumeMemory(m_nBlockBytes);
}

/*!
Estimate heap bytes required by call to standard constructor.

\param	type			Data type.
\param	nColumnHeight	Used to set m_nColumnHeight; number of array elements.
\param	nColumnWidth	Used to set m_nColumnWidth; number of values within each element.
\param	bNullable		True if data slots can be null; false otherwise.

\return		Estimate of heap space required in bytes.
*/
const int PYXValueColumn::estimateHeapBytes(
	const PYXValue::eType type,
	const int nColumnHeight,
	const int nColumnWidth,
	const bool bNullable	)
{
	int nSlotBytes, nBlockBytes;
	computeBasicDataBlockSize(type, nColumnHeight, nColumnWidth, &nSlotBytes, &nBlockBytes);
	int nBytes = nBlockBytes + sizeof(PYXValueColumn);

	if (type == PYXValue::knString)
	{
		// make a wild guess that average string size is 50
		int nStringBytes = sizeof(std::string) + knStringSizeGuess;
		nBytes += nStringBytes * nColumnHeight;
	}

	if (bNullable)
	{
		nBytes += (nColumnHeight + 7)/8;
	}

	return nBytes;
}

/*!
Default constructor creates an "empty" PYXValueColumn.  This will normally
only be used before a call to deserialize().
*/
PYXValueColumn::PYXValueColumn() :
	m_type(PYXValue::knNull),
	m_nSlotBytes(0),
	m_nColumnHeight(0),
	m_nColumnWidth(0),
	m_nBlockBytes(0),
	m_nHeapBytes(0),
	m_pValues(0),
	m_pNotNull(0)
{
}

/*!
Standard constructor allocates space for data.

\param type				base type of the array elements
\param nColumnHeight	number of elements in this array
\param nColumnWidth		number of base-type entries in each element
\param bNullable		true if elements can be null
*/
PYXValueColumn::PYXValueColumn(	const PYXValue::eType type,
								const int nColumnHeight,
								const int nColumnWidth,
								const bool bNullable	) :
	m_type(type),
	m_nColumnHeight(nColumnHeight),
	m_nColumnWidth(nColumnWidth)
{
	assert(nColumnHeight > 0);
	assert(nColumnWidth > 0);

	allocateDataBlock(type, nColumnHeight, nColumnWidth, bNullable);
}

/*!
Copy constructor.
*/
PYXValueColumn::PYXValueColumn(const PYXValueColumn& orig)
{
	m_type = orig.m_type;
	m_nSlotBytes = orig.m_nSlotBytes;
	m_nColumnHeight = orig.m_nColumnHeight;
	m_nColumnWidth = orig.m_nColumnWidth;
	m_nBlockBytes = orig.m_nBlockBytes;
	m_nHeapBytes = orig.m_nHeapBytes;

	m_pValues = new char[m_nBlockBytes];
	consumeMemory(m_nBlockBytes);

	int nNotNullTableSize = (m_nColumnHeight + 7)/8;
	if (orig.m_pNotNull != 0)
	{
		m_pNotNull = m_pValues + m_nBlockBytes - nNotNullTableSize;
	}
	else
	{
		m_pNotNull = 0;
	}

	if (m_type == PYXValue::knString)
	{
		// data are pointers to std::string: clone all the strings
		for (int nElement = 0; nElement < m_nColumnWidth * m_nColumnHeight; ++nElement)
		{
			std::string* pstr = reinterpret_cast<std::string**>(orig.m_pValues)[nElement];
			if (allOnes(&pstr,sizeof(std::string*)) || pstr == 0)
			{
				// copy null/flag string pointer in source
				reinterpret_cast<std::string**>(m_pValues)[nElement] = pstr;
			}
			else
			{
				// clone source string
				reinterpret_cast<std::string**>(m_pValues)[nElement] = new std::string(*pstr);
			}
		}

		// Copy the m_pNotNull part of the data.
		if (orig.m_pNotNull != 0)
		{
			memcpy(m_pNotNull, orig.m_pNotNull, nNotNullTableSize);
		}
	}
	else
	{
		// non-pointer types: just copy all the bits
		memcpy(m_pValues, orig.m_pValues, m_nBlockBytes);
	}
}

/*!
Destructor.
*/
PYXValueColumn::~PYXValueColumn()
{
	if (m_pValues != 0)
	{
		if (m_type == PYXValue::knString)
		{
			// if our data type is string, free all the strings before freeing
			// the block of pointers which point to them
			for (int nIndex = 0; nIndex < m_nColumnHeight * m_nColumnWidth; nIndex++)
			{
				std::string* pstr = reinterpret_cast<std::string**>(m_pValues)[nIndex];
				// only free valid pointers (not null, not all-1's flag value)
				if (!allOnes(&pstr,sizeof(std::string*)) && (pstr != 0))
				{
					delete pstr;
				}
			}
		}
		delete[] m_pValues;
		releaseMemory(m_nBlockBytes);
	}
}

/*!
Get value at given index.  If the returned value is null (type PYXValue::knNull)
and pbInitialized is nonzero, the bool variable pbInitialized gets set to true if the
indexed data slot was explicitly set to the null value, or false if getValue() is
returning null simply because the indexed data slot has not yet been initialized.

\param	nIndex			Index of data slot to query
\param	pbInitialized	Optional flag pointer (may be 0).
*/
PYXValue PYXValueColumn::getValue(const int nIndex, bool* pbInitialized) const
{
	assert((nIndex >= 0) && (nIndex < m_nColumnHeight));

	// get pointer to data element, or start of block if type is bool
	// (m_nSlotBytes will be 0)
	char* ptr = m_pValues + m_nSlotBytes * nIndex;
	assert(ptr != 0);

	if (m_pNotNull != 0)
	{
		if (!bitSet(m_pNotNull, nIndex))
		{
			// the value sought is null
			if (pbInitialized != 0)
			{
				if (m_type == PYXValue::knBool)
				{
					// ptr points to a bit vector: uninitialized values are 0,
					// explicitly setting the value to null sets value bit to 1
					*pbInitialized = bitSet(ptr, nIndex * m_nColumnWidth);
				}
				else
				{
					// ptr points to some block of bytes representing the value;
					// uninitialized bytes are always 0, explicitly setting the value to
					// null sets the first byte in the block to some nonzero value
					*pbInitialized = (*ptr != 0);
				}
			}

			return PYXValue();
		}
	}

	// we're about to return an actual value, which was indeed initialized
	if (pbInitialized)
	{
		*pbInitialized = true;
	}

	if (m_nColumnWidth == 1)	// simple types
	{
		switch (m_type)
		{
		case PYXValue::knBool:
			return PYXValue(bitSet(ptr, nIndex));

		case PYXValue::knChar:
			return PYXValue(*ptr);

		case PYXValue::knInt8:
			return PYXValue(*(reinterpret_cast<int8_t*>(ptr)));

		case PYXValue::knUInt8:
			return PYXValue(*(reinterpret_cast<uint8_t*>(ptr)));

		case PYXValue::knInt16:
			return PYXValue(*(reinterpret_cast<int16_t*>(ptr)));

		case PYXValue::knUInt16:
			return PYXValue(*(reinterpret_cast<uint16_t*>(ptr)));

		case PYXValue::knInt32:
			return PYXValue(*(reinterpret_cast<int32_t*>(ptr)));

		case PYXValue::knUInt32:
			return PYXValue(*(reinterpret_cast<uint32_t*>(ptr)));

		case PYXValue::knFloat:
			return PYXValue(*(reinterpret_cast<float*>(ptr)));

		case PYXValue::knDouble:
			return PYXValue(*(reinterpret_cast<double*>(ptr)));

		case PYXValue::knString:
			assert(*(reinterpret_cast<std::string**>(ptr)) != 0);
			return PYXValue(**(reinterpret_cast<std::string**>(ptr)));
		}
	}
	else	// array types
	{
		switch (m_type)
		{
		case PYXValue::knBool:
			{
			bool* pVal = new bool[m_nColumnWidth];
			for (int nElement = 0; nElement < m_nColumnWidth; ++nElement)
			{
				pVal[nElement] = bitSet(ptr, nIndex * m_nColumnWidth + nElement);
			}
			PYXValue& val = PYXValue(pVal, m_nColumnWidth);
			delete[] pVal;
			return val;
			}

		case PYXValue::knChar:
			return PYXValue(ptr, m_nColumnWidth);

		case PYXValue::knInt8:
			return PYXValue((reinterpret_cast<int8_t*>(ptr)), m_nColumnWidth);

		case PYXValue::knUInt8:
			return PYXValue((reinterpret_cast<uint8_t*>(ptr)), m_nColumnWidth);

		case PYXValue::knInt16:
			return PYXValue((reinterpret_cast<int16_t*>(ptr)), m_nColumnWidth);

		case PYXValue::knUInt16:
			return PYXValue((reinterpret_cast<uint16_t*>(ptr)), m_nColumnWidth);

		case PYXValue::knInt32:
			return PYXValue((reinterpret_cast<int32_t*>(ptr)), m_nColumnWidth);

		case PYXValue::knUInt32:
			return PYXValue((reinterpret_cast<uint32_t*>(ptr)), m_nColumnWidth);

		case PYXValue::knFloat:
			return PYXValue((reinterpret_cast<float*>(ptr)), m_nColumnWidth);

		case PYXValue::knDouble:
			return PYXValue((reinterpret_cast<double*>(ptr)), m_nColumnWidth);

		case PYXValue::knString:
			{
			// we have an array of std::string*; we need an array of std::string
			std::string* pVal = new std::string[m_nColumnWidth];
			for (int nElement = 0; nElement < m_nColumnWidth; ++nElement)
			{
				std::string* sptr = (reinterpret_cast<std::string**>(ptr))[nElement];
				if (sptr != 0 && !allOnes(sptr,sizeof(sptr)))
				{
					pVal[nElement] = *sptr;
				}
			}
			PYXValue& val = PYXValue(pVal, m_nColumnWidth);
			delete[] pVal;
			return val;
			}
		}
	}

	// if we ever get here, it's because m_type is corrupted
	assert(false);
	return PYXValue();
}

/*!
Get value at given index.

\param	nIndex			Index of data slot to query
\param	pbInitialized	Optional flag pointer (may be 0).

\return true if the data is not null, false if the data should be treated as null.
*/
bool PYXValueColumn::getValue(const int nIndex, PYXValue* pValue) const
{
	assert((nIndex >= 0) && (nIndex < m_nColumnHeight));

	// get pointer to data element, or start of block if type is bool
	// (m_nSlotBytes will be 0)
	char* ptr = m_pValues + m_nSlotBytes * nIndex;
	assert(ptr != 0);

	if (m_pNotNull != 0)
	{
		if (!bitSet(m_pNotNull, nIndex))
		{
			// the value sought is null
			return false;
		}
	}

	if (m_nColumnWidth == 1)	// simple types
	{
		switch (m_type)
		{
		case PYXValue::knBool:
			pValue->setBool(bitSet(ptr, nIndex));
			break;

		case PYXValue::knString:
			assert(*(reinterpret_cast<std::string**>(ptr)) != 0);
			pValue->setString(**(reinterpret_cast<std::string**>(ptr)));
			break;

		default:
			memcpy(pValue->getPtr(0), ptr, m_nSlotBytes);
			break;
		}
	}
	else	// array types
	{
		switch (m_type)
		{
		case PYXValue::knBool:
			for (int nElement = 0; nElement < m_nColumnWidth; ++nElement)
			{
				pValue->setBool(nElement, bitSet(ptr, nIndex * m_nColumnWidth + nElement));
			}
			break;

		case PYXValue::knString:
			{
				for (int nElement = 0; nElement < m_nColumnWidth; ++nElement)
				{
					std::string* sptr = (reinterpret_cast<std::string**>(ptr))[nElement];
					if (sptr != 0 && !allOnes(sptr,sizeof(sptr)))
					{
						pValue->setString(nElement, *sptr);
					}
				}
			}
			break;

		default:
			memcpy(pValue->getPtr(0), ptr, m_nSlotBytes);
			break;
		}
	}

	return true;
}

/*!
Set the value at given index.
*/
void PYXValueColumn::setValue(	const int nIndex,
								const PYXValue& value	)
{
	assert((nIndex >= 0) && (nIndex < m_nColumnHeight));
	assert(
		(m_nColumnWidth == 1 && (value.isNull() || value.getType() == m_type)) ||
		(m_nColumnWidth > 1 && (value.isNull() || (value.isArray() && value.getArrayType() == m_type)))
		);
	assert(!value.isNull() || (m_pNotNull != 0) && "null value passed to non-nullable column");

	// get pointer to data element, or start of block if type is bool
	// (m_nSlotBytes will be 0)
	char* ptr = m_pValues + m_nSlotBytes*nIndex;
	assert(ptr != 0);

	if (m_pNotNull != 0)
	{
		bool valueIsNull = value.isNull();
		setBit(m_pNotNull, nIndex, !valueIsNull);
		if (valueIsNull)
		{
			// we are explicitly setting a data slot to null.  The data bits are unused, so we
			// can set all of them to 1 to indicate that this is an explicit null, as
			// opposed to "data not yet initialized"
			if (m_type == PYXValue::knBool)
			{
				setBit(ptr, nIndex, true);
			}
			else
			{
				memset(ptr,0xFF,m_nSlotBytes);
			}
			return;
		}
	}

	if (m_nColumnWidth == 1)	// simple types
	{
		switch (m_type)
		{
		case PYXValue::knBool:
			setBit(ptr, nIndex, value.getBool());
			break;

		case PYXValue::knString:
			{
			// ptr is a Pointer to a Pointer to a String (pps)
			std::string** pps = reinterpret_cast<std::string**>(ptr);
			if (!allOnes(pps,sizeof(std::string*)) && *pps != 0)
			{
				// de-allocate the old string first
				m_nHeapBytes -= static_cast<int>((*pps)->size()) + sizeof(std::string);
				delete *pps;
			}
			// allocate a new string and clone the given one into it
			*pps = new std::string(value.getString());
			m_nHeapBytes += sizeof(std::string) + static_cast<int>((*pps)->size());
			break;
			}

		default:
			memcpy(ptr, value.getPtr(0), m_nSlotBytes);
			break;
		}
	}
	else	// array types
	{
		assert(value.getType() == PYXValue::knArray);
		assert(value.getArrayType() == m_type);
		assert(value.getArraySize() == m_nColumnWidth);

		switch (m_type)
		{
		case PYXValue::knBool:
			for (int nElement = 0; nElement < m_nColumnWidth; ++nElement)
			{
				setBit(ptr, nIndex * m_nColumnWidth + nElement, value.getBool(nElement));
			}
			break;

		case PYXValue::knString:
			{
				// ptr is a Pointer to a block of Pointers to Strings (pps)
				std::string** pps = (std::string**)ptr;
				for (int nElement = 0; nElement < m_nColumnWidth; ++nElement)
				{
					if (!allOnes(pps+nElement, sizeof(std::string*)) && 
						pps[nElement] != 0)
					{
						// de-allocate the old string first
						m_nHeapBytes -= static_cast<int>(pps[nElement]->size()) + sizeof(std::string);
						delete pps[nElement];
					}
					// allocate a new string and clone the given one into it
					pps[nElement] = new std::string(value.getString(nElement));
					m_nHeapBytes += sizeof(std::string) + static_cast<int>(pps[nElement]->size());
				}
			}
			break;

		default:
			memcpy(ptr, value.getPtr(0), m_nSlotBytes);
			break;
		}
	}
}

/*!
Current I/O format version: increment every time format changes.
*/
static int knIOFormatVersion = 1;

/*!
Serialize to binary stream.
*/
void PYXValueColumn::serialize(std::ostream& out)
{
	// version number as int
	out.WRITE(knIOFormatVersion);

	// type code as int, negated if nullable
	int tc = m_type;
	if (m_pNotNull != 0)
	{
		tc = -tc;
	}
	out.WRITE(tc);

	// number of elements
	out.WRITE(m_nColumnHeight);

	// elements' count
	out.WRITE(m_nColumnWidth);

	// fixed-length types and data-present vector can just be written out as a block
	if (m_type != PYXValue::knString)
	{
		out.write(static_cast<char*>(m_pValues), m_nBlockBytes);
	}
	else
	{
		// strings have to be serialized one at a time in length-prefixed format
		int linearIndex = 0;
		char* ptr = m_pValues;
		for (int nElement = 0; nElement < m_nColumnHeight; ++nElement)
		{
			// ptr points to a block of memory which may be one or more std::string* pointers, or
			// could be our special all-1's bit pattern used to flag explicit null values.
			bool bExplicitNull = allOnes(ptr, m_nSlotBytes);
			for (int nIndex = 0; nIndex < m_nColumnWidth; ++nIndex)
			{
				std::string* sptr = reinterpret_cast<std::string**>(m_pValues)[linearIndex];
				int nLength;		// at least 1 integer "length prefix" will be output
				if (bExplicitNull)
				{
					// explicit null: output dummy length-prefix -1
					nLength = -1;
					out.WRITE(nLength);
				}
				else if (sptr == 0)
				{
					// uninitialized: output dummy length-prefix -2
					nLength = -2;
					out.WRITE(nLength);
				}
				else
				{
					// valid string: output real length prefix followed by data bytes
					nLength = static_cast<int>(sptr->size());
					out.WRITE(nLength);
					for (int nCharIndex = 0; nCharIndex < nLength; ++nCharIndex)
					{
						out.put((*sptr)[nCharIndex]);
					}
				}
				linearIndex++;
			}
			ptr += m_nSlotBytes;
		}
	}
}

/*!
De-serialize from binary stream.
*/
void PYXValueColumn::deserialize(std::istream& in)
{
	// get version number
	int nFormatVersion;
	in.READ(nFormatVersion);

	switch (nFormatVersion)
	{
	case 1:
		// get type code and nullability
		int tc;
		in.READ(tc);
		bool bNullable;
		if (tc < 0)
		{
			tc = -tc;
			bNullable = true;
		}
		else
		{
			bNullable = false;
		}
		if (tc < 0 || tc >= PYXValue::knTypeCount)
		{
			throw PYXValueColumnException("PYXValueColumn: Invalid type code");
		}
		else m_type = static_cast<PYXValue::eType>(tc);

		// get number of elements
		in.READ(m_nColumnHeight);
		if (m_nColumnHeight <= 0)
		{
			throw PYXValueColumnException("PYXValueColumn: Invalid capacity");
		}

		// get elements' count
		in.READ(m_nColumnWidth);
		if (m_nColumnWidth <= 0)
		{
			throw PYXValueColumnException("PYXValueColumn: Invalid count");
		}

		// allocate data block
		allocateDataBlock(m_type, m_nColumnHeight, m_nColumnWidth, bNullable);

		// read data
		if (m_type != PYXValue::knString)
		{
			// fixed-length types can simply be read as one block
			in.read(m_pValues, m_nBlockBytes);
		}
		else
		{
			// read strings individually
			int linearIndex = 0;
			char* ptr = m_pValues;
			for (int nElement = 0; nElement < m_nColumnHeight; ++nElement)
			{
				for (int nIndex = 0; nIndex < m_nColumnWidth; ++nIndex)
				{
					int nLength;
					in.READ(nLength);
					if (nLength == -1)	// explicit null string value
					{
						assert(bNullable);
						memset(ptr, 0xFF, sizeof(std::string*));
						setBit(m_pNotNull, nElement, false);
					}
					else if (nLength == -2)	// uninitialized string slot
					{
						// uninitialized string: set pointer null
						memset(ptr,0,sizeof(std::string*));
						setBit(m_pNotNull, nElement, false);
					}
					else
					{
						// valid string
						setBit(m_pNotNull, nElement, true);
						std::string* sptr = ((std::string**)m_pValues)[linearIndex] = new std::string();
						sptr->resize(nLength);
						for (int nChar = 0; nChar < nLength; ++nChar)
						{
							(*sptr)[nChar] = in.get();
						}
						// add string size to heap bytes
						m_nHeapBytes += sizeof(std::string) + nLength;
					}
					linearIndex++;
					ptr += sizeof(std::string*);
				}
			}
		}

		// all done!
		break;

	default:
		throw PYXValueColumnException("PYXValueColumn: Unknown format version");
	}
}

/*!
Stream output operator: serialize to text stream.
*/
std::ostream& operator << (std::ostream& out, PYXValueColumn& pvv)
{
	out << "PYXValueColumn: ";
	out << "base type ";
	switch (pvv.m_type)
	{
	case PYXValue::knBool:
		out << "bool, ";
		break;
	case PYXValue::knChar:
		out << "char, ";
		break;
	case PYXValue::knInt8:
		out << "int8_t, ";
		break;
	case PYXValue::knUInt8:
		out << "uint8_t, ";
		break;
	case PYXValue::knInt16:
		out << "int16_t, ";
		break;
	case PYXValue::knUInt16:
		out << "uint16_t, ";
		break;
	case PYXValue::knInt32:
		out << "int32_t, ";
		break;
	case PYXValue::knUInt32:
		out << "uint32_t, ";
		break;
	case PYXValue::knFloat:
		out << "float, ";
		break;
	case PYXValue::knDouble:
		out << "double, ";
		break;
	case PYXValue::knString:
		out << "std::string, ";
		break;
	default:
		out << "INVALID! ";
	}
	out << "height: " << pvv.m_nColumnHeight;
	out << ", width: " << pvv.m_nColumnWidth;
	out << std::endl;

	for (int nElement = 0; nElement < pvv.m_nColumnHeight; ++nElement)
	{
		bool bInitialized;
		PYXValue value = pvv.getValue(nElement, &bInitialized);
		if (value.isNull())
		{
			if (bInitialized)
			{
				out << "<Explicit NULL>" << std::endl;
			}
			else
			{
				out << "<Uninitialized NULL>" << std::endl;
			}
		}
		else if (value.isArray())
		{
			out << "( ";
			for (int nIndex = 0; nIndex < pvv.m_nColumnWidth; ++nIndex)
			{
				if (value.getArrayType() == PYXValue::knBool)
				{
					if (value.getBool(nIndex))
					{
						out << "true";
					}
					else
					{
						out << "false";
					}
				}
				else if (value.getArrayType() == PYXValue::knString)
				{
					out << "\"" << value.getString(nIndex) << "\"";
				}
				else
				{
					out << value.getString(nIndex);
				}
				if (nIndex != (pvv.m_nColumnWidth - 1))
				{
					out << ", ";
				}
			}
			out << " )" << std::endl;
		}
		else if (value.isBool())
		{
			if (value.getBool())
			{
				out << "true" << std::endl;
			}
			else
			{
				out << "false" << std::endl;
			}
		}
		else if (value.isString())
		{
			out << "\"" << pvv.getValue(nElement).getString() << "\"" << std::endl;
		}
		else
		{
			out << pvv.getValue(nElement).getString() << std::endl;
		}
	}

	return out;
}

/*!
Stream input operator: de-serialize from text stream.
*/
std::istream& operator >> (std::istream& in, PYXValueColumn& pvv)
{
	assert(false && "not yet implemented");
	return in;
}
