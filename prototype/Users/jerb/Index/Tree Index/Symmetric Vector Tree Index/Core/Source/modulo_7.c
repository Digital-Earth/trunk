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

# include "../Headers/modulo_7.h"

/*
Modulo7
*/

Boolean_tb Modulo7_fbTest(void) {
	/* Addition with no carry. */
	if (Modulo7_i2 != Modulo7_feSum(Modulo7_i1, Modulo7_i1)) return mbVerify(0);
	
	/* Addition with carry. */
	if (Modulo7_i6 != Modulo7_feSum(Modulo7_i0, Modulo7_i6)) return mbVerify(0);

	/* Addition of two zero elements. */
	if (Modulo7_i0 != Modulo7_feSum(Modulo7_i0, Modulo7_i0)) return mbVerify(0);

	/* Negate zero. */
	if (Modulo7_i0 != Modulo7_feProduct(Modulo7_i0, Modulo7_i6)) return mbVerify(0);

	/* Negate non-zero. */
	if (Modulo7_i3 != Modulo7_feProduct(Modulo7_i4, Modulo7_i6)) return mbVerify(0);

	/* Double non-zero. */
	if (Modulo7_i5 != Modulo7_feProduct(Modulo7_i6, Modulo7_i2)) return mbVerify(0);

	/* Half non-zero. */
	if (Modulo7_i6 != Modulo7_feProduct(Modulo7_i5, Modulo7_i4)) return mbVerify(0);

	/* Sign. */
	if (Modulo7_i1 != Modulo7_feSign(Modulo7_i1)) return mbVerify(0);
	if (Modulo7_i1 != Modulo7_feSign(Modulo7_i2)) return mbVerify(0);
	if (Modulo7_i1 != Modulo7_feSign(Modulo7_i4)) return mbVerify(0);
	if (Modulo7_i6 != Modulo7_feSign(Modulo7_i6)) return mbVerify(0);
	if (Modulo7_i6 != Modulo7_feSign(Modulo7_i5)) return mbVerify(0);
	if (Modulo7_i6 != Modulo7_feSign(Modulo7_i3)) return mbVerify(0);
	if (Modulo7_i0 != Modulo7_feSign(Modulo7_i0)) return mbVerify(0);

	/* Product. */
	if (Modulo7_i6 != Modulo7_feProduct(Modulo7_i3, Modulo7_i2)) return mbVerify(0);
	if (Modulo7_i4 != Modulo7_feProduct(Modulo7_i3, Modulo7_i6)) return mbVerify(0);
	if (Modulo7_i5 != Modulo7_feProduct(Modulo7_i3, Modulo7_i4)) return mbVerify(0);
	if (Modulo7_i1 != Modulo7_feProduct(Modulo7_i3, Modulo7_i5)) return mbVerify(0);
	if (Modulo7_i3 != Modulo7_feProduct(Modulo7_i3, Modulo7_i1)) return mbVerify(0);

	/* Quotient. */
	if (Modulo7_i3 != Modulo7_feQuotient(Modulo7_i6, Modulo7_i2)) return mbVerify(0);
	if (Modulo7_i3 != Modulo7_feQuotient(Modulo7_i4, Modulo7_i6)) return mbVerify(0);
	if (Modulo7_i3 != Modulo7_feQuotient(Modulo7_i5, Modulo7_i4)) return mbVerify(0);
	if (Modulo7_i3 != Modulo7_feQuotient(Modulo7_i1, Modulo7_i5)) return mbVerify(0);
	if (Modulo7_i3 != Modulo7_feQuotient(Modulo7_i3, Modulo7_i1)) return mbVerify(0);

	return 1;
}
