# if !defined(HEADERS__DIRECTION_STRING)
	# define HEADERS__DIRECTION_STRING

	# include "Core/Headers/core.h"
	# include "Core/Headers/file.h"
	# include "Core/Headers/size.h"
	# include "direction.h"

	/* A null-terminated string of numeric directions.
	All memory management of the string is left up to the caller.
	*/
	typedef char DirectionString_tzc;

	/* Gets the direction.
	Either returns the direction, or 0 if it is empty (i.e. pointing to the null character).
	*/
	static INLINE int unsigned DirectionString_fiDirection(
		REGISTER DirectionString_tzc const rzcDirections[mReferenceConst(1)]
	) {
		mStrongAssert(rzcDirections);
		assert(0 == *rzcDirections || Direction_fbValid(*rzcDirections));

		return *rzcDirections;
	}

	/* Gets the direction. */
	Boolean_tb DirectionString_fbGetDirection(
		REGISTER DirectionString_tzc const rzcDirections[mReferenceConst(1)],
		REGISTER Direction_te reDirection[mReferenceConst(1)]
	);

	/* Sets the direction.
	It is up to the caller to ensure that the null terminator gets set properly.
	*/
	void DirectionString_fSetDirection(
		REGISTER DirectionString_tzc rzcDirections[mReference(1)],
		REGISTER Direction_te const eDirection
	);

	/* Writes the direction string to the stream with directions in ASCII format. */
	Boolean_tb DirectionString_fbWrite(
		REGISTER DirectionString_tzc const rzcDirections[mReference(1)],
		REGISTER File_ts rsOutput[mReferenceConst(1)]
	);

	/* Returns the number of directions in the string. */
	Size_tiu DirectionString_fiuCount(
		REGISTER DirectionString_tzc const rzcDirections[mReferenceConst(1)]
	);

	Boolean_tb DirectionString_fbEquivalent(
		REGISTER DirectionString_tzc const rzcDirectionsLeft[mReferenceConst(1)],
		REGISTER DirectionString_tzc const rzcDirectionsRight[mReferenceConst(1)]
	);

	void DirectionString_fCopySection(
		REGISTER DirectionString_tzc rzcDestination[mReferenceConst(1)],
		REGISTER DirectionString_tzc const rzcSource[mReferenceConst(1)],
		REGISTER Size_tiu const iuCount
	);

# endif
