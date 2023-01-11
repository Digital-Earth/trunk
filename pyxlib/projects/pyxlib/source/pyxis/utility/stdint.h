#ifndef PYXIS__UTILITY__STDINT_H
#define PYXIS__UTILITY__STDINT_H
/******************************************************************************
stdint.h

begin		: 2006-02-24
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

// C99 defines fixed-width integer types in <stdint.h>.
// MS Visual Studio .NET 2003 doesn't yet provide that file.
// This file, which defines a subset, can be used in the meantime.

//! Type definition for signed 8-bit integer.
PYXLIB_DECL typedef signed char int8_t;

//! Type definition for unsigned 8-bit integer.
PYXLIB_DECL typedef unsigned char uint8_t;

//! Type definition for signed 16-bit integer.
PYXLIB_DECL typedef signed short int16_t;

//! Type definition for unsigned 16-bit integer.
PYXLIB_DECL typedef unsigned short uint16_t;

//! Type definition for signed 32-bit integer.
PYXLIB_DECL typedef signed int int32_t;

//! Type definition for unsigned 32-bit integer.
PYXLIB_DECL typedef unsigned int uint32_t;

// We don't yet support 64-bit integers!
#if 0

//! Type definition for signed 64-bit integer.
PYXLIB_DECL typedef signed long int64_t;

//! Type definition for unsigned 64-bit integer.
PYXLIB_DECL typedef unsigned long uint64_t;

#endif

#endif // guard
