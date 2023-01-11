/******************************************************************************
addr_utils.cpp

begin		: 2007-10-25
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#include "addr_utils.h"

// pyxlib includes
#include "pyxis/derm/index_math.h"

namespace
{

const struct info
{
	int off;          // byte offset from 24-bit block
	int s1;           // primary shift
	int s2;           // secondary shift
	unsigned char m1; // primary mask
	unsigned char m2; // secondary mask
} infoTable[] =
{
	{ 0, 5, 0, 0xe0, 0x00 },
	{ 0, 2, 0, 0x1c, 0x00 },
	{ 0, 1, 7, 0x03, 0x80 },
	{ 1, 4, 0, 0x70, 0x00 },
	{ 1, 1, 0, 0x0e, 0x00 },
	{ 1, 2, 6, 0x01, 0xc0 },
	{ 2, 3, 0, 0x38, 0x00 },
	{ 2, 0, 0, 0x07, 0x00 }
};
const struct next
{
	int off;
	int info;
} nextTable[] =
{
	{ 0, 1 },
	{ 0, 1 },
	{ 1, 1 },
	{ 0, 1 },
	{ 0, 1 },
	{ 1, 1 },
	{ 0, 1 },
	{ 1, -7 },
};
const char digitToChar[] = { '0', '1', '2', '3', '4', '5', '6', '\0' };

struct AutoTest
{
	AutoTest()
	{
		PYXIcosIndex index("M-02003");
		Addr addr("5302003");

		assert(indexToAddr(index) == addr);
		assert(addrToIndex(addr) == index);

		PYXIcosIndex i2("M-020030");
		i2 = PYXIcosMath::move(i2, PYXMath::knDirectionOne);
		Addr a2("53020031");
		assert(addrToIndex(a2) == i2);

		// TEMP testing
		unsigned char addrTest[] = { 0x05, 0x39, 0x77, 0x05, 0x39, 0x77 };
		int d[16];
		d[0] = addrGetDigit(addrTest, 0);
		d[1] = addrGetDigit(addrTest, 1);
		d[2] = addrGetDigit(addrTest, 2);
		d[3] = addrGetDigit(addrTest, 3);
		d[4] = addrGetDigit(addrTest, 4);
		d[5] = addrGetDigit(addrTest, 5);
		d[6] = addrGetDigit(addrTest, 6);
		d[7] = addrGetDigit(addrTest, 7);
		d[8] = addrGetDigit(addrTest, 8);
		d[9] = addrGetDigit(addrTest, 9);
		d[10] = addrGetDigit(addrTest, 10);
		d[11] = addrGetDigit(addrTest, 11);
		d[12] = addrGetDigit(addrTest, 12);
		d[13] = addrGetDigit(addrTest, 13);
		d[14] = addrGetDigit(addrTest, 14);
		d[15] = addrGetDigit(addrTest, 15);

		addrSetDigit(addrTest, 0, 3);
		addrSetDigit(addrTest, 1, 2);
		addrSetDigit(addrTest, 2, 6);
		addrSetDigit(addrTest, 3, 4);
		addrSetDigit(addrTest, 4, 1);
		addrSetDigit(addrTest, 5, 7);
		addrSetDigit(addrTest, 6, 0);
		addrSetDigit(addrTest, 7, 5);
		assert(addrGetDigit(addrTest, 0) == 3);
		assert(addrGetDigit(addrTest, 1) == 2);
		assert(addrGetDigit(addrTest, 2) == 6);
		assert(addrGetDigit(addrTest, 3) == 4);
		assert(addrGetDigit(addrTest, 4) == 1);
		assert(addrGetDigit(addrTest, 5) == 7);
		assert(addrGetDigit(addrTest, 6) == 0);
		assert(addrGetDigit(addrTest, 7) == 5);
		assert(addrGetDigit(addrTest, 0) == 3);
		assert(addrGetDigit(addrTest, 1) == 2);
		assert(addrGetDigit(addrTest, 2) == 6);
		assert(addrGetDigit(addrTest, 3) == 4);
		assert(addrGetDigit(addrTest, 4) == 1);
		assert(addrGetDigit(addrTest, 5) == 7);
		assert(addrGetDigit(addrTest, 6) == 0);
		assert(addrGetDigit(addrTest, 7) == 5);

		char str[41];
		addrToString(addrTest, str);
		int n = 343;

		const char* s2 = "20102030405060";
		unsigned char a[20];
		addrFromString(a, s2);
		int b = 1;
		assert(addrGetDigit(a, 0) == 2);
		assert(addrGetDigit(a, b+0) == 0);
		assert(addrGetDigit(a, b+1) == 1);
		assert(addrGetDigit(a, b+2) == 0);
		assert(addrGetDigit(a, b+3) == 2);
		assert(addrGetDigit(a, b+4) == 0);
		assert(addrGetDigit(a, b+5) == 3);
		assert(addrGetDigit(a, b+6) == 0);
		assert(addrGetDigit(a, b+7) == 4);
		assert(addrGetDigit(a, b+8) == 0);
		assert(addrGetDigit(a, b+9) == 5);
		assert(addrGetDigit(a, b+10) == 0);
		assert(addrGetDigit(a, b+11) == 6);
		assert(addrGetDigit(a, b+12) == 0);
		assert(addrGetDigit(a, b+13) == 7);
	}
} autoTest;

}

/*
Encoding:
1-6  -> (1,1-6)
7-12 -> (2,1-6)
A-E  -> (3,1-5)
F-J  -> (4,1-5)
K-O  -> (5,1-5)
P-T  -> (6,1-5)
*/
unsigned char encodeIndexRoot(const PYXIcosIndex& index)
{
	int nRoot = index.getPrimaryResolution();

	if (nRoot <= 6)
	{
		return (1<<4) | (nRoot - 1 + 1);
	}
	else if (nRoot <= 12)
	{
		return (2<<4) | (nRoot - 7 + 1);
	}
	else if (nRoot <= 'E')
	{
		return (3<<4) | (nRoot - 'A' + 1);
	}
	else if (nRoot <= 'J')
	{
		return (4<<4) | (nRoot - 'F' + 1);
	}
	else if (nRoot <= 'O')
	{
		return (5<<4) | (nRoot - 'K' + 1);
	}
	else if (nRoot <= 'T')
	{
		return (6<<4) | (nRoot - 'P' + 1);
	}

	return 0xff;
}

