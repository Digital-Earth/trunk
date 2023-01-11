#ifndef EROSION_FILTER_H
#define EROSION_FILTER_H
/******************************************************************************
erosion_filter.h

begin		: 2006-05-23
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "filter_impl.h"

// standard includes

// forward declarations

/*!
PYXErosionFilter performs a morphological erosion operation on the data set. A 
structuring element is placed over the index and it's six neighbours, for the 
index which is being filter. If the structuring element contains a 1 then 
the cell that it is covering is considered if the structuring element contains 
a 0 it is not. Cells underneath the structuring element are then considered 
and examined for their minimum value. The index for which the filter is being 
applied is then assigned lowest value contained by the structuring element.
*/
//! PYXErosion Filter, performs morphological erosion on dataset. 
class PYXErosionFilter : public FilterImpl
{
public:

	//! Constants
	static const std::string kstrScope;

	//! Unit test method
	static void test();

	static PYXPointer<PYXErosionFilter> create()
	{
		return PYXNEW(PYXErosionFilter);
	}

	//! Default constructor.
	PYXErosionFilter();

	//! Destructor.
	virtual ~PYXErosionFilter(){;}


	//! Get the coverage value at the specified PYXIS index.
	virtual PYXValue getCoverageValue(
		const PYXIcosIndex& index,
		int nFieldIndex = 0	) const;
	
protected:

	//! Disable copy constructor
	PYXErosionFilter(const PYXErosionFilter&);

	//! Disable copy assignment
	void operator =(const PYXErosionFilter&);

private:

	//! The structuring element. 
	PYXValue m_structElement; 

};

#endif	// end if
