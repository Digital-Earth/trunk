/******************************************************************************
mulitplication_filter.cpp

begin		: 2006-05-24
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "multiplication_filter.h"

// local includes
#include "const_coverage.h"
#include "null_coverage.h"
//TODO: Deal with this.
#include "pyx_value_math.h"
#include "pyxis/utility/tester.h"


//! The name of the class 
const std::string PYXMultiplicationFilter::kstrScope = "PYXMultiplicationFilter"; 


//! The unit test class
TesterUnit<PYXMultiplicationFilter> gTester;

/*!
The unit test method for the class.
*/
void PYXMultiplicationFilter::test()
{
	boost::shared_ptr<PYXConstCoverage> spConstCoverage(new PYXConstCoverage(PYXValue(10)));
	boost::shared_ptr<PYXConstCoverage> spConstCoverage2(new PYXConstCoverage(PYXValue(10))); 
	boost::shared_ptr<PYXMultiplicationFilter> spMultFilter(new PYXMultiplicationFilter()); 

	spMultFilter->setInput(spConstCoverage); 
	spMultFilter->setSecondInput(spConstCoverage2); 

	int nNumIndices = 11; 
	std::vector<PYXIcosIndex> testIndices(nNumIndices); 
	
	//Random Indices
	testIndices[0] = "A-500";
	testIndices[1] = "1-502"; //Pentagon
	testIndices[2] = "C-504"; 
	testIndices[3] = "D-503"; 
	testIndices[4] = "E-506"; 
	testIndices[5] = "F-502"; //Ok
	testIndices[6] = "G-506"; 
	testIndices[7] = "3-504"; //Pentagon
	testIndices[8] = "C-502"; 
	testIndices[9] = "B-300"; 
	testIndices[10] = "K-501"; 

	for (int nCount = 0; nCount < nNumIndices; ++nCount)
	{
		TEST_ASSERT(100.0 == spMultFilter->getCoverageValue(testIndices[nCount]).getDouble());
	}

	//Test Null Values for both pentagons and hexagons 

	PYXIcosIndex hexIndex = "A-500"; 
	PYXIcosIndex pentIndex = "1-502";
	
	boost::shared_ptr<PYXMultiplicationFilter> spMult2 (new PYXMultiplicationFilter()); 

	boost::shared_ptr<PYXNullCoverage> spNullCoverage (new PYXNullCoverage()); 
	spMult2->setInput(spNullCoverage); 
	spMult2->setSecondInput(spConstCoverage); 


	// Test  Null 
	TEST_ASSERT(spMult2->getCoverageValue(hexIndex).isNull()); 
	TEST_ASSERT(spMult2->getCoverageValue(pentIndex).isNull()); 

}

/*!
Sets Second Input Coverage 

\param spInput		The input Coverage. 
*/
void PYXMultiplicationFilter::setSecondInput(boost::shared_ptr<PYXCoverage> spInput) 
{
	assert(spInput.get() != 0 && "Invalid parameter."); 
	m_spSecondInput = spInput; 
}

/*!
Get the coverage value at specified index. Takes the value(A) of image 1 at the specified index. 
Then multiplies to A by B at the specified index of image 2. The resulting value(rtnValue)
returned is rtnValue = A * B.  A Null value will be returned if:

1. The data types do not match  
2. The array size (Or the number of channels) in the data set do not match 
3. The data type is a String 
4. The data type is a boolean. 

\param	index		The PYXIS index
\param	nFieldIndex	The field index.

\return	A new normalized PYXValue containing the difference between the value on ImageA and ImageB
	    or a Null value. 
*/
PYXValue PYXMultiplicationFilter::getCoverageValue(
	const PYXIcosIndex &index,
	int nFieldIndex	) const 
{
	assert(PYXFilter::getInput() && "No Input Coverage Set"); 
	assert(m_spSecondInput && "No Second Input Coverage Set");
	
	PYXValue valueA = PYXFilter::getInput()->getCoverageValue(index, nFieldIndex); 
	PYXValue valueB = m_spSecondInput->getCoverageValue(index, nFieldIndex); 
	PYXValueMath::multiplyBy(&valueA, valueB);
	return valueA; 
}