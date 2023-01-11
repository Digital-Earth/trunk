#ifndef DIVISION_FILTER_H
#define DIVISION_FILTER_H
/******************************************************************************
division_filter.h

begin		: 2006-05-24
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "filter_impl.h"

// standard includes

// forward declarations

/*!
Division Filter. Filters images by dividing image A by B, for every 
index on the coverage. A value at a specific index in the resulting 
image rtnImage is the quotient of the values from image A at an index and 
image B at the same index. value at rtnImage(index) = ImageA(index) / ImageB(index).
*/
//! Division Filter filters images by dividing one data set by another. 
class PYXDivisionFilter : public FilterImpl
{
public:

	//! Constants
	static const std::string kstrScope;

	//! Unit test method
	static void test();

	//! Default constructor.
	PYXDivisionFilter() {;}

	//! Destructor.
	virtual ~PYXDivisionFilter() {;}

	//! Get the coverage value at the specified PYXIS index.
	virtual PYXValue getCoverageValue(
		const PYXIcosIndex& index,
		int nFieldIndex = 0	) const;

	//! Set second input coverage for this filter. 
	void setSecondInput(boost::shared_ptr<PYXCoverage> spInput);

	//! Get second input coverage for filter. 
	boost::shared_ptr<PYXCoverage> getSecondInput() {return m_spSecondInput;} 

protected:

	//! Disable copy constructor
	PYXDivisionFilter(const PYXDivisionFilter&);

	//! Disable copy assignment
	void operator =(const PYXDivisionFilter&);

private:

	//!Second input coverage. 
	boost::shared_ptr<PYXCoverage> m_spSecondInput; 
};

#endif	// end if