int decodeIndexRoot(const Addr& addr)
{
	int n1 = addr[0];
	int n2 = addr[1];

	if (n1 == 1 && 1 <= n2 && n2 <= 6)
	{
		return (1 - 1) + n2;
	}
	else if (n1 == 2 && 1 <= n2 && n2 <= 6)
	{
		return (7 - 1) + n2;
	}
	else if (n1 == 3 && 1 <= n2 && n2 <= 5)
	{
		return ('A' - 1) + n2;
	}
	else if (n1 == 4 && 1 <= n2 && n2 <= 5)
	{
		return ('F' - 1) + n2;
	}
	else if (n1 == 5 && 1 <= n2 && n2 <= 5)
	{
		return ('K' - 1) + n2;
	}
	else if (n1 == 6 && 1 <= n2 && n2 <= 5)
	{
		return ('P' - 1) + n2;
	}

	return 0xff;
}

Addr indexToAddr(const PYXIcosIndex& index)
{
	Addr addr;

	// TODO this could be set as a whole character for performance
	unsigned char nRootCode = encodeIndexRoot(index);
	addr[0] = nRootCode>>4;
	addr[1] = nRootCode&0xf;

	const PYXIndex& sub = index.getSubIndex();
	int nRes = 2;
	while (nRes <= index.getResolution())
	{
		addr[nRes] = sub.getDigit(nRes - 2);
		++nRes;
	}
	if (nRes <= Addr::maxn)
	{
		addr[nRes] = 0xf;
	}

	return addr;
}

