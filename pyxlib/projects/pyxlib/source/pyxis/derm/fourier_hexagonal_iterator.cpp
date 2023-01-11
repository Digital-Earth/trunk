/******************************************************************************
hexagonal_iterator

begin		: 2006-06-06
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/fourier_hexagonal_iterator.h"

// pyxlib includes
#include "pyxis/derm/neighbour_iterator.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <cmath>

//! The unit test class
Tester<PYXHexagonalIterator> gTester;

/*!
The unit test method for the class.
*/
void PYXHexagonalIterator::test()
{
	PYXIcosIndex aNullIndex;
	PYXIcosIndex aValidIndex = "A-02000"; 
	int nABigRadius = 30; 
	int nASmallRadius = -20; 
	int nZeroRadius = 1; 
	int nRaidus = 10; 

	PYXHexagonalIterator::testClassI(10);
	PYXHexagonalIterator::testClassII(10);
	PYXHexagonalIterator::testPentagons(10);
}

//Used only for testing.
void PYXHexagonalIterator::testPentagons(int nRadius)
{
	PYXIcosIndex index = "5-2010000000";
	PYXHexagonalIterator testIterator(index, nRadius);
	int nCellCounter = 0;
	std::vector<PYXIcosIndex> vecIndices; 
	bool bIndexExists = false; 
	
	while(!testIterator.end())
	{
		PYXIcosIndex pushBackIndex = testIterator.getIndex();
		for (int vecIndex = 0; vecIndex < static_cast<signed int> (vecIndices.size()); ++vecIndex)
		{
			if (vecIndices[vecIndex] == pushBackIndex) 
			{
				bIndexExists = true; 
				break; 
			}
		}
		if (!bIndexExists) 
		{
			vecIndices.push_back(pushBackIndex); 
		}
		
		testIterator.m_nJ = testIterator.m_nJ +
			static_cast<int>(testIterator.m_nK);
		TEST_ASSERT(testIterator.m_nK <= nRadius);
		TEST_ASSERT(testIterator.m_nJ <= nRadius);
		TEST_ASSERT((testIterator.m_nJ - testIterator.m_nK) <= nRadius);
		++nCellCounter;
		testIterator.next();
	}
	TEST_ASSERT(nCellCounter == vecIndices.size()); 

}

//Used only for testing.
void PYXHexagonalIterator::testClassI(int nRadius)
{
	PYXIcosIndex index = "C-0602000400000";  
	PYXIcosIndex startIndex(index); 

	std::vector<PYXIcosIndex> vecIndices; 
	std::vector<PYXIcosIndex> vecTestIndices;
	bool bIndexExists = false; 
	int nCount = 0; 
	PYXHexagonalIterator hexIt(index,nRadius); 
	PYXMath::eHexDirection movDir = PYXMath::knDirectionZero; 
	int nDir = 0; 

	for(int n = 0; n < nRadius; ++n)
	{
		for (int nDir = 1; nDir < 7; nDir++) 
		{
			if (n > 0) 
			{
				++nDir; 
				PYXMath::eHexDirection direction(
					static_cast<PYXMath::eHexDirection>(nDir)	);
				startIndex = hexIt.moveIndex(index, direction, n);
			}

			PYXNeighbourIterator itNeighbour(startIndex);

			while (!itNeighbour.end())
			{
				PYXIcosIndex pushBackIndex = itNeighbour.getIndex(); 
				bIndexExists = false; 
				for (	int vecIndex = 0;
						vecIndex<static_cast<signed int>(vecIndices.size());
						++vecIndex	)
				{
					if (vecIndices[vecIndex] == pushBackIndex) 
					{
						bIndexExists = true; 
					}
				}
				if (!bIndexExists)
				{
					vecIndices.push_back(pushBackIndex); 
				}
				itNeighbour.next();
			}
		}
	}

	bool bIndex = false; 
	int nCellCounter = 0; 
	while (!hexIt.end())
	{	
		PYXIcosIndex pushBackIndex(hexIt.getIndex());

		bIndex = false;
		
		for (	int vecIndex = 0;
				vecIndex<static_cast<signed int>(vecTestIndices.size());
				++vecIndex	)
		{
			if (vecTestIndices[vecIndex] == pushBackIndex) 
			{
				bIndex = true; 
				break; 
			}
		}
		if (!bIndex)
		{
			vecTestIndices.push_back(pushBackIndex);
		}
		hexIt.m_nJ = hexIt.m_nJ + static_cast<int>(hexIt.m_nK);
		TEST_ASSERT(hexIt.m_nK <= nRadius); 
		TEST_ASSERT(hexIt.m_nJ <= nRadius); 
		TEST_ASSERT((hexIt.m_nJ - hexIt.m_nK) <= nRadius);
		++nCellCounter;
		hexIt.next(); 
	}

	TEST_ASSERT(hexIt.getTotalCells() == nCellCounter);

	bool bHasIndex = false; 
	int nCounter =0; 

	for(int vecIndex = 0; vecIndex < 
		static_cast<signed int>(vecIndices.size()); ++vecIndex) 
	{
		bHasIndex = false; 
		for(int testVecIndex = 0; testVecIndex < 
			static_cast<signed int> (vecTestIndices.size()); ++testVecIndex)
		{
			if (vecIndices[vecIndex] == vecTestIndices[testVecIndex]) 
			{
				bHasIndex = true;  
				break;
			}
			else 
			{
				bHasIndex = false; 
			}
		}
		if(!bHasIndex) 
		{
			nCounter++; 
		}
		TEST_ASSERT(bHasIndex == true); 
	}
}

