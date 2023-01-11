# if !defined(HEADERS__DIRECTION_SET)
	# define HEADERS__DIRECTION_SET

	/* A direction set is a value type that represents a set of directions. */

	# include "Core/Headers/bit.h"
	# include "Core/Headers/boolean.h"
	# include "Core/Headers/core.h"
	# include "direction.h"

	typedef char DirectionSet_tc;

	/* O(1). */
	static INLINE DirectionSet_tc DirectionSet_fcEmpty(void) {
		return 0;
	}

	/* O(1). */
	static INLINE DirectionSet_tc DirectionSet_fcFull(void) {
		return 127 /* 01111111 */;
	}

	/* O(1). Returns set containing only Direction_i111. */
	static INLINE DirectionSet_tc DirectionSet_fcCenterOnly(void) {
		return 64 /* 01000000 */;
	}
	
	/* O(1) insert operation that returns the new direction set.  Compare with original to verify success. */
	static INLINE DirectionSet_tc DirectionSet_fcInsert(
		REGISTER DirectionSet_tc const cDirectionSet,
		REGISTER Direction_te const eDirection
	) {
		assert(Direction_fbValid(eDirection));

		return (DirectionSet_tc const)Bit_miOn(cDirectionSet, (eDirection - 1));
	}

	/* O(1) remove operation that returns the new direction set.  Compare with original to verify success. */
	static INLINE DirectionSet_tc DirectionSet_fcRemove(
		REGISTER DirectionSet_tc const cDirectionSet,
		REGISTER Direction_te const eDirection
	) {
		assert(Direction_fbValid(eDirection));

		return (DirectionSet_tc const)Bit_miOff(cDirectionSet, (eDirection - 1));
	}

	/* O(1) operation that checks for containment. */
	static INLINE Boolean_tb DirectionSet_fbContains(
		REGISTER DirectionSet_tc const cDirectionSet,
		REGISTER Direction_te const eDirection
	) {
		assert(Direction_fbValid(eDirection));

		return Bit_mbOn(cDirectionSet, eDirection - 1);
	}

	/* O(n), where n is the number of elements in the set. */
	static INLINE int unsigned DirectionSet_fiuCount(
		REGISTER DirectionSet_tc const cDirectionSet
	) {
		return Bit_fiuOnCount(cDirectionSet);
	}

	static INLINE Boolean_tb DirectionSet_fbEmpty(
		REGISTER DirectionSet_tc const cDirectionSet
	) {
		return cDirectionSet == DirectionSet_fcEmpty();
	}
	
	static INLINE Boolean_tb DirectionSet_fbFull(
		REGISTER DirectionSet_tc const cDirectionSet
	) {
		return cDirectionSet == DirectionSet_fcFull();
	}

	/* Returns true if there is one element in the set.  O(1). */
	static INLINE Boolean_tb DirectionSet_fbSingleton(
		REGISTER DirectionSet_tc const cDirectionSet
	) {
		return Bit_fbSingleOn(cDirectionSet);
	}

	/* Returns either the lowest direction, or 0 if none. */
	int DirectionSet_fiLowest(
		REGISTER DirectionSet_tc const cDirectionSet
	);

	Boolean_tb DirectionSet_fbTest(void);

# endif
