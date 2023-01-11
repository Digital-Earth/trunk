/******************************************************************************
close_filter.cpp

begin		: 2006-07-31
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "close_filter.h"

// local includes
#include "single_res_cache_filter.h"
//#include "pyx_const_coverage.h"
#include "dilation_filter.h"
#include "erosion_filter.h"

#include "pyxis/utility/tester.h"

// standard includes

//! The unit test class
TesterUnit<PYXCloseFilter> gTester;

/*!
The unit test method for the class.
*/
void PYXCloseFilter::test()
{
	//int nCovValue = 100; 
	//boost::shared_ptr<PYXConstCoverage> spCoverage(
	//	new PYXConstCoverage(PYXValue(nCovValue)));
	//boost::shared_ptr<PYXCloseFilter> spClose(new PYXCloseFilter());
	//spClose->setInput(spCoverage);

	//PYXIcosIndex index = "A-0500";
	//PYXIcosIndex pentIndex = "C-2005000"; 
	//PYXIcosIndex anotherIndex = "D-030040"; 
	//
	//TEST_ASSERT(spClose->getCoverageValue(index).getInt32() == nCovValue);
	//TEST_ASSERT(spClose->getCoverageValue(pentIndex).getInt32() == nCovValue);
	//TEST_ASSERT(spClose->getCoverageValue(anotherIndex).getInt32() == nCovValue);
}

/*!
Default Constructor.
*/
PYXCloseFilter::PYXCloseFilter()
{
	m_spCache = PYXSingleResCacheFilter::create();
	m_spDilate = PYXDilationFilter::create();
	m_spErode = PYXErosionFilter::create();
}

/*!
Destructor.
*/
PYXCloseFilter::~PYXCloseFilter()
{
}

/*!
Sets the input coverage, chaining together the supporting objects 
required to perform a morphological close operation. 

\param spCoverage	The input coverage. 
*/
void PYXCloseFilter::setInput(PYXPointer<PYXCoverage> spCoverage)
{
	assert(spCoverage.get() != 0 && "Invalid input parameter."); 
	m_spDilate->setInput(spCoverage); 
	m_spCache->setInput(m_spDilate); 
	m_spErode->setInput(m_spCache);
	PYXFilter::setInput(m_spErode);
}
