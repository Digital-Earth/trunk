/******************************************************************************
value_math.cpp

begin		: 2006-11-07
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "value_math.h"

// local includes
#include "tester.h"

// standard includes

//! The unit test class
Tester<PYXValueMath> gTester;

/*!
The unit test method for the class.
*/
void PYXValueMath::test()
{
	double data1[] = { 0.1, 1.2, 2.3, 3.4 };
	int nSize1 = sizeof(data1)/sizeof(data1[0]);

	double data2[] = { 3.4, 1.5, 2.1, 0.1 };
	int nSize2 = sizeof(data2)/sizeof(data2[0]);

	int data3[] = { 1, 4, 3, 2 };
	int nSize3 = sizeof(data3)/sizeof(data3[0]);

	PYXValue v1(data1, nSize1);
	PYXValue v2(data2, nSize2);
	PYXValue v3(data3, nSize3);
	PYXValue vNull;

	//Addition
	//--------
	//pvA + pvB = pvC (with type of pvA)
	{
		PYXValue pvA(v1);
		PYXValue pvB(v3);
		addInto(&pvA, pvB);
		TEST_ASSERT (v1.getType() == pvA.getType());
	}

	//pvA + null = pvA (with type of pvA)
	{
		PYXValue pvA(v1);
		PYXValue pvB(vNull);
		addInto(&pvA, pvB);
		TEST_ASSERT (v1.getType() == pvA.getType());
		TEST_ASSERT (v1 == pvA);
	}

	//null + pvB = pvB (with type of pvB)
	{
		PYXValue pvA(vNull);
		PYXValue pvB(v3);
		addInto(&pvA, pvB);
		TEST_ASSERT (v3.getType() == pvA.getType());
		TEST_ASSERT (v3 == pvA);
	}

	//pvA + const = pvC (with type of pvA)
	{
		PYXValue pvA(v1);
		addInto(&pvA, 2.0);
		TEST_ASSERT (v1.getType() == pvA.getType());
	}

	//null + const = null 
	{
		PYXValue pvA(vNull);
		addInto(&pvA, 2.0);
		TEST_ASSERT (vNull.getType() == pvA.getType());
		TEST_ASSERT (pvA.isNull());
	}

	//Subtraction
	//-----------
	//pvA - pvB = pvC (with type of pvA)
	{
		PYXValue pvA(v1);
		PYXValue pvB(v3);
		subtractFrom(&pvA, pvB);
		TEST_ASSERT (v1.getType() == pvA.getType());
	}

	//pvA - null = pvA (with type of pvA)
	{
		PYXValue pvA(v1);
		PYXValue pvB(vNull);
		subtractFrom(&pvA, pvB);
		TEST_ASSERT (v1.getType() == pvA.getType());
		TEST_ASSERT (v1 == pvA);
	}

	//null - pvB = -pvB (with type of pvB)
	{
		PYXValue pvA(vNull);
		PYXValue pvB(v3);
		subtractFrom(&pvA, pvB);
		TEST_ASSERT (v3.getType() == pvA.getType());
	}

	//pvA - const = pvC (with type of pvA)
	{
		PYXValue pvA(v1);
		subtractFrom(&pvA, 2.0);
		TEST_ASSERT (v1.getType() == pvA.getType());
	}

	//null - const = null 
	{
		PYXValue pvA(vNull);
		subtractFrom(&pvA, 2.0);
		TEST_ASSERT (vNull.getType() == pvA.getType());
		TEST_ASSERT (pvA.isNull());
	}


	//Multiplication
	//--------------
	//pvA * pvB = pvC (with type of pvA)
	{
		PYXValue pvA(v1);
		PYXValue pvB(v3);
		multiplyBy(&pvA, pvB);
		TEST_ASSERT (v1.getType() == pvA.getType());
	}

	//pvA * null = null 
	{
		PYXValue pvA(v1);
		PYXValue pvB(vNull);
		multiplyBy(&pvA, pvB);
		TEST_ASSERT (pvA.isNull());
	}

	//null * pvB = null 
	{
		PYXValue pvA(vNull);
		PYXValue pvB(v3);
		multiplyBy(&pvA, pvB);
		TEST_ASSERT (pvA.isNull());
	}

	//pvA * const = pvC (with type of pvA)
	{
		PYXValue pvA(v1);
		multiplyBy(&pvA, 2.0);
		TEST_ASSERT (v1.getType() == pvA.getType());
	}

	//null * const = null 
	{
		PYXValue pvA(vNull);
		multiplyBy(&pvA, 2.0);
		TEST_ASSERT (vNull.getType() == pvA.getType());
		TEST_ASSERT (pvA.isNull());
	}

	//Division
	//--------
	//pvA / pvB = pvC (with type of pvA)
	{
		PYXValue pvA(v1);
		PYXValue pvB(v3);
		divideBy(&pvA, pvB);
		TEST_ASSERT (v1.getType() == pvA.getType());
	}

	//pvA / null = null 
	{
		PYXValue pvA(v1);
		PYXValue pvB(vNull);
		divideBy(&pvA, pvB);
		TEST_ASSERT (pvA.isNull());
	}

	//null / pvB = null 
	{
		PYXValue pvA(vNull);
		PYXValue pvB(v3);
		divideBy(&pvA, pvB);
		TEST_ASSERT (pvA.isNull());
	}

	//pvA / const = pvC (with type of pvA)
	{
		PYXValue pvA(v1);
		divideBy(&pvA, 2.0);
		TEST_ASSERT (v1.getType() == pvA.getType());
	}

	//null / const = null 
	{
		PYXValue pvA(vNull);
		divideBy(&pvA, 2.0);
		TEST_ASSERT (pvA.isNull());
	}
}

