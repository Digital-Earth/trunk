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

# include "../Headers/modulo_7_set.h"

/*
Modulo7Set
*/

/* Returns lowest direction, or 0 if none. */
int Modulo7Set_fiLowest(
	REGISTER Modulo7Set_tc const cModulo7Set
) {
	/* An array mapping a nybble to one-based position of the lowest 'on' bit. */
	static int const aiLowestOffset[16] = {
		0 /* 0000 */,
		1 /* 0001 */,
		2 /* 0010 */,
		1 /* 0011 */,
		3 /* 0100 */,
		1 /* 0101 */,
		2 /* 0110 */,
		1 /* 0111 */,
		4 /* 1000 */,
		1 /* 1001 */,
		2 /* 1010 */,
		1 /* 1011 */,
		3 /* 1100 */,
		1 /* 1101 */,
		2 /* 1110 */,
		1 /* 1111 */
	};

	REGISTER int iLowestOffset = aiLowestOffset[cModulo7Set & 15 /* 1111 */];
	mAssert((cModulo7Set >> 4) < 16);
	if (0 == iLowestOffset && 0 != (iLowestOffset = aiLowestOffset[cModulo7Set >> 4])) iLowestOffset += 4;
	return iLowestOffset;
}

Boolean_tb Modulo7Set_fbTest(void) {
	REGISTER Modulo7Set_tc cModulo7Set = Modulo7Set_fcEmpty();

	mAssert(0 == Modulo7Set_fiuCount(cModulo7Set));

	/* Add. */
	cModulo7Set = Modulo7Set_fcInsert(cModulo7Set, Modulo7_i4);
	if (1 != Modulo7Set_fiuCount(cModulo7Set)) return mbVerify(0);
	if (!Modulo7Set_fbContains(cModulo7Set, Modulo7_i4)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i1)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i2)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i3)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i5)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i6)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i0)) return mbVerify(0);

	/* Add another. */
	cModulo7Set = Modulo7Set_fcInsert(cModulo7Set, Modulo7_i3);
	if (2 != Modulo7Set_fiuCount(cModulo7Set)) return mbVerify(0);
	if (!Modulo7Set_fbContains(cModulo7Set, Modulo7_i4)) return mbVerify(0);
	if (!Modulo7Set_fbContains(cModulo7Set, Modulo7_i3)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i1)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i2)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i5)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i6)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i0)) return mbVerify(0);

	/* Add same again; verify that nothing changes. */
	if (cModulo7Set != Modulo7Set_fcInsert(cModulo7Set, Modulo7_i3)) return mbVerify(0);

	/* Remove one. */
	cModulo7Set = Modulo7Set_fcRemove(cModulo7Set, Modulo7_i3);
	if (1 != Modulo7Set_fiuCount(cModulo7Set)) return mbVerify(0);
	if (!Modulo7Set_fbContains(cModulo7Set, Modulo7_i4)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i1)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i2)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i3)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i5)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i6)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i0)) return mbVerify(0);
	
	/* Remove other one. */
	cModulo7Set = Modulo7Set_fcRemove(cModulo7Set, Modulo7_i4);
	if (0 != Modulo7Set_fiuCount(cModulo7Set)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i4)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i1)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i2)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i3)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i5)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i6)) return mbVerify(0);
	if (Modulo7Set_fbContains(cModulo7Set, Modulo7_i0)) return mbVerify(0);

	/* Singleton. */
	{
		REGISTER Modulo7Set_tc cSet = Modulo7Set_fcEmpty();
		if (Modulo7Set_fbSingleton(cSet)) return mbVerify(0);
		cSet = Modulo7Set_fcInsert(cSet, Modulo7_i6);
		if (!Modulo7Set_fbSingleton(cSet)) return mbVerify(0);
		cSet = Modulo7Set_fcInsert(cSet, Modulo7_i2);
		if (Modulo7Set_fbSingleton(cSet)) return mbVerify(0);
		cSet = Modulo7Set_fcRemove(cSet, Modulo7_i6);
		if (!Modulo7Set_fbSingleton(cSet)) return mbVerify(0);
	}

	return 1;
}
