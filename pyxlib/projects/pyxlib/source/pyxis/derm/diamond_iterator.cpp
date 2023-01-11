/******************************************************************************
diamond_iterator.cpp

begin		: 2006-06-22
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/diamond_iterator.h"

// pyxlib includes
#include "pyxis/derm/index_math.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <cmath>

//! The unit test class
Tester<PYXDiamondIterator> gTester;

/*!
The unit test method for the class.
*/
void PYXDiamondIterator::test()
{
	// TODO: Improve this unit test to ensure that the "diamond" is traversed
	// in the correct order, and that the appropriate number of cells are 
	// returned.

	PYXIcosIndex index = "C-0200000"; 
	PYXIcosIndex pentIndex = "5-20000000"; 
	PYXIcosIndex startIndex = "C-0204040";  
	PYXIcosIndex pentStart = "5-20000404";
	int nRes = 2; 
	int nCellCount = 0; 

	PYXDiamondIterator it(index,nRes); 
	TEST_ASSERT(startIndex == it.getIndex()); 
	while (!it.end())
	{
		it.next(); 
		++nCellCount;
	}
	
	TEST_ASSERT(nCellCount == it.getTotalCells()); 

	PYXDiamondIterator diaIt(pentIndex,nRes); 
	nCellCount = 0; 
	TEST_ASSERT(pentStart == diaIt.getIndex()); 
	while (!diaIt.end())
	{
		//TRACE_INFO(""<<diaIt.getIndex()); 
		diaIt.next(); 
		++nCellCount;
	}

	TEST_ASSERT(nCellCount == it.getTotalCells()); 
}

/*!
Default Constructor.
*/
PYXDiamondIterator::PYXDiamondIterator(const PYXIcosIndex& index,int res):
	m_startIndex(index),
	m_nTotalCells(static_cast<int>(pow(9.0,res))),  
	m_nCount(0), 
	m_currentIndex(m_startIndex),
	m_diagIndex(m_startIndex),
	m_nDiag(0),
	m_nDiagDirect(PYXMath::knDirectionZero)
{ 
	assert(!index.isNull() && "Centre index cannot be null.");
	std::string strIndex;
	m_fRowLength = sqrt(static_cast<double>(m_nTotalCells));
	PYXIndex subIndex = m_startIndex.getSubIndex();
	PYXIndex resIndex;
	PYXMath::eHexClass type = PYXMath::getHexClass(m_startIndex.getResolution());
	m_nDiagDirect = (type == PYXMath::knClassI) ? 
		PYXMath::knDirectionOne : PYXMath::knDirectionSix;
	
	for (int n = 0; n < res; ++n)
	{
		strIndex = (type == PYXMath::knClassI) ?
			("40" + strIndex) : ("04" + strIndex);
	}

	PYXIndex aIndex(strIndex); 
	PYXMath::add(
		subIndex, aIndex, m_startIndex.getResolution(), &resIndex, false);

	std::stringstream ss;
	ss << m_startIndex.getPrimaryResolution();
	ss << "-";
	ss << resIndex.toString();
	ss >> strIndex;
	m_startIndex = PYXIcosIndex(strIndex);
	m_currentIndex = m_startIndex;
	m_diagIndex = m_startIndex;
}

/*!
Destructor.
*/
PYXDiamondIterator::~PYXDiamondIterator()
{
}

/*!
Advances to the next cell by performing a move in the two direction and returning 
the resulting index. It is only possible to move in the two direction 
sqrt(totalCells) times. Once it is not possible to move in the two direction 
any longer. A move in the diagonal direction is peformed on the index at the 
begining of the row. The diagonal direction is determined by the class of the
hexagon. The diagonal direction will be 1 for class 1 and 6 for class 2 hexagons. 
Counter is reset and the index is advanced by moving in the
two direction again. This works for pentagonal cells because pentagons are always 
missing direction 1 or 4. 
*/
void PYXDiamondIterator::next() 
{
	if (m_nCount < (m_fRowLength-1)) 
	{
		m_nCount++; 
		m_currentIndex = PYXIcosMath::move(m_currentIndex, PYXMath::knDirectionTwo);
	}
	else 
	{
		m_nDiag++; 
		m_nCount = 0; 
		m_currentIndex = PYXIcosMath::move(m_diagIndex, m_nDiagDirect);
		m_diagIndex = PYXIcosMath::move(m_diagIndex, m_nDiagDirect);
	}
}

/*
Determines if the end of the diamond has been reached by determining if the
currentdiagonal row  number is greater then or equal sqrt(totalCells).
If it is greater or equal to then the end of the diamond has been reached
and all cells have been covered. 

\return true or false indicating whether all cells have been 
		iterated over or not. 
*/
bool PYXDiamondIterator::end() const 
{
	return m_nDiag < m_fRowLength ? false : true; 
}

/*!
Gets the current index that the iterator is currently on. 

\return The current index of the iterator. 
*/
const PYXIcosIndex& PYXDiamondIterator::getIndex() const
{
	return m_currentIndex; 
}
