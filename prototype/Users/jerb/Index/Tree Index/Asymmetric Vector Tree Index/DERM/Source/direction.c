# include "../Headers/direction.h"

/*
Direction
*/

# if !defined(Direction__NO_LOOKUPS)

	Direction_te Direction_feNegative(
		REGISTER Direction_te const eDirection
	) {
		assert(Direction_fbValid(eDirection));

		{
			static Direction_te const aeResult[] = {7, 6, 5, 4, 3, 2, 1, 7};
			return aeResult[eDirection];
		}
	}

	Direction_te Direction_feReverse(
		REGISTER Direction_te const eDirection
	) {
		assert(Direction_fbValid(eDirection));

		{
			static Direction_te const aeResult[] = {7, 4, 2, 6, 1, 5, 3, 7};
			return aeResult[eDirection];
		}
	}

	Direction_te Direction_feDouble(
		REGISTER Direction_te const eDirection
	) {
		assert(Direction_fbValid(eDirection));

		{
			static Direction_te const aeResult[] = {7, 2, 4, 6, 1, 3, 5, 7};
			return aeResult[eDirection];
		}
	}

	Direction_te Direction_feNegativeDouble(
		REGISTER Direction_te const eDirection
	) {
		assert(Direction_fbValid(eDirection));

		{
			static Direction_te const aeResult[] = {7, 5, 3, 1, 6, 4, 2, 7};
			return aeResult[eDirection];
		}
	}

	Direction_te Direction_feHalf(
		REGISTER Direction_te const eDirection
	) {
		assert(Direction_fbValid(eDirection));

		{
			static Direction_te const aeResult[] = {7, 4, 1, 5, 2, 6, 3, 7};
			return aeResult[eDirection];
		}
	}

	Direction_te Direction_feNegativeHalf(
		REGISTER Direction_te const eDirection
	) {
		assert(Direction_fbValid(eDirection));

		{
			static Direction_te const aeResult[] = {7, 3, 6, 2, 5, 1, 4, 7};
			return aeResult[eDirection];
		}
	}

	int unsigned Direction_fiuBitSum(
		REGISTER Direction_te const eDirection
	) {
		assert(Direction_fbValid(eDirection));

		{
			static int unsigned const aiuResult[] = {0U, 1U, 1U, 2U, 1U, 2U, 2U, 3U};
			return aiuResult[eDirection];
		}
	}

	Direction_te Direction_feSum(
		REGISTER Direction_te const eAugend,
		REGISTER Direction_te const eAddend
	) {
		assert(Direction_fbValid(eAugend));
		assert(Direction_fbValid(eAddend));

		{
			static Direction_te const aeResult[][8] = {
				{7, 7, 7, 7, 7, 7, 7, 7},
				{7, 2, 3, 4, 5, 6, 7, 1},
				{7, 3, 4, 5, 6, 7, 1, 2},
				{7, 4, 5, 6, 7, 1, 2, 3},
				{7, 5, 6, 7, 1, 2, 3, 4},
				{7, 6, 7, 1, 2, 3, 4, 5},
				{7, 7, 1, 2, 3, 4, 5, 6},
				{7, 1, 2, 3, 4, 5, 6, 7}
			};
			return aeResult[eAugend][eAddend];
		}
	}

	/* Multiplicand is to Product as Multiplier is to Direction_i001. */
	Direction_te Direction_feProduct(
		REGISTER Direction_te const eMultiplicand,
		REGISTER Direction_te const eMultiplier
	) {
		assert(Direction_fbValid(eMultiplicand));
		assert(Direction_fbValid(eMultiplier));

		{
			static Direction_te const aeResult[][8] = {
				{7, 7, 7, 7, 7, 7, 7, 7},
				{7, 1, 2, 3, 4, 5, 6, 7},
				{7, 2, 4, 6, 1, 3, 5, 7},
				{7, 3, 6, 2, 5, 1, 4, 7},
				{7, 4, 1, 5, 2, 6, 3, 7},
				{7, 5, 3, 1, 6, 4, 2, 7},
				{7, 6, 5, 4, 3, 2, 1, 7},
				{7, 7, 7, 7, 7, 7, 7, 7}
			};
			return aeResult[eMultiplicand][eMultiplier];
		}
	}

	/* Quotient is to Direction_i001 as Dividend is to Divisor. */
	Direction_te Direction_feQuotient(
		REGISTER Direction_te const eDividend,
		REGISTER Direction_te const eDivisor
	) {
		assert(Direction_fbValid(eDividend));
		assert(Direction_fbValid(eDivisor));

		{
			static Direction_te const aeResult[][8] = {
				{7, 7, 7, 7, 7, 7, 7, 7},
				{7, 1, 4, 5, 2, 3, 6, 7},
				{7, 2, 1, 3, 4, 6, 5, 7},
				{7, 3, 5, 1, 6, 2, 4, 7},
				{7, 4, 2, 6, 1, 5, 3, 7},
				{7, 5, 6, 4, 3, 1, 2, 7},
				{7, 6, 3, 2, 5, 4, 1, 7},
				{7, 7, 7, 7, 7, 7, 7, 7}
			};
			return aeResult[eDividend][eDivisor];
		}
	}

