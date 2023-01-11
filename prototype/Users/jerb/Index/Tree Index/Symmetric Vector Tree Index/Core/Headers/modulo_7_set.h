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

# if !defined(HEADERS__MODULO_7_SET)
	# define HEADERS__MODULO_7_SET

	/* A modulo 7 set is a value type that represents a set of modulo 7 numbers. */

	# include "bit.h"
	# include "boolean.h"
	# include "core.h"
	# include "modulo_7.h"

	typedef char Modulo7Set_tc;

	/* O(1). */
	static INLINE Modulo7Set_tc Modulo7Set_fcEmpty(void) {
		return 0;
	}

	/* O(1). */
	static INLINE Modulo7Set_tc Modulo7Set_fcFull(void) {
		return 127 /* 01111111 */;
	}

	/* O(1) insert operation that returns the new direction set.  Compare with original to verify success. */
	static INLINE Modulo7Set_tc Modulo7Set_fcInsert(
		REGISTER Modulo7Set_tc const cModulo7Set,
		REGISTER Modulo7_te const eModulo7
	) {
		mAssert(Modulo7_fbValid(eModulo7));

		return (Modulo7Set_tc const)Bit_miOn(cModulo7Set, eModulo7);
	}

	/* O(1). Returns set containing only the single direction. */
	static INLINE Modulo7Set_tc Modulo7Set_fcSingleton(
		REGISTER Modulo7_te const eModulo7
	) {
		return Modulo7Set_fcInsert(Modulo7Set_fcEmpty(), eModulo7);
	}
	
	/* O(1) remove operation that returns the new direction set.  Compare with original to verify success. */
	static INLINE Modulo7Set_tc Modulo7Set_fcRemove(
		REGISTER Modulo7Set_tc const cModulo7Set,
		REGISTER Modulo7_te const eModulo7
	) {
		mAssert(Modulo7_fbValid(eModulo7));

		return (Modulo7Set_tc const)Bit_miOff(cModulo7Set, eModulo7);
	}

	/* O(1) operation that checks for containment. */
	static INLINE Boolean_tb Modulo7Set_fbContains(
		REGISTER Modulo7Set_tc const cModulo7Set,
		REGISTER Modulo7_te const eModulo7
	) {
		mAssert(Modulo7_fbValid(eModulo7));

		return Bit_mbOn(cModulo7Set, eModulo7);
	}

	/* O(n), where n is the number of elements in the set. */
	static INLINE int unsigned Modulo7Set_fiuCount(
		REGISTER Modulo7Set_tc const cModulo7Set
	) {
		return Bit_fiuOnCount(cModulo7Set);
	}

	static INLINE Boolean_tb Modulo7Set_fbEmpty(
		REGISTER Modulo7Set_tc const cModulo7Set
	) {
		return cModulo7Set == Modulo7Set_fcEmpty();
	}
	
	static INLINE Boolean_tb Modulo7Set_fbFull(
		REGISTER Modulo7Set_tc const cModulo7Set
	) {
		return cModulo7Set == Modulo7Set_fcFull();
	}

	/* Returns true if there is one element in the set.  O(1). */
	static INLINE Boolean_tb Modulo7Set_fbSingleton(
		REGISTER Modulo7Set_tc const cModulo7Set
	) {
		return Bit_fbSingleOn(cModulo7Set);
	}

	/* Returns either the lowest direction, or 0 if none. */
	int Modulo7Set_fiLowest(
		REGISTER Modulo7Set_tc const cModulo7Set
	);

	Boolean_tb Modulo7Set_fbTest(void);

# endif