void PYXValueMath::zero(PYXValue* val)
{
	int nLimit = val->getArraySize();
	for (int nIndex = 0; nIndex < nLimit; ++nIndex)
	{
		val->setDouble(nIndex, 0);
	}
}

// TODO: these functions need to change to throw exceptions.
// we should define some PYXValueMath exceptions for 
// divide by zero, operation on type which does not support this type
// of operation, and illegal operation on PYXValues with 
// different number of channels.

/*! 
Copies values from assignMe into assignInto while preserving the type of assignInto.

If the number of channels does not match, then no operation is performed.

 \param assignIntoVal	The PYXValue to have its values replaced.
 \param assignMe		The PYXValue to assigned.
*/
void PYXValueMath::assignInto(PYXValue* assignIntoVal, const PYXValue& assignMe)
{
	if (assignIntoVal->getArraySize() != assignMe.getArraySize())
	{
		return;
	}
	int nLimit = assignIntoVal->getArraySize();
	for (int nIndex = 0; nIndex < nLimit; ++nIndex)
	{
		assignIntoVal->setDouble(nIndex, assignMe.getDouble(nIndex));
	}
}

/*! 
Add a PYXValue to a PYXValue channel by channel if the 
number of channels is the same for each PYXValue.  If there is only
one channel for the addMe value, it is added to each channel
of the addIntoVal as if it was adding a scalar. 

Null Handling:  If the addMe value is null, then it is treated as adding
zero.  If the addIntoVal is null, then it is treated as adding to zero.

Otherwise, if there is no null handling and the number of channels does not match,
then no operation is performed.

 \param addIntoVal	The PYXValue to be added to.
 \param addMe		The PYXValue to add.
*/
void PYXValueMath::addInto(PYXValue* addIntoVal, const PYXValue& addMe)
{
	// we can't add bools
	if ((addIntoVal->getType() == PYXValue::knBool) ||
		(addMe.getType() == PYXValue::knBool))
	{
		return;
	}

	if (addMe.isNull())
	{
		return;
	}

	if (addIntoVal->isNull())
	{
		*addIntoVal =  addMe;
		return;
	}

	if (addMe.getArraySize() == 1)
	{
		addInto(addIntoVal, addMe.getDouble());
		return;
	}

	if (addIntoVal->getArraySize() !=  addMe.getArraySize())
	{
		return;
	}

	int nLimit = addIntoVal->getArraySize();
	for (int nIndex = 0; nIndex < nLimit; ++nIndex)
	{
		addIntoVal->setDouble(nIndex, addIntoVal->getDouble(nIndex) + 
			addMe.getDouble(nIndex));
	}
	return;
}

