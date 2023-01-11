/******************************************************************************
difference_filter.cpp

begin		: 2006-04-25
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "difference_filter.h"

// local includes
//#include "const_coverage.h"
#include "pyxis/utility/tester.h"

// boost includes

// standard includes
#include <math.h>

//! The name of the class
const std::string PYXDifferenceFilter::kstrScope = "PYXDifferenceFilter";

//! The unit test class
TesterUnit<PYXDifferenceFilter> gTester;

/*!
The unit test method for the class.
*/
void PYXDifferenceFilter::test()
{
	//PYXIcosIndex index = "A-0500"; 
	//boost::shared_ptr<PYXConstCoverage> spCoverageOne(
	//	new PYXConstCoverage(PYXValue(long(200)))); 
	//boost::shared_ptr<PYXConstCoverage> spCoverageTwo(
	//	new PYXConstCoverage(PYXValue(long(50))));
	//boost::shared_ptr<PYXDifferenceFilter> spDifference(
	//	new PYXDifferenceFilter()); 
	//
	//spDifference->setInput(spCoverageOne); 
	//spDifference->setSecondInput(spCoverageTwo); 
	//// ABS(200 - 50) should be 150
	//TEST_ASSERT(150 == spDifference->getCoverageValue(index).getInt32());   

	//spDifference->setInput(spCoverageTwo); 
	//spDifference->setSecondInput(spCoverageOne); 
	//// ABS(50 - 200) should be 150
	//TEST_ASSERT(150 == spDifference->getCoverageValue(index).getInt32());   

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

	//PYXValue result = spDifference->getCoverageValue(index);
	//// Test for multiple channels 
	//for (int nChannel = 0; nChannel < knNumChannels; ++ nChannel)
	//{
	//	// ABS(100 - 200) should be 100
	//	TEST_ASSERT(100 == result.getInt32(nChannel));
	//}
}

/*!
Default Constructor.
*/
PYXDifferenceFilter::PYXDifferenceFilter()
{
}

/*!
Get the coverage value at specified index. Takes the value(A) of image 1 at the
specified index. Then subtracts from A the value(B) at the specified index of
image 2. The resulting value(C) returned is C = ABS(A - B).  The return value's type
will be the type of value A.  A null value will be returned if:

1. Both A and B are null.  
2. The array sizes (Or the number of channels) in the data sets do not match 
3. The data type is a String 
4. The data type is a boolean. 

\param	index		The PYXIS index
\param	nFieldIndex	The field index.

\return	A PYXValue containing the distance between the value
		on ImageA and ImageB or a Null value. 
*/
PYXValue PYXDifferenceFilter::getCoverageValue(
	const PYXIcosIndex& index, 
	int nFieldIndex	) const
{
	assert(getInput() && "No Input Coverage Set"); 
	assert(m_spSecondInput && "No Second Input Coverage Set");
	
	PYXValue valueA = getInput()->getCoverageValue(index, nFieldIndex); 

	PYXValue valueB = m_spSecondInput->getCoverageValue(index, nFieldIndex); 
 
	if((valueA.getArraySize() != valueB.getArraySize())|| 
		(valueA.getType() == PYXValue::knString) || 
		(valueA.getType() == PYXValue::knBool) ||
		(valueB.getType() == PYXValue::knString) || 
		(valueB.getType() == PYXValue::knBool) ||
		(valueA.isNull() && valueB.isNull()))
	{
		return PYXValue(); 
	}

	//Preserve Type, even though operated on as double 
	PYXValue returnValue(valueA);
	for (int nChannel = 0; nChannel < valueA.getArraySize(); ++nChannel) 
	{
		// Caluclate the distance between the two images for a given channel 
		returnValue.setDouble(nChannel, abs(valueA.getDouble(nChannel) - valueB.getDouble(nChannel))); 
	}

	return returnValue;  
}

/*!
Sets Second Input Coverage 

\param spInput		The input Coverage. 
*/
void PYXDifferenceFilter::setSecondInput(PYXPointer<const PYXCoverage> spInput) 
{
	assert(spInput && "Invalid argument.");  // Make sure valid pointer

	m_spSecondInput = spInput; 
}