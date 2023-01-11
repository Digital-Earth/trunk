/* Copyright (c) 2007 Jason Erb

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

# if !defined(HEADERS__CHAR_STRING)
	# define HEADERS__CHAR_STRING

	# include "core.h"
	# include "size.h"
	# include <string.h>

	/* A null-terminated array of characters.
	All memory management of the string is left up to the caller.
	*/
	typedef char CharString_tzc;

	/* Resolves to the number of chars in the string (not including the terminator). */
	# define CharString_miuCount(rzcCharString) (strlen(rzcCharString))

	/* Resolves to true if equivalent. */
	# define CharString_mbEquivalent(rzcCharStringLeft, rzcCharStringRight) ( \
		0 == strcmp((rzcCharStringLeft), (rzcCharStringRight)) \
	)

	/* Resolves to the beginning section of a string. */
	# define CharString_mCopy(rzcCharString, rzcSource, iuSize) (strncpy((rzcCharString), (rzcSource), (iuSize)))

	/* Push a character onto the end of the string. */
	static INLINE Size_tiu CharString_fiuPush(
		REGISTER Size_tiu iuSize,
		REGISTER CharString_tzc rzcCharString[mReferenceConst(iuSize + 2)],
		REGISTER char const cChar
	) {
		mAssert(rzcCharString);
		mAssert(iuSize == CharString_miuCount(rzcCharString));
		mAssert(iuSize < miuUnsignedTypeMaximum(Size_tiu));
		mAssert(0 == rzcCharString[iuSize]); /* Null terminator. */

		rzcCharString[iuSize] = cChar;
		rzcCharString[++iuSize] = 0;
		return iuSize;
	}
	
	/* Pop a character from the end of the string. */
	static INLINE Size_tiu CharString_fiuPop(
		REGISTER Size_tiu iuSize,
		REGISTER CharString_tzc rzcCharString[mReferenceConst(iuSize + 1)]
	) {
		mAssert(rzcCharString);
		mAssert(iuSize == CharString_miuCount(rzcCharString));
		mAssert(0 < iuSize);
		mAssert(0 == rzcCharString[iuSize]); /* Null terminator. */

		rzcCharString[--iuSize] = 0;
		return iuSize;
	}

	/* Tests the code. */
	Boolean_tb CharString_fbTest(void);

# endif
