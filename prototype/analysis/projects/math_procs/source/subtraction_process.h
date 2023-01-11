#ifndef SUBTRACTION_FILTER_H
#define SUBTRACTION_FILTER_H
/******************************************************************************
subtraction_filter.h

begin		: 2006-04-25
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "filter_impl.h"

// boost includes 

// standard includes

// forward declarations

/*!
Subtraction Filter. Filters images by subtracting image A from image B, for every index on 
the coverage. A value at a specific index in the resulting image C is the difference in the
values from image A at an index and image B at the same index. 
value at C(index) = ImageA(index) - ImageB(index).
*/
//! Subtraction filter, subtracts 2 images returning the difference.
class PYXSubtractionFilter  : public FilterImpl
{
public:
	//! Constants
	static const std::string kstrScope;

	//! Unit test method
	static void test();

	//! Default constructor.
	PYXSubtractionFilter();

	//! Destructor.
	virtual ~PYXSubtractionFilter(){;}

	//! Get the coverage value at the specified PYXIS index.
	virtual PYXValue getCoverageValue(	
		const PYXIcosIndex& index,
		int nFieldIndex = 0	) const;
	
	//! Get the second input required for this filter. 
	boost::shared_ptr<const PYXCoverage> getSecondInput(){return m_spSecondInput;}
	
	//! Set second input required for filter. 
	virtual void setSecondInput(boost::shared_ptr<const PYXCoverage> spInput); 

private:

	//! Disable copy constructor
	PYXSubtractionFilter(const PYXSubtractionFilter&);

	//! Disable copy constructor
	void operator =(const PYXSubtractionFilter&);

	//! Second input coverage.
	boost::shared_ptr<const PYXCoverage> m_spSecondInput; 
};

#endif	// end if