//Used only for testing.
void PYXHexagonalIterator::testClassII(int nRadius)
{
	PYXIcosIndex index = "C-06020004000000";  
	PYXIcosIndex startIndex(index); 

	std::vector<PYXIcosIndex> vecIndices; 
	std::vector<PYXIcosIndex> vecTestIndices;
	bool bIndexExists = false; 
	int nCount = 0; 
	PYXHexagonalIterator hexIt(index, nRadius); 
	PYXMath::eHexDirection movDir = PYXMath::knDirectionZero; 
	int nDir = 0; 

	for(int n = 0; n < nRadius; ++n)
	{
		for (int nDir = 1; nDir < 7; nDir++) 
		{
			if (n > 0) 
			{
				++nDir; 
				PYXMath::eHexDirection direction = static_cast<
					PYXMath::eHexDirection>(nDir); 
				startIndex = hexIt.moveIndex(index, direction, n);
			}

			PYXNeighbourIterator itNeighbour(startIndex);
			while (!itNeighbour.end())
			{
				PYXIcosIndex pushBackIndex = itNeighbour.getIndex(); 
				bIndexExists = false; 
				for (	int vecIndex = 0;
						vecIndex<static_cast<signed int>(vecIndices.size());
						++vecIndex	)
				{
					if (vecIndices[vecIndex] == pushBackIndex) 
					{
						bIndexExists = true; 
					}
				}
				if(!bIndexExists) 
				{
					vecIndices.push_back(pushBackIndex); 
				}
				itNeighbour.next();
			}
		}
		
	}
	bool bIndex = false; 
	int nCellCounter = 0; 
	while (!hexIt.end())
	{	
		PYXIcosIndex pushBackIndex(hexIt.getIndex());

		bIndex = false ;
		
		for(int vecIndex = 0; vecIndex <
					static_cast<signed int>(vecTestIndices.size()); ++vecIndex) 
		{
				if (vecTestIndices[vecIndex] == pushBackIndex) 
				{
					bIndex = true; 
					break; 
				}
		}
		if (!bIndex)
		{
			vecTestIndices.push_back(pushBackIndex);
		}
		
		hexIt.m_nJ = hexIt.m_nJ + static_cast<int>(hexIt.m_nK);
		TEST_ASSERT(hexIt.m_nK <= nRadius); 
		TEST_ASSERT(hexIt.m_nJ <= nRadius); 
		TEST_ASSERT((hexIt.m_nJ - hexIt.m_nK) <= nRadius);
		++nCellCounter;
		hexIt.next(); 
	}

	TEST_ASSERT(hexIt.getTotalCells() == nCellCounter);

	bool bHasIndex = false; 
	int nCounter =0; 

	for(int vecIndex = 0; vecIndex < 
		static_cast<signed int>(vecIndices.size()); ++vecIndex) 
	{
		bHasIndex = false; 
		for(int testVecIndex = 0; testVecIndex < 
			static_cast<signed int> (vecTestIndices.size()); ++testVecIndex)
		{

			if (vecIndices[vecIndex] == vecTestIndices[testVecIndex]) 
			{
				bHasIndex = true;  
				break;
			}
			else 
			{
				bHasIndex = false; 
			}
		}
		if(!bHasIndex) 
		{
			nCounter++; 
		}
		TEST_ASSERT(bHasIndex == true); 
	}
}

