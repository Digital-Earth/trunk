/******************************************************************************
fourier_diamond_iterator.cpp

begin		: 2006-06-05
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/fourier_diamond_iterator.h"

// pyxlib includes
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <cmath>

//! Constants.
const double kfScaleBase = 3; 
const double kfBaseSpatialCells = 9;

//! The unit test class
Tester<PYXFourierDiamondIterator> gTester;

/*!
The unit test method for the class.
*/
void PYXFourierDiamondIterator::test()
{

	PYXIcosIndex index = "C-00000000000000000"; 
	int nRadius = 4; 
	int count = 0; 
	PYXFourierDiamondIterator diamondIt(index,nRadius); 
	while(!diamondIt.end())
	{
		diamondIt.getIndex(); 
		diamondIt.next(); 
		count++;
	}

	TEST_ASSERT(count == diamondIt.getTotalCells()); 
}

/*!
Default Constructor.
*/
PYXFourierDiamondIterator::PYXFourierDiamondIterator(const PYXIcosIndex &index,
													  int nRadius):
	m_index(index)
{
	assert(!index.isNull() && "Centre index cannot be null."); 
	m_nRadius = nRadius; // Radius of the diamond. 
	m_fN = (pow(kfScaleBase,nRadius)); 
	m_nMaxJ = static_cast<int>((m_fN - 1) / 2);
	m_nMaxK = static_cast<int>((m_fN - 1) / 2);
	m_nMinJ = -m_nMaxJ; 
	m_nMinK= -m_nMaxK;
	m_nJ = m_nMinJ;
	m_nK = m_nMinK;
	m_nTotalCells = static_cast<unsigned int>((pow(kfBaseSpatialCells, nRadius))); 
}

/*!
Destructor.
*/
PYXFourierDiamondIterator::~PYXFourierDiamondIterator()
{
}

