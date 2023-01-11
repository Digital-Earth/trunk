#ifndef POWER_FILTER_H
#define POWER_FILTER_H
/******************************************************************************
power_filter.h

begin		: 2006-05-24
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "filter_impl.h"
// standard includes

// forward declarations

/*!
Power filter raises a data source to a user defined constant exponent. Every 
PYXValue at a specified index is raised to the exponent. If the exponent 
is 2 then every value on the coveraged would be squared. 
*/
//! Power Filter raises raises data sets to Y^X. 
class PYXPowerFilter : public FilterImpl
{
public:

	//! Constants
	static const std::string kstrScope;

	//! Unit test method
	static void test();

	//! Default constructor.
	PYXPowerFilter();

	//! Destructor.
	virtual ~PYXPowerFilter(){;}

	//! Get the coverage value at the specified PYXIS index.
	virtual PYXValue getCoverageValue(
		const PYXIcosIndex& index,
		int nFieldIndex = 0	) const;

	//! Sets the exponent to raise dataset to. 
	void setExponent(double fExponent) {m_fExponent = fExponent;}
	
protected:

	//! Disable copy constructor
	PYXPowerFilter(const PYXPowerFilter&);

	//! Disable copy assignment
	void operator =(const PYXPowerFilter&);

private:
	
	//! The power to raise the data source to. 
	double  m_fExponent; 

};

#endif	// End if
