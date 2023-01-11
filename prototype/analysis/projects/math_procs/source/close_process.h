#ifndef CLOSE_FILTER_H
#define CLOSE_FILTER_H
/******************************************************************************
close_filter.h

begin		: 2006-07-31
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "filter_impl.h"
#include "pyxis/utility/app_services.h"

// standard includes

// forward declarations
class PYXErosionFilter; 
class PYXGeometryIteratorFilter;
class PYXSingleResCacheFilter;
class PYXDilationFilter;

/*!
PYXCloseFilter, performs a morphological close operation on a coverage. A close
is performed by dilating the input coverage and caching those results. Following
the dilation of the coverage, the coverage is then eroded and the resulting 
output is a close at a specific index.
*/
//! PYXCloseFilter, performs a morophilogical close on a coverage.
class PYXCloseFilter : public FilterImpl
{
public:

	//! Unit test method
	static void test();

	//! Default constructor.
	PYXCloseFilter();

	//! Destructor.
	virtual ~PYXCloseFilter();

	//! Set the input coverage (ownership shared with caller)
	virtual void setInput(PYXPointer<PYXCoverage> spInput);
	
protected:

	//! Disable copy constructor
	PYXCloseFilter(const PYXCloseFilter&);

	//! Disable copy assignment
	void operator =(const PYXCloseFilter&);

private:

	//! Erosion filter, performs erosion on the dilated coverage.
	PYXPointer<PYXErosionFilter> m_spErode; 

	//! Dilation filter, performs a dilation on the input coverage.
	PYXPointer<PYXDilationFilter> m_spDilate; 

	//! Cache filter, used to temporaily store values after dilation.
	PYXPointer<PYXSingleResCacheFilter> m_spCache;
};
#endif	//End if
