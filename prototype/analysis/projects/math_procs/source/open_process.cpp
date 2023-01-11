/******************************************************************************
open_filter.cpp

begin		: 2006-07-31
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "open_filter.h"

// local includes
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"
//#include "binary_coverage.h"
//#include "const_coverage.h"
#include "single_res_cache_filter.h"
#include "pyxis/utility/tester.h"

// standard includes

//! The unit test class
TesterUnit<PYXOpenFilter> gTester;

/*!
The unit test method for the class.
*/
void PYXOpenFilter::test()
{
//	int nCovValue = 100; 
//	boost::shared_ptr<PYXConstCoverage> spCoverage(
//		new PYXConstCoverage(PYXValue(nCovValue)));
//	boost::shared_ptr<PYXOpenFilter> spOpen(new PYXOpenFilter());
//	spOpen->setInput(spCoverage);
//
//	PYXIcosIndex index = "A-0500";
//	PYXIcosIndex pentIndex = "C-2005000"; 
//	PYXIcosIndex anotherIndex = "D-030040"; 
//	
//	TEST_ASSERT(spOpen->getCoverageValue(index).getInt32() == nCovValue);
//	TEST_ASSERT(spOpen->getCoverageValue(pentIndex).getInt32() == nCovValue);
//	TEST_ASSERT(spOpen->getCoverageValue(anotherIndex).getInt32() == nCovValue);
}


/*!
Default Constructor.
*/
PYXOpenFilter::PYXOpenFilter()
{
	m_spCache = PYXSingleResCacheFilter::create();
	m_spErode = PYXErosionFilter::create(); 
	m_spDilate = PYXDilationFilter::create();
}

/*!
Destructor.
*/
PYXOpenFilter::~PYXOpenFilter()
{
}

/*!
Sets the input coverage, chaining together the supporting objects 
required to perform a morphological open. 

\param spCoverage	The input coverage. 
*/
void PYXOpenFilter::setInput(PYXPointer<PYXCoverage> spCoverage)
{
	assert(spCoverage && "Invalid input parameter.");
	PYXFilter::setInput(m_spDilate);
	m_spErode->setInput(spCoverage);
	m_spCache->setInput(m_spErode);
	m_spDilate->setInput(m_spCache);
}
