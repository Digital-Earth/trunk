# include "../Headers/direction_set.h"

/*
DirectionSet
*/

/* Returns lowest direction, or 0 if none. */
int DirectionSet_fiLowest(
	REGISTER DirectionSet_tc const cDirectionSet
) {
	/* An array mapping a nybble to one-based position of the lowest 'on' bit. */
	static int const aiLowestOffset[] = {
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

	REGISTER int iLowestOffset = aiLowestOffset[cDirectionSet & 15 /* 1111 */];
	mStrongAssert((cDirectionSet >> 4) < 16);
	if (0 == iLowestOffset && 0 != (iLowestOffset = aiLowestOffset[cDirectionSet >> 4])) iLowestOffset += 4;
	return iLowestOffset;
}

Boolean_tb DirectionSet_fbTest(void) {
	REGISTER DirectionSet_tc cDirectionSet = DirectionSet_fcEmpty();

	assert(0 == DirectionSet_fiuCount(cDirectionSet));

	/* Add. */
	cDirectionSet = DirectionSet_fcInsert(cDirectionSet, Direction_i100);
	if (8 != cDirectionSet) return mbVerify(0);
	if (1 != DirectionSet_fiuCount(cDirectionSet)) return mbVerify(0);
	if (!DirectionSet_fbContains(cDirectionSet, Direction_i100)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i001)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i010)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i011)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i101)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i110)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i111)) return mbVerify(0);

	/* Add another. */
	cDirectionSet = DirectionSet_fcInsert(cDirectionSet, Direction_i011);
	if (12 != cDirectionSet) return mbVerify(0);
	if (2 != DirectionSet_fiuCount(cDirectionSet)) return mbVerify(0);
	if (!DirectionSet_fbContains(cDirectionSet, Direction_i100)) return mbVerify(0);
	if (!DirectionSet_fbContains(cDirectionSet, Direction_i011)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i001)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i010)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i101)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i110)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i111)) return mbVerify(0);

	/* Add same again; verify that nothing changes. */
	if (cDirectionSet != DirectionSet_fcInsert(cDirectionSet, Direction_i011)) return mbVerify(0);

	/* Remove one. */
	cDirectionSet = DirectionSet_fcRemove(cDirectionSet, Direction_i011);
	if (8 != cDirectionSet) return mbVerify(0);
	if (1 != DirectionSet_fiuCount(cDirectionSet)) return mbVerify(0);
	if (!DirectionSet_fbContains(cDirectionSet, Direction_i100)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i001)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i010)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i011)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i101)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i110)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i111)) return mbVerify(0);
	
	/* Remove other one. */
	cDirectionSet = DirectionSet_fcRemove(cDirectionSet, Direction_i100);
	if (0 != cDirectionSet) return mbVerify(0);
	if (0 != DirectionSet_fiuCount(cDirectionSet)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i100)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i001)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i010)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i011)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i101)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i110)) return mbVerify(0);
	if (DirectionSet_fbContains(cDirectionSet, Direction_i111)) return mbVerify(0);

	/* Singleton. */
	{
		REGISTER DirectionSet_tc cSet = DirectionSet_fcEmpty();
		if (DirectionSet_fbSingleton(cSet)) return mbVerify(0);
		cSet = DirectionSet_fcInsert(cSet, Direction_i110);
		if (!DirectionSet_fbSingleton(cSet)) return mbVerify(0);
		cSet = DirectionSet_fcInsert(cSet, Direction_i010);
		if (DirectionSet_fbSingleton(cSet)) return mbVerify(0);
		cSet = DirectionSet_fcRemove(cSet, Direction_i110);
		if (!DirectionSet_fbSingleton(cSet)) return mbVerify(0);
	}

	return 1;
}
