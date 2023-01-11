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

# if !defined(HEADERS__SIZE)
	# define HEADERS__SIZE

	# include "boolean.h"
	# include "core.h"
	# include <stdlib.h> /* size_t */
	# include <stddef.h> /* offsetof */

	/* This is a format specifier for the type, to be used in print format strings. */
	# if (mbC99() || (mbGnuC() && defined(_STDINT_H)))
		# define Size_mrzcFormatter() "zu"
	# else
		# define Size_mrzcFormatter() "lu"	/* 'size_t' isn't bigger than 'long' prior to C99.	*/
	# endif

	/* Fundamental sizes. */
	# define Size_miuCharCount(Expression) (sizeof (Expression))
	# define Size_miuCharOffset(Expression, Field) (offsetof(Expression, Field))
	# define Size_miuArrayElementCount(aArray) (Size_miuCharCount(aArray) / Size_miuCharCount(*(aArray)))

	/* A 'size' type is large enough to hold the byte count of any allocated object. */
	typedef size_t Size_tiu;

	# define Size_miuMaximum() miuUnsignedTypeMaximum(Size_tiu)

	static INLINE Boolean_tb Size_fbCanAdd(
		REGISTER Size_tiu const iuAugend,
		REGISTER Size_tiu const iuAddend
	) {
		return ((miuUnsignedTypeMaximum(Size_tiu) - iuAddend) >= iuAugend);
	}

	static INLINE Boolean_tb Size_fbAdd(
		REGISTER Size_tiu riuAugend[mReferenceConst(1)],
		REGISTER Size_tiu const iuAddend
	) {
		mStrongAssert(riuAugend);

		if (!Size_fbCanAdd(*riuAugend, iuAddend)) return 0;
		*riuAugend += iuAddend;
		return 1;
	}

	static INLINE Boolean_tb Size_fbOdd(
		REGISTER Size_tiu const iuCount
	) {
		return 0 != (iuCount & 1);
	}

# endif
