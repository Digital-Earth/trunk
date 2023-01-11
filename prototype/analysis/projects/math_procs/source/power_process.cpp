/******************************************************************************
power_filter.cpp

begin		: 2006-05-24
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "power_filter.h"

// local includes
//#include "null_coverage.h"
//#include "const_coverage.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <math.h>

//! The name of the class. 
const std::string PYXPowerFilter::kstrScope ="PYXPowerFilter";

//! The unit test class
TesterUnit<PYXPowerFilter> gTester;

/*!
The unit test method for the class.
*/
void PYXPowerFilter::test()
{
	 
	//boost::shared_ptr<PYXConstCoverage> spConstCov(new PYXConstCoverage(PYXValue(10))); 
	//boost::shared_ptr<PYXPowerFilter> spPower(new PYXPowerFilter()); 
	//spPower->setInput(spConstCov); 
	//spPower->setExponent(2); 

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
	//	TEST_ASSERT(100.0 == spPower->getCoverageValue(testIndices[nCount]).getDouble());
	//}

	//PYXIcosIndex hexIndex = "A-500"; 
	//PYXIcosIndex pentIndex = "1-502";

	//boost::shared_ptr<PYXPowerFilter> spPowFilt(new PYXPowerFilter()); 
	//boost::shared_ptr<PYXNullCoverage> spNullCoverage (new PYXNullCoverage()); 
	//spPowFilt->setInput(spNullCoverage); 

	//// Test 1 Input Null 
	//TEST_ASSERT(spPowFilt->getCoverageValue(hexIndex).isNull()); 
	//TEST_ASSERT(spPowFilt->getCoverageValue(pentIndex).isNull());
}


/*!
Default Constructor.
*/
PYXPowerFilter::PYXPowerFilter():m_fExponent(1)
{
}

/*!
Gets the coverage value at the specific index. The return value is a new 
PYXValue at the index raised to an exponent. 

\param index		The pyxis index. 
\param nFieldIndex	The field index. 

\return The PYXValue at the index raised an exponent. 
*/
PYXValue PYXPowerFilter::getCoverageValue(
	const PYXIcosIndex &index,
	int nFieldIndex	) const 
{
	assert(getInput() && "No input coverage set.");
	PYXValue rtnValue = getInput()->getCoverageValue(index, nFieldIndex); 

	if ((rtnValue.getType() == PYXValue::knString)||
		(rtnValue.getType() == PYXValue::knBool)||
		(rtnValue.isNull()))
	{
		return PYXValue(); 
	}
	int nElements = rtnValue.getArraySize(); 

	for (int nElement = 0; nElement < nElements; ++nElement) 
	{
		rtnValue.setDouble(nElement,pow(rtnValue.getDouble(),m_fExponent)); 
	}
	return rtnValue; 
}