/*!
Default Constructor.
*/
PYXHexagonalIterator::PYXHexagonalIterator(
	const PYXIcosIndex& index, const int nRadius):
	m_nRadius(nRadius),
	m_currentIndex(index),
	m_nCurrM(0),
	m_copyIndex(index),
	m_nJ(0),
	m_nK(0)
{
	assert(!index.isNull() && "Centre index cannot be null.");
	assert(nRadius <= 42 && "Radius cannot exceed 42.");
	assert(nRadius >= 1 && "Cannot have a radius less then 1."); 
	  
	m_nTotalCells = 3 * static_cast<unsigned int>(
		pow(static_cast<double>(m_nRadius), 2)) + 3 * m_nRadius + 1;

	calcK();
	calcJ();
}

/*!
Destructor.
*/
PYXHexagonalIterator::~PYXHexagonalIterator()
{
} 

/*! 
Advances the iteration to the next cell by incrementing M by one.
after the incrementing M new values for J, K are calculated
based on the new value of M. J, K are then adjusted so
they can be converted using the table for conversion provided
by Andy Vince.
*/
void PYXHexagonalIterator::next()
{
	++m_nCurrM;
	calcK();
	calcJ();

	m_nK = static_cast<int>(m_nK);
	m_nJ = m_nJ - static_cast<int>(m_nK);
}

/*!
Checks to determine if all the cells have been iterated over.
All the cells have been iterated over when the value for
M is one less then the total number of the cells.

\return		A boolean value indicating whether all the cells have
			been iterated over. 
*/
bool PYXHexagonalIterator::end() const
{
	return m_nCurrM < m_nTotalCells ? false : true; 
}

/*!
Calculates the value of U, which is a value used in the
calculation of the inverse of Phi. U is calculated as the minimum
value of three possible values. There are two different methods
for calculating U. The first involves the formula's listed 
below. The second is a variation on the formula's where the 
total number of cells to be iterated over is factored in.
One implemention with a default parameter of zero is 
sufficiant if the variation on the forumla is required the 
caller only needs provide a parameter. 

Formulas.
	 1. ((n - m) / 3n + 2)
	 2. ((n - m) / 3n + 1)
	 3. n. 

Variation. 
	1. ((n - m + N) / 3n + 2)

Where.
	n = Radius of the hexagon. 
	m = The current m of of the iteration. 
	N = Total number of cells to be iterated over. 

\param nTotCells	The total number of cells to be iterated over. 
					by default parameter is zero. 

 \return	The minimum value of the three calculations 
			listed above. 
*/
double PYXHexagonalIterator::calcU(int nTotCells) 
{
	double nVal_1 = ((m_nRadius - m_nCurrM) + nTotCells) /
		(3.0 * m_nRadius + 2);
	double nVal_2 = ((m_nRadius - m_nCurrM) + nTotCells) /
		(3.0 * m_nRadius + 1);

	double nMinVal = std::min<double>(nVal_1, nVal_2);
	nMinVal = std::min<double>(nMinVal, m_nRadius);

	return nMinVal;
}