/*! 
Add a constant to all channels of a PYXValue.

Null Handling:  If the addIntoVal is null then no operation is performed.

 \param addIntoVal	The PYXValue to be added to.
 \param fAddMe		The value to add to each channel of the PYXValue.
*/
void PYXValueMath::addInto(PYXValue* addIntoVal, const double fAddMe)
{
	// we can't add bools
	if (addIntoVal->getType() == PYXValue::knBool)
	{
		return;
	}

	if (addIntoVal->isNull())
	{
		return;
	}

	int nLimit = addIntoVal->getArraySize();
	for (int nIndex = 0; nIndex < nLimit; ++nIndex)
	{
		addIntoVal->setDouble(nIndex, addIntoVal->getDouble(nIndex) + fAddMe);
	}
	return;
}

/*! 
Subtract a PYXValue from a PYXValue channel by channel if the 
number of channels is the same for each PYXValue.  If there is only
one channel for the subtractMe value, it is subtracted from each channel
of the subtractFromVal as if it was subtracting a scalar. 

Null Handling:  If subtractMe value is null, then it is treated as subtracting
zero.  If the subtractFromVal is null, then it is treated as subtracting from zero.

Otherwise, if there is no null handling and the number of channels does not match,
then no operation is performed.

\param subtractFromVal	The PYXValue to be subtracted from.
\param subtractMe		The PYXValue to subtract.
*/
void PYXValueMath::subtractFrom(PYXValue* subtractFromVal, const PYXValue& subtractMe)
{
	// we can't subtract bools or strings
	if ((subtractFromVal->getType() == PYXValue::knBool) ||
		(subtractMe.getType() == PYXValue::knBool) ||
		(subtractFromVal->getType() == PYXValue::knString) ||
		(subtractMe.getType() == PYXValue::knString))
	{
		return;
	}

	if (subtractMe.isNull())
	{
		return;
	}

	if (subtractFromVal->isNull())
	{
		*subtractFromVal = subtractMe;
		multiplyBy (subtractFromVal, -1.0);
		return;
	}

	if (subtractMe.getArraySize() == 1)
	{
		addInto(subtractFromVal, subtractMe.getDouble());
		return;
	}

	if (subtractFromVal->getArraySize() !=  subtractMe.getArraySize())
	{
		return;
	}

	int nLimit = subtractFromVal->getArraySize();
	for (int nIndex = 0; nIndex < nLimit; ++nIndex)
	{
		subtractFromVal->setDouble(nIndex, subtractFromVal->getDouble(nIndex) -
			subtractMe.getDouble(nIndex));
	}
	return;
}

/*! 
Subtract a constant from all channels of a PYXValue.

Null Handling:  If the subtractFromVal is null no operation is performed.

\param subtractFromVal	The PYXValue to be subtracted from.
\param fSubtractMe		The value to subtract from each channel of the PYXValue.
*/
void PYXValueMath::subtractFrom(PYXValue* subtractFromVal, const double fSubtractMe)
{
	// we can't subtract bools or strings
	if ((subtractFromVal->getType() == PYXValue::knBool) ||
		(subtractFromVal->getType() == PYXValue::knString))
	{
		return;
	}

	if (subtractFromVal->isNull())
	{
		return;
	}

	int nLimit = subtractFromVal->getArraySize();
	for (int nIndex = 0; nIndex < nLimit; ++nIndex)
	{
		subtractFromVal->setDouble(nIndex, subtractFromVal->getDouble(nIndex) - 
			fSubtractMe);
	}
	return;
}

/*! 
Divide PYXvalues.
Divide a PYXValue by a PYXValue channel by channel if the 
number of channels is the same for each PYXValue.  If there is only
one channel for the divideByMe value, it is divided by each channel
of the divideVal as if it was dividing by a scalar. 

Null Handling:  If either operand is null, then no divideVal is set to null.

Otherwise, if the number of channels does not match,
then no operation is performed.

If any individual division operation attempts to divide by zero, that 
operation will be skipped.

\param divideVal		The PYXValue to be divided.
\param divideByMe		The PYXValue to divide by.
*/
void PYXValueMath::divideBy(PYXValue* divideVal, const PYXValue& divideByMe)
{
	// we can't divide bools or strings
	if ((divideVal->getType() == PYXValue::knBool) ||
		(divideByMe.getType() == PYXValue::knBool) ||
		(divideVal->getType() == PYXValue::knString) ||
		(divideByMe.getType() == PYXValue::knString))
	{
		return;
	}

	if (divideVal->isNull())
	{
		return;
	}

	if (divideByMe.isNull())
	{
		*divideVal = PYXValue();
		return;
	}

	if (divideByMe.getArraySize() == 1)
	{
		divideBy(divideVal, divideByMe.getDouble());
		return;
	}

	if (divideVal->getArraySize() !=  divideByMe.getArraySize())
	{
		return;
	}

	int nLimit = divideVal->getArraySize();
	for (int nIndex = 0; nIndex < nLimit; ++nIndex)
	{
		double fDivisor = divideByMe.getDouble(nIndex);
		// should we throw an exception for divide by zero?  Probably.
		if (fDivisor != 0.0)
		{
			divideVal->setDouble(nIndex, divideVal->getDouble(nIndex) / fDivisor);
		}
	}
	return;
}

