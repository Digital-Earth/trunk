# include "../Headers/direction_string.h"
# include <string.h>

/*
DirectionString
*/

/* Gets the direction. */
Boolean_tb DirectionString_fbGetDirection(
	REGISTER DirectionString_tzc const rzcDirections[mReferenceConst(1)],
	REGISTER Direction_te reDirection[mReferenceConst(1)]
) {
	mStrongAssert(rzcDirections);
	mStrongAssert(reDirection);

	if (0 == *rzcDirections) return 0;
	*reDirection = DirectionString_fiDirection(rzcDirections);
	assert(Direction_fbValid(*reDirection));
	return 1;
}

/* Sets the direction.
It is up to the caller to ensure that the null terminator gets set properly.
*/
void DirectionString_fSetDirection(
	REGISTER DirectionString_tzc rzcDirections[mReference(1)],
	REGISTER Direction_te const eDirection
) {
	mStrongAssert(rzcDirections);
	assert(Direction_fbValid(eDirection));

	*rzcDirections = (char const)eDirection;
}

/* Writes the direction string to the stream with directions in ASCII format. */
Boolean_tb DirectionString_fbWrite(
	REGISTER DirectionString_tzc const rzcDirections[mReference(1)],
	REGISTER File_ts rsOutput[mReferenceConst(1)]
) {
	mStrongAssert(rzcDirections);
	mStrongAssert(rsOutput);

	for (; *rzcDirections; ++rzcDirections) {
		REGISTER Direction_te const eDirection = DirectionString_fiDirection(rzcDirections);
		assert(Direction_fbValid(eDirection));
		if (!File_fbWrite(rsOutput, (char unsigned const)(eDirection + '0'))) return 0;
	}
	return 1;
}

Size_tiu DirectionString_fiuCount(
	REGISTER DirectionString_tzc const rzcDirections[mReferenceConst(1)]
) {
	mStrongAssert(rzcDirections);
	
	return strlen(rzcDirections);
}

Boolean_tb DirectionString_fbEquivalent(
	REGISTER DirectionString_tzc const rzcDirectionsLeft[mReferenceConst(1)],
	REGISTER DirectionString_tzc const rzcDirectionsRight[mReferenceConst(1)]
) {
	mStrongAssert(rzcDirectionsLeft);
	mStrongAssert(rzcDirectionsRight);

	return 0 == strcmp(rzcDirectionsLeft, rzcDirectionsRight);
}

void DirectionString_fCopySection(
	REGISTER DirectionString_tzc rzcDestination[mReferenceConst(1)],
	REGISTER DirectionString_tzc const rzcSource[mReferenceConst(1)],
	REGISTER Size_tiu const iuCount
) {
	mStrongAssert(rzcDestination);
	mStrongAssert(rzcSource);
	
	memcpy(rzcDestination, rzcSource, iuCount);
}
