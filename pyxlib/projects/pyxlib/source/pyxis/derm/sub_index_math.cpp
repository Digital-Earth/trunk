/******************************************************************************
sub_index_math.cpp

begin		: 2003-12-03
copyright	: (C) 2003 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/sub_index_math.h"

// pyxlib includes
#include "pyxis/derm/exceptions.h"
#include "pyxis/derm/hexagon.h"
#include "pyxis/utility/coord_polar.h"
#include "pyxis/utility/math_utils.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <algorithm>
#include <cassert>
#include <cmath>
#include <string>

//! The digit mask for values in the lookup table
static const int knDigitMask = 0xF;

//! The number of bits per digit for values in the lookup table
static const int knBitsPerDigit = 4;

//! The maximum absolute resolution - must match # digits in PYXIndex
const int PYXMath::knMaxAbsResolution = 40;

//! The maximum relative resolution
/*
This is limited by the maximum number of cells that can fit into an unsigned
integer.
*/
const int PYXMath::knMaxRelResolution = 18;

//! The distance between hexagons at resolution zero
const double PYXMath::kfResZeroIntercell = 1.0;

//! Resolution size vectors
std::vector<double> PYXMath::m_vecInterCellDistances;

//! Multiplication factors
static const unsigned int pnMultiplicationFactors[7] =
{
	0x0, 0x104, 0x205, 0x306, 0x401, 0x502, 0x603
};

//! Tester class
Tester<PYXMath> gTesterA;

//! Test method
void PYXMath::test()
{
	PYXIndex		index1;
	PYXIndex		index2;
	PYXIndex		index3;
	FactorVector	vecFactors;

	// test the add and multiply at the same time
	eHexDirection	nDirection;
	int				nDirCounter;
	int				nMultiple;
	int				nResolution;

	// set the test values to 0 index at resolution 10
	nResolution = 10;
	
	// choose a number of test calculations to make
	const int nTestCount = 50;

	// loop through and compare the add value with the multiply value
	for (nDirCounter = 1; nDirCounter <= Hexagon::knNumSides; nDirCounter++)
	{
		index1.reset();
		index1.setResolution(nResolution);
		nDirection = static_cast<eHexDirection>(nDirCounter);

		for (nMultiple = 1; nMultiple < nTestCount; nMultiple++)
		{
			move(index1, nDirection, &index1);
			multiply(nMultiple, nDirection, nResolution, &index2);
			TEST_ASSERT(index2 == index1);

			// test the factor method
			factor(index1, vecFactors);
			index3.reset();
			index3.setResolution(nResolution);

			int	nIndex;
			for (nIndex = 0; nIndex < Hexagon::knNumSides; nIndex++)
			{
				multiply(	vecFactors[nIndex], 
							static_cast<eHexDirection>(nIndex + 1),
							nResolution,
							&index2	);
				add(index2, index3, nResolution, &index3);
			}

			TEST_ASSERT(index3 == index1);
		}
	}

	// test the add of two digits that can grow
	index1 = "403";
	index2 = "404";
	
	try
	{
		PYXIndex sum;
		add(index1, index2, 3, &sum, false);
		// should have thrown an exception since the number 
		// was not allowed to grow
		TEST_ASSERT(false);
	}
	catch (PYXException&)
	{
		// should be here so we won't throw and kill testing
		TEST_ASSERT(true);
	}

	// test the add of two digits that can't grow

	try
	{
		PYXIndex sum;
		add(index1, index2, 3, &sum, true);
		TEST_ASSERT(true);
	}
	catch (PYXException&)
	{
		// should not have thrown an exception because 
		// the number was allowed to grow
		TEST_ASSERT(false);
	}

	// test the zoomIn and zoomOut methods
	index1 = "403";

	try
	{
		zoomIn(&index1);
		index2 = "4030";
		TEST_ASSERT(index1 == index2);
	}
	catch (PYXException&)
	{
		TEST_ASSERT(false);
	}

	try
	{
		zoomOut(&index1);
		index2 = "403";
		TEST_ASSERT(index1 == index2);
	}
	catch (PYXException&)
	{
		TEST_ASSERT(false);
	}

	// test calc descendant index
	PYXIndex parent = "10203";
	PYXIndex child = "30201";

	try
	{
		calcDescendantIndex(parent, child);

		// expecting exception since not a descendant
		TEST_ASSERT(false);
	}
	catch (PYXException&)
	{
		// ignore
	}

	child = "10203040506";
	PYXIndex relativeIndex = calcDescendantIndex(parent, child);
	TEST_ASSERT(PYXIndex("040506") == relativeIndex);

	// test the rotation method
	index1 = "010203040506";
	index2 = "060102030405";
	PYXIndex rotIndex1;
	PYXIndex rotIndex2;

	rotIndex1 = index1;
	rotateIndex(&rotIndex1, 1, knCW);
	TEST_ASSERT(rotIndex1 == index2);

	rotIndex1 = index2;
	rotateIndex(&rotIndex1, 1, knCCW);
	TEST_ASSERT(index1 == rotIndex1);

	// rotate the same number 3 in both directions
	rotIndex1 = index1;
	rotateIndex(&rotIndex1, 3, knCCW);

	rotIndex2 = index1;
	rotateIndex(&rotIndex2, 3, knCW);

	// should have come out to be the same regardless of dir
	TEST_ASSERT(rotIndex1 == rotIndex2);

	// test the rotate direction with both positive and negative values
	TEST_ASSERT(rotateDirection(knDirectionOne, -11) == knDirectionTwo);
	TEST_ASSERT(rotateDirection(knDirectionOne, 11) == knDirectionSix);
	TEST_ASSERT(rotateDirection(knDirectionFive, 2) == knDirectionOne);

	// test the factor method
	index1 = "6020";
	eHexDirection nDirA;
	int nMoveA;
	eHexDirection nDirB;
	int nMoveB;
	factor(index1, &nDirA, &nMoveA, &nDirB, &nMoveB);
	TEST_ASSERT(nDirA == knDirectionSix);
	TEST_ASSERT(nMoveA == 4);
	TEST_ASSERT(nDirB == knDirectionFive);
	TEST_ASSERT(nMoveB == 1);

	// test indexToPolar and polarToIndex

	index1 = "00";
	PYXCoordPolar pt;
	PYXCoordPolar ptTest;
	indexToPolar(index1, &pt);
	TEST_ASSERT(MathUtils::equal(pt.radius(), 0.0));

	polarToIndex(pt, index1.getResolution(), index2, true);
	TEST_ASSERT(index1 == index2);

	index1 = "01";
	double fAngle = 0.0;
	int nCount;
	for (nCount = 0; nCount < Hexagon::knNumSides; ++nCount)
	{
		indexToPolar(index1, &pt);

		ptTest.setRadius(m_vecInterCellDistances[3]);
		ptTest.setAngleInDegrees(fAngle);
		TEST_ASSERT(pt.equal(ptTest));

		polarToIndex(pt, index1.getResolution(), index2, true);
		TEST_ASSERT(index1 == index2);
		rotateIndex(&index1, 1, knCCW);
		fAngle += 60.0;

		if (fAngle > 180.0)
		{
			fAngle -= 360.0;
		}
	}

	index1 = "001";
	fAngle = -30.0;
	for (nCount = 0; nCount < Hexagon::knNumSides; ++nCount)
	{
		indexToPolar(index1, &pt);

		ptTest.setRadius(m_vecInterCellDistances[4]);
		ptTest.setAngleInDegrees(fAngle);
		TEST_ASSERT(pt.equal(ptTest));

		polarToIndex(pt, index1.getResolution(), index2, true);
		TEST_ASSERT(index1 == index2);
		rotateIndex(&index1, 1, knCCW);
		fAngle += 60.0;

		if (fAngle > 180.0)
		{
			fAngle -= 360.0;
		}
	}

	index1 = "10";
	fAngle = -30.0;
	for (nCount = 0; nCount < Hexagon::knNumSides; ++nCount)
	{
		indexToPolar(index1, &pt);

		ptTest.setRadius(m_vecInterCellDistances[2]);
		ptTest.setAngleInDegrees(fAngle);
		TEST_ASSERT(pt.equal(ptTest));

		polarToIndex(pt, index1.getResolution(), index2, true);
		TEST_ASSERT(index1 == index2);
		rotateIndex(&index1, 1, knCCW);
		fAngle += 60.0;

		if (fAngle > 180.0)
		{
			fAngle -= 360.0;
		}
	}

	index1 = "010";
	fAngle = 0.0;
	for (nCount = 0; nCount < Hexagon::knNumSides; ++nCount)
	{
		indexToPolar(index1, &pt);

		ptTest.setRadius(m_vecInterCellDistances[3]);
		ptTest.setAngleInDegrees(fAngle);
		TEST_ASSERT(pt.equal(ptTest));

		polarToIndex(pt, index1.getResolution(), index2, true);
		TEST_ASSERT(index1 == index2);
		rotateIndex(&index1, 1, knCCW);
		fAngle += 60.0;

		if (fAngle > 180.0)
		{
			fAngle -= 360.0;
		}
	}

	{
		// Testing calcAncestorIndex
		TEST_ASSERT(calcAncestorIndex(PYXIndex("010203"), PYXIndex("0")) == PYXIndex("0"));
		TEST_ASSERT(calcAncestorIndex(PYXIndex("010203"), PYXIndex("01")) == PYXIndex("01"));
		TEST_ASSERT(calcAncestorIndex(PYXIndex("010203"), PYXIndex("010")) == PYXIndex("010"));
		TEST_ASSERT(calcAncestorIndex(PYXIndex("010203"), PYXIndex("0102")) == PYXIndex("0102"));
		TEST_ASSERT(calcAncestorIndex(PYXIndex("010203"), PYXIndex("01020")) == PYXIndex("01020"));
		TEST_ASSERT(calcAncestorIndex(PYXIndex("010203"), PYXIndex("010203")) == PYXIndex("010203"));
		TEST_ASSERT(calcAncestorIndex(PYXIndex("010203"), PYXIndex("0102030")) == PYXIndex("010203"));
		TEST_ASSERT(calcAncestorIndex(PYXIndex("010203"), PYXIndex("01020300")) == PYXIndex("010203"));
		TEST_ASSERT(calcAncestorIndex(PYXIndex("010203"), PYXIndex("01020304")) == PYXIndex("010203"));

		TEST_ASSERT(calcAncestorIndex(PYXIndex("010203"), PYXIndex("010204")) == PYXIndex("01020"));
		TEST_ASSERT(calcAncestorIndex(PYXIndex("010203"), PYXIndex("010304")) == PYXIndex("010"));
		TEST_ASSERT(calcAncestorIndex(PYXIndex("010203"), PYXIndex("020304")) == PYXIndex("0"));

		TEST_ASSERT(calcAncestorIndex(PYXIndex("010203"), PYXIndex("10203")) == PYXIndex());

		TEST_ASSERT(calcAncestorIndex(PYXIndex("010203"), PYXIndex()) == PYXIndex());
		TEST_ASSERT(calcAncestorIndex(PYXIndex(), PYXIndex("010203")) == PYXIndex());
		TEST_ASSERT(calcAncestorIndex(PYXIndex(), PYXIndex()) == PYXIndex());
	}
}

