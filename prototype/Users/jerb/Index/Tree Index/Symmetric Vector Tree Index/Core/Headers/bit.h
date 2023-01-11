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

# if !defined(HEADERS__BIT)
	# define HEADERS__BIT

	# include "boolean.h"
	# include "core.h"

	# define Bit_miCountInChar() CHAR_BIT
	# define Bit_miuCount(Expression) (Bit_miCountInChar() * Size_miuCharCount(Expression))
	
	/* Bit offset is 0-based, from right.
	The assertion is present because left-shifting off the end is undefined.
	*/
	# define Bit_miuMask(iuBitOffset) ( \
		mAssert((iuBitOffset) >= 0 && (iuBitOffset) < miuUnsignedTypeMaximum(int long unsigned)), \
		(1LU << (iuBitOffset)) \
	)

	/* O(1). */
	# define Bit_mbOn(iBits, iuBitOffset) !!((iBits) & Bit_miuMask(iuBitOffset))

	/* O(1). */
	# define Bit_miOn(iBits, iuBitOffset) ((iBits) | Bit_miuMask(iuBitOffset))
	
	/* O(1). */
	# define Bit_miOff(iBits, iuBitOffset) ((iBits) & ~Bit_miuMask(iuBitOffset)) 

	/* O(1). */
	# define Bit_miLowestOff(iBits) ((iBits) & ((iBits) - 1))

	/* Count the 'on' bits using the Kernighan method.  O(n), where n is the number of 'on' bits. */
	static INLINE int unsigned Bit_fiuOnCount(
		REGISTER int unsigned iuBits
	) {
		REGISTER int unsigned iuCount = 0;
		for (; iuBits; ++iuCount) iuBits &= (iuBits - 1); /* Clear the least significant 'on' bit. */
		return iuCount;
	}
	
	static INLINE Boolean_tb Bit_fbSingleOn(
		REGISTER int unsigned const iuBits
	) {
		return iuBits && (0 == Bit_miLowestOff(iuBits)); /* Clear the least significant 'on' bit. */
 	}
 	
# endif
