# if !defined(HEADERS__DIRECTION)
	# define HEADERS__DIRECTION

	/* The hexagonal directions that this code implements are described here:
	https://www.pyxisinnovation.com/pyxinternalwiki/index.php?title=Indexing_PYXIS_Hexagons

	Preprocessor definitions:
	- Direction__NO_LOOKUPS:
		- If defined, indicates that all operations are to be done algorithmically instead of 
		with lookup tables.  This will make the code slower but possibly slightly smaller.
	*/

	# include "Core/Headers/boolean.h"
	# include "Core/Headers/core.h"

	typedef enum {
		Direction_i001 = 1, /* Idempotent under multiplication and division. */
		Direction_i010 = 2,
		Direction_i011 = 3,
		Direction_i100 = 4,
		Direction_i101 = 5,
		Direction_i110 = 6,
		Direction_i111 = 7 /* Center direction. Idempotent under addition. */
	} Direction_te;

	static INLINE Boolean_tb Direction_fbValid(
		REGISTER Direction_te const eDirection
	) {
		return (eDirection) > 0 && (eDirection) <= Direction_i111;
	}

	/* Returns true if each is the negative of other. */
	static INLINE Boolean_tb Direction_fbOppositeVertices(
		REGISTER Direction_te const eDirection0,
		REGISTER Direction_te const eDirection1
	) {
		assert(Direction_fbValid(eDirection0));
		assert(Direction_fbValid(eDirection1));

		return (eDirection0 + eDirection1) == Direction_i111;
	}

	# if defined(Direction__NO_LOOKUPS)
	
		static INLINE Direction_te Direction_feNegative(
			REGISTER Direction_te const eDirection
		) {
			assert(Direction_fbValid(eDirection));

			if (Direction_i111 == eDirection) return Direction_i111;
			return ~eDirection & 7 /* 0111 */;
		}

		static INLINE Direction_te Direction_feReverse(
			REGISTER Direction_te const eDirection
		) {
			assert(Direction_fbValid(eDirection));

			return ((eDirection & 1 /* 0001 */) << 2) | (eDirection & 2 /* 0010 */) | (eDirection >> 2);
		}

		static INLINE Direction_te Direction_feDouble(
			REGISTER Direction_te const eDirection
		) {
			assert(Direction_fbValid(eDirection));

			return ((eDirection << 1) | (eDirection >> 2)) & 7 /* 0111 */;
		}

		static INLINE Direction_te Direction_feNegativeDouble(
			REGISTER Direction_te const eDirection
		) {
			assert(Direction_fbValid(eDirection));

			if (Direction_i111 == eDirection) return Direction_i111;
			return ~((eDirection << 1) | (eDirection >> 2)) & 7 /* 0111 */;
		}

		static INLINE Direction_te Direction_feHalf(
			REGISTER Direction_te const eDirection
		) {
			assert(Direction_fbValid(eDirection));

			return (eDirection >> 1) | ((eDirection & 1 /* 0001 */) << 2);
		}

		static INLINE Direction_te Direction_feNegativeHalf(
			REGISTER Direction_te const eDirection
		) {
			assert(Direction_fbValid(eDirection));

			return ~((eDirection >> 1) | (eDirection << 2)) & 7 /* 0111 */;
		}

		static INLINE int unsigned Direction_fiuBitSum(
			REGISTER Direction_te const eDirection
		) {
			assert(Direction_fbValid(eDirection));

			return (eDirection & 1) + ((eDirection >> 1) & 1) + (eDirection >> 2);
		}

		/* Performs modulo 7 addition. */
		static INLINE Direction_te Direction_feSum(
			REGISTER Direction_te const eAugend,
			REGISTER Direction_te const eAddend
		) {
			assert(Direction_fbValid(eAugend));
			assert(Direction_fbValid(eAddend));

			{
				REGISTER int unsigned const iuResult = (eAugend + eAddend);
				return iuResult - ((iuResult >> 3U) * 7U);
			}
		}

		/* Multiplicand is to Product as Multiplier is to Direction_i001. */
		static INLINE Direction_te Direction_feProduct(
			REGISTER Direction_te const eMultiplicand,
			REGISTER Direction_te const eMultiplier
		) {
			assert(Direction_fbValid(eMultiplicand));
			assert(Direction_fbValid(eMultiplier));

			{
				REGISTER int unsigned iuMultiple = (eMultiplicand * eMultiplier);
				if (iuMultiple > Direction_i111) {
					iuMultiple %= Direction_i111;
					if (0 == iuMultiple) return Direction_i111;
				}
				return iuMultiple;
			}
		}

		/* Quotient is to Direction_i001 as Dividend is to Divisor. */
		static INLINE Direction_te Direction_feQuotient(
			REGISTER Direction_te const eDividend,
			REGISTER Direction_te const eDivisor
		) {
			assert(Direction_fbValid(eDividend));
			assert(Direction_fbValid(eDivisor));

			if (eDividend == Direction_i111 || eDivisor == Direction_i111) return Direction_i111;
			if (eDividend == eDivisor) return Direction_i001;
			if (Direction_fbOppositeVertices(eDividend, eDivisor)) return Direction_i110; /* -1 (6) */
			if (Direction_fiuBitSum(eDividend) == Direction_fiuBitSum(eDivisor)) return Direction_i010 << (eDividend == Direction_feHalf(eDivisor));
			if (eDividend == Direction_feNegativeHalf(eDivisor)) return Direction_i011;
			assert(eDividend == Direction_feNegativeDouble(eDivisor));
			return Direction_i101;
		}

	# else

		Direction_te Direction_feNegative(
			REGISTER Direction_te const eDirection
		);

		Direction_te Direction_feReverse(
			REGISTER Direction_te const eDirection
		);
		
		Direction_te Direction_feDouble(
			REGISTER Direction_te const eDirection
		);

		Direction_te Direction_feNegativeDouble(
			REGISTER Direction_te const eDirection
		);

		Direction_te Direction_feHalf(
			REGISTER Direction_te const eDirection
		);

		Direction_te Direction_feNegativeHalf(
			REGISTER Direction_te const eDirection
		);

		int unsigned Direction_fiuBitSum(
			REGISTER Direction_te const eDirection
		);

		/* Performs modulo 7 addition. */
		Direction_te Direction_feSum(
			REGISTER Direction_te const eAugend,
			REGISTER Direction_te const eAddend
		);

		/* Multiplicand is to Product as Multiplier is to Direction_i001. */
		Direction_te Direction_feProduct(
			REGISTER Direction_te const eMultiplicand,
			REGISTER Direction_te const eMultiplier
		);

		/* Quotient is to Direction_i001 as Dividend is to Divisor. */
		Direction_te Direction_feQuotient(
			REGISTER Direction_te const eDividend,
			REGISTER Direction_te const eDivisor
		);

	# endif

	/* Uses rand(); assumes that 'srand' has already been called with a proper seed. */
	static INLINE Direction_te Direction_feRandom(
		REGISTER Boolean_tb const bIncludeZero
	) {
		return (Direction_te)((rand() % (5 + !!bIncludeZero)) + 1);
	}

	/* Tests the direction code. */
	Boolean_tb Direction_fbTest(void);

# endif