/*!
Get the class of hexagon for a given resolution.

\param	nResolution	The resolution.

\return	The hexagon class.
*/
PYXMath::eHexClass PYXMath::getHexClass(int nResolution)
{
	return (0 == (nResolution & 1)) ? knClassI : knClassII;
}

/*!
Get the alternate hexagon class. This method returns knClassII if knClassI is
specified and vice versa.

\param	nClass	The hex class.

\return	The alternate hex class.
*/
PYXMath::eHexClass PYXMath::getAltHexClass(eHexClass nClass)
{
	return ((knClassI == nClass) ? knClassII : knClassI);
}

/*!
Calculate the new cell index when we move one hexagon in a specific direction.

\param	start			The starting index.
\param	nHexDirection	The direction.
\param	pResult			The result (out).

\return	The new cell index.
*/
void PYXMath::move(	const PYXIndex& start,
					eHexDirection nHexDirection,
					PYXIndex* pResult	)
{
	// pResult is checked and reset in add method

	PYXIndex addend;
	addend.appendDigit(nHexDirection);

	eHexClass nHexClass = getHexClass(start.getResolution());
	add(start, addend, nHexClass, pResult);
}

/*!
Add two indices. The algorithm will attempt to create a sum with the specified
resolution. The sum's resolution may be greater if the new cell is off the
grid. Set grow to true to allow this to happen, otherwise an exception is
thrown.

\param	first		The augend.
\param	second		The addend.
\param	nResolution	The resolution of the grid where the addition is performed.
\param	pSum		The sum.
\param	bGrow		true if the resolution of the sum can increase.
*/
void PYXMath::add(	const PYXIndex& first,
					const PYXIndex& second,
					int nResolution,
					PYXIndex* pSum,
					bool bGrow	)
{
	// pSum is checked and reset in add method

	try
	{
		// determine the addition table to use for the first digit
		eHexClass nHexClass = getHexClass(nResolution);

		add(first, second, nHexClass, pSum);

		// try to reduce the sum's resolution to the grid resolution
		while (pSum->getResolution() > nResolution)
		{
			if (pSum->getDigit(0) == 0)
			{
				pSum->stripLeft();
			}
			else
			{
				// resolution has increased, we have gone off the grid
				if (!bGrow)
				{
					PYXTHROW(	PYXMathException,
								"Index resolution has increased."	);
				}

				break;
			}
		}
	}
	catch (PYXException& e)
	{
		PYXRETHROW(	e, PYXMathException,
					"Unable to add indices '" << first << "' and '" << second << "'."	);
	}
}