/*! 
Divide all elements of a PYXvalue by a constant.

If divideVal is null, no operation is performed. 

If divideByMe is zero, no operation is performed. 

 \param divideVal		The PYXValue to be divided.
 \param fDivideByMe		The value to divide all elements by.
*/
void PYXValueMath::divideBy(PYXValue* divideVal, const double fDivideByMe) 
{
	// we can't divide bools or strings
	if ((divideVal->getType() == PYXValue::knBool) ||
		(divideVal->getType() == PYXValue::knString))
	{
		return;
	}

	if (divideVal->isNull())
	{
		return;
	}

	if (fDivideByMe == 0.0)
	{
		// should we throw an exception for divide by zero?  Probably.
		return;
	}

	int nLimit = divideVal->getArraySize();
	for (int nIndex = 0; nIndex < nLimit; ++nIndex)
	{
		divideVal->setDouble(nIndex, divideVal->getDouble(nIndex) / fDivideByMe);
	}
	return;
}

/*! 
Multiply PYXvalues.
Multiply a PYXValue by a PYXValue channel by channel if the 
number of channels is the same for each PYXValue.  If there is only
one channel for the multiplyByMe value, it is multiplied by each channel
of the multiplyVal as if it was multiplying by a scalar. 

Null Handling:  If either operand is null, then null is returned.

Otherwise, if the number of channels does not match,
then no operation is performed.

 \param multiplyVal		The PYXValue to be multiplied.
 \param multiplyByMe	The PYXValue to multiply by.
*/
void PYXValueMath::multiplyBy(PYXValue* multiplyVal, const PYXValue& multiplyByMe)
{
	// we can't divide bools or strings
	if ((multiplyVal->getType() == PYXValue::knBool) ||
		(multiplyByMe.getType() == PYXValue::knBool) ||
		(multiplyVal->getType() == PYXValue::knString) ||
		(multiplyByMe.getType() == PYXValue::knString))
	{
		return;
	}

	if (multiplyVal->isNull())
	{
		return;
	}

	if (multiplyByMe.isNull())
	{
		*multiplyVal = PYXValue();
		return;
	}

	if (multiplyByMe.getArraySize() == 1)
	{
		multiplyBy(multiplyVal, multiplyByMe.getDouble());
		return;
	}

	if (multiplyVal->getArraySize() !=  multiplyByMe.getArraySize())
	{
		return;
	}

	int nLimit = multiplyVal->getArraySize();
	for (int nIndex = 0; nIndex < nLimit; ++nIndex)
	{
		multiplyVal->setDouble(nIndex, multiplyVal->getDouble(nIndex) * 
			multiplyByMe.getDouble(nIndex));
	}
	return;
}

/*! 
Multiply all elements of a PYXvalue by a scalar.

If multiplyVal is null, no operation is performed.

 \param multiplyVal		The PYXValue to be multiplied.
 \param fMultiplyByMe	The value to multiply all elements by.
*/
void PYXValueMath::multiplyBy(PYXValue* multiplyVal, const double fMultiplyByMe) 
{
	// we can't divide bools or strings
	if ((multiplyVal->getType() == PYXValue::knBool) ||
		(multiplyVal->getType() == PYXValue::knString))
	{
		return;
	}

	if (multiplyVal->isNull())
	{
		return;
	}

	int nLimit = multiplyVal->getArraySize();
	for (int nIndex = 0; nIndex < nLimit; ++nIndex)
	{
		multiplyVal->setDouble(nIndex, multiplyVal->getDouble(nIndex) * fMultiplyByMe);
	}
	return;
}

