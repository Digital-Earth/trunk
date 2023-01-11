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

# if !defined(HEADERS__MEMORY)
	# define HEADERS__MEMORY

	# include "boolean.h"
	# include "core.h"
	# include "size.h"
	# include <stdlib.h> /* realloc	*/

	/* These return a pointer to newly-allocated memory. */
	# define Memory_mpAllocate(iuSize) ((0 < (iuSize))? malloc(iuSize): 0)
	# define Memory_mpAllocateAndZero(iuSize) ((0 < (iuSize))? calloc(1, (iuSize)): 0)

	/* These modify the first argument, the pointer, to point to the result, and return 0 if unsuccessful. */
	# define Memory_mbAllocate(pResult, iuSize) (0 != ((pResult) = Memory_mpAllocate(iuSize)))
	# define Memory_mbAllocateAndZero(pResult, iuSize) (0 != ((pResult) = Memory_mpAllocateAndZero(iuSize)))
	# define Memory_mbReallocate(pResult, iuSize) Memory_fbReallocate((void * * const)&(pResult), (iuSize))
	# define Memory_mbDeallocate(pResult) (free(pResult), 1)
	# define Memory_mbDeallocateAndZero(pResult) (Memory_mbDeallocate(pResult), 0 == ((pResult) = 0))

	/* This reallocates the memory pointed to and modifies the pointer accordingly (or returns 0 if unsuccessful). */
	static INLINE Boolean_tb Memory_fbReallocate(
		REGISTER void * rpMemory[mReferenceConst(1)],
		REGISTER Size_tiu const iuByteCount
	) {
		mAssert(rpMemory);

		if (0 == iuByteCount) {
			/* This is here because the return of 'realloc(0, 0)' is implementation-defined. */
			Memory_mbDeallocateAndZero(*rpMemory);
			return 1;
		}
		{
			/* If '*rpMemory' is null, 'realloc' acts like 'malloc'.	
			Note that if this is unsuccessful, the original memory block is unchanged.
			*/
			REGISTER void * const pNew = realloc(*rpMemory, iuByteCount);
			
			/* If the size could not be changed, return false. */
			if (!pNew) return 0;

			*rpMemory = pNew;
		}
		return 1;
	}

# endif
