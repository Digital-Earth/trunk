#ifndef DIALATION_FILTER_H
#define DIALATION_FILTER_H
/******************************************************************************
dialation_filter.h

begin		: 2006-05-23
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "filter_impl.h"

// standard includes

// forward declarations

/*!
PYXDilationFilter performs a morphological dilation operation on the data set. A 
structuring element is placed over the index and it's six neighbours, for the 
index which is being filter. If the structuring element contains a 1 then 
the cell that it is covering is considered if the structuring element contains 
a 0 it is not. Cells underneath the structuring element are then considered 
and examined for their maximum value. The index for which the filter is being 
applied is then assigned the maximum value contained by the structuring element.
*/
//! PYXDilationFilter, performs morphological dilation on dataset. 
class PYXDilationFilter : public FilterImpl
{
public:

	//! Constants
	static const std::string kstrScope;

	//! Unit test method
	static void test();

	//! Comprehensive test method
	static void detailedTest();

	static PYXPointer<PYXDilationFilter> create()
	{
		return PYXNEW(PYXDilationFilter);
	}

	//! Default constructor.
	PYXDilationFilter();

	//! Destructor.
	virtual ~PYXDilationFilter(){;}

	//! Get the coverage value at the specified PYXIS index.
	virtual PYXValue getCoverageValue(
		const PYXIcosIndex& index,
		int nFieldIndex = 0) const;

protected:

	//! Disable copy constructor
	PYXDilationFilter(const PYXDilationFilter&);

	//! Disable copy assignment
	void operator =(const PYXDilationFilter&);

private:

	//! The structuring element. 
	PYXValue m_structElement; 
	
};

#endif	// End if
