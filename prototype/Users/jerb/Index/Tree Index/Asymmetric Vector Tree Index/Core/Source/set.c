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

# include <assert.h>
# include "../Headers/Bit.h"
# include "../Headers/Set.h"
# include "../Headers/Size.h"

typedef tiuSet tiuMask;

static tbBoolean fbMask(
	REGISTER int unsigned const iuElement,
	REGISTER tiuMask riuMask[mReferenceConst(1)]
) {
	mStrongAssert(riuMask);

	if (iuElement >= Bit_miuCount(*riuMask)) return 0; /* Too big. */
	*riuMask = (1U << iuElement);
	return 1;
}

/*
Set
*/

tiuSet Set_fiuEmpty(void) {
	return 0;
}

tbBoolean Set_fbAdd(
	REGISTER tiuSet riuSet[mReferenceConst(1)],
	REGISTER int unsigned const iuElement
) {
	mStrongAssert(riuSet);

	{
		tiuMask iuMask;
		if (!fbMask(iuElement, &iuMask)) return 0; /* Not enough room. */
		if (*riuSet & iuMask) return 0; /* Already there. */
		*riuSet |= iuMask;
	}
	return 1;
}

tbBoolean Set_fbRemove(
	REGISTER tiuSet riuSet[mReferenceConst(1)],
	REGISTER int unsigned const iuElement
) {
	mStrongAssert(riuSet);

	{
		tiuMask iuMask;
		if (!fbMask(iuElement, &iuMask)) return 0; /* Too big. */
		if (!(*riuSet & iuMask)) return 0; /* Not there. */
		*riuSet &= (~iuMask);
	}
	return 1;
}

tbBoolean Set_fbPop(
	REGISTER tiuSet riuSet[mReferenceConst(1)],
	REGISTER int unsigned * const piuElement
) {
	mStrongAssert(riuSet);

	{
		REGISTER tiuSet iuSet = *riuSet;
		REGISTER int unsigned iuElement = 0;
		for (; iuSet; iuSet >>= 1, ++iuElement) if (iuSet & 1) {
			if (piuElement) *piuElement = iuElement;
			*riuSet &= (*riuSet - 1); /* Clear the least significant bit set. */
			return 1;
		}
	}
	return 0;
}

tbBoolean Set_fbContains(
	REGISTER tiuSet const iuSet,
	REGISTER int unsigned const iuElement
) {
	tiuMask iuMask;
	return (fbMask(iuElement, &iuMask) && (iuSet & iuMask));
}

/* Uses Kernighan method: http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan */
int unsigned Set_fiuCount(
	REGISTER tiuSet iuSet
) {
	REGISTER int unsigned iuCount = 0;
	for (; iuSet; ++iuCount) iuSet &= (iuSet - 1); /* Clear the least significant bit set.  */
	return iuCount;
}

tbBoolean Set_fbTest(void) {
	tiuSet iuSet = Set_fiuEmpty();
	int unsigned iuElement = 0;

	assert(0 == Set_fiuCount(iuSet));

	/* Add. */
	assert(Set_fbAdd(&iuSet, 4));
	assert(16 == iuSet);
	assert(1 == Set_fiuCount(iuSet));
	assert(Set_fbContains(iuSet, 4));
	assert(!Set_fbContains(iuSet, 3));

	/* Add another. */
	assert(Set_fbAdd(&iuSet, 3));
	assert(24 == iuSet);
	assert(2 == Set_fiuCount(iuSet));
	assert(Set_fbContains(iuSet, 4));
	assert(Set_fbContains(iuSet, 3));

	/* Add same again; verify failure. */
	assert(!Set_fbAdd(&iuSet, 3));
	assert(24 == iuSet);
	assert(2 == Set_fiuCount(iuSet));
	assert(Set_fbContains(iuSet, 4));
	assert(Set_fbContains(iuSet, 3));

	/* Add one that's too large; verify failure. */
	assert(!Set_fbAdd(&iuSet, 256));
	assert(24 == iuSet);
	assert(2 == Set_fiuCount(iuSet));
	assert(Set_fbContains(iuSet, 4));
	assert(Set_fbContains(iuSet, 3));
	assert(!Set_fbContains(iuSet, 256));

	/* Pop. */
	assert(Set_fbPop(&iuSet, &iuElement));
	assert(3 == iuElement);
	assert(16 == iuSet);
	assert(1 == Set_fiuCount(iuSet));
	assert(Set_fbContains(iuSet, 4));
	assert(!Set_fbContains(iuSet, 3));

	/* Remove same one; verify failure. */
	assert(!Set_fbRemove(&iuSet, iuElement));
	assert(16 == iuSet);
	assert(1 == Set_fiuCount(iuSet));
	assert(Set_fbContains(iuSet, 4));
	assert(!Set_fbContains(iuSet, 3));
	
	/* Remove other one. */
	assert(Set_fbRemove(&iuSet, 4));
	assert(0 == iuSet);
	assert(0 == Set_fiuCount(iuSet));
	assert(!Set_fbContains(iuSet, 4));
	assert(!Set_fbContains(iuSet, 3));

	return 1;
}
