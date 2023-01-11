#ifndef	ADDITION_FILTER_H
#define ADDITION_FILTER_H
/******************************************************************************
addition_filter.h

begin		: 2006-05-24
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "filter_impl.h"
// standard includes

// forward declarations

/*!
Addition Filter. Filters images by adding image A to image B, for every index on 
the coverage. A value at a specific index in the resulting image rtnImage is the sum of 
the values from image A at an index and image B at the same index. 
value at rtnImage(index) = ImageA(index) + ImageB(index).
*/
//! AdditionFilter filters images by adding two datasets together. 
class PYXAdditionFilter : public FilterImpl
{
public:

	//! Constants
	static const std::string kstrScope;

	//! Unit test method
	static void test();
	
	//! Default constructor.
	PYXAdditionFilter(){;}

	//! Destructor.
	virtual ~PYXAdditionFilter(){;}

	//! Get the coverage value at the specified PYXIS index.
	virtual PYXValue getCoverageValue(
		const PYXIcosIndex& index,
		int nFieldIndex = 0	) const;

	//! Set second input coverage for this filter. 
	void setSecondInput(PYXPointer<PYXCoverage> spInput);

	//! Get second input coverage for filter. 
	PYXPointer<PYXCoverage> getSecondInput(){return m_spSecondInput;} 

protected:

	//! Disable copy constructor
	PYXAdditionFilter(const PYXAdditionFilter&);

	//! Disable copy assignment
	void operator =(const PYXAdditionFilter&);

private:

	//!Second input coverage. 
	PYXPointer<PYXCoverage> m_spSecondInput; 
};

#endif	//guard