/*!
Add two indices.

\param	first		The augend.
\param	second		The addend.
\param	nHexClass	The hexagon class.
\param	pSum		The sum (out).

\return	The sum.
*/
void PYXMath::add(	PYXIndex first,
					PYXIndex second,
					eHexClass nHexClass,
					PYXIndex* pSum	)
{
	assert(pSum != 0);
	pSum->reset();

	// Reverse buffer (so we can avoid prepends).
	int nChar = 0;
	char arrayChar[PYXMath::knMaxAbsResolution + 1];

	int nMaxResolution = std::max(first.getResolution(), second.getResolution());

	if (nHexClass == knClassII)
	{
		first.appendDigit(0);
		second.appendDigit(0);
	}

	int nA1;
	int nB1;
	pairToIntegers(first.stripRightPair(), &nA1, &nB1);

	int nATemp;
	int nBTemp;
	pairToIntegers(second.stripRightPair(), &nATemp, &nBTemp);

	nA1 += nATemp;
	nB1 += nBTemp;

	PYXIndex* pPrependIndex = 0;

	int nK = 0;
	int nNumPairs = nMaxResolution / 2;
	while ((nK <= nNumPairs) || (nA1 != 0) || (nB1 != 0))
	{
		// optimization for different sized indices
		if ((nA1 == 0) && (nB1 == 0))
		{
			if (first.m_nDigitCount == 0)
			{
				second.appendPair(0);
				pPrependIndex = &second;
				break;
			}

			if (second.m_nDigitCount == 0)
			{
				first.appendPair(0);
				pPrependIndex = &first;
				break;
			}
		}

		++nK;

		int nR1;
		int nS1;
		divideByThree(&nA1, &nB1, &nR1, &nS1);

		int nA2;
		int nB2;
		pairToIntegers(first.stripRightPair(), &nA2, &nB2);

		int nATemp;
		int nBTemp;
		pairToIntegers(second.stripRightPair(), &nATemp, &nBTemp);

		nA2 += nATemp;
		nB2 += nBTemp;

		nA1 += nA2;
		nB1 += nB2;

		nATemp = nA1;
		nBTemp = nB1;
		int nR2;
		int nS2;
		divideByThree(&nATemp, &nBTemp, &nR2, &nS2);

		char nPair1 = '0';
		char nPair2;
		if ((nS1 == 2) && ((nR2 == 1) || (nR2 == -2)))
		{
			nPair2 = '6';
			--nA1;
			++nB1;
		}
		else if ((nS1 == 2) && ((nR2 == 2) || (nR2 == -1)))
		{
			nPair2 = '4';
			++nA1;
			++nB1;
		}
		else if ((nS1 == -2) && ((nR2 == 1) || (nR2 == -2)))
		{
			nPair2 = '1';
			--nA1;
			--nB1;
		}
		else if ((nS1 == -2) && ((nR2 == 2) || (nR2 == -1)))
		{
			nPair2 = '3';
			++nA1;
			--nB1;
		}
		else
		{
			unsigned int nPair = integersToPair(nR1, nS1);
			nPair1 = (nPair & 0xF) + '0';
			nPair2 = ((nPair >> 4) & 0xF) + '0';
		}

		arrayChar[nChar++] = nPair1;
		arrayChar[nChar++] = nPair2;
	}

	// Prepend index into reverse buffer.
	if (pPrependIndex != 0)
	{
		for (int n = 0; n != pPrependIndex->m_nDigitCount; ++n)
		{
			pSum->m_pcDigits[pSum->m_nDigitCount++] = pPrependIndex->m_pcDigits[n];
		}
		pSum->m_pcDigits[pSum->m_nDigitCount] = 0;
	}

	// Reverse copy buffer into index.
	while (0 < nChar)
	{
		pSum->m_pcDigits[pSum->m_nDigitCount++] = arrayChar[--nChar];
	}
	pSum->m_pcDigits[pSum->m_nDigitCount] = 0;

	if (nHexClass == knClassII)
	{
		pSum->stripRight();
	}

	// try to make the result resolution match the input resolution
	pSum->adjustResolutionLeft(nMaxResolution);
}

/*!
Subtract an index from this one. The algorithm will attempt to create a sum with
the specified resolution. 

\param	first		The minuend
\param	second		The subtrahend
\param	nResolution	The resolution of the grid where the subtraction is performed.
\param	pDifference	The difference (out).
*/
void PYXMath::subtract(	const PYXIndex& first,
					  	const PYXIndex& second,
						int nResolution,
						PYXIndex* pDifference	)
{
	// pDifference checked and reset in add method

	PYXIndex subtrahend = second;

	try
	{
		// negate the second argument
		subtrahend.negate();

		// perform normal addition
		add(first, subtrahend, nResolution, pDifference, false);
	}
	catch (PYXException& e)
	{
		PYXRETHROW(	e, PYXMathException,
					"Unable to subtract index '" << second << "' from '" << first << "'."	);
	}
}

