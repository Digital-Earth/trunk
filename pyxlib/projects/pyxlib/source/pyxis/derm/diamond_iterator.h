#ifndef PYXIS__DERM__DIAMOND_ITERATOR
#define PYXIS__DERM__DIAMOND_ITERATOR
/******************************************************************************
diamond_iterator.h

begin		: 2006-06-22
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/iterator.h"
#include "pyxis/derm/sub_index_math.h"

/*!
PYXDiamondIterator, iterates in a diamond shape to collect points, 
to perform a fourier transform on. The diamond shape was provided 
by Andy Vince. It is believed that this diamond shape for the 
lattice points is faster then the hexagonal shape when performing 
the FFT. This is a modification of the algorithm proposed by 
Andy for iterating over the points. This algorithm handles 
both classes of hexagons and pentagonal cells by performing a 
series of moves in the appropriate direction. 
*/
//! Iterates over a lattice in a dimaond shape. 
class PYXLIB_DECL PYXDiamondIterator : public PYXIterator
{
public:

	//! Unit test method
	static void test();

	//! Default constructor.
	PYXDiamondIterator(const PYXIcosIndex &index,int res);

	//! Destructor.
	virtual ~PYXDiamondIterator();

	//!	See if we have covered all the cells.
	virtual bool end() const;

	//! Move to the next cell.
	virtual void next();
	
	//! Get the total number of cells in the diamond. 
	int getTotalCells(){return m_nTotalCells;}

	//! Returns the current PYXIcosIndex. 
	virtual const PYXIcosIndex& getIndex() const ;

protected:

	//! Disable copy constructor
	PYXDiamondIterator(const PYXDiamondIterator&);

	//! Disable copy assignment
	void operator =(const PYXDiamondIterator&);

private:
	
	//! Starting index at the bottom of the diamond. 
	PYXIcosIndex m_startIndex; 

	//! Bottom most index on row of diagonals.
	PYXIcosIndex m_diagIndex; 

	//! Current index. 
	PYXIcosIndex m_currentIndex; 

	//! Length of the rows in the diamond. 
	double m_fRowLength; 

	//! Total number of cells to be iterated over. 
	int m_nTotalCells; 

	/*! Count the difference between the current cell and the bottom 
	cell of the row.
	*/
	int m_nCount; 

	//! Current diagonal row. 
	int m_nDiag; 

	//! The diagonal direction in which to move along the diamond. 
	PYXMath::eHexDirection m_nDiagDirect; 
};

#endif	//guard
