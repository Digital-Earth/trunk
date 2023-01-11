/******************************************************************************
subtraction_filter.cpp

begin		: 2006-04-25
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "subtraction_filter.h"

// local includes
//#include "const_coverage.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value_math.h"

// boost includes

// standard includes

//! The name of the class
const std::string PYXSubtractionFilter::kstrScope = "PYXSubtractionFilter";

//! The unit test class
TesterUnit<PYXSubtractionFilter> gTester;

/*!
The unit test method for the class.
*/
void PYXSubtractionFilter::test()
{
	//PYXIcosIndex index = "A-0500"; 
	//boost::shared_ptr<PYXConstCoverage> spCoverageOne(
	//	new PYXConstCoverage(PYXValue(long(200)))); 
	//boost::shared_ptr<PYXConstCoverage> spCoverageTwo(
	//	new PYXConstCoverage(PYXValue(long(50))));
	//boost::shared_ptr<PYXSubtractionFilter> spSubtract(
	//	new PYXSubtractionFilter()); 
	//
	//spSubtract->setInput(spCoverageOne); 
	//spSubtract->setSecondInput(spCoverageTwo); 

	//// 200 - 50 should be 150
	//TEST_ASSERT(150 == spSubtract->getCoverageValue(index).getInt32());   

	//const int knNumChannels = 10;
	//const int32_t initializeArray100[knNumChannels] = 
	//    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100};
	//PYXValue tenChannelValue100 = PYXValue(initializeArray100, knNumChannels);
	//const int32_t initializeArray200[knNumChannels] = 
	//    {200, 200, 200, 200, 200, 200, 200, 200, 200, 200};
	//PYXValue tenChannelValue200 = PYXValue(initializeArray200, knNumChannels);
	//// Change coverages into 10 channel data sources.
	//spCoverageOne->setReturnValue(tenChannelValue100); 
	//spCoverageTwo->setReturnValue(tenChannelValue200); 

	//PYXValue result = spSubtract->getCoverageValue(index);
	//// Test for multiple channels 
	//for (int nChannel = 0; nChannel < knNumChannels; ++ nChannel)
	//{
	//	// 100 - 200 should be -100
	//	TEST_ASSERT(-100 == result.getInt32(nChannel));
	//}
}

/*!
Default Constructor.
*/
PYXSubtractionFilter::PYXSubtractionFilter()
{
}

/*!
Get the coverage value at specified index. Takes the value(A) of image 1 at the
specified index. Then subtracts from A the value(B) at the specified index of
image 2. The resulting value(C) returned is C = A - B.  The operation follows
the rules of PYXValueMath as outlined in PYXValueMath.h

\param	index		The PYXIS index
\param	nFieldIndex	The field index.

\return	A PYXValue containing the difference between the value
		on ImageA and ImageB or a Null value. 
*/
PYXValue PYXSubtractionFilter::getCoverageValue(
	const PYXIcosIndex& index, 
	int nFieldIndex	) const
{
	assert(getInput() && "No Input Coverage Set"); 
	assert(m_spSecondInput && "No Second Input Coverage Set");
	
	PYXValue valueA = getInput()->getCoverageValue(index, nFieldIndex); 
	PYXValue valueB = m_spSecondInput->getCoverageValue(index, nFieldIndex); 
	PYXValueMath::subtractFrom(&valueA, valueB);
	return valueA;  
}

/*!
Sets Second Input Coverage 

\param spInput		The input Coverage. 
*/
void PYXSubtractionFilter::setSecondInput(boost::shared_ptr<const PYXCoverage> spInput) 
{
	assert(spInput && "Invalid argument.");  // Make sure valid pointer

	m_spSecondInput = spInput; 
}