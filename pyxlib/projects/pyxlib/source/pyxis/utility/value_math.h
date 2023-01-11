#ifndef VALUE_MATH_H
#define VALUE_MATH_H
/******************************************************************************
value_math.h

begin		: 2006-11-07
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "value.h"

// standard includes

// forward declarations

/*!
Encapsulation and definitions of PYXValue math.

How it does work:

Number of Channels in Operands
==============================

For operations with two PYXValue operands the operands must
have the same number of channels.  The exceptions to this are
if the second operand has only one channel, it will be replicated
to the same number of channels as the first operand.  In other words,
it will be treated as an operation between a PYXValue and a scalar.
The other exception to this is when dealing with Null PYXValues.

Definitions - 
pvA, pvB, and pvC and PYXValues (non null)
null is a PYXVlaue that is null NOTE: this means it has no type information.
const is a constant value and has a type of double.

Addition
--------
pvA + pvB = pvC (with type of pvA)
pvA + null = pvA (with type of pvA)
null + pvB = pvB (with type of pvB)
pvA + const = pvC (with type of pvA)
null + const = null 

Subtraction
-----------
pvA - pvB = pvC (with type of pvA)
pvA - null = pvA (with type of pvA)
null - pvB = -pvB (with type of pvB)
pvA - const = pvC (with type of pvA)
null - const = null 

Multiplication
--------------
pvA * pvB = pvC (with type of pvA)
pvA * null = null 
null * pvB = null 
pvA * const = pvC (with type of pvA)
null * const = null 

Division
--------
pvA / pvB = pvC (with type of pvA)
pvA / null = null 
null / pvB = null 
pvA / const = pvC (with type of pvA)
null / const = null 


This is how I thought I would like it to work, but it doesn't
because a 1 channel double null PYXValue is NOT different from a 
3 channel int null PYXValue.

Null Value Rules
================

For addition: 
  A + null = A
  null + A = A (with the type from the null)

For subtraction: 
  A - null = A
  null - A = -A (with the type from the null)

For Multiplication:
  A * null = null
  null * A = null

For Division
  A / null = A (TODO: throw an exception)
  null / A = null

Unsupported Types
=================
Addition: bool
Subtraction: bool string
Division: bool string
Multiplication: bool string

*/
//! Static class to implement math functions for the PYXValue class.
class PYXLIB_DECL PYXValueMath
{
public:

	//! Unit test method
	static void test();

	//! Set all the elements of a PYXValue to zero.
	static void zero(PYXValue* val);

	/*! Assign a values from one PYXValue into a first PYXValue while
	retaining the type of the first PYXValue.
	*/
	static void assignInto(PYXValue* assignIntoVal, const PYXValue& assignMe);
	
	//! Addition of two PYXValues.
	static void addInto(PYXValue* addIntoVal, const PYXValue& addMe);

	//! Addition of a scalar.
	static void addInto(PYXValue* addIntoVal, const double fAddMe);

	//! Subtraction of two PYXValues.
	static void subtractFrom(PYXValue* subtractFromVal, const PYXValue& subtractMe);

	//! Subtraction of a scalar.
	static void subtractFrom(PYXValue* subtractFromVal, const double fSubtractMe);

	//! Division of a PYXValue by a PYXValue.
	static void divideBy(PYXValue* divideVal, const PYXValue& divideByMe);

	//! Division of a PYXValue by a scalar.
	static void divideBy(PYXValue* divideVal, const double fDivideByMe);

	//! Multiplication of a PYXValue by a PYXValue.
	static void multiplyBy(PYXValue* multiplyVal, const PYXValue& multiplyByMe);

	//! Multiplication of a PYXValue by a scalar.
	static void multiplyBy(PYXValue* multiplyVal, const double fMultiplyByMe);

protected:

	//! Default constructor -- never constuct.
	PYXValueMath() {}

	//! Destructor.
	virtual ~PYXValueMath() {}

	//! Disable copy constructor
	PYXValueMath(const PYXValueMath&);

	//! Disable copy assignment
	void operator =(const PYXValueMath&);

private:

};

#endif	// End Guard
