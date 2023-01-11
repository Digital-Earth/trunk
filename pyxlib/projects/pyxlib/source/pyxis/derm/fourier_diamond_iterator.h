#ifndef PYXIS__DERM__FOURIER_DIAMOND_ITERATOR_H
#define PYXIS__DERM__FOURIER_DIAMOND_ITERATOR_H
/******************************************************************************
fourier_diamond_iterator.h

begin		: 2006-06-05
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/iterator.h"

struct PYXLIB_DECL jkPair
{
	int nK; 
	int nJ; 
};

/*!
PYXFourierDiamondIterator, this is an iterator for iterating over all cells in the 
diamond spatial domain. When performing the the diamond fourier transform on some face 
of the earth, this iterator is to be used. The diamond covers 9^n cells where n is the 
radius of the tile the diamond is constructed on. The diamond gets anchored to a specific 
face on the Icosahedron. This iterator iterates in the form of a j, k which are 
base vectors for the diamond domain. Both K iterates from (-N -1) / 2 to N-1 /2 
for J times where J goes from (-N -1) / 2 to N-1 /2. Where N = 3 ^n where n = resolution 
of the tile, it is important to note that n is not the cell resolution. 
*/
//! PYXFourierDiamondIterator. Iterates over all the cells on a diamond. 
class PYXLIB_DECL PYXFourierDiamondIterator : public PYXIterator
{
public:

	//! Unit test method
	static void test();

	//! Default constructor.
	PYXFourierDiamondIterator(const PYXIcosIndex& index, int nRadius);

	//! Destructor.
	virtual ~PYXFourierDiamondIterator();

	//! Move to the next cell.
	virtual void next();

	//! See if all the cells have been iterated over. 
	virtual bool end() const;

	//! Get the index that is related to this j,k
	virtual const PYXIcosIndex& getIndex() const;

	//! Return the total number of cells that will be iterated over. 
	int getTotalCells() {return m_nTotalCells;}

	//! Allows the setting of values J,K to an arbitrary value. 
	void setJK(int k, int j)
	{
		m_nJ = j;
		m_nK=k;
	} 

	//! Returns a structure containing the current values of J,K. 
	jkPair getJK(); 
	
protected:

	//! Disable copy constructor
	PYXFourierDiamondIterator(const PYXFourierDiamondIterator&);

	//! Disable copy assignment
	void operator =(const PYXFourierDiamondIterator&);

private:

	//! convert j, k to a PYXIndex. 
	PYXIndex convertToPyxIndex()const; 

	//! Return a cell number based on the values of j,k. 
	std::string jkToIndexLookUp(int r, int s)const; 

private:

	//! The centre index the diamond is constructed on. 
	PYXIcosIndex m_index; 

	//! Total number of cells to be iterated over. 
	int m_nTotalCells; 
		
	//! Maximum value for J.
	int m_nMaxJ;

	//! Maximum value for K. 
	int m_nMaxK;

	//! Minimum value for J.
	int m_nMinJ;

	//! Minimum value for K. 
	int m_nMinK;
	
	//! Current value of J. 
	int	m_nJ;

	//! Current value of K. 
	int m_nK;

	//! N according to the algorithm. 
	double m_fN;

	//! Radius (aka resolution) of tile.
	unsigned int m_nRadius; 

	//! Current index based on values of j,k.
	mutable PYXIcosIndex m_currentIndex; 

	//! Structure containing J, K as an ordered pair. 
	jkPair jk; 
};

#endif	//guard