/*!
Take the direction and number of hexagons to move and calculate the new cell
index relative to the origin. The method will attempt to create an index with
the specified resolution.  The multiplication done in this method is always
provides an index referenced to the origin.  As the number of hexagons we move
away from the origin increases (nFactor) a pattern of the indices can be 
observed.  For a description of the pattern found in multiplication look
in the document "Tesseral Arithmetic".

\param	nFactor		The number of hexagons to move.
\param	nDir		The direction to move.
\param	nResolution	The resolution of the grid where the multiplication is performed.
\param	pProduct	The product (out).

\return	The product.
*/
void PYXMath::multiply(	int nFactor,
						eHexDirection nDir,
						int nResolution,
						PYXIndex* pProduct)
{
	// verify that the resolution is valid
	assert(0 <= nResolution);

	assert(pProduct != 0);
	pProduct->reset();

	// handle negative factors by reversing direction
	if (nFactor < 0)
	{
		nDir = negateDir(nDir);
		nFactor = -nFactor;
	}
	// If the factor is any greater, then multiplier (below) will overflow.
	// This value is 3^0 + 3^1 + 3^2 + ... + 3^19
	assert(nFactor <= 1743392200);

	PYXIndex product;

	try
	{
		// find the base pattern by looking up the key value for the direction
		unsigned int nKeyValue = pnMultiplicationFactors[nDir];
		
		// Reverse buffer (so we can avoid prepends).
		int nDigit = 0;
		int arrayDigit[PYXMath::knMaxAbsResolution + 1];
		
		// place the base pattern in an array for easy reference
		unsigned int pnValue[3];
		pnValue[0] = (nKeyValue >> (knBitsPerDigit * 2)) & knDigitMask;
		pnValue[1] = nKeyValue & knDigitMask;
		pnValue[2] = 0;

		// calculate 3 to the power of nIteration to follow the pattern
		int nIteration = 0;
		int nMultiplier = 1;

		/* 
		It takes an increasing number of movements to add a digit to the
		overall index.  The first digit requires 1 movement the third 
		requires an additional 1 the fifth requires an additional 3, 
		7th -> 9, 9th -> 27...
		*/
		while (0 < nFactor)
		{
			if (PYXMath::knMaxAbsResolution <= nDigit)
			{
				PYXTHROW(	PYXMathException,
							"Index resolution has overflowed."	);
			}

			// at least every second digit will be zero
			if (1 == (nDigit & 1))
			{
				arrayDigit[nDigit++] = 0;
			}
			else
			{
				assert(0 <= nMultiplier);

				// calculate the value for this place by using the formula
				int nIndex = ((nFactor - 1) / nMultiplier) % 3;

				arrayDigit[nDigit++] = pnValue[nIndex];
				nFactor -= nMultiplier;
				++nIteration;
				nMultiplier *= 3;
			}
		}

		// Reverse copy buffer into index.
		while (0 < nDigit)
		{
			pProduct->m_pcDigits[pProduct->m_nDigitCount++] = arrayDigit[--nDigit] + '0';
		}
		pProduct->m_pcDigits[pProduct->m_nDigitCount] = 0;
  
		// increase the resolution
		if (nResolution > pProduct->getResolution())
		{
			pProduct->adjustResolutionLeft(nResolution);
		}
	}
	catch (PYXException& e)
	{
		PYXRETHROW(	e, PYXMathException,
					"Unable to multiply: factor = '" << nFactor
					<< "', direction = '" << nDir
					<< "', resolution = '" << nResolution << "'."	);
	}
}

/*!
Assume p is polar coord relative to direction 1 for the hex,
with units specified in intercell distance for the hex resolution.
Then u is the number of moves in the 1 direction, v in the 2 direction.
*/
void polarToUV(const PYXCoordPolar& p, double& u, double& v)
{
	// Convert polar to xy.
	double x = p.radius() * cos(p.angle());
	double y = p.radius() * sin(p.angle());

	// Convert xy to rc.
	double ry = 2*MathUtils::kfSqrt3/3.0 * y; // row-scaled y
	double r = MathUtils::round(ry); // chosen row
	double rd = ry - r;              // hex-relative coord
	double absrd = abs(rd);

	bool bOdd = ((int)r)%2 != 0; // NOTE what if int overflows?

	double cx = bOdd ? (x-0.5) : x;  // column-offset x
	double c = MathUtils::round(cx); // chosen column
	double cd = cx - c;              // hex-relative coord

	if ((1/3.0) < absrd)
	{
		double abscd = abs(cd);
		if (0.25 < abscd)
		{
			if ((0.5 - absrd) / (abscd - 0.25) < (2/3.0))
			{
				if (bOdd)
				{
					if (0 < cd)
					{
						++c;
					}
				}
				else if (cd < 0)
				{
					--c;
				}
				(rd < 0) ? --r : ++r;
			}
		}
	}

	// Convert rc to uv.
	int ri = (int)r;
	v = r;
	u = c - ((ri<0) ? (ri-1) : ri)/2;
}

/*!
Convert polar coordinates to a PYXIndex. The angle must be measured counter-
clockwise relative to direction one and the radius is specified in inter-cell
units at resolution zero which also corresponds to the length of a side of the
icosahedron.  An exception is thrown if the resolution size is exceeded and 
bGrow is false.

\param	pt			The polar coordinates.
\param	nResolution	The target resolution.
\param	index		The PYXIS index (out)
\param	bGrow		Indicate if the index is permitted to grow beyond the 
					tesselation or not.
*/
void PYXMath::polarToIndex(	const PYXCoordPolar& pt,
							int nResolution,
							PYXIndex& index,
							bool bGrow	)
{
	/*
	Convert the passed polar coordinate to one where the angle
	is correct for the current class and the radius is measured in
	inter-cell distances at the specified resolution.
	*/
	PYXCoordPolar relativeCoord(pt.radius(), pt.angle());	
	if (getHexClass(nResolution) == PYXMath::knClassI)
	{
		relativeCoord.setAngle(pt.angle() + MathUtils::kf30Rad);
	}
	relativeCoord.setRadius(
		relativeCoord.radius() / calcInterCellDistance(nResolution + 2));

	double u, v;
	polarToUV(relativeCoord, u, v);

	index.reset();
	multiply(static_cast<int>(u), knDirectionOne, nResolution, &index);

	PYXIndex tmp;
	multiply(static_cast<int>(v), knDirectionTwo, nResolution, &tmp);
	add(index, tmp, nResolution, &index, bGrow);

}

/*!
Extract the number of cells of movement that is represented in a particular
direction of a polar coordinate.

\param nDirection	The axis (1/3, 2/4, 3/6) that should be extracted from
					the polar coordinate.
\param pPolar		The polar coordinate relative to the 1 direction at the 
					specified resolution. The distance of the polar angle must
					be specified in intercell distances at the desired extraction
					resolution. This polar coordinate is reduced by the component 
					direction that is extracted and returned. (in/out)

\return The number of cell movements that are available in the polar coordinate
		in the specified direction, at the specified resolution.
*/
int PYXMath::extractDirectionComponent(
	eHexDirection nDirection, 
	PYXCoordPolar* pPolar	)
{
	assert (nDirection != knDirectionZero);
	assert (pPolar != 0);

	// determine the extracted angle
	double fExtractedAngle = 0;
	if (nDirection == knDirectionTwo ||
		nDirection == knDirectionFive)
	{
		fExtractedAngle = MathUtils::kf60Rad;
	}
	else if (nDirection == knDirectionThree ||
			 nDirection == knDirectionSix)
	{
		fExtractedAngle = MathUtils::kf120Rad;
	}
	
	// compute the discrete value of the component direction
	PYXCoordPolar extractedPolar(
		MathUtils::round(pPolar->radius() * cos(pPolar->angle() - fExtractedAngle)), fExtractedAngle);

	// remove the extracted component from the original
	*pPolar -= extractedPolar;

	return static_cast<int>(extractedPolar.radius());
}

