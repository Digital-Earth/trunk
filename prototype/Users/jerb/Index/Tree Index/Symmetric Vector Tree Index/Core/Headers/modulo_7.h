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

# if !defined(HEADERS__MODULO_7)
	# define HEADERS__MODULO_7

	# include "boolean.h"
	# include "core.h"

	# define Modulo7_miFromASCII(cASCII) ((cASCII) - '0')
	# define Modulo7_miAsASCII(eModulo7) ((eModulo7) + '0')

	/* Hygienic macro that resolves to the negation of the value. */
	# define Modulo7_meNegative(eValue) (Modulo7_feProduct(Modulo7_iSignNegative, (eValue)))

	/* Uses rand(); assumes that 'srand' has already been called with a proper seed. */
	# define Modulo7_meRandom() ((Modulo7_te)(rand() % 7))

	/* Uses rand(); assumes that 'srand' has already been called with a proper seed. */
	# define Modulo7_meRandomNonZero() ((Modulo7_te)((rand() % 6) + 1))

	/* Using zero-based values, where Modulo7_i0 is defined as 0, performs the best. */
	typedef enum {
		Modulo7_i0 = 0,
		Modulo7_i1 = 1,
		Modulo7_i2 = 2,
		Modulo7_i3 = 3,
		Modulo7_i4 = 4,
		Modulo7_i5 = 5,
		Modulo7_i6 = 6,
		Modulo7_iMinimum = Modulo7_i0,
		Modulo7_iMaximum = Modulo7_i6,
		Modulo7_iMinimumNonZero = Modulo7_i1,
		Modulo7_iMaximumNonZero = Modulo7_i6
	} Modulo7_te;

	/* The ASCII representations for each modulo 7 value. */
	typedef enum {
		Modulo7_iASCII0 = Modulo7_miAsASCII(Modulo7_i0),
		Modulo7_iASCII1 = Modulo7_miAsASCII(Modulo7_i1),
		Modulo7_iASCII2 = Modulo7_miAsASCII(Modulo7_i2),
		Modulo7_iASCII3 = Modulo7_miAsASCII(Modulo7_i3),
		Modulo7_iASCII4 = Modulo7_miAsASCII(Modulo7_i4),
		Modulo7_iASCII5 = Modulo7_miAsASCII(Modulo7_i5),
		Modulo7_iASCII6 = Modulo7_miAsASCII(Modulo7_i6),
		Modulo7_iASCIIMinimum = Modulo7_iASCII0,
		Modulo7_iASCIIMaximum = Modulo7_iASCII6,
		Modulo7_iASCIIMinimumNonZero = Modulo7_iASCII1,
		Modulo7_iASCIIMaximumNonZero = Modulo7_iASCII6
	} Modulo7_teASCII;

	/* Sign, including zero for "no sign". */
	typedef enum {
		Modulo7_iSign0 = Modulo7_i0,
		Modulo7_iSignPositive = Modulo7_i1,
		Modulo7_iSignNegative = Modulo7_i6
	} Modulo7_teSign;

	/* Returns true if the modulo 7 value is valid. */
	static INLINE Boolean_tb Modulo7_fbValid(
		REGISTER Modulo7_te const eModulo7
	) {
		return Modulo7_iMinimum <= eModulo7 && eModulo7 <= Modulo7_iMaximum;
	}
	
	/* Returns true if the ascii value is a valid representation of a modulo 7 value. */
	static INLINE Boolean_tb Modulo7_fbValidASCII(
		REGISTER char const cASCII
	) {
		return Modulo7_iASCIIMinimum <= cASCII && cASCII <= Modulo7_iASCIIMaximum;
	}

	/* This throws an assertion if the char represents an invalid modulo 7 number. */
	static INLINE Modulo7_te Modulo7_feFromASCII(
		REGISTER char const cASCII
	) {
		mAssert(Modulo7_fbValidASCII(cASCII));

		return Modulo7_miFromASCII(cASCII);
	}

	static INLINE char Modulo7_fcAsASCII(
		REGISTER Modulo7_te const eModulo7
	) {
		mAssert(Modulo7_fbValid(eModulo7));
		
		return (char const)Modulo7_miAsASCII(eModulo7);
	}

	/* Each non-zero number is a power of two in either the positive or negative
	(i.e. 1, 2, 4, -1 (6), -2 (5), -4 (3)).
	Returns 1 for positive, 6 (-1) for negative, or 0 for zero.
	*/
	static Modulo7_teSign Modulo7_feSign(
		REGISTER Modulo7_te const eModulo7
	) {
		mAssert(Modulo7_fbValid(eModulo7));

		{
			static Modulo7_te const aeResult[7] = {0, 1, 1, 6, 1, 6, 6};
			return aeResult[eModulo7];
		}
	}

	/* Returns true if the direction is a sign direction (1, 6, or 0). */
	static Boolean_tb Modulo7_fbSign(
		REGISTER Modulo7_te const eModulo7
	) {
		mAssert(Modulo7_fbValid(eModulo7));

		{
			static Boolean_tb const abResult[7] = {1, 1, 0, 0, 0, 0, 1};
			return abResult[eModulo7];
		}
	}

	/* Returns the positive or negative non-zero sign,
	depending on the argument.
	*/
	static Modulo7_teSign Modulo7_feNonZeroSign(
		REGISTER Boolean_tb const bPositive
	) {
		mAssert(Boolean_fbValid(bPositive));

		{
			static Modulo7_te const aeResult[2] = {6, 1};
			return aeResult[bPositive];
		}
	}

	/* Returns true if the value is a non-zero sign. */
	static Boolean_tb Modulo7_fbNonZeroSign(
		REGISTER Modulo7_te const eModulo7
	) {
		mAssert(Modulo7_fbValid(eModulo7));

		{
			static Boolean_tb const abResult[7] = {0, 1, 0, 0, 0, 0, 1};
			return abResult[eModulo7];
		}
	}

	/* Performs modulo 7 addition. */
	static Modulo7_te Modulo7_feSum(
		REGISTER Modulo7_te const eAugend,
		REGISTER Modulo7_te const eAddend
	) {
		mAssert(Modulo7_fbValid(eAugend));
		mAssert(Modulo7_fbValid(eAddend));

		{
			static Modulo7_te const aeResult[7][7] = {
				{0, 1, 2, 3, 4, 5, 6},
				{1, 2, 3, 4, 5, 6, 0},
				{2, 3, 4, 5, 6, 0, 1},
				{3, 4, 5, 6, 0, 1, 2},
				{4, 5, 6, 0, 1, 2, 3},
				{5, 6, 0, 1, 2, 3, 4},
				{6, 0, 1, 2, 3, 4, 5}
			};
			return aeResult[eAugend][eAddend];
		}
	}

	/* Returns the product such that the multiplicand is to 
	the product as the multiplier is to 1.
	*/
	static Modulo7_te Modulo7_feProduct(
		REGISTER Modulo7_te const eMultiplicand,
		REGISTER Modulo7_te const eMultiplier
	) {
		mAssert(Modulo7_fbValid(eMultiplicand));
		mAssert(Modulo7_fbValid(eMultiplier));

		{
			static Modulo7_te const aeResult[7][7] = {
				{0, 0, 0, 0, 0, 0, 0},
				{0, 1, 2, 3, 4, 5, 6},
				{0, 2, 4, 6, 1, 3, 5},
				{0, 3, 6, 2, 5, 1, 4},
				{0, 4, 1, 5, 2, 6, 3},
				{0, 5, 3, 1, 6, 4, 2},
				{0, 6, 5, 4, 3, 2, 1}
			};
			return aeResult[eMultiplicand][eMultiplier];
		}
	}

	/* Returns the quotient such that the quotient is to 1 as 
	the dividend is to the divisor.
	*/
	static Modulo7_te Modulo7_feQuotient(
		REGISTER Modulo7_te const eDividend,
		REGISTER Modulo7_te const eDivisor
	) {
		mAssert(Modulo7_fbValid(eDividend));
		mAssert(Modulo7_fbValid(eDivisor));

		{
			static Modulo7_te const aeResult[7][7] = {
				{0, 0, 0, 0, 0, 0, 0},
				{0, 1, 4, 5, 2, 3, 6},
				{0, 2, 1, 3, 4, 6, 5},
				{0, 3, 5, 1, 6, 2, 4},
				{0, 4, 2, 6, 1, 5, 3},
				{0, 5, 6, 4, 3, 1, 2},
				{0, 6, 3, 2, 5, 4, 1}
			};
			return aeResult[eDividend][eDivisor];
		}
	}

	/* Returns the reflection of the first through the second. */
	static Modulo7_te Modulo7_feReflection(
		REGISTER Modulo7_te const eReflect,
		REGISTER Modulo7_te const eThrough
	) {
		mAssert(Modulo7_fbValid(eReflect));
		mAssert(Modulo7_fbValid(eThrough));

		{
			static Modulo7_te const aeResult[7][7] = {
				{0, 0, 0, 0, 0, 0, 0},
				{6, 1, 4, 2, 2, 4, 1},
				{5, 4, 2, 1, 1, 2, 4},
				{4, 5, 6, 3, 3, 6, 5},
				{3, 2, 1, 4, 4, 1, 2},
				{2, 3, 5, 6, 6, 5, 3},
				{1, 6, 3, 5, 5, 3, 6}
			};
			return aeResult[eReflect][eThrough];
		}
	}

	/* Returns the reflection of the value through Modulo7_i1. */
	static Modulo7_te Modulo7_feReflectionThrough1(
		REGISTER Modulo7_te const eReflect
	) {
		mAssert(Modulo7_fbValid(eReflect));

		{
			static Modulo7_te const aeResult[7] = {0, 1, 4, 5, 2, 3, 6};
			return aeResult[eReflect];
		}
	}

	/* Returns the negated reflection of the value through Modulo7_i1. */
	static Modulo7_te Modulo7_feNegativeReflectionThrough1(
		REGISTER Modulo7_te const eReflect
	) {
		mAssert(Modulo7_fbValid(eReflect));

		{
			static Modulo7_te const aeResult[7] = {0, 6, 3, 2, 5, 4, 1};
			return aeResult[eReflect];
		}
	}

	/* Tests the code. */
	Boolean_tb Modulo7_fbTest(void);

# endif
