#pragma once
#ifndef VIEW_MODEL__TUV_H
#define VIEW_MODEL__TUV_H
/******************************************************************************
tuv.h

begin		: 2007-12-14
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"

// view model includes

// pyxlib includes

// standard includes

// forward declarations
class PYXIcosIndex;

//! TUV is laid out like this: 0x00VVUUTT
typedef unsigned int TUV;

// Some helpful masks.
const int knTuvMaskTBits = 0x00000003; // only bits
const int knTuvMaskT = 0x000000ff; // whole byte
const int knTuvMaskU = 0x0000ff00;
const int knTuvMaskV = 0x00ff0000;

// Some helpful getters.
inline int tuvGetTBits(TUV tuv) { return tuv & 0x03; }
inline int tuvGetT(TUV tuv) { return tuv & 0xff; }
inline int tuvGetU(TUV tuv) { return (tuv >> 8) & 0xff; }
inline int tuvGetV(TUV tuv) { return (tuv >> 16) & 0xff; }

// Some helpful setters.
inline void tuvSetT(TUV& tuv, int t) { (tuv &= 0xffffff00) |= t; }
inline void tuvSetU(TUV& tuv, int u) { (tuv &= 0xffff00ff) |= (u << 8);}
inline void tuvSetV(TUV& tuv, int v) { (tuv &= 0xff00ffff) |= (v << 16);}

//! Makes a tuv.
inline TUV tuvMake(int t, int u, int v)
{
	return (v << 16) | (u << 8) | t;
}

// GRATUITOUS DOCUMENTATION FOR PACKED FORMAT
// Assume tuv coordinates are organized into groups of 4 stored in blocks of 9 bytes:
// block[0] is the 4 t coordinates: 0xt3t2t1t0
// block[1] is u0, block[2] is v0
// block[3] is u1, block[4] is v1
// block[5] is u2, block[6] is v2
// block[7] is u3, block[8] is v3
// The t coordinates can be 0, 1, or 2; the unused value 3 can function as a terminator.
// Hence, we can in the first byte, encode whether there are 0, 1, 2, 3, or 4 coords present,
// and omit subsequent uv pairs as necessary.

//! Packs four t coordinates into a single byte.
inline int tuvPack4T(int t0, int t1, int t2, int t3)
{
	return (t3<<6) | (t2<<4) | (t1<<2) | (t0);
}

//! Returns the number of bytes required to store nCount packed tuvs.
inline int tuvGetRequiredPackBytes(int nCount)
{
	// (nCount/4)*9 + 1 + (nCount%4)*2
	return (nCount>>2)*9 + 1 + ((nCount&0x03)<<1);
}

//! Returns the number of tuv coordinates present.
int tuvCountPacked(const unsigned char* pPack);

/*!
Packs tuv data in groups of 4.
\param pPack	Packed tuv data.
\param pTuv		Unpacked tuv data.
\return The number of tuvs packed (0 to 4).
*/
int tuvPack4(unsigned char* pPack, const TUV* pTuv);

/*!
Unpacks tuv data in groups of 4.
\param pPack	Packed tuv data.
\param pTuv		Unpacked tuv data.
\return The number of tuvs unpacked (0 to 4).
*/
int tuvUnpack4(const unsigned char* pPack, TUV* pTuv);

/*!
Packs at most N tuv coordinates.
\param pPack		Packed tuv data.
\param pTuv			Unpacked tuv data.
\param nMaxCount	The maximum number to pack.
\return The number of tuvs packed (0 to N).
*/
int tuvPackN(unsigned char* pPack, const TUV* pTuv, int nMaxCount);

/*!
Unpacks at most N tuv coordinates.
\param pPack		Packed tuv data.
\param pTuv			Unpacked tuv data.
\param nMaxCount	The maximum number to unpack.
\return The number of tuvs unpacked (0 to N).
*/
int tuvUnpackN(const unsigned char* pPack, TUV* pTuv, int nMaxCount);

/*!
Converts a tuv to an index.
\param origin	The origin of the tuv coordinate system, at the desired res.
\param tuv		The tuv.
\param index	The index.
*/
void tuvToIndex(const PYXIcosIndex& origin, TUV tuv, PYXIcosIndex& index);

/*!
Converts a tuv from an index.
\param origin	The root of the tuv coordinate system, at the desired res.
\param index	The index.
\param t		The t coordinate.
\param u		The u coordinate.
\param v		The v coordinate.
*/
void tuvFromIndexSafe(const PYXIcosIndex& root, const PYXIcosIndex& index, int& t, int& u, int& v);

/*!
Converts a tuv from an index.
\param root		The origin of the tuv coordinate system, at the desired res.
\param index	The index.
\param t		The t coordinate.
\param u		The u coordinate.
\param v		The v coordinate.
*/
void tuvFromIndex(const PYXIcosIndex& root, const PYXIcosIndex& index, int& t, int& u, int& v);

/*!
Converts a tuv from an index.
\param origin	The origin of the tuv coordinate system, at the desired res.
\param tuv		The tuv.
\param index	The index.
*/
void tuvFromIndex(const PYXIcosIndex& origin, TUV& tuv, const PYXIcosIndex& index);

/*!
Converts several tuvs to indexes.
\param origin	The origin of the tuv coordinate system, at the desired res.
\param pTuv		The tuv array.
\param pIndex	The index array.
\param nCount	How many to convert.
*/
void tuvToIndexes(const PYXIcosIndex& origin, const TUV* pTuv, PYXIcosIndex* pIndex, int nCount);

/*!
Converts several tuvs from indexes.
\param origin	The origin of the tuv coordinate system, at the desired res.
\param pTuv		The tuv array.
\param pIndex	The index array.
\param nCount	How many to convert.
*/
void tuvFromIndexes(const PYXIcosIndex& origin, TUV* pTuv, const PYXIcosIndex* pIndex, int nCount);

#endif