/*!
Convert a PYXIndex to polar coordinates. The angle is measured counter-
clockwise relative to direction one and the radius is specified in inter-cell
units at resolution zero (i.e. 01-0), which also corresponds to the length of a
side of the icosahedron.

\param	index	The index.
\param	pPolar	The polar coordinates (out).
*/
void PYXMath::indexToPolar(const PYXIndex& index, PYXCoordPolar* pPolar)
{
	if (0 != pPolar)
	{
		//std::vector<double> vecFactors(2 * Hexagon::knNumSides, 0.0);
		double vecFactors[12];
		memset(vecFactors, 0, sizeof(vecFactors));

		PYXMath::eHexClass nClass = knClassI;

		char* ptr = const_cast<char*>(index.m_pcDigits);
		char* ptrEnd = ptr + index.m_nDigitCount;

		// nIndex starts at two so that we don't have to reference knMinSubRes
		for (int nIndex = 2; ptr < ptrEnd; ++ptr, ++nIndex)
		{
			if (*ptr != '0')
			{
				int nDir = *ptr - '0';

				if (PYXMath::knClassII == nClass)
				{
					vecFactors[nDir - 1] += m_vecInterCellDistances[nIndex];
				}
				else
				{
					vecFactors[nDir + Hexagon::knNumSides - 1] += m_vecInterCellDistances[nIndex];
				}
			}

			nClass = getAltHexClass(nClass);
		}

		// calculate the ClassII directional components
		double fD1 = vecFactors[0] - vecFactors[3];
		double fD2 = vecFactors[1] - vecFactors[4];
		double fD3 = vecFactors[2] - vecFactors[5];

		double fX = fD1 + (fD2 - fD3) * MathUtils::kfCos60;
		double fY = (fD2 + fD3) * MathUtils::kfSin60;

		// calculate the ClassI directional components
		fD1 = vecFactors[6] - vecFactors[9];
		fD2 = vecFactors[7] - vecFactors[10];
		fD3 = vecFactors[8] - vecFactors[11];

		fX += (fD1 + fD2) * MathUtils::kfCos30;
		fY += fD3 + (fD2 - fD1) * MathUtils::kfSin30;

		double fRadius = sqrt(fX * fX + fY * fY);
		double fAngle = atan2(fY, fX);

		pPolar->setRadius(fRadius);
		pPolar->setAngle(fAngle);
	}
}


/*!
This method is a subroutine used in Andy Vince's implementation of the add
method.

\param	nPair	Two PYXIS digits as a hex number
\param	pnA		The first integer (out)
\param	pnB		The second integer (out)
*/
void PYXMath::pairToIntegers(unsigned int nPair, int* pnA, int* pnB)
{
	assert(pnA != 0);
	assert(pnB != 0);

	switch (nPair)
	{
		case 0x00:
			*pnA = 0;
			*pnB = 0;
			break;

		case 0x01:
			*pnA = 2;
			*pnB = 0;
			break;

		case 0x02:
			*pnA = 1;
			*pnB = 1;
			break;

		case 0x03:
			*pnA = -1;
			*pnB = 1;
			break;

		case 0x04:
			*pnA = -2;
			*pnB = 0;
			break;

		case 0x05:
			*pnA = -1;
			*pnB = -1;
			break;

		case 0x06:
			*pnA = 1;
			*pnB = -1;
			break;

		case 0x10:
			*pnA = 3;
			*pnB = 1;
			break;

		case 0x20:
			*pnA = 0;
			*pnB = 2;
			break;

		case 0x30:
			*pnA = -3;
			*pnB = 1;
			break;

		case 0x40:
			*pnA = -3;
			*pnB = -1;
			break;

		case 0x50:
			*pnA = 0;
			*pnB = -2;
			break;

		case 0x60:
			*pnA = 3;
			*pnB = -1;
			break;

		default:
			// invalid PYXIS digits
			assert(false);
			break;
	}
}

/*!
This method is a subroutine used in Andy Vince's implementation of the add
method.

\param	nA		The first integer (out)
\param	nB		The second integer (out)

\return	nPair	Pair of PYXIS digits as a hex number.
*/
unsigned int PYXMath::integersToPair(int nA, int nB)
{
	unsigned int nPair = 0x00;

	switch (nA)
	{
		case -3:
			{
				switch(nB)
				{
					case -1:
						nPair = 0x40;
						break;

					case 1:
						nPair = 0x30;
						break;

					default:
						// invalid pair
						assert(false);
						break;
				}
			}
			break;

		case -2:
			{
				if (nB == 0)
				{
					nPair = 0x04;
					break;
				}
				else
				{
					// invalid pair
					assert(false);
				}
			}
			break;

		case -1:
			{
				switch(nB)
				{
					case -1:
						nPair = 0x05;
						break;

					case 1:
						nPair = 0x03;
						break;

					default:
						// invalid pair
						assert(false);
						break;
				}
			}
			break;

		case 0:
			{
				switch(nB)
				{
					case -2:
						nPair = 0x50;
						break;

					case 0:
						nPair = 0x00;
						break;

					case 2:
						nPair = 0x20;
						break;

					default:
						// invalid pair
						assert(false);
						break;
				}
			}
			break;
		
		case 1:
			{
				switch(nB)
				{
					case -1:
						nPair = 0x06;
						break;

					case 1:
						nPair = 0x02;
						break;

					default:
						// invalid pair
						assert(false);
						break;
				}
			}
			break;
			
		case 2:
			{
				if (nB == 0)
				{
					nPair = 0x01;
					break;
				}
				else
				{
					// invalid pair
					assert(false);
				}
			}
			break;

		case 3:
			{
				switch(nB)
				{
					case -1:
						nPair = 0x60;
						break;

					case 1:
						nPair = 0x10;
						break;

					default:
						// invalid pair
						assert(false);
						break;
				}
			}
			break;		
					
		default:
			// invalid pair of digits
			assert(false);
			break;
	}

	return nPair;
}

/*!
This method is a subroutine used in Andy Vince's implementation of the add
method.

\param	pnA	The first integer (in/out)
\param	pnB	The second integer (in/out)
\param	pnR	The first remainder (out)
\param	pnS	The second remainder (out)
*/
void PYXMath::divideByThree(	int* pnA,
								int* pnB,
								int* pnR,
								int* pnS	)
{
	assert(pnA != 0);
	assert(pnB != 0);
	assert(pnR != 0);
	assert(pnS != 0);

	// perform divide by 3 with remainder in (0, 1, 2)
	int nA = ((*pnA >= 0) ? *pnA : (*pnA - 2)) / 3;
	int nB = ((*pnB >= 0) ? *pnB : (*pnB - 2)) / 3;

	*pnR = *pnA - (nA * 3);
	*pnS = *pnB - (nB * 3);

	*pnA = nA;
	*pnB = nB;

	if (*pnS == (*pnR + 1))
	{
		(*pnS) -= 3;
		++(*pnB);
	}
	else if (*pnS == (*pnR - 1))
	{
		(*pnR) -= 3;
		++(*pnA);
	}
	else if ((*pnR == 2) && (*pnS == 2))
	{
		*pnR = -1;
		*pnS = -1;
		++(*pnA);
		++(*pnB);
	}
}
	
