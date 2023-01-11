#ifndef OPEN_FILTER_H
#define OPEN_FILTER_H
/******************************************************************************
open_filter_h

begin		: 2006-07-26
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "filter_impl.h"
#include "erosion_filter.h"
#include "dilation_filter.h"

// standard includes

// forward declarations
class PYXSingleResCacheFilter;

/*!
PYXOpenFilter performs a mathematical morphological open. An open is an 
erosion of a signal followed by a dilation of the signal. This open is 
accomplished by performing an erosion on the input coverage, then caching
the result of the erosion. After the erosion has been cached the data is 
then passed through a dilation filter to perform the full open. 
*/
//! PYXOpenFilter, performs morphological open operation. 
class PYXOpenFilter : public FilterImpl
{
public:

	//! Unit test method
	static void test();

	//! Default constructor.
	PYXOpenFilter();

	//! Destructor.
	virtual ~PYXOpenFilter();

	//! Set the input coverage (ownership shared with caller)
	virtual void setInput(PYXPointer<PYXCoverage> spInput);

protected:

	//! Disable copy constructor
	PYXOpenFilter(const PYXOpenFilter&);

	//! Disable copy assignment
	void operator =(const PYXOpenFilter&);

private:

	/*!
	   Erosion filter used for eroding the original input 
	   coverage. This is create on construction of the 
	   open filter. The erosion happens on setInput. 
	*/
	PYXPointer<PYXErosionFilter> m_spErode; 

	/*!
	   Internal cache to temporalily hold the results of the 
	   eroded input coverage, prior to being dilated. Created 
	   on construction and has it's input set on setInput.
	*/
	PYXPointer<PYXSingleResCacheFilter> m_spCache;  

	/*!
	  Dialation filter used for dilating the cached eroded 
	  coverage. 
	*/
	PYXPointer<PYXDilationFilter> m_spDilate; 

};

#endif	// End if
