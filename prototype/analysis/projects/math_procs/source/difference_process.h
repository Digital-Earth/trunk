#ifndef DIFFERENCE_FILTER_H
#define DIFFERENCE_FILTER_H
/******************************************************************************
difference_filter.h

begin		: 2006-10-12
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "filter_impl.h"


// boost includes 

// standard includes

// forward declarations

/*!
Difference Filter. Calculates the difference between two coverages
by returning ABS(A - B), or the distance between the data values at
each position in the coverage.
*/
//! Difference filter. Calculates the difference between two coverages.
class PYXDifferenceFilter  : public FilterImpl
{
public:
	//! Constants
	static const std::string kstrScope;

	//! Unit test method
	static void test();

	//! Default constructor.
	PYXDifferenceFilter();

	//! Destructor.
	virtual ~PYXDifferenceFilter(){;}

	//! Get the coverage value at the specified PYXIS index.
	virtual PYXValue getCoverageValue(	
		const PYXIcosIndex& index,
		int nFieldIndex = 0	) const;
	
	//! Get the second input required for this filter. 
	PYXPointer<const PYXCoverage> getSecondInput(){return m_spSecondInput;}
	
	//! Set second input required for filter. 
	virtual void setSecondInput(PYXPointer<const PYXCoverage> spInput); 

private:

	//! Disable copy constructor
	PYXDifferenceFilter(const PYXDifferenceFilter&);

	//! Disable copy constructor
	void operator =(const PYXDifferenceFilter&);

	//! Second input coverage.
	PYXPointer<const PYXCoverage> m_spSecondInput; 
};

#endif	//end if