/*!
This method will determine the parent index of the passed index.  

\param pyxIndex	The index for which the parent index is being requested

\return The parent index value
*/
PYXIndex PYXMath::getParent(const PYXIndex& pyxIndex)
{
	PYXIndex parentIndex = pyxIndex;

	if (pyxIndex.getResolution() > 0)
	{
		parentIndex.stripRight();
	}
	else
	{
		PYXTHROW(	PYXMathException,
					"Invalid resolution: '" << parentIndex.getResolution() << "'."	);
	}

	return parentIndex;
}

/*!
Factor an index into its component directions.  For more details on factoring
see the "Tesseral Arithmetic" document.

\param	pyxIndex	The index to factor.
\param	vecFactors	Vector of factors.
*/
void PYXMath::factor(PYXIndex pyxIndex, FactorVector& vecFactors)
{
	// clear out factor vector
	vecFactors.resize(Hexagon::knNumSides);
	vecFactors.assign(Hexagon::knNumSides, 0);

	try
	{
		/*
		the number of digits in the index is a good basis to find how far away
		from the origin the given index is located
		*/
		PYXIndex reduce;
		int nDigits = pyxIndex.m_nDigitCount;
		reduce.setDigitCount(nDigits);
		
		/*
		find the most significant digit in the number the most significant 
		digit indicates which direction the index can be located in with 
		reference to the origin 
		*/
		int nPosition;
		eHexDirection nHexDirection =
			static_cast<eHexDirection>(pyxIndex.mostSignificant(&nPosition));

		// iterate through the address until we reach the origin
		while (0 <= nPosition)
		{

			/*
			Using an altered version of the formula found in the multiply 
			method we take a guess at how far away from the origin the index is
			*/
			int nMultiple = 1;
			int nFactor = 1;

			int nCount = nPosition / 2;
			for (; nCount > 0; --nCount)
			{
				nMultiple += nFactor;
				nFactor *= 3;
			}

			/*
			Find the address of our educated guess about where the 
			index is located
			*/
			multiply(nMultiple, nHexDirection, reduce.getResolution(), &reduce);
			
			// we subtract our educated guess from the working index
			subtract(pyxIndex, reduce, pyxIndex.getResolution(), &pyxIndex);
			
			// we store the number of movements we just made
			vecFactors[nHexDirection - 1] += nMultiple;
			
			// find the overall direction in our working direction and continue
			nHexDirection =
				static_cast<eHexDirection>(pyxIndex.mostSignificant(&nPosition));
		}
	}
	catch (PYXException& e)
	{
		PYXRETHROW(e, PYXMathException, "Unable to factor '" << pyxIndex << "'.");
	}
}

/*!
Factor an index into its two main directions and determine the number of
moves in each direction at the index's resolution. The number of moves are
guaranteed to be zero or greater. If the number of moves in both directions
is equal, then then lower direction is considered primary.

\param	index	The index to factor.
\param	pnDirA	The primary direction (out).
\param	pnMoveA	The number of moves in the primary direction (out).
\param	pnDirB	The secondary direction (out).
\param	pnMoveB	The number of moves in the secondary direction (out).
*/
void PYXMath::factor(	const PYXIndex& index,
						eHexDirection* pnDirA,
						int* pnMoveA,
						eHexDirection* pnDirB,
						int* pnMoveB	)
{
	if ((0 != pnDirA) && (0 != pnMoveA) && (0 != pnDirB) && (0 != pnMoveB))
	{
		int vecFactors[12] = { 0 }; // 2 * Hexagon::knNumSides == 12

		bool bAlt = false;
		int nDistance = 1;
		char* ptrEnd = const_cast<char*>(index.m_pcDigits);
		char* ptr = ptrEnd + index.m_nDigitCount - 1;
		for (; ptr >= ptrEnd; --ptr)
		{
			int nDir = *ptr - '1';

			if (bAlt)
			{
				if (nDir >= 0)
				{
					vecFactors[nDir + Hexagon::knNumSides] += nDistance;
				}

				nDistance *= 3;
			}
			else if (nDir >= 0)
			{
				vecFactors[nDir] += nDistance;
			}

			bAlt = !bAlt;
		}

		// calculate the directional components
		int nD1 = vecFactors[0] - vecFactors[3];
		int nD2 = vecFactors[1] - vecFactors[4];
		int nD3 = vecFactors[2] - vecFactors[5];

		// calculate the alternate directional components
		int nDA1 = vecFactors[6] - vecFactors[9];
		int nDA2 = vecFactors[7] - vecFactors[10];
		int nDA3 = vecFactors[8] - vecFactors[11];

		/*
		Calculate the contribution of the alternate class directional
		components to the directional components in the base resolution.
		*/
		if (getHexClass(index.getResolution()) == knClassI)
		{
			nD1 += nDA1;
			nD2 += nDA1;

			nD2 += nDA2;
			nD3 += nDA2;

			nD3 += nDA3;
			nD1 -= nDA3;
		}
		else	// knClassII
		{
			nD3 -= nDA1;
			nD1 += nDA1;

			nD1 += nDA2;
			nD2 += nDA2;

			nD2 += nDA3;
			nD3 += nDA3;
		}

		/*
		Reduce the total number of moves. On return, the number of moves in at
		least one direction will be zero.
		*/
		reduceFactors(&nD1, &nD2, &nD3);

		// Assume dir 1 and 2 are used.
		int nDir1 = 1;
		int nDir2 = 2;

		// If dir 3 is used, copy it as dir 1 or 2.
		if (nD3)
		{
			if (nD2)
			{
				nD1 = nD3;
				nDir1 = 3;
			}
			else
			{
				nD2 = nD3;
				nDir2 = 3;
			}
		}

		// Negate directions as needed.
		if (nD1 < 0)
		{
			nD1 = -nD1; // abs
			nDir1 += 3; // negate
		}
		if (nD2 < 0)
		{
			nD2 = -nD2; // abs
			nDir2 += 3; // negate
		}

		// Choose primary direction: greatest moves, or lowest direction
		// if same moves.
		if (nD1 < nD2 || (nD1 == nD2 && nDir2 < nDir1))
		{
			// Swap.
			int nTmp = nD1;
			nD1 = nD2;
			nD2 = nTmp;
			nTmp = nDir1;
			nDir1 = nDir2;
			nDir2 = nTmp;
		}

		// Return results.
		*pnDirA = static_cast<PYXMath::eHexDirection>(nDir1);
		*pnMoveA = nD1;
		*pnDirB = static_cast<PYXMath::eHexDirection>(nDir2);
		*pnMoveB = nD2;
	}
}