# endif

Boolean_tb Direction_fbTest(void) {
	/* Addition with no carry. */
	if (Direction_i010 != Direction_feSum(Direction_i001, Direction_i001)) return mbVerify(0);
	
	/* Addition with carry. */
	if (Direction_i110 != Direction_feSum(Direction_i111, Direction_i110)) return mbVerify(0);

	/* Addition of two zero elements. */
	if (Direction_i111 != Direction_feSum(Direction_i111, Direction_i111)) return mbVerify(0);

	/* Negate zero. */
	if (Direction_i111 != Direction_feNegative(Direction_i111)) return mbVerify(0);

	/* Negate non-zero. */
	if (Direction_i011 != Direction_feNegative(Direction_i100)) return mbVerify(0);
	
	/* Reversal. */
	if (Direction_i100 != Direction_feReverse(Direction_i001)) return mbVerify(0);
	if (Direction_i010 != Direction_feReverse(Direction_i010)) return mbVerify(0);
	if (Direction_i001 != Direction_feReverse(Direction_i100)) return mbVerify(0);
	if (Direction_i011 != Direction_feReverse(Direction_i110)) return mbVerify(0);
	if (Direction_i101 != Direction_feReverse(Direction_i101)) return mbVerify(0);
	if (Direction_i110 != Direction_feReverse(Direction_i011)) return mbVerify(0);
	if (Direction_i111 != Direction_feReverse(Direction_i111)) return mbVerify(0);

	/* Double non-zero. */
	if (Direction_i101 != Direction_feDouble(Direction_i110)) return mbVerify(0);

	/* Half non-zero. */
	if (Direction_i110 != Direction_feHalf(Direction_i101)) return mbVerify(0);

	/* Bit sum. */
	if (1 != Direction_fiuBitSum(Direction_i001)) return mbVerify(0);
	if (1 != Direction_fiuBitSum(Direction_i010)) return mbVerify(0);
	if (1 != Direction_fiuBitSum(Direction_i100)) return mbVerify(0);
	if (2 != Direction_fiuBitSum(Direction_i110)) return mbVerify(0);
	if (2 != Direction_fiuBitSum(Direction_i101)) return mbVerify(0);
	if (2 != Direction_fiuBitSum(Direction_i011)) return mbVerify(0);
	if (3 != Direction_fiuBitSum(Direction_i111)) return mbVerify(0);

	/* Product. */
	if (Direction_i110 != Direction_feProduct(Direction_i011, Direction_i010)) return mbVerify(0);
	if (Direction_i100 != Direction_feProduct(Direction_i011, Direction_i110)) return mbVerify(0);
	if (Direction_i101 != Direction_feProduct(Direction_i011, Direction_i100)) return mbVerify(0);
	if (Direction_i001 != Direction_feProduct(Direction_i011, Direction_i101)) return mbVerify(0);
	if (Direction_i011 != Direction_feProduct(Direction_i011, Direction_i001)) return mbVerify(0);

	/* Quotient. */
	if (Direction_i011 != Direction_feQuotient(Direction_i110, Direction_i010)) return mbVerify(0);
	if (Direction_i011 != Direction_feQuotient(Direction_i100, Direction_i110)) return mbVerify(0);
	if (Direction_i011 != Direction_feQuotient(Direction_i101, Direction_i100)) return mbVerify(0);
	if (Direction_i011 != Direction_feQuotient(Direction_i001, Direction_i101)) return mbVerify(0);
	if (Direction_i011 != Direction_feQuotient(Direction_i011, Direction_i001)) return mbVerify(0);

	return 1;
}
