/******************************************************************************
tuv.cpp

begin		: 2007-12-14
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#include "tuv.h"

// pyxlib includes
#include "pyxis/derm/cursor.h"
#include "pyxis/utility/tester.h"

// boost includes

// standard includes
#include <cassert>

namespace TUVTester
{

struct UnitTester
{

static TUV makeTuvFromIndex(PYXIcosIndex & root,PYXIcosIndex & index)
{
	int t,u,v;

	tuvFromIndex(root,index,t,u,v);

	return tuvMake(t,u,v);
}

static void test()
{
	int requiredPackBytes[] = { 1, 3, 5, 7, 10, 12, 14, 16, 19, 21 };
	for (int n = 0; n != 10; ++n)
	{
		TEST_ASSERT(tuvGetRequiredPackBytes(n) == requiredPackBytes[n]);
	}

	unsigned char buf[] =
	{
		tuvPack4T(0, 1, 2, 3), 10, 20, 30, 40, 50, 60, 70, 80
	};
	unsigned char buf2[] =
	{
		tuvPack4T(0, 1, 2, 1), 10, 20, 30, 40, 50, 60, 70, 80,
		tuvPack4T(0, 1, 2, 3), 11, 22, 33, 44, 55, 66, 77, 88
	};

	TEST_ASSERT(tuvCountPacked(buf) == 3);
	TEST_ASSERT(tuvCountPacked(buf2) == 7);

	TUV tuvBuf[8];

	{
		int nCount = tuvUnpack4(buf, tuvBuf);
		TEST_ASSERT(nCount == 3);
		TEST_ASSERT(tuvBuf[0] == tuvMake(0, 10, 20));
		TEST_ASSERT(tuvBuf[1] == tuvMake(1, 30, 40));
		TEST_ASSERT(tuvBuf[2] == tuvMake(2, 50, 60));
	}

	{
		// When we unpacked, we didn't set the fourth tuv, so set it now so we terminate.
		tuvBuf[3] = 0xffffffff;

		unsigned char bufTmp[9];
		int nCount = tuvPack4(bufTmp, tuvBuf);
		for (int n = 0; n != tuvGetRequiredPackBytes(nCount); ++n)
		{
			TEST_ASSERT(buf[n] == bufTmp[n]);
		}
	}

	{
		int nCount = tuvUnpackN(buf2, tuvBuf, 8);
		TEST_ASSERT(nCount == 7);
		TEST_ASSERT(tuvBuf[0] == tuvMake(0, 10, 20));
		TEST_ASSERT(tuvBuf[1] == tuvMake(1, 30, 40));
		TEST_ASSERT(tuvBuf[2] == tuvMake(2, 50, 60));
		TEST_ASSERT(tuvBuf[3] == tuvMake(1, 70, 80));
		TEST_ASSERT(tuvBuf[4] == tuvMake(0, 11, 22));
		TEST_ASSERT(tuvBuf[5] == tuvMake(1, 33, 44));
		TEST_ASSERT(tuvBuf[6] == tuvMake(2, 55, 66));
	}

	{
		// When we unpacked, we didn't set the eighth tuv, so set it now so we terminate.
		tuvBuf[7] = 0xffffffff;

		unsigned char bufTmp[18];
		int nCount = tuvPackN(bufTmp, tuvBuf, 8);
		for (int n = 0; n != tuvGetRequiredPackBytes(nCount); ++n)
		{
			TEST_ASSERT(buf2[n] == bufTmp[n]);
		}
	}

	{
		PYXIcosIndex origin("A-000000000000");
		TUV tuv[] =
		{
			tuvMake(0, 0, 0),
			tuvMake(0, 0, 1),
			tuvMake(0, 1, 0),
			tuvMake(0, 1, 1),
			tuvMake(0, 0, 2),
			tuvMake(0, 2, 0),
			tuvMake(0, 2, 2),
			tuvMake(0, 3, 3),
			tuvMake(1, 0, 0),
			tuvMake(1, 0, 1),
			tuvMake(1, 1, 0),
			tuvMake(1, 1, 1),
			tuvMake(2, 0, 0),
			tuvMake(2, 0, 1),
			tuvMake(2, 1, 0),
			tuvMake(2, 1, 1),
		};
		PYXIcosIndex expected[] =
		{
			PYXIcosIndex("A-000000000000"),
			PYXIcosIndex("A-000000000002"),
			PYXIcosIndex("A-000000000006"),
			PYXIcosIndex("A-000000000001"),
			PYXIcosIndex("A-000000000205"),
			PYXIcosIndex("A-000000000603"),
			PYXIcosIndex("A-000000000104"),
			PYXIcosIndex("A-000000000100"),
			PYXIcosIndex("A-000000000000"),
			PYXIcosIndex("A-000000000004"),
			PYXIcosIndex("A-000000000002"),
			PYXIcosIndex("A-000000000003"),
			PYXIcosIndex("A-000000000000"),
			PYXIcosIndex("A-000000000006"),
			PYXIcosIndex("A-000000000004"),
			PYXIcosIndex("A-000000000005"),
		};
		const int nCount = sizeof(tuv)/sizeof(tuv[0]);
		PYXIcosIndex index[nCount];

		TUV tuv2;
		tuvFromIndex(origin, tuv2, expected[0]);
		TEST_ASSERT(tuv2 == tuv[0]);
		tuvToIndexes(origin, tuv, index, nCount);
		for (int n = 0; n != nCount; ++n)
		{
			TEST_ASSERT(index[n] == expected[n]);
			if (tuvGetV(tuv[n]))
			{
				tuvFromIndex(origin, tuv2, expected[n]);
				TEST_ASSERT(tuv2 == tuv[n]);
			}
		}		
	}

	{
		//South Hexagons
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("K-0500"),PYXIcosIndex("K-050050010204002")) == tuvMake(1,200,15));
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("K-0506"),PYXIcosIndex("K-050060000010204")) == tuvMake(0,235,232));
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("K-0505"),PYXIcosIndex("K-050050000050202")) == tuvMake(0,243,5));
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("K-0504"),PYXIcosIndex("K-050050000050503")) == tuvMake(1,11,242));

		//North Hexagons
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("8-0202"),PYXIcosIndex("8-204050003030004")) == tuvMake(2,242,36));
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("8-2040"),PYXIcosIndex("8-204050003030001")) == tuvMake(0,35,242));
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("3-3002"),PYXIcosIndex("3-303060006000306")) == tuvMake(2,243,25));
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("3-3003"),PYXIcosIndex("3-303060006000006")) == tuvMake(0,28,243));

		//South Pentagons
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("8-00"),PYXIcosIndex("8-0003000605020")) == tuvMake(0,68,142)); //in the gap!
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("8-00"),PYXIcosIndex("8-0050000205005")) == tuvMake(0,6,238)); //in the gap!
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("8-00"),PYXIcosIndex("8-0030000502000")) == tuvMake(0,231,237)); //in the gap!
		
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("8-00"),PYXIcosIndex("8-0030000603004")) == tuvMake(2,5,237)); //outside the gap!
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("8-00"),PYXIcosIndex("8-0050000020106")) == tuvMake(1,235,4)); //outside the gap!

		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("8-03"),PYXIcosIndex("8-0030000040604")) == tuvMake(1,3,236)); //pentagon snowflake - in gap
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("8-03"),PYXIcosIndex("8-0050000040206")) == tuvMake(1,235,237)); //pentagon snowflake - in gap
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("8-03"),PYXIcosIndex("8-0050000040206")) == tuvMake(1,235,237)); //pentagon snowflake - in gap
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("8-03"),PYXIcosIndex("8-0030000303003")) == tuvMake(2,231,13)); //pentagon snowflake - in gap

		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("8-02"),PYXIcosIndex("8-0020000030000")) == tuvMake(1,234,234)); //pentagon snowflake - not in gap

		//North Pentagons
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("3-0"),PYXIcosIndex("3-002000050003")) == tuvMake(0,80,153)); //in the gap!
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("3-0"),PYXIcosIndex("3-020000050105")) == tuvMake(0,3,236)); //in the gap!
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("3-0"),PYXIcosIndex("3-060000306020")) == tuvMake(0,230,238)); //in the gap!
		
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("3-0"),PYXIcosIndex("3-060000403003")) == tuvMake(2,6,227)); //outside the gap!
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("3-0"),PYXIcosIndex("3-020000503060")) == tuvMake(1,238,11)); //outside the gap!

		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("3-2"),PYXIcosIndex("3-020000010606")) == tuvMake(2,234,4)); //pentagon snowflake - in gap
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("3-2"),PYXIcosIndex("3-060000010005")) == tuvMake(2,234,235)); //pentagon snowflake - in gap		
		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("3-2"),PYXIcosIndex("3-020000205003")) == tuvMake(1,7,238)); //pentagon snowflake - in gap

		TEST_ASSERT(makeTuvFromIndex(PYXIcosIndex("3-3"),PYXIcosIndex("3-030000020402")) == tuvMake(2,236,233)); //pentagon snowflake - not in gap
	}
}

};

Tester<UnitTester> gTester;

}

int tuvCountPacked(const unsigned char* pPack)
{
	assert(pPack);

	int nCount = 0;

	while (true)
	{
		unsigned int tByte = *pPack;
#if 1
		if ((tByte & 0x03) != 0x03) { tByte>>=2; ++nCount; } else break;
		if ((tByte & 0x03) != 0x03) { tByte>>=2; ++nCount; } else break;
		if ((tByte & 0x03) != 0x03) { tByte>>=2; ++nCount; } else break;
		if ((tByte & 0x03) != 0x03) { pPack+=9; ++nCount; } else break;
#else
		// TODO attempt to twiddle bits
		(tByte>>1) &= 0x55;
		ptr += 9;
#endif
	}

	return nCount;
}

int tuvPack4(unsigned char* pPack, const TUV* pTuv)
{
	*pPack = 0x00;

	int nCount = 0;

	while (nCount != 4)
	{
		int t = tuvGetTBits(*pTuv);

		if (t == 3)
		{
			break;
		}

		const int nShift = nCount * 2;

		// Set t.
		*pPack |= (t << nShift);

		// Set u and v.
		(*reinterpret_cast<TUV*>(pPack + nShift) &= 0xff0000ff) |=
			(*pTuv & 0x00ffff00);

		++pTuv;
		++nCount;
	}

	// Terminate unused t coords.
	*pPack |= (0xff << (nCount * 2));

	return nCount;
}

int tuvUnpack4(const unsigned char* pPack, TUV* pTuv)
{
	assert(pPack && pTuv);

	int nCount = 0;

	unsigned int tByte = 0x00000300 | *pPack;

	while ((tByte & 0x03) != 0x03)
	{
		*pTuv++ = (*reinterpret_cast<const TUV*>(pPack) & 0x00ffff00) | (tByte & 0x03);
		tByte >>= 2;
		pPack += 2;
		++nCount;
	}

	return nCount;
}

int tuvPackN(unsigned char* pPack, const TUV* pTuv, int nMaxCount)
{
	assert(pPack && pTuv && 0 <= nMaxCount);

	int nCount = 0;
	int nTarget = 4;

	while (nCount <= nMaxCount)
	{
		*pPack = 0x00;

		while (nCount != nTarget && nCount != nMaxCount)
		{
			int t = tuvGetTBits(*pTuv);

			if (t == 3)
			{
				break;
			}

			const int nShift = (nCount % 4) * 2;

			// Set t.
			*pPack |= (t << nShift);

			// Set u and v.
			(*reinterpret_cast<TUV*>(pPack + nShift) &= 0xff0000ff) |=
				(*pTuv & 0x00ffff00);

			++pTuv;
			++nCount;
		}

		if (nCount != nTarget)
		{
			// Terminate unused t coords.
			*pPack |= (0xff << ((nCount % 4) * 2));

			break;
		}

		pPack += 9;
		nTarget += 4;
	}

	return nCount;
}

int tuvUnpackN(const unsigned char* pPack, TUV* pTuv, int nMaxCount)
{
	assert(pPack && pTuv && 0 <= nMaxCount);

	int nCount = 0;
	int nTarget = 4;

	while (nCount < nMaxCount)
	{
		unsigned int tByte = 0x00000300 | *pPack;

		while (nCount < nMaxCount && (tByte & 0x03) != 0x03)
		{
			*pTuv++ = (*reinterpret_cast<const TUV*>(pPack) & 0x00ffff00) | (tByte & 0x03);
			tByte >>= 2;
			pPack += 2;
			++nCount;
		}

		if (nCount != nTarget)
		{
			break;
		}

		++pPack;
		nTarget += 4;
	}

	return nCount;
}

void tuvToIndex(const PYXIcosIndex& origin, TUV tuv, PYXIcosIndex& index)
{
	assert(!origin.isNull() && tuvGetT(tuv) != 3);

	// Set up cursor in v dir.
	PYXCursor c(origin, static_cast<PYXMath::eHexDirection>(origin.isNorthern() ? 2 : 5));

	switch (tuvGetT(tuv))
	{
		case 2:
			c.left();
			c.left();
			// fall through
		case 1:
			c.left();
			c.left();
	}

	// Move in v dir.
	int v = tuvGetV(tuv);
	c.forward(v);
	
	// Change to u dir.
	c.right();
	if (tuvGetT(tuv) || origin.isHexagon())
	{
		c.right();
	}

	// Move in u dir.
	int u = tuvGetU(tuv);
	c.forward(u);	

	index = c.getIndex();
}

void tuvFromIndexSafe(const PYXIcosIndex& root, const PYXIcosIndex& index, int& t, int& u, int& v)
{
	const int knResDiff = index.getResolution() - root.getResolution();

	assert(!root.isNull() && !index.isNull() && root.isAncestorOf(index) && knResDiff % 2);

	// Get last 11 digits.
	PYXIndex subseq = index.getSubIndex().subseq(
		index.getSubIndex().getDigitCount() - knResDiff, knResDiff);
	if (PYXMath::getHexClass(root.getResolution()) == PYXMath::knClassI)
	{
		subseq.prependDigit(0);
	}

	// Factor subsequence.
	PYXMath::eHexDirection nDir1; // primary dir
	PYXMath::eHexDirection nDir2; // secondary dir
	int nMoveU;
	int nMoveV;
	PYXMath::factor(subseq, &nDir1, &nMoveU, &nDir2, & nMoveV);
	int nDirD = nDir2 - nDir1;
	if (nDirD < -3)
	{
		nDirD += 6;
	}
	else if (3 < nDirD)
	{
		nDirD -= 6;
	}

	// Guess the U dir.
	int nDirU = root.isNorthern()
		? (((nDir1 + 4) | 0x1) % 6 + 1)
		: ((nDir1 - 1) | 0x1);
	int nDirV;

	
	//there is no problem with U Axes. However, when we are on petagon, we will need to rotate the UV after we done.
	bool coordBasedOnUAxes = false;

	if (nDirU == nDir1 && nDirD < 0 && 0 < nMoveV)
	{
		// Switch to previous square.
		nDirV = nDirU;
		nDirU = (nDirV + 3) % 6 + 1;
		std::swap(nDir1, nDir2);
		std::swap(nMoveU, nMoveV);
		nDirD = -nDirD;
	}
	else
	{
		// Stay on this square.
		if (nDirD < 0 && nMoveV != 0)
		{
			std::swap(nDir1, nDir2);
			std::swap(nMoveU, nMoveV);
			nDirD = -nDirD;
		}
		
		coordBasedOnUAxes = nDirU == nDir1;	
		nDirV = (nDirU + 1) % 6 + 1;
	}

	if (nDirD == 1)
	{
		if (nDir1 != nDirU)
		{
			// Expand cw.
			nMoveV += nMoveU;
		}
		else // nDir1 == nDir
		{
			// Expand ccw.
			nMoveU += nMoveV;
		}
	}
	else // nDirD == 2
	{
		if (nDir1 != nDirU)
		{
			if (nDir1 % 6 + 1  == nDirU)
			{
				// Rotate ccw.
				int nTmp = nMoveV;
				nMoveV -= nMoveU;
				nMoveU = nTmp;
			}
			else
			{
				// Rotate cw.
				int nTmp = nMoveU;
				nMoveU -= nMoveV;
				nMoveV = nTmp;
			}
		}
	}
	
	t = nDirU * 2 % 3;
	u = nMoveU;
	v = nMoveV;

	if (t == 0 && root.isPentagon() && coordBasedOnUAxes)
	{	
		//Rotate axes
		int old_v=v;
		int old_u=u;
		
		v = old_u;
		u = old_u-old_v;
	}	
}

void tuvFromIndex(const PYXIcosIndex& root, const PYXIcosIndex& index, int& t, int& u, int& v)
{
	// TODO this will be the new algorithm once it is working.

	if (root.isAncestorOf(index))
	{
		tuvFromIndexSafe(root, index, t, u, v);
		return;
	}

	// Use a temporary root.
	PYXIcosIndex tmpRoot(index);
	tmpRoot.setResolution(root.getResolution());
	tuvFromIndexSafe(tmpRoot, index, t, u, v);

	// Reparent to desired root.
	// TODO quicker way to get the rotation
	PYXMath::eHexDirection nDir = PYXIcosMath::getNeighbourDirection(root, tmpRoot);
	int nRot = 0;
	PYXIcosIndex rootCopy(root);
	PYXIcosMath::move(&rootCopy, nDir, &nRot);
	
	
	if (tmpRoot.isPentagon() && t==0)
	{		
		if (root.getClass() == 2)
		{
			//NOTE: the PYXIcosMath::move return nRot==0. However, our UV cordiantes are calucalted on the other side of the gap.
			//Therefore, we add a Rotation to fix that

			nRot++;
		}
	}

	// "Right" means we step off the right edge of a square,
	// otherwise we step off the left edge.
	bool bRight = (nDir + root.getClass() + (root.isNorthern() ? 1 : 0)) % 2 != 0;

	// "T rotation" is how we must modify T for the desired root.
	int nTRot;
	if (bRight)
	{
		nTRot = 3 - (((nRot % 5) + 3) / 2);

		//we have a boundry case that the index in low resolution is not acentor of the parent resolution.
		//and because v==0, the cell be in both partents. and the TUV cordiantes and PYXMath decided the other way around in this case.
		//so, we switch the TUV.
		if (v==0)
		{
			t=(t+2) % 3; // which is t=(t-1)%3 - but it doens't work no negative numbers
			//swap between u and v.
			v=u;
			u=0;
		}
	}
	else
	{
		nTRot = 2 - (((nRot / 2) + 1) % 3);
	}

	// Rotate T... but only if we are not crossing hemispheres.
	if (root.isNorthern() == tmpRoot.isNorthern())
	{
		(t += nTRot) %= 3;
	}

	// Transform U and V
	int uCpy = u;
	if (bRight)
	{
		if (nRot % 2)
		{
			if (v <= 243)
			{
				// Case AR
				u = 486 - u;
				v = 243 - v;
			}
			else
			{
				// Case CR
				u = -243 + v;
				v =  243 + v - uCpy;
				(t += 2) %= 3;
			}
		}
		else
		{
			if (v <= 243 + u)
			{
				// Case BR
				u = 486 - v;
				v = 243 - v + uCpy;
			}
			else
			{
				// Case DR
				u = -243 - uCpy + v;
				v =  243 - uCpy;
				(t += 2) %= 3;
			}
		}
	}
	else
	{
		if (nRot % 2)
		{
			if (u <= 243)
			{
				// Case AL
				u = 243 - u;
				v = 486 - v;
			}
			else
			{
				// Case CL
				u =  243 + uCpy - v;
				v = -243 + uCpy;
				(t += 1) %= 3;
			}
		}
		else
		{
			if (u <= 243 + v)
			{
				// Case BL
				u = 243 - uCpy + v;
				v = 486 - uCpy;
			}
			else
			{
				// Case DL
				u =  243 - v;
				v = -243 - v + uCpy;
				(t += 1) %= 3;
			}
		}
	}
}

void tuvFromIndex(const PYXIcosIndex& origin, TUV& tuv, const PYXIcosIndex& index)
{
	assert(!origin.isNull() && origin.getResolution() == index.getResolution());

	// TODO this algorithm is simple brute force search. we could be smarter.

	if (origin == index)
	{
		tuv = tuvMake(0, 0, 0);
		return;
	}

	const bool bIsHexagon = origin.isHexagon();

	for (int t = 0; t != 3; ++t)
	{
		PYXCursor cv(origin, static_cast<PYXMath::eHexDirection>(origin.isNorthern() ? 2 : 5));
		switch (t)
		{
			case 2:
				cv.left();
				cv.left();
				// fall through
			case 1:
				cv.left();
				cv.left();
		}

		// Skip v=0 to make shared edges between sheared squares unambiguous.
		cv.forward();

		for (int v = 1; v != 244; ++v, cv.forward())
		{
			PYXCursor cu(cv);
			cu.right();
			if (t || bIsHexagon)
			{
				cu.right();
			}

			for (int u = 0; u != 244; ++u, cu.forward())
			{
				if (cu.getIndex() == index)
				{
					tuv = tuvMake(t, u, v);
					return;
				}
			}
		}
	}

	assert(false && "not found");
	tuv = tuvMake(0, 0, 0);
}

void tuvToIndexes(const PYXIcosIndex& origin, const TUV* pTuv, PYXIcosIndex* pIndex, int nCount)
{
	assert(!origin.isNull() && pTuv && pIndex && 0 <= nCount);

	// TODO an obvious optimization is to reuse cursors from one tuv to the next

	while (nCount--)
	{
		tuvToIndex(origin, *pTuv++, *pIndex++);
	}
}

void tuvFromIndexes(const PYXIcosIndex& origin, TUV* pTuv, const PYXIcosIndex* pIndex, int nCount)
{
	assert(!origin.isNull() && pTuv && pIndex && 0 <= nCount);

	while (nCount--)
	{
		tuvFromIndex(origin, *pTuv++, *pIndex++);
	}
}