void PYXMath::factor(	const PYXIndex& index,
						int* pnMove2,
						int* pnMove6	)
{
	PYXMath::eHexDirection dirA, dirB;
	int nStepsA, nStepsB;
	factor(index, &dirA, &nStepsA, &dirB, &nStepsB);
	int nMove2 = 0;
	int nMove6 = 0;
	switch (dirA)
	{
	case knDirectionOne:
		nMove2 += nStepsA;
		nMove6 += nStepsA;
		break;
	case knDirectionTwo:
		nMove2 += nStepsA;
		break;
	case knDirectionThree:
		nMove6 -= nStepsA;
		break;
	case knDirectionFour:
		nMove2 -= nStepsA;
		nMove6 -= nStepsA;
		break;
	case knDirectionFive:
		nMove2 -= nStepsA;
		break;
	case knDirectionSix:
		nMove6 += nStepsA;
		break;
	}
	switch (dirB)
	{
	case knDirectionOne:
		nMove2 += nStepsB;
		nMove6 += nStepsB;
		break;
	case knDirectionTwo:
		nMove2 += nStepsB;
		break;
	case knDirectionThree:
		nMove6 -= nStepsB;
		break;
	case knDirectionFour:
		nMove2 -= nStepsB;
		nMove6 -= nStepsB;
		break;
	case knDirectionFive:
		nMove2 -= nStepsB;
		break;
	case knDirectionSix:
		nMove6 += nStepsB;
		break;
	}

	*pnMove2 = nMove2;
	*pnMove6 = nMove6;
}
/*!
Reduce a set of three factors expressed in terms of direction one, two and
three to a set of factors that use at most two of the three directions. This
algorithm minimizes the number of steps required to get from point A to point
B and is useful when gridding a line. On return from this method, at least one
of the factors is zero and the other two have been adjusted accordingly.

\param	pnD1	The number of moves in direction one (in/out)
\param	pnD2	The number of moves in direction two (in/out)
\param	pnD3	The number of moves in direction three (in/out)
*/
void PYXMath::reduceFactors(int* pnD1, int* pnD2, int* pnD3)
{
	assert(pnD1 != 0);
	assert(pnD2 != 0);
	assert(pnD3 != 0);

	if ((abs(*pnD1) <= abs(*pnD2)) && (abs(*pnD1) <= abs(*pnD3)))
	{
		*pnD2 += *pnD1;
		*pnD3 -= *pnD1;
		*pnD1 = 0;
	}
	else if ((abs(*pnD2) <= abs(*pnD1)) && (abs(*pnD2) <= abs(*pnD3)))
	{
		*pnD1 += *pnD2;
		*pnD3 += *pnD2;
		*pnD2 = 0;
	}
	else // ((abs(*pnD3) <= abs(*pnD1)) && (abs(*pnD3) <= abs(*pnD2)))
	{
		*pnD1 -= *pnD3;
		*pnD2 += *pnD3;
		*pnD3 = 0;
	}
}

/*!
Zoom in to the next higher resolution. The new cell is either the origin
child or a vertex child of the source cell.  If the direction is set to 
knDirectionZero (default) the new index will be the origin child.  If the 
direction is set to a value other than knDirectionZero the new index will
be the cooresponding vertex child

\param	pIndex		The cell index (in/out)
\param	nHexDirection	The direction of the zoom in

\return true if the zoom in was successful otherwise false

\sa PYXMath::eHexDirection
*/
bool PYXMath::zoomIn(PYXIndex* pIndex, PYXMath::eHexDirection nHexDirection)
{
	assert(0 != pIndex);

	try
	{
		if (pIndex->hasVertexChildren() || nHexDirection == PYXMath::knDirectionZero)
		{
			pIndex->appendDigit(nHexDirection);
			return true;
		}

		return false;
	}
	catch (PYXException& e)
	{
		PYXRETHROW(	e, PYXMathException,
					"Unable to zoom in on index '" << *pIndex <<
					"' in the '" << nHexDirection << "' direction."	);
	}
}

/*!
Zoom out to the next lower resolution. The new cell either encloses the current
cell or the current cell is located at one of its vertices. 

\param	pIndex		The cell index (in/out)
\param	pnDirection	The direction of the new index relative to the old index (out)

\return	true if the zoom out operation was successful otherwise false.
*/
bool PYXMath::zoomOut(	PYXIndex* pIndex,
						PYXMath::eHexDirection* pnDirection	)
{
	assert(0 != pIndex);

	if (pIndex->isNull())
	{
		return false;
	}

	// assign the direction if a valid pointer was passed
	if (0 != pnDirection)
	{
		*pnDirection = static_cast<eHexDirection>(pIndex->stripRight());
	}
	else
	{
		pIndex->stripRight();
	}
		
	return true;
}

/*!
Returns the direction offset from the origin.  Determined by 
examining the index and returning the most significant digit
in the PYXIndex.

\param	pyxIndex	The cell index.

\return	The direction with reference to the origin.
*/
PYXMath::eHexDirection PYXMath::originDir(const PYXIndex& pyxIndex)
{
	int nLocation;

	return static_cast<eHexDirection>(pyxIndex.mostSignificant(&nLocation));
}

/*!
Calculate the common ancestor index of the two indices. This method
accesses private members of PYXIndex for performance reasons.

\param	index1	The first index.
\param	index2	The second index.

\return	The common ancestor index of the two indices, or the null index if
		there is no common ancestor.
*/
PYXIndex PYXMath::calcAncestorIndex(	const PYXIndex& index1,
										const PYXIndex& index2	)
{
	int nDigits = std::min(index1.m_nDigitCount, index2.m_nDigitCount);

	PYXIndex indexAncestor;

	for (int n = 0; n != nDigits; ++n)
	{
		if (index1.m_pcDigits[n] != index2.m_pcDigits[n])
		{
			break;
		}
		indexAncestor.m_pcDigits[n] = index1.m_pcDigits[n];
		++indexAncestor.m_nDigitCount;
	}

	// Null terminate.
	indexAncestor.m_pcDigits[indexAncestor.m_nDigitCount] = 0;

	return indexAncestor;
}

/*!
Calculate the relative descendant index from a parent to child. If
the passed child index is not a descendant index an exception is
thrown.

\param	parent	The parent index.
\param	child	The child index.

\return	The relative descendant index of the child to the parent.
*/
PYXIndex PYXMath::calcDescendantIndex(	const PYXIndex& parent,
										const PYXIndex& child	)
{
	// child must have greater resolution than parent
	if (parent.m_nDigitCount >= child.m_nDigitCount)
	{
		PYXTHROW(	PYXMathException,
					"'" << child << "' is not a child of '" << parent << "'."	);
	}

	// confirm that child is a child of parent
	if (0 != memcmp(parent.m_pcDigits, child.m_pcDigits, parent.m_nDigitCount))
	{
		PYXTHROW(	PYXMathException,
					"'" << child << "' is not a child of '" << parent << "'."	);
	}

	// copy unique part of child index into relative index
	PYXIndex relativeIndex;

	int nNumDigits = child.m_nDigitCount - parent.m_nDigitCount;
	if (0 < nNumDigits)
	{
		// copy null terminator as well
		memcpy(	relativeIndex.m_pcDigits,
				child.m_pcDigits + parent.m_nDigitCount,
				nNumDigits + 1);
	}

	relativeIndex.m_nDigitCount = nNumDigits;

	return relativeIndex;
}

