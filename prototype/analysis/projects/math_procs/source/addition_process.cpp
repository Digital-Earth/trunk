/******************************************************************************
pyx_addition_filter.cpp

begin		: 2006-05-24
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "addition_filter.h"
//TODO: Deal with commented out header files.
// local includes
//#include "pyx_binary_coverage.h"
//#include "pyx_const_coverage.h"
//#include "pyx_null_coverage.h"
#include "pyxis/utility/value_math.h"
#include "pyxis/utility/tester.h"

// standard includes

//! The unit test class
TesterUnit<PYXAdditionFilter> gTester;

//! The name of the class 
const std::string PYXAdditionFilter::kstrScope = "PYXAdditionFilter"; 

/*!
The unit test method for the class.
*/
void PYXAdditionFilter::test()
{
	//TODO: Fix this later. CH
	//boost::shared_ptr<PYXConstCoverage> spConstCoverage(new PYXConstCoverage(PYXValue(100)));
	//boost::shared_ptr<PYXConstCoverage> spConstCoverage2(new PYXConstCoverage(PYXValue(100))); 
	//boost::shared_ptr<PYXAdditionFilter> spAddFilter(new PYXAdditionFilter()); 

	//spAddFilter->setInput(spConstCoverage); 
	//spAddFilter->setSecondInput(spConstCoverage2); 

	//int nNumIndices = 11; 
	//std::vector<PYXIcosIndex> testIndices(nNumIndices); 
	//
	////Random Indices
	//testIndices[0] = "A-500";
	//testIndices[1] = "1-502"; //Pentagon
	//testIndices[2] = "C-504"; 
	//testIndices[3] = "D-503"; 
	//testIndices[4] = "E-506"; 
	//testIndices[5] = "F-502"; //Ok
	//testIndices[6] = "G-506"; 
	//testIndices[7] = "3-504"; //Pentagon
	//testIndices[8] = "C-502"; 
	//testIndices[9] = "B-300"; 
	//testIndices[10] = "K-501"; 

	//for (int nCount = 0; nCount < nNumIndices; ++nCount)
	//{
	//	TEST_ASSERT(200.0 == spAddFilter->getCoverageValue(testIndices[nCount]).getDouble());
	//}
	//
	//PYXIcosIndex hexIndex = "A-500"; 
	//PYXIcosIndex pentIndex = "1-502";
	//
	//boost::shared_ptr<PYXAdditionFilter> spAdd (new PYXAdditionFilter()); 

	//boost::shared_ptr<PYXNullCoverage> spNullCoverage (new PYXNullCoverage()); 
	//spAdd->setInput(spNullCoverage); 
	//spAdd->setSecondInput(spConstCoverage); 

	//// Test  Null 
	//TEST_ASSERT(spAdd->getCoverageValue(hexIndex).isNull()); 
	//TEST_ASSERT(spAdd->getCoverageValue(pentIndex).isNull()); 
}

/*!
Sets Second Input Coverage 

\param spInput		The input Coverage. 
*/
void PYXAdditionFilter::setSecondInput(PYXPointer<PYXCoverage> spInput) 
{
	assert(spInput.get() != 0 && "Invalid parameter."); 
	m_spSecondInput = spInput; 
}

/*!
Get a coverage value at specified index. Takes the value(A) of image 1 at the
specified index. Then add to A the value(B) at the specified index of image 2.
The resulting value is A + B.

A Null value will be returned if:

1. The data types do not match  
2. The array size (Or the number of channels) in the data set do not match 
3. The data type is a String 
4. The data type is a boolean. 

\param	index		The PYXIS index
\param	nFieldIndex	The field index.

\return	A new normalized PYXValue containing the difference between the value on ImageA and ImageB
	    or a Null value. 
*/
PYXValue PYXAdditionFilter::getCoverageValue(
	const PYXIcosIndex &index,
	int nFieldIndex	) const 
{
	assert(PYXFilter::getInput() && "No Input Coverage Set"); 
	assert(m_spSecondInput && "No Second Input Coverage Set");
	
	PYXValue valueA = PYXFilter::getInput()->getCoverageValue(index, nFieldIndex); 
	PYXValue valueB = m_spSecondInput->getCoverageValue(index, nFieldIndex); 
	PYXValueMath::addInto(&valueA, valueB);
	//TRACE_DEBUG("Sum (" << PYXValue::getTypeAsString(valueA.getArrayType())<< ") " << valueA.toString()); 
	return valueA; 
}