/*!
Calculates the value of L, which is a value used in the
calculation of the inverse of Phi. L is calculated as the maximum
value of three possible values. There are two different methods
for calculating L. The first involves the formula's listed 
below. The second is a variation on the formula's where the 
total number of cells to be iterated over is factored in.
One implemention with a default parameter of zero is 
sufficiant if the variation on the forumla is required the 
caller only needs provide a parameter. 

Formulas.
	 1. ((-n - m) / 3n + 2)
	 2. ((-n - m) / 3n + 1)
	 3. -n. 

Variation. 
	1. ((-n - m + N) / 3n + 2)

Where.
	n = Radius of the hexagon. 
	m = The current m of of the iteration. 
	N = Total number of cells to be iterated over. 

\param nTotCells	The total number of cells to be iterated over. 
					by default parameter is zero. 

 \return	The maximum value of the three calculations 
			listed above. 
*/
double PYXHexagonalIterator::calcL(int nTotCells) 
{
	double nVal_1 =(((-1.0 * m_nRadius) - m_nCurrM) + nTotCells) /
			(3.0 * m_nRadius + 2);
	double nVal_2 = (((-1.0 * m_nRadius) - m_nCurrM) + nTotCells) /
			(3.0 * m_nRadius + 1);

	double nMaxVal = std::max(nVal_1, nVal_2);
	nMaxVal = std::max(nMaxVal, (-1.0 * m_nRadius));

	return nMaxVal;
}

/*!
Calculates the value of K by calculating the inverse function
of Phi. Result of this calculation is the integer which lies
between L and U. L and U are without the variation of the
formula. If there is an integer that statifies the condition
then that is K. If there is no integer then L and U are
recalculated with the variation on the forumla.
K is now the integer the falls between L and U.
*/
void PYXHexagonalIterator::calcK() 
{
	double nL = calcL();
	double nU = calcU();
	double nLPrime = calcL(m_nTotalCells);
	double nUprime = calcU(m_nTotalCells);
	int nTempK = 0;
	
	nTempK = nL < 0 ? static_cast<int> (nL) : static_cast<int> (nU);
	if (nTempK >= nL && nTempK <= nU)
	{
		m_nK =  nTempK;
	}
	else
	{
		m_nK = static_cast<int>(calcL(m_nTotalCells)) >= calcL(m_nTotalCells) ? 
			static_cast<int>(calcL(m_nTotalCells)) : static_cast<int>(calcU(m_nTotalCells));
	}
}

/*
A helper method to calculate a class two PYXIndex. The index is calculated
by using a divide by three algorithm to calculate two quotients and 
two remainders, which are based on the values of J, K. These 
quotients and remainders are then applied to a look up table to determine 
the appropriate two digits to add to the index. The quotients are then 
adjusted so the next divide by three will provide another set of 
quotients and remainders that when applied to the look up table 
will result in the appropriate digits added to the index. The algorithm
will loop through this caluclation for the resolution / 2 times. 

\param nA	The value A required for divide by three. 
\param nB	The value B required for divide by three. 
\param nResolution	The resolution of the PYXIndex returned. 

\return A class two PYXIndex. 
*/
PYXIndex PYXHexagonalIterator::calcClassTwoIndex(
	int nA, int nB, int nResolution) const
{
	int m = 0; 
	std::string strIndex = ""; 

		while (m <= nResolution)
		{
 			DivideByThreeResult res(nA, nB);

			if ((res.s != 2 && res.s != -2) || res.aPrime % 3 == 0)
			{
				nA = res.aPrime;
				nB = res.bPrime;
				strIndex = jkToIndexLookUp(res.r,res.s,PYXMath::knClassII) + strIndex;
			}
			else if ((res.s == 2) && (res.aPrime % 3 == 1 || res.aPrime % 3 == -2 ))
			{
				strIndex = "60" + strIndex;
				nA = res.aPrime - 1;
				nB = res.bPrime + 1;
			}
			else if ((res.s == 2) &&  (res.aPrime % 3 == 2 || res.aPrime % 3 == -1))
			{
				strIndex = "40" + strIndex;
				nA = res.aPrime + 1;
				nB = res.bPrime + 1;
			}
			else if ((res.s == -2) &&  (res.aPrime % 3 == -1 || (res.aPrime % 3 == 2)))
			{
				strIndex = "30" + strIndex;
				nA = res.aPrime + 1;
				nB = res.bPrime - 1;
			}		
			else if (res.r == 0 && res.s == 2 && res.aPrime == -1)
			{
				strIndex =  "20" + strIndex;
				nA = res.aPrime;
				nB = res.bPrime;
			}	
			else if (res.r == 0 && res.s == -2 && res.aPrime % 3 == -1)
			{
				strIndex =  "50" + strIndex;
				nA = res.aPrime + 1;
				nB = res.bPrime + 1;
			}	
			else if ((res.s == -2) && (res.aPrime % 3 == 1 || res.aPrime % 3 == -2))
			{
				strIndex = "10" + strIndex;
				nA = res.aPrime - 1;
				nB = res.bPrime - 1;
			}
			m++;
		}
		return PYXIndex(strIndex); 
}