/*!
If a given index is rotated about the origin by 60 degrees the resulting
PYXIS index would be the corresponding index in the adjacent sector.  The 
caller of the function must provide the number of sector (60 degree) rotations
to perform as well as which direction to rotate in. The number should be 
between 1 and 5 noting that a value of 6 will bring the index around a 
complete revolution back to the same address

\param pIndex		The index to be rotated (in/out)
\param nRotate		The number of times to rotate by 60 degrees (1 vertex)
\param nDirection	The direction of the rotation

\return		The new index after the rotation
*/
void PYXMath::rotateIndex(	PYXIndex* pIndex,
							unsigned int nRotate,
							eRotateDir nDirection	)
{
	// make sure the count is between 0 and 6
	assert((Hexagon::knNumSides > static_cast<int>(nRotate)) && (0 < nRotate));
	nRotate %= Hexagon::knNumSides;

	// check for reverse rotate
	if (nDirection == knCW)
	{
		/*
		Since the rotation occurs in counter-clockwise format we convert in the
		following manner:
		1=5, 2=4, 3=3, 4=3, 5=1
		*/
		nRotate = Hexagon::knNumSides - nRotate;
	}

	// loop through each digit in the index
	--nRotate;
	char* ptr = &(pIndex->m_pcDigits[0]);
	for (int nCounter = 0; nCounter < pIndex->m_nDigitCount; nCounter++)
	{
		if ('0' != *ptr)
		{
			*ptr = ((*ptr - '0' + nRotate) % Hexagon::knNumSides) + '1';
		}

		++ptr;
	}
}

/*!
Rotate a direction by the specified number of 30 degree rotations.

\param nDirection	The direction to rotate.
\param nRotate		The number of rotations to perform (positive for counter-
					clockwise rotations and negative for clockwise rotations).

\return The resulting value after the rotation.
*/
PYXMath::eHexDirection PYXMath::rotateDirection(	eHexDirection nDirection,
													int nRotate	)
{
	// zero values do not rotate
	if (knDirectionZero == nDirection)
	{
		return knDirectionZero;
	}

	// convert any negative values to positive
	if (nRotate < 0)
	{
		nRotate = (nRotate  % Hexagon::knNumSides) + Hexagon::knNumSides;
	}

	return static_cast<PYXMath::eHexDirection>(
		((nDirection + nRotate - 1) % Hexagon::knNumSides) + 1	);
}

/*!
This method examines the most significant digit in the index and returns
the relative direction of the index from the origin.

\return The direction of the index. knDirectionZero if it is the origin.

\sa eHexDirection
*/
PYXMath::eHexDirection PYXMath::hexSector(const PYXIndex& pyxIndex)
{
	PYXMath::eHexDirection nSector = knDirectionZero;
	
	if (!pyxIndex.isNull())
	{
		int nDirection = pyxIndex.mostSignificant(0);
		nSector = static_cast<eHexDirection>(nDirection);
	}
	else
	{
		PYXTHROW(PYXMathException, "Null index.");
	}
	
	return nSector;
}

PYXMath::eHexDirection PYXMath::hexSector(double angle)
{
	eHexDirection nDir = knDirectionOne;
	double fAbsAngle = abs(angle);

	if (fAbsAngle <= MathUtils::kf30Rad)
	{
		// already knDirectionOne
	}
	else if (fAbsAngle <= MathUtils::kf90Rad)
	{
		nDir = (0 <= angle) ? knDirectionTwo : knDirectionThree;
	}
	else if (fAbsAngle <= MathUtils::kf150Rad)
	{
		nDir = (0 <= angle) ? knDirectionThree : knDirectionTwo;
	}
	// else already knDirectionOne

	return nDir;
}


/*!
This method is called at application startup to initialize a vector of
inter-cell distances for each resolution in resolution 0 units.
It also initializes all static variables.

Refer to the "Static Initialization Order" document for more details.
*/
void PYXMath::initStaticData()
{
	// reserve size to avoid resizing
	m_vecInterCellDistances.reserve(knMaxAbsResolution + 1);

	// calculate the circumradius of a resolution 0 hexagon
	double fDistance = kfResZeroIntercell;

	for (int nIndex = 0; nIndex <= knMaxAbsResolution; ++nIndex)
	{
		// store the hexagon diameter size in this vector
		m_vecInterCellDistances.push_back(fDistance);

		// calculate the radius for the next resolution
		fDistance /= MathUtils::kfSqrt3;
	}
}


/*!
Calculate the inter-cell distance for a given resolution (0 based)

\param	nResolution	The resolution (0 based)

\return	The inter-cell distance in resolution 0 units
*/
double PYXMath::calcInterCellDistance(int nResolution)
{
	// make sure we are within our offset and cell vectors
	if (static_cast<int>(m_vecInterCellDistances.size()) <= nResolution)
	{
		PYXTHROW(	PYXMathException,
					"Invalid resolution: '" << nResolution << "'."	);
	}

	return m_vecInterCellDistances[nResolution];
}

/*!
Calculate the circumradius of a hexagon for a given resolution

\param	nResolution	The resolution (0 based)

\return	The circumradius in resolution 0 units.
*/
double PYXMath::calcCircumRadius(int nResolution)
{
	// make sure we are within our offset and cell vectors
	if (static_cast<int>(m_vecInterCellDistances.size()) <= nResolution + 1)
	{
		PYXTHROW(	PYXMathException,
					"Invalid resolution: '" << nResolution << "'."	);
	}

	return m_vecInterCellDistances[nResolution + 1];
}

/*!
Calculate the inradius of a hexagon for a given resolution

\param	nResolution	The resolution (0 based)

\return	The inradius in resolution 0 units.
*/
double PYXMath::calcInRadius(int nResolution)
{
	return calcInterCellDistance(nResolution) / 2.0;
}

/*!
Determine if the index has only a single direction component.

\param index	The PYXIndex being examined

\return	True if the index can be reached with a multiplication from the origin
*/
bool PYXMath::isInLine(const PYXIndex& index)
{
	bool bIsInline = true;

	if (!index.isNull())
	{
		int nCount;
		int nDirVal = index.mostSignificant(&nCount);
		int nDigit = 0;
		nCount++;
		while (nCount < index.m_nDigitCount)
		{
			nDigit = index.getDigit(nCount);
			if (nDigit != 0 && nDigit != nDirVal) 
			{
				bIsInline = false;
				break;
			}
			nCount++;
		}
	}
	else
	{
		bIsInline = false;
	}

	return bIsInline;
}
