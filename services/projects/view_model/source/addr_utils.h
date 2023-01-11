#pragma once
#ifndef VIEW_MODEL__ADDR_UTILS_H
#define VIEW_MODEL__ADDR_UTILS_H
/******************************************************************************
addr_utils.h

begin		: 2007-10-25
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"

// view model includes
#include "addr.h"

// pyxlib includes
#include "pyxis/derm/index.h"

unsigned char encodeIndexRoot(const PYXIcosIndex& index);

int decodeIndexRoot(const Addr& addr);

Addr indexToAddr(const PYXIcosIndex& index);

PYXIcosIndex addrToIndex(const Addr& addr);

////////////////////////////////////////////////////////////////////////////////
// This next group of functions treats an index as an array of 3-bit values.

// Returns the number of bytes required to store digits.
// Number of digits must be positive (non-zero).
inline int addrSizeToByteCount(int nSize)
{
	assert(0 < nSize);
	return (nSize*3-1)/8+1; // 8-bit char assumed
}

int addrGetSize(const unsigned char* addr);
int addrGetSize(const unsigned char* addr, int nMaxSize);

int addrGetDigit(const unsigned char* addr, int n);
void addrSetDigit(unsigned char* addr, int n, int nDigit);

void addrToBytes(const unsigned char* addr, unsigned char* bytes);
void addrFromBytes(unsigned char* addr, const unsigned char* bytes);

void addrToString(const unsigned char* addr, char* str);
void addrFromString(unsigned char* addr, const char* str);

////////////////////////////////////////////////////////////////////////////////

void addr4to3(const Addr& a4, unsigned char* a3);

#endif
