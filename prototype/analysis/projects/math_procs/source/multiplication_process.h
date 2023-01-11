#ifndef MULTIPLICATION_FILTER_H
#define MULTIPLICATION_FILTER_H
/******************************************************************************
multiplication_filter.h

begin		: 2006-05-24
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "filter_impl.h"
// standard includes

// forward declarations

/*!
Mulitplication Filter. Filters images by mulitpling image A by B, for every i
ndex on the coverage. A value at a specific index in the resulting image rtnImage 
is the product of the values from image A at an index and image B at the same index. 
value at rtnImage(index) = ImageA(index) * ImageB(index).
*/
//!Multiplication Filter filters images by multiplying two datasets together. 
class PYXMultiplicationFilter : public FilterImpl
{
public:

	//! Constants
	static const std::string kstrScope;

	//! Unit test method
	static void test();

	//! Default constructor.
	PYXMultiplicationFilter(){;}

	//! Destructor.
	virtual ~PYXMultiplicationFilter(){;}

	//! Get the coverage value at the specified PYXIS index.
	virtual PYXValue getCoverageValue(
		const PYXIcosIndex& index,
		int nFieldIndex = 0	) const;

	//! Set second input coverage for this filter. 
	void setSecondInput(boost::shared_ptr<PYXCoverage> spInput);

	//! Get second input coverage for filter. 
	boost::shared_ptr<PYXCoverage> getSecondInput(){return m_spSecondInput;} 

protected:

	//! Disable copy constructor
	PYXMultiplicationFilter(const PYXMultiplicationFilter&);

	//! Disable copy assignment
	void operator =(const PYXMultiplicationFilter&);

private:

	//!Second input coverage. 
	boost::shared_ptr<PYXCoverage> m_spSecondInput; 
};

#endif	// end if
