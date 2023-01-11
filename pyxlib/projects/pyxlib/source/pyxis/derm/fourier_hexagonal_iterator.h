#ifndef PYXIS__DERM__FOURIER_HEXAGONAL_ITERATOR_H
#define PYXIS__DERM__FOURIER_HEXAGONAL_ITERATOR_H
/******************************************************************************
fourier_hexagonal_iterator.h

begin		: 2006-06-06
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/iterator.h"
#include "pyxis/derm/sub_index_math.h"

/*!
PYXHexagonalIterator, iterates over a group of cells in such a way that
the cells form a larger hexagon when iteration is complete. This
iterator is used for collecting the cells in the specific way that
is required by the hexagonal fourier transform. This is done by calculating
the inverse function of phi for a specific 'm' to arrive at j,k basis vectors
which are then converted to a pyxis index.
*/
//! PYXHexagonalIterator, iterates over a group of cells to form a hexagon.
class PYXLIB_DECL PYXHexagonalIterator : public PYXIterator
{
public:

	//! Unit test method
	static void test();

	//! Default constructor.
	PYXHexagonalIterator(const PYXIcosIndex& index, const int nRadius);

	//! Destructor.
	virtual ~PYXHexagonalIterator();

	//! Move to the next cell.
	virtual void next();

	//! See if all the cells have been iterated over.
	virtual bool end() const;

	//! Get the current index being iterated over.
	const PYXIcosIndex& getIndex() const;

	//! Get the total number of cells to be iterated over. 
	int getTotalCells() {return m_nTotalCells;}

	//! Get the current value for K. 
	int getK() {return static_cast<int>(m_nK);}

	//! Get the current value for J. 
	int getJ() {return m_nJ;}

private: 

	//Test methods.
	static void testClassI(int nRadius); 
	static void testClassII(int nRadius); 
	static void testPentagons(int nRadius); 
	
	//! Convert J, K to a PYXIcosIndex anchored to centre of the hexagon.
	PYXIcosIndex convertToPyxIndex() const;

	//! Look up r and s for the correct digits of the PYXIndex.
	std::string jkToIndexLookUp(
		int r, int s, PYXMath::eHexClass hexClass) const;

	//! Set the value of M.
	void setM(int nM) { m_nCurrM = nM;}

	//! Calculate U for inverse of Phi.
	double calcU(int nTotCells = 0); 

	//! Calculate L for inverse of Phi.
	double calcL(int nTotCells = 0);

	//! Calculate the value of K.
	void calcK(); 

	//! Calculate the value of J. 
	void calcJ(); 

	//! Calculate a class one PYXIndex based on A,B.
	PYXIndex calcClassOneIndex(int nA, int nB, int nResolution) const;

	//! Calculate a class two PYXIndex based on A,B.
	PYXIndex calcClassTwoIndex(int nA, int nB, int nResolution) const;

	//! Move the index in the specified direction a number of times. 
	PYXIcosIndex moveIndex(
		const PYXIcosIndex& index, PYXMath::eHexDirection dir, int distance) const;
	
protected:

	//! Disable copy constructor
	PYXHexagonalIterator(const PYXHexagonalIterator&);

	//! Disable copy assignment
	void operator =(const PYXHexagonalIterator&);

private:

	/*!
	J according to the algorithm "The Inverse function of Phi" by Andy Vince.
	j is initalized to zero on construction and is 	calculated in on CalcJ,
	the value of j relies on the current M that is	being iterated over.
	It also relies on the value of K and the total numberof cells to be
	iterated over.
	*/ 
	int m_nJ; 

	/*!
	K according to the algorithm "The Inverse function of Phi" by Andy Vince.
	K is initalized to zero on construction. K is calculated on calcK and
	is the integer that lies between U and L. U and L are calculated on
	calcU and calcL. The value of k relies on the current m that is being
	iterated over as well as the radius of the hexagon and the total number
	of cells if the calculation of U and L relies on the total number of cells. 
	*/
	double m_nK; 

	/*!
	This is the radius or "n" in algorithm "The Inverse function of Phi"
	by Andy Vince. This value received as a parameter on construction
	of the the iterater and it does not change over the life time
	of this object. 
	*/
	int m_nRadius; 

	/*!
	M in the algorithm "The Inverse function of Phi" by Andy Vince.
	This value is intialized on construction to zero and is incremented
	by one on each call to next. The m will range between zero and 
	one minus the total number of cells. 
	*/
	int m_nCurrM; 

	/*!
	"N" in the algorithm "The Inverse function of Phi" by Andy Vince.
	This value is calculated on construction of the iterator,and 
	remains constant throughout the lifetime of the object. 
	*/
	int m_nTotalCells; 

	/*
	The PYXIcosIndex of the current cell, based on j,k that is being 
	iterated over. This index is intialized to the centre index of the 
	hexagon on construction. It changes on every call to getIndex. 
	This index must be mutable so it's value can change inside of 
	a constant function. 
	*/
	mutable PYXIcosIndex m_currentIndex; 

	/*
	Is a copy of the centre index provided on construction. This index
	is initalized on construction it's value is never changed throughout
	the lifetime of the iterater. It's used in convertToPyxIndex to
	add the newly calculated indices to the centre index. This results in 
	all all the indices calculated forming a hexagon shape once the iteration
	is complete.
	*/
	PYXIcosIndex m_copyIndex;
};

struct PYXLIB_DECL DivideByThreeResult {
	int aPrime;
	int bPrime;
	int r;
	int s;
	DivideByThreeResult(int a, int b)
	{
		aPrime = a / 3;
		bPrime = b / 3;
		r = a % 3;
		s = b % 3;

		if(s < 0) 
		{
			s += 3;
			--bPrime;
		}
		if(r < 0)
		{
			r += 3;
			--aPrime;
		}
		
		if (s == (r + 1))
		{
			s = s - 3;
			bPrime++;
		}
		else if(s == (r -1))
		{
			r = r - 3;
			aPrime++;
		}
		else if((r == s) && (s == 2))
		{
			r = -1;
			s = -1;
			aPrime++;
			bPrime++;
		}
	}
};

#endif	//guard