/*
A helper method to calculate a class one PYXIndex. The index is calculated
by using a divide by three algorithm to calculate two quotients and 
two remainders, which are based on the values of J, K. These 
quotients and remainders are then applied to a look up table to determine 
the appropriate two digits to add to the index. The quotients are then 
adjusted so the next divide by three will provide another set of 
quotients and remainders that when applied to the look up table 
will result in the appropriate digits added to the index. The algorithm
will loop through this caluclation for the resolution / 2 times. 

\param nA	The value A required for divide by three. 
\param nB	The value B required for divide by three.
\param nResolution	The resolution of the PYXIndex returned.

\return A class two PYXIndex.
					
*/
PYXIndex PYXHexagonalIterator::calcClassOneIndex(
	int nA, int nB, int nResolution) const
{
		int m = 0; 
		std::string strIndex = ""; 
 		while (m <= nResolution)
		{
			DivideByThreeResult res(nA, nB);
			if((res.s != 2 && res.s != -2) || res.aPrime % 3 == 0)
			{
				nA = res.aPrime;
				nB = res.bPrime;
				strIndex = jkToIndexLookUp(res.r,res.s,PYXMath::knClassI) + strIndex;
			}
			else if((res.s == 2) && (res.aPrime % 3 == 1 || res.aPrime % 3 == -2))
			{
				strIndex = "50" + strIndex;
				nA = res.aPrime - 1;
				nB = res.bPrime + 1;
			}
			else if ((res.s == 2) &&  (res.aPrime % 3 == 2 || res.aPrime % 3 == -1))
			{
				strIndex = "30" + strIndex;
				nA = res.aPrime + 1;
				nB = res.bPrime + 1;
			}
			else if ((res.s == -2) &&  (res.aPrime % 3 == -1  || res.aPrime % 3 == 2))
			{
				strIndex = "20" + strIndex;
				nA = res.aPrime + 1;
				nB = res.bPrime - 1;
			}
			else if ((res.s == -2) && (res.aPrime % 3 == 1 || res.aPrime % 3 == -2))
			{
				strIndex = "60" + strIndex; 
				nA = res.aPrime - 1;
				nB = res.bPrime - 1;
			}
			m++;
		}
		return PYXIndex(strIndex); 
}


/*
Converts the current values of J, K to a PYXIcosIndex by determining 
which class of index needs to be calculated. Converting J, K to 
A and B, so a divide by three algorithm can be employed. After 
the calculation to A, B the actual calcuation of the index 
is delegated to helper methods based on the class of index that needs 
to be calculated. After calculating a PYXIndex the retuned PYXIndex 
is adjusted in resolution and added to the index at the centre of the 
hexagon to provide an index that is anchored to the middle of the hexagon. 
This index is turned into a string and an IcosIndex is built up from the 
string. 

\return A PYXIcosIndex based on the values of J, K. 
*/
PYXIcosIndex PYXHexagonalIterator::convertToPyxIndex() const
{

	PYXMath::eHexClass indexClass = PYXMath::getHexClass(
		m_currentIndex.getResolution());
	int a = m_nJ - static_cast<int>(m_nK);
	int b = m_nJ + static_cast<int>(m_nK);
	
	//Calculates how many PYXDigits comprise an Index. 
	int nDigits = static_cast<int>(log(static_cast<double>(m_nTotalCells) / log(9.0))) +1;

	PYXIndex tempIndex = (indexClass == PYXMath::knClassI) ? 
		calcClassOneIndex(a, b, nDigits) : calcClassTwoIndex(a, b, nDigits);

	PYXIndex subIndex = m_copyIndex.getSubIndex(); 
	PYXIndex sumIndex; 
	while(tempIndex.getResolution() < subIndex.getResolution()) 
	{
		tempIndex.prependDigit(0); 
	}
	PYXMath::add(subIndex, tempIndex, subIndex.getResolution(), &sumIndex, false);
	std::stringstream ss;
	std::string strNewIndex;

	// Assemble return index using strings.
	int primRes = m_copyIndex.getPrimaryResolution();
	ss <<  primRes;
	ss << "-";
	ss << sumIndex.toString();
	ss >> strNewIndex;

	return PYXIcosIndex(strNewIndex);
}