/*! Structure containing the values of APrime, Bprime 
    r and s,as well as a divide by three function to be used 
	in the algorithm for converting a j,k to a PYXIS index. 
*/
struct DivideByThreeResult{
	int aPrime;
	int bPrime;
	int r;
	int s;
	DivideByThreeResult( int a, int b)
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
		if(r <0) 
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

/*!
Moves to the next cell by adding one to k. Then ensuring that 
k hasn't exceeded the maximum k value. If so j is incremented 
and k is reset back to the minimum k value.
*/
void PYXFourierDiamondIterator::next() 
{
	// check to ensure adding one isn't going to be over the end. 
	if((m_nK == m_nMaxK)) 
	{
		m_nK = m_nMinK; 
		++m_nJ; 
	}
	else 
	{
		++m_nK; 
	}
}

/*!
Converts the base vectors j, k to a PYXIndex, by employing 
a divide by three algorithm and referencing a look up table 
by the quotients and modulos of the divide by three. Its important 
to note that where this algorithm various from the algorithm 
provided by Andy Vince. Some additions to the modulo values 
were made to allow the code logic to conform with the math 
logic. 

\return PYXIndex representing the raw index of a specified cell on a 
		particular face. 
*/
PYXIndex PYXFourierDiamondIterator::convertToPyxIndex() const
{
	std::stringstream ss; 
	int nRes = m_nRadius; 
	int m = 1; 
	int a = m_nJ - m_nK; 
	int b = m_nJ + m_nK; 
 	int test = 0; 
	std::string strIndex;  
	
	while (m <= nRes) 
	{
		DivideByThreeResult res( a, b);
		
		// The original algorithm used
		// if((res.s != 2 && res.s != -2) || (((res.s == 2 || res.s == -2) && res.aPrime % 3 == 0)))
		// which is redundant.
		if((res.s != 2 && res.s != -2) || res.aPrime % 3 == 0)
		{
			a = res.aPrime; 
			b = res.bPrime;
			
			strIndex = jkToIndexLookUp(res.r,res.s) + strIndex; 
		}
		
		else if((res.s == 2) && (res.aPrime % 3== 1 || res.aPrime == -2 ))
		{
			strIndex = "50" + strIndex; 
			a = res.aPrime - 1;
			b = res.bPrime + 1; 
		}
		else if((res.s == 2) &&  (res.aPrime % 3 == 2 || res.aPrime % 3 == -1))
		{
			strIndex = "30" + strIndex; 
			a = res.aPrime + 1; 
			b = res.bPrime + 1; 
		}
		
		else if((res.s == -2) &&  (res.aPrime % 3 == -1 || res.aPrime % 3 ==2))
		{
			strIndex = "20" + strIndex; 
			a = res.aPrime + 1; 
			b = res.bPrime - 1;
		}
		
		else if((res.s == -2) && (res.aPrime % 3 ==1 || res.aPrime % 3 == -2))
		{
			strIndex = "60" + strIndex; 
			
			a = res.aPrime - 1; 
			b = res.bPrime - 1; 
		}
		m++; 
	}
		
	return PYXIndex(strIndex); 
}

/*
Looks up a cell index based on the modulus of the divide by three 
algorithm. A string is returned in this case as opposed to an int 
to allow for an index to be "00". Where an int would only return a "0". 

\return A string representation of a cell index. 
*/
std::string PYXFourierDiamondIterator::jkToIndexLookUp(int r, int s) const
{
	if (r == 0 && s == 0)
	{
		return "00"; 
	}
	else if (r == 1 && s == 1) 
	{
		return "01"; 
	}
	else if (r == -1 && s ==1)
	{
		return "02"; 
	}
	else if (r == -2 && s == 0)
	{
		return "03"; 
	}
	else if (r == -1 && s == -1) 
	{
		return "04"; 
	}
	else if (r == 1 && s == -1)
	{
		return "05"; 
	}
	else if (r == 2 && s == 0)
	{
		return "06";
	}
	else if (r == 0 && s == 2)
	{
		return "10"; 
	}
	else if (r == 0 && s == -2)
	{
		return "40"; 
	}
	return ""; 
}

/*!
Check to see if all the cells have been iterated over by 
determining if J is equal to or greater then the maximum 
J. 

\return true if all the cells have been iterated over, false otherwise. 
*/
bool PYXFourierDiamondIterator::end() const
{
	return m_nJ > m_nMaxJ ? true : false; 
}

/*!
Get the PYXIcosIndex that is represented by the current values of j,k.
The primary resolution of the centre index, the proper formating and 
then PYXIndex is read into a string stream to enable constructing a 
string representation of a PYXIcosIndex. A PYXIcosIndex is then 
constructed from the string representation. 

\return PYXIcosIndex that correspondes to j, k anchored on the same face
		that the centre index has.
*/
const PYXIcosIndex& PYXFourierDiamondIterator::getIndex() const
{
	PYXIndex calcIndex = convertToPyxIndex() ;
	PYXIndex subIndex = m_index.getSubIndex();
	std::stringstream ss; 
	std::string strIcosIndex; 
	PYXIndex sumIndex; 

	//Ensure that indices are at the same resolution before adding them. 
	while(calcIndex.getResolution() < subIndex.getResolution()) 
	{
		calcIndex.prependDigit(0); 
	}
	
	PYXMath::add(subIndex, calcIndex,subIndex.getResolution(), &sumIndex, false); 

	//Reassemble PYXIcosIndex in a String stream. 
	ss << m_index.getPrimaryResolution();
	ss << "-"; 
	ss << sumIndex.toString();
	ss >> strIcosIndex;  // Dump index to a string. 

	//Required to return a reference to an PYXIcosIndex. 
	m_currentIndex = PYXIcosIndex(strIcosIndex);

	return m_currentIndex; 
}

/*!
Returns a structure containing the current values of J, K as an ordered pair. 

\return jk A structure containing J,K
*/
jkPair PYXFourierDiamondIterator::getJK()
{
	jk.nJ = m_nJ; 
	jk.nK = m_nK; 
	return jk; 
}