PYXIcosIndex addrToIndex(const Addr& addr)
{
	PYXIcosIndex index;

	assert(decodeIndexRoot(addr) != 0xff);
	index.setPrimaryResolution(decodeIndexRoot(addr));

	int nRes = 2;
	while (nRes <= addr.maxn)
	{
		if (addr[nRes] == 0xf)
		{
			break;
		}
		PYXIcosMath::zoomIntoNeighbourhood(&index, static_cast<PYXMath::eHexDirection>(addr[nRes]));
		++nRes;
	}

	return index;
}

int addrGetSize(const unsigned char* addr)
{
	int n = 0;
	while (addrGetDigit(addr, n) != 0x7)
	{
		++n;
	}
	return n;
}

int addrGetSize(const unsigned char* addr, int nMaxSize)
{
	assert(0 <= nMaxSize);
	int n = 0;
	while (n < nMaxSize && addrGetDigit(addr, n) != 0x7)
	{
		++n;
	}
	return n;
}

int addrGetDigit(const unsigned char* addr, int n)
{
	const info* pInfo = infoTable + n%8;
	addr += (n/8*3) + pInfo->off;
	if (pInfo->s2)
	{
		return (*addr & pInfo->m1) << pInfo->s1 | (*(addr+1) & pInfo->m2) >> pInfo->s2;
	}
	else
	{
		return (*addr & pInfo->m1) >> pInfo->s1;
	}
}

void addrSetDigit(unsigned char* addr, int n, int nDigit)
{
	const info* pInfo = infoTable + n%8;
	addr += (n/8*3) + pInfo->off;
	if (pInfo->s2)
	{
		(*addr &= ~pInfo->m1) |= (nDigit >> pInfo->s1);
		(*(addr+1) &= ~pInfo->m2) |= (nDigit << pInfo->s2);
	}
	else
	{
		(*addr &= ~pInfo->m1) |= (nDigit << pInfo->s1);
	}
}

// Assumes str memory is sufficient to hold addr
void addrToString(const unsigned char* addr, char* str)
{
	const info* pInfo = infoTable;
	const next* pNext = nextTable;
	while (true)
	{
		int nDigit = (pInfo->s2)
			? ((*addr & pInfo->m1) << pInfo->s1 | (*(addr+1) & pInfo->m2) >> pInfo->s2)
			: ((*addr & pInfo->m1) >> pInfo->s1);

		*str++ = digitToChar[nDigit];

		if (nDigit == 7)
		{
			break;
		}

		addr += pNext->off;
		pInfo += pNext->info;
		pNext += pNext->info;
	}
}

// Assumes addr memory is sufficient to hold str
void addrFromString(unsigned char* addr, const char* str)
{
	const info* pInfo = infoTable;
	const next* pNext = nextTable;
	while (true)
	{
		unsigned char nDigit = (*str)
			? *str - '0'
			: 0x7;

		if (pInfo->s2)
		{
			(*addr &= ~pInfo->m1) |= (nDigit >> pInfo->s1);
			(*(addr+1) &= ~pInfo->m2) |= (nDigit << pInfo->s2);
		}
		else
		{
			(*addr &= ~pInfo->m1) |= (nDigit << pInfo->s1);
		}

		if (!*str++)
		{
			break;
		}

		addr += pNext->off;
		pInfo += pNext->info;
		pNext += pNext->info;
	}
}

void addr4to3(const Addr& a4, unsigned char* a3)
{
	int n = 0;
	int nDigit;

	do
	{
		nDigit = a4[n];
		if (nDigit == 0xf)
		{
			nDigit = 0x7;
		}
		addrSetDigit(a3, n, nDigit);
		++n;
	} while (nDigit != 0x7);
}