/*!
Looks up the appropriate pair of digits to add into the PYXIndex based 
on the values of r and s, as well as if the index being contstruced 
is a class one or a class two index. 

\param r	The remainder r from divide by three algorithm. 
\param s	The remainder s from divide by three algorithm. 
\param hexClass		The class of the index that is being constructed. 

\return	 A string containing a pair of digits based on r and s to be 
		 added to the PYXIndex. 
*/
std::string PYXHexagonalIterator::jkToIndexLookUp(int r, int s, PYXMath::eHexClass hexClass) const
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
		if (hexClass == PYXMath::knClassI)
		{
			return "10";
		}
		else
		{
			return "20";
		}
	}
	else if (r == 0 && s == -2)
	{
		if (hexClass == PYXMath::knClassI)
		{
			return "40";
		}
		else
		{
			return "50";
		}
	}
	return "";
}

/*
Calculates a value of J by calculating a sum based on the current m,
the radius and the current value of k. The value for j is then that
sum mod the total number of cells. In some instances a negative value
of mod was required however the modulus operater only returned the positive
equivalent of the value needed. Therefore it was nesecarry to add a subtraction
to achieve the negative value equivalent to the positive value provided by the
modulus operater.
*/
void PYXHexagonalIterator::calcJ()
{
	int sum = static_cast<int>((m_nCurrM + ((3 * m_nRadius) + 2) * m_nK));
	if (sum == 0) 
	{
		m_nJ = 0;
	}
	else
	{
		m_nJ = sum >= (m_nTotalCells - m_nRadius) ?
			sum - m_nTotalCells : sum % m_nTotalCells;
	}
}

/*!
Gets the current PYXIcosIndex of the cell that is being iterated over.
Delegates to convertToPyxIndex to convert the current values of j,k
to a PYXIcosIndex. 

\return The current PYXIcosIndex of the cell that is being iterated over.
*/
const PYXIcosIndex& PYXHexagonalIterator::getIndex() const 
{
	m_currentIndex = convertToPyxIndex();
	return m_currentIndex;
}

/*!
Moves an index a specified number of times in a particular direction.
Since it is not currently possibly to move more then one cell at a time in
any given direction due to the problems incurred with crossing tesselations,
this is accomplished by moving one move at a time, for x times in a 
direction. Will throw an exception if the move crosses a tesselation.

\param index		The starting index to move from.
\param dir			The direction to move in.
\param nDistance	The number of times to move in the specified direction.

\return A new PYXIcosIndex which has been moved in the requested direction
		the requested number of times. 
*/
PYXIcosIndex PYXHexagonalIterator::moveIndex(
	const PYXIcosIndex& index,PYXMath::eHexDirection dir, int nDistance) const
{
	PYXIcosIndex rtnIndex(index);

	for(int nTimesMoved = 0; nTimesMoved < nDistance; ++nTimesMoved)
	{
		rtnIndex = PYXIcosMath::move(rtnIndex, dir);

		if (rtnIndex.getPrimaryResolution() != index.getPrimaryResolution())
		{
			PYXTHROW(PYXException, "Move crosses a tesselation.");
		}
	}
	return rtnIndex;
}
