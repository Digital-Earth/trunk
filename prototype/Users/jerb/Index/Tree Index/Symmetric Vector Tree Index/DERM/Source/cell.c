# include "../Headers/cell.h"
# include "../../Core/Headers/char_string.h"
# include "../../Core/Headers/file.h"
# include "../../Core/Headers/memory.h"
# include <time.h>

/* Resolves to true if the direction count and pentagon count represent a hexagon.
Otherwise, they represent a pentagon or root.
*/
# define mbHexagon(cDirectionCount, cPentagonCount) ((cPentagonCount) < (cDirectionCount))

/* Resolves to true if the directions are on the same triangle. */
# define mbSameTriangle(eDirection1, eDirection2) (Modulo7_feSign(eDirection1) == Modulo7_feSign(eDirection2))

/* The direction count of various vertex ancestors in a direction string for a cell.
These cannot be redefined; this enumeration only serves to create self-documenting code.
*/
enum {
	iPrimaryVertexDirectionCount = 1,
	iSecondaryVertexDirectionCount = 3,
	iDenormalizedVertexDirectionCount = 4
};

/* Returns the adjustment to apply to the sum of two adjacent vertex directions
to result in the direction of the vector sum.
*/
static Modulo7_te feAdjustmentFromAdjacentVertexSum(
	REGISTER Boolean_tb const bParentDirectionCountOdd
) {
	mAssert(Boolean_fbValid(bParentDirectionCountOdd));

	{
		static Modulo7_te const aeResult[2] = {Modulo7_i2, Modulo7_i6};
		return aeResult[bParentDirectionCountOdd];
	}
}

/* The sum of any two adjacent vertex vectors is a vector in the direction of
one of the addends.  Returns the adjustment to apply to this to get the other addend.
*/
static Modulo7_te feAdjustmentToOtherAdjacentVertexAddend(
	REGISTER Boolean_tb const bAddendDirectionCountOdd
) {
	mAssert(Boolean_fbValid(bAddendDirectionCountOdd));

	{
		/* Returns one of the vertex directions adjacent to Modulo7_i1. */
		static Modulo7_te const aeResult[2] = {Modulo7_i5, Modulo7_i3};
		return aeResult[bAddendDirectionCountOdd];
	}
}

/* Returns the last direction of the cell.  Assumes that the cell contains at least one direction. */
static INLINE Modulo7_te feLast(
	REGISTER char const cDirectionCount,
	REGISTER char const rzcASCIIDirections[mReferenceConst(cDirectionCount + 1)]
) {
	mAssert(rzcASCIIDirections);
	mAssert(0 < cDirectionCount);
	mAssert((int unsigned const)cDirectionCount <= CharString_miuCount(rzcASCIIDirections));

	return Modulo7_miFromASCII(rzcASCIIDirections[cDirectionCount - 1]);
}

/* Sets the last direction and adjusts the pentagon count as necessary. */
static INLINE void fSetLast(
	REGISTER char cDirectionCount,
	REGISTER char rcPentagonCount[mReferenceConst(1)],
	REGISTER char rzcASCIIDirections[mReference(cDirectionCount + 1)],
	REGISTER Modulo7_te const eDirection
) {
	mAssert(rcPentagonCount);
	mAssert(rzcASCIIDirections);
	mAssert(0 < cDirectionCount);
	mAssert((int unsigned const)cDirectionCount <= CharString_miuCount(rzcASCIIDirections));
	mAssert(Modulo7_fbValid(eDirection));

	/* Advance the array pointer. Decrement direction count to optimize away further subtractions. */
	rzcASCIIDirections += --cDirectionCount;

	/* If there is no chance for a hexagonal state change, set it and return.
	This is the case if the parent is a hexagon (note that 'cDirectionCount' has been decremented),
	or both the old and new directions are either centroid or vertex.
	*/
	if (mbHexagon(cDirectionCount, *rcPentagonCount) || (
		(Modulo7_i0 == eDirection) == (Modulo7_iASCII0 == *rzcASCIIDirections)
	)) {
		*rzcASCIIDirections = Modulo7_fcAsASCII(eDirection);
		return;
	}

	/* Set the cell direction. */
	*rzcASCIIDirections = Modulo7_fcAsASCII(eDirection);

	/* It was a pentagon, but we don't know if it still is a pentagon. The parent is a pentagon, though.
	Set the pentagon count to the parent direction count (note that 'cDirectionCount' has been decremented),
	then increment pentagon count until secondary vertex direction count (or no more directions),
	then increment pentagon count for each remaining centroid direction.
	*/
	for (*rcPentagonCount = cDirectionCount; *rcPentagonCount < iSecondaryVertexDirectionCount; ) {
		++*rcPentagonCount;
		if (!*++rzcASCIIDirections) return;
	}
	{
		/* Avoid extra pointer dereferences in the loop. */
		REGISTER char cPentagonCount = *rcPentagonCount;
		for (; Modulo7_iASCII0 == *rzcASCIIDirections; ++cPentagonCount, ++rzcASCIIDirections);
		*rcPentagonCount = cPentagonCount;
	}
}

/* Returns true if this is a denormalized pole. */
static INLINE Boolean_tb fbDenormalizedPole(
	REGISTER char const cDirectionCount,
	REGISTER char const rzcASCIIDirections[mReferenceConst(cDirectionCount + 1)]
) {
	mAssert(rzcASCIIDirections);
	mAssert((int unsigned const)cDirectionCount <= CharString_miuCount(rzcASCIIDirections));

	return (
		iDenormalizedVertexDirectionCount <= cDirectionCount &&
		Modulo7_iASCII0 != rzcASCIIDirections[iDenormalizedVertexDirectionCount - 1]
	);
}

/* Denormalizes the pole and returns the adjusted direction. */
static INLINE Modulo7_te feDenormalizePole(
	REGISTER char rcPentagonCount[mReferenceConst(1)],
	REGISTER char rzcASCIIDirections[mReferenceConst(iDenormalizedVertexDirectionCount + 1)]
) {
	mAssert(rcPentagonCount);
	mAssert(rzcASCIIDirections);
	mAssert(iDenormalizedVertexDirectionCount <= CharString_miuCount(rzcASCIIDirections));
	mAssert(!fbDenormalizedPole(iDenormalizedVertexDirectionCount, rzcASCIIDirections));

	{
		REGISTER Modulo7_te const eOldSecondaryVertex = feLast(iSecondaryVertexDirectionCount, rzcASCIIDirections);
		REGISTER Modulo7_te const eNewDenormalizedVertex = Modulo7_feNegativeReflectionThrough1(eOldSecondaryVertex);

		/* Since hexagon state will not be changed, the directions can be set directly. */
		rzcASCIIDirections[iPrimaryVertexDirectionCount - 1] = Modulo7_fcAsASCII(
			Modulo7_meNegative(feLast(iPrimaryVertexDirectionCount, rzcASCIIDirections))
		);
		rzcASCIIDirections[iSecondaryVertexDirectionCount - 1] = Modulo7_iASCII0;

		/* This one changes the hexagon state, but we know it will be a hexagon. */
		*rcPentagonCount = iSecondaryVertexDirectionCount;
		rzcASCIIDirections[iDenormalizedVertexDirectionCount - 1] = Modulo7_fcAsASCII(eNewDenormalizedVertex);

		/* Return the direction adjustment.
		Modulo7_i4 is required due to lattice arrangement and underlap placement.
		*/
		return Modulo7_feQuotient(Modulo7_feProduct(Modulo7_i4, eNewDenormalizedVertex), eOldSecondaryVertex);
	}
}

/* Normalizes the pole and returns the adjusted direction. */
static INLINE Modulo7_te feNormalizePole(
	REGISTER char rcPentagonCount[mReferenceConst(1)],
	REGISTER char rzcASCIIDirections[mReferenceConst(iDenormalizedVertexDirectionCount + 1)]
) {
	mAssert(rcPentagonCount);
	mAssert(rzcASCIIDirections);
	mAssert(iDenormalizedVertexDirectionCount <= CharString_miuCount(rzcASCIIDirections));
	mAssert(fbDenormalizedPole(iDenormalizedVertexDirectionCount, rzcASCIIDirections));

	{
		REGISTER Modulo7_te const eOldDenormalizedVertex = feLast(iDenormalizedVertexDirectionCount, rzcASCIIDirections);
		REGISTER Modulo7_te const eNewSecondaryVertex = Modulo7_feNegativeReflectionThrough1(eOldDenormalizedVertex);

		/* Since hexagon state will not be changed, the directions can be set directly. */
		rzcASCIIDirections[iPrimaryVertexDirectionCount - 1] = Modulo7_fcAsASCII(
			Modulo7_meNegative(feLast(iPrimaryVertexDirectionCount, rzcASCIIDirections))
		);
		rzcASCIIDirections[iSecondaryVertexDirectionCount - 1] = Modulo7_fcAsASCII(eNewSecondaryVertex);

		/* This one changes the hexagon state. */
		fSetLast(iDenormalizedVertexDirectionCount, rcPentagonCount, rzcASCIIDirections, Modulo7_i0);

		/* Return the direction adjustment.
		Modulo7_i2 is required due to lattice arrangement and underlap placement.
		*/
		return Modulo7_feQuotient(Modulo7_feProduct(Modulo7_i2, eNewSecondaryVertex), eOldDenormalizedVertex);
	}
}

/* Returns the adjacent underlap direction for the cell. */
static INLINE Modulo7_te fePentagonalUnderlap(
	REGISTER char const cDirectionCount,
	REGISTER char const cPentagonCount,
	REGISTER char const rzcASCIIDirections[mReferenceConst(cDirectionCount + 1)]
) {
	mAssert(rzcASCIIDirections);
	mAssert(0 < cDirectionCount);
	mAssert((int unsigned const)cDirectionCount <= CharString_miuCount(rzcASCIIDirections));
	mAssert(iPrimaryVertexDirectionCount <= cDirectionCount);

	/* If it's a hexagon, there are no underlaps. */
	if (mbHexagon(cDirectionCount, cPentagonCount)) return Modulo7_i0;

	/* If there is a secondary vertex, the underlap is double the secondary vertex direction. */
	if (iSecondaryVertexDirectionCount <= cDirectionCount) {
		REGISTER Modulo7_te const eSecondaryVertex = feLast(iSecondaryVertexDirectionCount, rzcASCIIDirections);
		if (Modulo7_i0 != eSecondaryVertex) return Modulo7_feProduct(Modulo7_i2, eSecondaryVertex);
	}

	/* The underlap is the primary vertex direction. */
	return feLast(iPrimaryVertexDirectionCount, rzcASCIIDirections);
}

/* For a secondary vertex cell, the only valid vertex direction is the opposite of the cell vertex. */
static INLINE Boolean_tb fbSecondaryVertexUnderlap(
	REGISTER char const rzcASCIIDirections[mReferenceConst(iSecondaryVertexDirectionCount + 1)],
	REGISTER Modulo7_te const eDirection
) {
	mAssert(rzcASCIIDirections);
	mAssert(Modulo7_fbValid(eDirection));
	mAssert(Modulo7_i0 != eDirection);

	{
		/* If there is a secondary vertex, return true if the direction isn't its negation. */
		REGISTER Modulo7_te const eSecondaryVertex = feLast(iSecondaryVertexDirectionCount, rzcASCIIDirections);
		return Modulo7_i0 != eSecondaryVertex && Modulo7_meNegative(eSecondaryVertex) != eDirection;
	}
}

/* If the direction represents an underlap, convert to overlap.  If it is invalid, return Modulo7_i0.
Otherwise, return direction unchanged.
*/
static INLINE Modulo7_te feNormalizeDirection(
	REGISTER char const cDirectionCount,
	REGISTER char const cPentagonCount,
	REGISTER char const rzcASCIIDirections[mReferenceConst(cDirectionCount + 1)],
	REGISTER Modulo7_te const eDirection
) {
	mAssert(rzcASCIIDirections);
	mAssert(0 < cDirectionCount);
	mAssert((int unsigned const)cDirectionCount <= CharString_miuCount(rzcASCIIDirections));
	mAssert(Modulo7_fbValid(eDirection));
	mAssert(Modulo7_i0 != eDirection);

	/* If it's not an underlap, return it unchanged. */
	if (eDirection != fePentagonalUnderlap(cDirectionCount, cPentagonCount, rzcASCIIDirections)) return eDirection;

	/* Bump the direction to the overlapping adjacent direction. */
	return Modulo7_feProduct(eDirection, feAdjustmentToOtherAdjacentVertexAddend(mbOdd(cDirectionCount)));
}

/* Returns true if the direction represents a normalized child of the cell.
Allows denormalized pole.
*/
static INLINE Boolean_tb fbChild(
	REGISTER char const cDirectionCount,
	REGISTER char const cPentagonCount,
	REGISTER char const rzcASCIIDirections[mReferenceConst(cDirectionCount + 1)],
	REGISTER Modulo7_te const eDirection
) {
	mAssert(rzcASCIIDirections);
	mAssert((int unsigned const)cDirectionCount <= CharString_miuCount(rzcASCIIDirections));
	mAssert(Modulo7_fbValid(eDirection));

	/* Root only has non-zero sign directions as children. */
	if (0 == cDirectionCount) return Modulo7_fbNonZeroSign(eDirection);

	/* Everyone else has a centroid child. */
	if (Modulo7_i0 == eDirection) return 1;

	/* Only centroids have vertex children. */
	if (Modulo7_iASCII0 != rzcASCIIDirections[cDirectionCount - 1]) return 0;

	switch (cDirectionCount) {
	case iSecondaryVertexDirectionCount:
		/* Do not allow pole-denormalized cells. */
		return 0;
	case iDenormalizedVertexDirectionCount:
		/* Do not allow children that fall inside the secondary vertex underlap. */
		if (fbSecondaryVertexUnderlap(rzcASCIIDirections, eDirection)) return 0;
		/* Fall out... */
	}

	/* Do not allow children that fall inside the pentagonal underlap. */
	return eDirection != fePentagonalUnderlap(cDirectionCount, cPentagonCount, rzcASCIIDirections);
}

/* Returns the new direction count.  Assumes that there is sufficient capacity. */
static INLINE char fcPush(
	REGISTER char const cDirectionCount,
	REGISTER char rcPentagonCount[mReferenceConst(1)],
	REGISTER char rzcASCIIDirections[mReferenceConst(cDirectionCount + 2)],
	REGISTER Modulo7_te const eDirection
) {
	mAssert(rcPentagonCount);
	mAssert(rzcASCIIDirections);
	mAssert((int unsigned const)cDirectionCount == CharString_miuCount(rzcASCIIDirections));
	mAssert(fbChild(cDirectionCount, *rcPentagonCount, rzcASCIIDirections, eDirection));
	mAssert(Modulo7_fbValid(eDirection));

	/* Adjust the pentagon count. */
	if (cDirectionCount == *rcPentagonCount && (
		Modulo7_i0 == eDirection || cDirectionCount < iSecondaryVertexDirectionCount
	)) ++*rcPentagonCount;

	/* Push the direction. */
	return (char const)CharString_fiuPush(cDirectionCount, rzcASCIIDirections, Modulo7_fcAsASCII(eDirection));
}

/* Returns the new direction count.  Assumes that there is at least one direction to pop. */
static INLINE char fcPop(
	REGISTER char const cDirectionCount,
	REGISTER char rcPentagonCount[mReferenceConst(1)],
	REGISTER char rzcASCIIDirections[mReferenceConst(cDirectionCount + 1)]
) {
	mAssert(rcPentagonCount);
	mAssert(rzcASCIIDirections);
	mAssert((int unsigned const)cDirectionCount == CharString_miuCount(rzcASCIIDirections));
	mAssert(0 < cDirectionCount);

	/* Adjust the pentagon count. */
	if (cDirectionCount == *rcPentagonCount) --*rcPentagonCount;

	/* Pop the direction. */
	return (char const)CharString_fiuPop(cDirectionCount, rzcASCIIDirections);
}

/* Forward declaration. */
static Modulo7_te feToAdjacent(
	REGISTER char const cDirectionCount,
	REGISTER char rcPentagonCount[mReferenceConst(1)],
	REGISTER char rzcASCIIDirections[mReferenceConst(cDirectionCount + 1)],
	REGISTER Modulo7_te eDirection
);

/* Moves to the adjacent and returns the adjustment to the original direction. */
static INLINE Modulo7_te feAdjustmentToAdjacent(
	REGISTER char const cDirectionCount,
	REGISTER char rcPentagonCount[mReferenceConst(1)],
	REGISTER char rzcASCIIDirections[mReferenceConst(cDirectionCount + 1)],
	REGISTER Modulo7_te const eDirection
) {
	mAssert(rcPentagonCount);
	mAssert(rzcASCIIDirections);
	mAssert((int unsigned const)cDirectionCount <= CharString_miuCount(rzcASCIIDirections));
	mAssert(Modulo7_fbValid(eDirection));

	/* Move to the adjacent, then divide the resulting direction by the original and 
	return the quotient, which is the resulting adjustment factor.
	*/
	return Modulo7_feQuotient(
		feToAdjacent(cDirectionCount, rcPentagonCount, rzcASCIIDirections, eDirection),
		eDirection
	);
}

/* Moves to the neighbour in the given direction (multiplied first by the adjustment factor), 
and returns the new, cumulative adjustment factor.
*/
static INLINE Modulo7_te feToAdjustedAdjacent(
	REGISTER char const cDirectionCount,
	REGISTER char rcPentagonCount[mReferenceConst(1)],
	REGISTER char rzcASCIIDirections[mReferenceConst(cDirectionCount + 1)],
	REGISTER Modulo7_te const eDirection,
	REGISTER Modulo7_te const eAdjustmentFactor
) {
	mAssert(rcPentagonCount);
	mAssert(rzcASCIIDirections);
	mAssert((int unsigned const)cDirectionCount <= CharString_miuCount(rzcASCIIDirections));
	mAssert(Modulo7_fbValid(eDirection));
	mAssert(Modulo7_fbValid(eAdjustmentFactor));

	/* Adjust the direction by the factor, move, get the adjustment factor for the move,
	and adjust the adjustment factor to return accordingly.
	*/
	return Modulo7_feProduct(
		eAdjustmentFactor,
		feAdjustmentToAdjacent(cDirectionCount, rcPentagonCount, rzcASCIIDirections, Modulo7_feProduct(
			eDirection,
			eAdjustmentFactor
		))
	);
}

/* To adjacent of the hexagonal centroid child of a vertex child, requiring centroid parent normalization. */
static INLINE Modulo7_te feToAdjacentFromChildOfVertexHexagon(
	REGISTER char const cDirectionCount,
	REGISTER char rcPentagonCount[mReferenceConst(1)],
	REGISTER char rzcASCIIDirections[mReferenceConst(cDirectionCount + 1)],
	REGISTER Modulo7_te const eDirection
) {
	mAssert(rcPentagonCount);
	mAssert(rzcASCIIDirections);
	mAssert(2 < cDirectionCount);
	mAssert((int unsigned const)cDirectionCount <= CharString_miuCount(rzcASCIIDirections));
	mAssert(Modulo7_iASCII0 == rzcASCIIDirections[cDirectionCount - 1]);
	mAssert(Modulo7_iASCII0 != rzcASCIIDirections[cDirectionCount - 2]);
	mAssert(Modulo7_fbValid(eDirection));
	mAssert(Modulo7_i0 != eDirection);
	mAssert(!fbChild(cDirectionCount - 1, *rcPentagonCount, rzcASCIIDirections, eDirection));
	mAssert(mbHexagon(cDirectionCount - 1, *rcPentagonCount));
	mAssert(mbHexagon(cDirectionCount, *rcPentagonCount));

	{
		/* Get the parent direction.  We save a subtraction by inlining feLast. */
		REGISTER Modulo7_te const eParentCellDirection = Modulo7_feFromASCII(rzcASCIIDirections[cDirectionCount - 2]);

		/* If the parent direction is adjacent to direction, do it last. Otherwise, unwanted recursion may result. */
		REGISTER Boolean_tb const bParentCellDirectionLast = (
			Modulo7_i0 != Modulo7_feSum(eParentCellDirection, eDirection) &&
			!mbSameTriangle(eParentCellDirection, eDirection)
		);

		/* Get the addends. */
		Modulo7_te aeAddends[2];
		aeAddends[0] = eParentCellDirection;
		aeAddends[1] = Modulo7_feProduct(
			eParentCellDirection,
			feAdjustmentToOtherAdjacentVertexAddend(mbOdd(cDirectionCount))
		);

		/* Set parent to centroid, which may make the cell a pentagon. */
		fSetLast(cDirectionCount - 1, rcPentagonCount, rzcASCIIDirections, Modulo7_i0);

		/* Set cell to the 1st addend direction, which will return the cell to a hexagon
		(although the pentagon count may have changed from the initial one).
		*/
		fSetLast(cDirectionCount, rcPentagonCount, rzcASCIIDirections, aeAddends[bParentCellDirectionLast]);
		mAssert(mbHexagon(cDirectionCount, *rcPentagonCount));

		/* Move in the direction, then the 2nd addend direction, and return the adjusted direction. */
		return Modulo7_feProduct(eDirection, feToAdjustedAdjacent(
			cDirectionCount,
			rcPentagonCount,
			rzcASCIIDirections,
			aeAddends[!bParentCellDirectionLast],
			feAdjustmentToAdjacent(cDirectionCount, rcPentagonCount, rzcASCIIDirections, eDirection)
		));
	}
}

/* Allows denormalized pole. */
static Modulo7_te feToAdjacent(
	REGISTER char const cDirectionCount,
	REGISTER char rcPentagonCount[mReferenceConst(1)],
	REGISTER char rzcASCIIDirections[mReferenceConst(cDirectionCount + 1)],
	REGISTER Modulo7_te eDirection
) {
	mAssert(rcPentagonCount);
	mAssert(rzcASCIIDirections);
	mAssert(2 < cDirectionCount);
	mAssert((int unsigned const)cDirectionCount <= CharString_miuCount(rzcASCIIDirections));
	mAssert(Modulo7_fbValid(eDirection));
	mAssert(Modulo7_i0 != eDirection);

	/* Normalize the direction. */
	eDirection = feNormalizeDirection(cDirectionCount, *rcPentagonCount, rzcASCIIDirections, eDirection);
	mAssert(Modulo7_i0 != eDirection);

	{
		REGISTER Modulo7_te const eCellDirection = feLast(cDirectionCount, rzcASCIIDirections);
		if (Modulo7_i0 != eCellDirection) {
			/* Every vertex child is supposed to have a centroid parent. */
			mAssert(Modulo7_iASCII0 == rzcASCIIDirections[cDirectionCount - 2]);

			if (eCellDirection != eDirection) {
				REGISTER Modulo7_te eSum = Modulo7_feSum(eDirection, eCellDirection);

				/* If "adjacent in from vertex" case... */
				if (mbSameTriangle(eCellDirection, eDirection)) {
					/* If underlapping secondary vertex case, use component vector addition:
					adjacent in = adjacent out and back to destination cell (which will be normalized).
					*/
					if (
						iSecondaryVertexDirectionCount == cDirectionCount - 2 &&
						Modulo7_iASCII0 != rzcASCIIDirections[iSecondaryVertexDirectionCount - 1]
					) return Modulo7_feProduct(eDirection, feToAdjustedAdjacent(
						cDirectionCount,
						rcPentagonCount,
						rzcASCIIDirections,
						Modulo7_meNegative(eCellDirection),
						feAdjustmentToAdjacent(cDirectionCount, rcPentagonCount, rzcASCIIDirections, eSum)
					));

					/* If stepping into an underlap, step over it. */
					if (eSum == fePentagonalUnderlap(cDirectionCount - 1, *rcPentagonCount, rzcASCIIDirections)) {
						eDirection = Modulo7_meNegative(eCellDirection);
						eSum = Modulo7_feSum(eSum, eDirection);
					}

					/* General case; set the sum.
					Since hexagon state will not be changed, the direction can be set directly.
					*/
					rzcASCIIDirections[cDirectionCount - 1] = Modulo7_fcAsASCII(eSum);
					return eDirection;
				}

				/* If "adjacent out from vertex" case, move parent in the direction between, and fall through to 
				"straight in from vertex" case to set the cell to center.
				*/
				if (Modulo7_i0 != eSum) eDirection = Modulo7_feProduct(eDirection, feAdjustmentToAdjacent(
					cDirectionCount - 1,
					rcPentagonCount,
					rzcASCIIDirections,
					Modulo7_feProduct(eSum, feAdjustmentFromAdjacentVertexSum(!mbOdd(cDirectionCount)))
				));

				/* "Straight in from vertex" case.  Set to center.  This can change the hexagon state. */
				fSetLast(cDirectionCount, rcPentagonCount, rzcASCIIDirections, Modulo7_i0);
				return eDirection;
			}

			/* "Straight out from vertex" case.
			Move grandparent in direction and set grandchild to the opposite direction.
			The grandchild can be set directly since hexagon state or rotation will not be changed.
			*/
			eDirection = feToAdjacent(cDirectionCount - 2, rcPentagonCount, rzcASCIIDirections, eDirection);
			rzcASCIIDirections[cDirectionCount - 1] = Modulo7_fcAsASCII(Modulo7_meNegative(eDirection));
			return eDirection;
		}
	}

	/* "Out from centroid" case.
	If it is a child, set it.  Otherwise, deal with special cases.
	Rather than calling fbChild, an optimized check is done here based on known information.
	*/
	mAssert(Modulo7_i0 != eDirection);
	mAssert(fePentagonalUnderlap(cDirectionCount, *rcPentagonCount, rzcASCIIDirections) != eDirection);
	mAssert(fePentagonalUnderlap(cDirectionCount - 1, *rcPentagonCount, rzcASCIIDirections) != eDirection);
	if (Modulo7_iASCII0 != rzcASCIIDirections[cDirectionCount - 2] || (
		iSecondaryVertexDirectionCount == cDirectionCount - 2 &&
		fbSecondaryVertexUnderlap(rzcASCIIDirections, eDirection)
	)) {
		/* If moving out from a cell whose parent is a hexagonal vertex child,
		do component vector addition from the centroid sibling of the vertex parent.
		*/
		if (mbHexagon(cDirectionCount, *rcPentagonCount)) return (
			feToAdjacentFromChildOfVertexHexagon(cDirectionCount, rcPentagonCount, rzcASCIIDirections, eDirection)
		);

		/* Move from pole-denormalized version of cell. */
		eDirection = feToAdjacent(cDirectionCount, rcPentagonCount, rzcASCIIDirections, Modulo7_feProduct(
			eDirection,
			feDenormalizePole(rcPentagonCount, rzcASCIIDirections)
		));
	} else fSetLast(cDirectionCount, rcPentagonCount, rzcASCIIDirections, eDirection);

	/* If pole-denormalized, normalize. */
	if (fbDenormalizedPole(cDirectionCount, rzcASCIIDirections)) {
		eDirection = Modulo7_feProduct(eDirection, feNormalizePole(rcPentagonCount, rzcASCIIDirections));
	}

	/* Normalize the incoming direction and return. */
	return Modulo7_meNegative(
		feNormalizeDirection(cDirectionCount, *rcPentagonCount, rzcASCIIDirections, Modulo7_meNegative(eDirection))
	);
}

/*
Cell
*/

/* Vertex offsets in the cell char array.
The char array can be considered a flattened struct, with offsets
of fields given by the following constants.
These should not be redefined; the purpose of this definition is self-documenting code.
*/
typedef enum {
	iMaximumDirectionCountOffset = 0,
	iDirectionCountOffset = 1,
	iPentagonCountOffset = 2,
	iASCIIDirectionsOffset = 3
} teFieldOffset;

/* Accessor macros that treat the cell char array as a struct. */
# define mrcMaximumDirectionCount(rzcCell) ((rzcCell) + iMaximumDirectionCountOffset)
# define mrcDirectionCount(rzcCell) ((rzcCell) + iDirectionCountOffset)
# define mrcPentagonCount(rzcCell) ((rzcCell) + iPentagonCountOffset)
# define mrzcASCIIDirections(rzcCell) ((rzcCell) + iASCIIDirectionsOffset)

/* Moves to the child.  Assumes that there is sufficient capacity. */
static INLINE Boolean_tb fbToChild(
	REGISTER Cell_tzc rzcCell[mReferenceConst(Cell_iMinimumCount)],
	REGISTER Modulo7_te const eDirection
) {
	mAssert(Cell_fbValid(rzcCell));
	mAssert(Modulo7_fbValid(eDirection));
	mAssert(*mrcDirectionCount(rzcCell) < *mrcMaximumDirectionCount(rzcCell));

	{
		REGISTER char * const rcDirectionCount = mrcDirectionCount(rzcCell);
		REGISTER char * const rcPentagonCount = mrcPentagonCount(rzcCell);
		REGISTER char * const rzcASCIIDirections = mrzcASCIIDirections(rzcCell);
	
		/* If not a child, return false. */
		if (!fbChild(*rcDirectionCount, *rcPentagonCount, rzcASCIIDirections, eDirection)) return 0;

		/* Push the direction. */
		*rcDirectionCount = fcPush(*rcDirectionCount, rcPentagonCount, rzcASCIIDirections, eDirection);
	}
	return 1;
}

/* Moves to the parent.  Assumes that there is one. */
static INLINE void fToParent(
	REGISTER Cell_tzc rzcCell[mReferenceConst(Cell_iMinimumCount)]
) {
	mAssert(Cell_fbValid(rzcCell));
	mAssert(0 < *mrcDirectionCount(rzcCell));
	mAssert(*mrcDirectionCount(rzcCell) <= *mrcMaximumDirectionCount(rzcCell));

	{
		REGISTER char * const rcDirectionCount = mrcDirectionCount(rzcCell);
		
		/* Pop the direction. */
		*rcDirectionCount = fcPop(*rcDirectionCount, mrcPentagonCount(rzcCell), mrzcASCIIDirections(rzcCell));
	}
}

/* Forward declaration. */
static Boolean_tb fbIterate(
	REGISTER Cell_tzc rzcCell[mReferenceConst(Cell_iMinimumCount)],
	REGISTER Modulo7_te const reChildOrder[mReferenceConst(7)],
	REGISTER char const cDirectionCount,
	REGISTER Boolean_tb (* const fbAction)(
		Cell_tzc const rzcCell[mReference(Cell_iMinimumCount)],
		void * pData
	),
	REGISTER void * const pData
);

/* Assumes that the direction count is greater than the one we're on,
and that the desired direction count is within cell capacity.
*/
static INLINE Boolean_tb fbIterateChild(
	REGISTER Cell_tzc rzcCell[mReferenceConst(Cell_iMinimumCount)],
	REGISTER int const iChildIndex,
	REGISTER Modulo7_te const reChildOrder[mReferenceConst(7)],
	REGISTER char const cDirectionCount,
	REGISTER Boolean_tb (* const fbAction)(
		Cell_tzc const rzcCell[mReference(Cell_iMinimumCount)],
		void * pData
	),
	REGISTER void * const pData
) {
	mAssert(Cell_fbValid(rzcCell));
	mAssert(0 <= iChildIndex && iChildIndex < 7);
	mAssert(reChildOrder);
	mAssert(Modulo7_fbValid(reChildOrder[iChildIndex]));
	mAssert(fbAction);
	mAssert(*mrcDirectionCount(rzcCell) < cDirectionCount);
	mAssert(*mrcDirectionCount(rzcCell) < *mrcMaximumDirectionCount(rzcCell));

	if (fbToChild(rzcCell, reChildOrder[iChildIndex])) {
		REGISTER Boolean_tb const bReturn = fbIterate(rzcCell, reChildOrder, cDirectionCount, fbAction, pData);
		fToParent(rzcCell);
		return bReturn;
	}
	return 1;
}

/* Assumes that the desired direction count is within cell capacity. */
static Boolean_tb fbIterate(
	REGISTER Cell_tzc rzcCell[mReferenceConst(Cell_iMinimumCount)],
	REGISTER Modulo7_te const reChildOrder[mReferenceConst(7)],
	REGISTER char const cDirectionCount,
	REGISTER Boolean_tb (* const fbAction)(
		Cell_tzc const rzcCell[mReference(Cell_iMinimumCount)],
		void * pData
	),
	REGISTER void * const pData
) {
	mAssert(Cell_fbValid(rzcCell));
	mAssert(reChildOrder);
	mAssert(Modulo7_fbValid(reChildOrder[0]));
	mAssert(Modulo7_fbValid(reChildOrder[1]));
	mAssert(Modulo7_fbValid(reChildOrder[2]));
	mAssert(Modulo7_fbValid(reChildOrder[3]));
	mAssert(Modulo7_fbValid(reChildOrder[4]));
	mAssert(Modulo7_fbValid(reChildOrder[5]));
	mAssert(Modulo7_fbValid(reChildOrder[6]));
	mAssert(fbAction);
	mAssert(*mrcDirectionCount(rzcCell) <= *mrcMaximumDirectionCount(rzcCell));

	{
		REGISTER char const cCellDirectionCount = *mrcDirectionCount(rzcCell);
		if (cCellDirectionCount < cDirectionCount) return (
			fbIterateChild(rzcCell, 0, reChildOrder, cDirectionCount, fbAction, pData) &&
			fbIterateChild(rzcCell, 1, reChildOrder, cDirectionCount, fbAction, pData) &&
			fbIterateChild(rzcCell, 2, reChildOrder, cDirectionCount, fbAction, pData) &&
			fbIterateChild(rzcCell, 3, reChildOrder, cDirectionCount, fbAction, pData) &&
			fbIterateChild(rzcCell, 4, reChildOrder, cDirectionCount, fbAction, pData) &&
			fbIterateChild(rzcCell, 5, reChildOrder, cDirectionCount, fbAction, pData) &&
			fbIterateChild(rzcCell, 6, reChildOrder, cDirectionCount, fbAction, pData)
		);
		return cCellDirectionCount == cDirectionCount && fbAction(rzcCell, pData);
	}
}

/* Test connectivities of cell. */
static Boolean_tb fbTestCell(
	REGISTER Cell_tzc const rzcCell[mReferenceConst(Cell_iMinimumCount)],
	REGISTER Cell_tzc rzcCellBuffer[mReferenceConst(Cell_iMinimumCount)]
) {
	mAssert(Cell_fbValid(rzcCell));
	mAssert(Cell_fbValid(rzcCellBuffer));

	/* Copy the cell into the buffer. */
	if (!Cell_fbCopy(rzcCellBuffer, rzcCell)) return mbVerify(0);

	{
		# if !defined(NDEBUG)
			File_ts sOutput = File_mInitializer();
			if (!File_fbOutput(&sOutput)) return mbVerify(0);

			/* Write the index. */
			if (
				!File_fbWrite(&sOutput, '\"') ||
				!File_fbWriteString(&sOutput, Cell_frzcAsString(rzcCell)) ||
				!File_fbWriteString(&sOutput, "\"\n")
			) return mbVerify(0);
		# endif

		/* Move to each possible direction and back, and ensure that we always end up where we started. */
		{
			REGISTER Modulo7_te eDirection = Modulo7_iMinimumNonZero;
			for (; eDirection <= Modulo7_iMaximumNonZero; ++eDirection) {
				/* Move in the direction. */
				REGISTER Modulo7_te eNewDirection = Cell_feToAdjacent(rzcCellBuffer, eDirection);

				# if !defined(NDEBUG)
					if (!File_fbWriteString(&sOutput, " + ") ||
						!File_fbWrite(&sOutput, Modulo7_fcAsASCII(eDirection)) ||
						!File_fbWriteString(&sOutput, " = \"") ||
						!File_fbWriteString(&sOutput, Cell_frzcAsString(rzcCellBuffer)) ||
						!File_fbWriteString(&sOutput, "\";")
					) return mbVerify(0);
				# endif

				/* Negate the resulting direction to move back again. */
				eNewDirection = Modulo7_meNegative(eNewDirection);

				# if !defined(NDEBUG)
					if (
						!File_fbWriteString(&sOutput, " + ") ||
						!File_fbWrite(&sOutput, Modulo7_fcAsASCII(eNewDirection)) ||
						!File_fbWriteString(&sOutput, " = \"")
					) return mbVerify(0);
				# endif

				/* Move back again. */
				eNewDirection = Cell_feToAdjacent(rzcCellBuffer, eNewDirection);

				# if !defined(NDEBUG)
					if (
						!File_fbWriteString(&sOutput, Cell_frzcAsString(rzcCellBuffer)) ||
						!File_fbWriteString(&sOutput, "\"\n")
					) return mbVerify(0);
				# endif

				/* Verify that we have reached the original index. */
				if (!Cell_fbEquivalent(rzcCellBuffer, rzcCell)) return mbVerify(0);

				# if !defined(NDEBUG)
					/* Confirm that the direction is correct. */
					{
						REGISTER Modulo7_te const eExpectedDirection = Modulo7_meNegative(
							Cell_feNormalizeDirection(rzcCell, eDirection)
						);
						if (eNewDirection != eExpectedDirection) return mbVerify(0);
					}
				# endif
			}
		}

		# if !defined(NDEBUG)
			{
				REGISTER char const cDirectionCount = *mrcDirectionCount(rzcCell);

				/* Move in a triangle from each possible direction, and ensure that we always end up where we started. */
				if (3 < cDirectionCount) {
					REGISTER Boolean_tb const bDirectionCountOdd = mbOdd(cDirectionCount);

					REGISTER Modulo7_te eDirection = Modulo7_iMinimumNonZero;
					for (; eDirection <= Modulo7_iMaximumNonZero; ++eDirection) {
						/* Move in the direction. */
						REGISTER Modulo7_te eNewDirection = Cell_feToAdjacent(rzcCellBuffer, eDirection);

						/* Move to the adjacent twice, such that we should end up on the original cell. */
						eNewDirection = Cell_feToAdjacent(rzcCellBuffer, Modulo7_meNegative(
							Modulo7_feProduct(eNewDirection, feAdjustmentToOtherAdjacentVertexAddend(bDirectionCountOdd))
						));
						eNewDirection = Cell_feToAdjacent(rzcCellBuffer, Modulo7_meNegative(
							Modulo7_feProduct(eNewDirection, feAdjustmentToOtherAdjacentVertexAddend(bDirectionCountOdd))
						));

						/* Verify that we have reached the original index. */
						if (!Cell_fbEquivalent(rzcCellBuffer, rzcCell)) return mbVerify(0);
					}
				}
			}
		# endif
	}
	return 1;
}

/* Used by iteration test. */
static Boolean_tb fbIterationTestAction(
	REGISTER Cell_tzc const rzcCell[mReference(Cell_iMinimumCount)],
	REGISTER void * pData
) {
	mAssert(Cell_fbValid(rzcCell));
	mAssert(pData);
	
	return fbTestCell(rzcCell, pData);
}

Boolean_tb Cell_fbValid(
	REGISTER Cell_tzc const rzcCell[mReferenceConst(Cell_iMinimumCount)]
) {
	return (
#if 0 /* TODO */
		fbValid(...) &&
#endif
		!fbDenormalizedPole(*mrcDirectionCount(rzcCell), mrzcASCIIDirections(rzcCell))
	);
}

Cell_tzc * Cell_fpzcAllocate(
	REGISTER char const cMaximumDirectionCount
) {
	REGISTER Cell_tzc * pzcCell;
	if (!Memory_mbAllocate(pzcCell, Cell_iMinimumCount + cMaximumDirectionCount)) return 0;
	mAssert(pzcCell);
	*mrcMaximumDirectionCount(pzcCell) = cMaximumDirectionCount;
	Cell_fInitialize(pzcCell);
	return pzcCell;
}

void Cell_fDeallocate(
	REGISTER Cell_tzc * rpzcCell[mReferenceConst(1)]
) {
	mAssert(rpzcCell);
	mAssert(Cell_fbValid(*rpzcCell));

	mbVerify(Memory_mbDeallocateAndZero(*rpzcCell));
}

Boolean_tb Cell_fbCopy(
	REGISTER Cell_tzc rzcCell[mReferenceRestrictConst(Cell_iMinimumCount)],
	REGISTER Cell_tzc const rzcSource[mReferenceRestrictConst(Cell_iMinimumCount)]
) {
	mAssert(Cell_fbValid(rzcCell));
	mAssert(Cell_fbValid(rzcSource));
	
	{
		REGISTER char const cSourceMaximumDirectionCount = *mrcMaximumDirectionCount(rzcSource);
		if (*mrcMaximumDirectionCount(rzcCell) < cSourceMaximumDirectionCount) return 0;
		CharString_mCopy(rzcCell, rzcSource, Cell_iMinimumCount + cSourceMaximumDirectionCount);
	}
	return 1;
}

Boolean_tb Cell_fbEquivalent(
	REGISTER Cell_tzc const rzcLeftCell[mReferenceConst(Cell_iMinimumCount)],
	REGISTER Cell_tzc const rzcRightCell[mReferenceConst(Cell_iMinimumCount)]
) {
	mAssert(Cell_fbValid(rzcLeftCell));
	mAssert(Cell_fbValid(rzcRightCell));

	return CharString_mbEquivalent(mrzcASCIIDirections(rzcLeftCell), mrzcASCIIDirections(rzcRightCell));
}

void Cell_fInitialize(
	REGISTER Cell_tzc rzcCell[mReferenceConst(Cell_iMinimumCount)]
) {
	mAssert(rzcCell);

	*mrcDirectionCount(rzcCell) = 0;
	*mrcPentagonCount(rzcCell) = 0;
	*mrzcASCIIDirections(rzcCell) = 0;
}

Boolean_tb Cell_fbPushString(
	REGISTER Cell_tzc rzcCell[mReferenceConst(Cell_iMinimumCount)],
	REGISTER char const rzcString[mReference(1)]
) {
	mAssert(Cell_fbValid(rzcCell));
	mAssert(rzcString);

	for (; ; ++rzcString) {
		REGISTER char const cASCII = *rzcString;
		if (!cASCII) return 1;
		if (!Modulo7_fbValidASCII(cASCII) || !Cell_fbToChild(rzcCell, Modulo7_miFromASCII(cASCII))) return 0;
	}
}

char const * Cell_frzcAsString(
	REGISTER Cell_tzc const rzcCell[mReferenceConst(Cell_iMinimumCount)]
) {
	mAssert(Cell_fbValid(rzcCell));

	return mrzcASCIIDirections(rzcCell);
}

char Cell_fcMaximumDirectionCount(
	REGISTER Cell_tzc const rzcCell[mReferenceConst(Cell_iMinimumCount)]
) {
	mAssert(Cell_fbValid(rzcCell));

	return *mrcMaximumDirectionCount(rzcCell);
}

char Cell_fcDirectionCount(
	REGISTER Cell_tzc const rzcCell[mReferenceConst(Cell_iMinimumCount)]
) {
	mAssert(Cell_fbValid(rzcCell));

	return *mrcDirectionCount(rzcCell);
}

Modulo7_teSign Cell_feClass(
	REGISTER Cell_tzc const rzcCell[mReferenceConst(Cell_iMinimumCount)]
) {
	mAssert(Cell_fbValid(rzcCell));

	{
		REGISTER char const cDirectionCount = *mrcDirectionCount(rzcCell);
		if (0 == cDirectionCount) return Modulo7_i0;
		return Modulo7_feNonZeroSign(mbOdd(cDirectionCount));
	}
}

Modulo7_te Cell_fePrimaryVertex(
	REGISTER Cell_tzc const rzcCell[mReferenceConst(Cell_iMinimumCount)]
) {
	mAssert(Cell_fbValid(rzcCell));

	if (*mrcDirectionCount(rzcCell) < iPrimaryVertexDirectionCount) return Modulo7_i0;
	return feLast(iPrimaryVertexDirectionCount, mrzcASCIIDirections(rzcCell));
}

Modulo7_te Cell_feSecondaryVertex(
	REGISTER Cell_tzc const rzcCell[mReferenceConst(Cell_iMinimumCount)]
) {
	mAssert(Cell_fbValid(rzcCell));

	if (*mrcDirectionCount(rzcCell) < iSecondaryVertexDirectionCount) return Modulo7_i0;
	return feLast(iSecondaryVertexDirectionCount, mrzcASCIIDirections(rzcCell));
}

Modulo7_te Cell_feVertex(
	REGISTER Cell_tzc const rzcCell[mReferenceConst(Cell_iMinimumCount)]
) {
	mAssert(Cell_fbValid(rzcCell));

	{
		REGISTER Modulo7_te const eVertex = Cell_feSecondaryVertex(rzcCell);
		if (Modulo7_i0 != eVertex) return eVertex;
	}
	return Cell_fePrimaryVertex(rzcCell);
}

Boolean_tb Cell_fbHexagon(
	REGISTER Cell_tzc const rzcCell[mReferenceConst(Cell_iMinimumCount)]
) {
	mAssert(Cell_fbValid(rzcCell));

	return mbHexagon(*mrcDirectionCount(rzcCell), *mrcPentagonCount(rzcCell));
}

Boolean_tb Cell_fbCentroid(
	REGISTER Cell_tzc const rzcCell[mReferenceConst(Cell_iMinimumCount)]
) {
	mAssert(Cell_fbValid(rzcCell));

	{
		REGISTER char const cDirectionCount = *mrcDirectionCount(rzcCell);
		return 0 == cDirectionCount || Modulo7_iASCII0 == mrzcASCIIDirections(rzcCell)[cDirectionCount - 1];
	}
}

Modulo7_te Cell_feNormalizeDirection(
	REGISTER Cell_tzc const rzcCell[mReferenceConst(Cell_iMinimumCount)],
	REGISTER Modulo7_te eDirection
) {
	mAssert(Cell_fbValid(rzcCell));
	mAssert(Modulo7_fbValid(eDirection));

	if (Modulo7_i0 != eDirection) {
		REGISTER char const cDirectionCount = *mrcDirectionCount(rzcCell);
		if (iSecondaryVertexDirectionCount < cDirectionCount) {
			return feNormalizeDirection(cDirectionCount, *mrcPentagonCount(rzcCell), mrzcASCIIDirections(rzcCell), eDirection);
		}
		if (cDirectionCount == iSecondaryVertexDirectionCount) {
			REGISTER char const * const rzcASCIIDirections = mrzcASCIIDirections(rzcCell);
			eDirection = feNormalizeDirection(cDirectionCount, *mrcPentagonCount(rzcCell), rzcASCIIDirections, eDirection);
			if (!fbSecondaryVertexUnderlap(rzcASCIIDirections, eDirection)) return eDirection;
		}
	}
	return Modulo7_i0;
}

Boolean_tb Cell_fbChild(
	REGISTER Cell_tzc const rzcCell[mReferenceConst(Cell_iMinimumCount)],
	REGISTER Modulo7_te const eDirection
) {
	mAssert(Cell_fbValid(rzcCell));
	mAssert(Modulo7_fbValid(eDirection));

	return fbChild(*mrcDirectionCount(rzcCell), *mrcPentagonCount(rzcCell), mrzcASCIIDirections(rzcCell), eDirection);
}

Boolean_tb Cell_fbToChild(
	REGISTER Cell_tzc rzcCell[mReferenceConst(Cell_iMinimumCount)],
	REGISTER Modulo7_te const eDirection
) {
	mAssert(Cell_fbValid(rzcCell));
	mAssert(Modulo7_fbValid(eDirection));

	/* If insufficient capacity, return false. */
	if (*mrcDirectionCount(rzcCell) == *mrcMaximumDirectionCount(rzcCell)) return 0;

	return fbToChild(rzcCell, eDirection);
}

Boolean_tb Cell_fbToParent(
	REGISTER Cell_tzc rzcCell[mReferenceConst(Cell_iMinimumCount)]
) {
	mAssert(Cell_fbValid(rzcCell));

	/* If root, return false. */
	if (0 == *mrcDirectionCount(rzcCell)) return 0;

	fToParent(rzcCell);
	return 1;
}

Modulo7_te Cell_feToAdjacent(
	REGISTER Cell_tzc rzcCell[mReferenceConst(Cell_iMinimumCount)],
	REGISTER Modulo7_te eDirection
) {
	mAssert(Cell_fbValid(rzcCell));
	mAssert(Modulo7_fbValid(eDirection));

	eDirection = Cell_feNormalizeDirection(rzcCell, eDirection);
	if (Modulo7_i0 == eDirection) return Modulo7_i0;
	return feToAdjacent(*mrcDirectionCount(rzcCell), mrcPentagonCount(rzcCell), mrzcASCIIDirections(rzcCell), eDirection);
}

Boolean_tb Cell_fbIterate(
	REGISTER Cell_tzc rzcCell[mReferenceConst(Cell_iMinimumCount)],
	REGISTER Modulo7_te const reChildOrder[mReferenceConst(7)],
	REGISTER char const cDirectionCount,
	REGISTER Boolean_tb (* const fbAction)(
		Cell_tzc const rzcCell[mReference(Cell_iMinimumCount)],
		void * pData
	),
	REGISTER void * const pData
) {
	mAssert(Cell_fbValid(rzcCell));
	mAssert(reChildOrder);
	mAssert(Modulo7_fbValid(reChildOrder[0]));
	mAssert(Modulo7_fbValid(reChildOrder[1]));
	mAssert(Modulo7_fbValid(reChildOrder[2]));
	mAssert(Modulo7_fbValid(reChildOrder[3]));
	mAssert(Modulo7_fbValid(reChildOrder[4]));
	mAssert(Modulo7_fbValid(reChildOrder[5]));
	mAssert(Modulo7_fbValid(reChildOrder[6]));
	mAssert(fbAction);

	/* If too deep for the cell, don't iterate at all. */
	return (
		cDirectionCount <= *mrcMaximumDirectionCount(rzcCell) && 
		fbIterate(rzcCell, reChildOrder, cDirectionCount, fbAction, pData)
	);
}

Boolean_tb Cell_fbTest(void) {
	/* Miscellaneous functional tests. */
	{
		enum {iMaximumDirectionCount = 45};

		/* Test construction. */
		Cell_tzc Cell_mConstruct(azcCell, iMaximumDirectionCount);
		if (iMaximumDirectionCount != *mrcMaximumDirectionCount(azcCell)) return mbVerify(0);
		if (0 != *mrcDirectionCount(azcCell)) return mbVerify(0);
		if (0 != *mrcPentagonCount(azcCell)) return mbVerify(0);
		if (0 != *mrzcASCIIDirections(azcCell)) return mbVerify(0);

		/* Test going to parent of root. */
		if (Cell_fbToParent(azcCell)) return mbVerify(0);
	}

	/* Allocation and deallocation. */
	{
		enum {iMaximumDirectionCount = 45};

		/* Test allocation. */
		Cell_tzc * pzcCellBuffer = Cell_fpzcAllocate(iMaximumDirectionCount);
		if (0 == pzcCellBuffer) return mbVerify(0);
		if (iMaximumDirectionCount != *mrcMaximumDirectionCount(pzcCellBuffer)) return mbVerify(0);
		if (0 != *mrcDirectionCount(pzcCellBuffer)) return mbVerify(0);
		if (0 != *mrcPentagonCount(pzcCellBuffer)) return mbVerify(0);
		if (0 != *mrzcASCIIDirections(pzcCellBuffer)) return mbVerify(0);

		/* Test deallocation. */
		Cell_fDeallocate(&pzcCellBuffer);
		if (pzcCellBuffer) return mbVerify(0);
	}

	printf("Exhaustive connectivity test:\n");
	{
		enum {
			# if defined(PROFILE)
				iMaximumDirectionCount = 10
			# elif defined(NDEBUG)
				iMaximumDirectionCount = 16
			# else
				iMaximumDirectionCount = 8
			# endif
		};

		Cell_tzc Cell_mConstruct(azcCell, iMaximumDirectionCount);
		Cell_tzc Cell_mConstruct(azcCellBuffer, iMaximumDirectionCount);

		static Modulo7_te const aeChildOrder[7] = {
			Modulo7_i0,
			Modulo7_i1,
			Modulo7_i2,
			Modulo7_i3,
			Modulo7_i4,
			Modulo7_i5,
			Modulo7_i6
		};

		REGISTER char cDirectionCount = 0;
		for (; cDirectionCount <= iMaximumDirectionCount; ++cDirectionCount) {
			printf("Resolution %d...\n", cDirectionCount);
			mAssert(0 == *mrcDirectionCount(azcCell));
			{
				REGISTER clock_t const tClock = clock();
				if (!Cell_fbIterate(azcCell, aeChildOrder, cDirectionCount, fbIterationTestAction, azcCellBuffer)) return mbVerify(0);
				{
					REGISTER double const dSeconds = ((double)(clock() - tClock) / CLOCKS_PER_SEC);
					printf(" Total: %g seconds.\n", dSeconds);
				}
				if (0 != Cell_fcDirectionCount(azcCell)) return mbVerify(0);
			}
		}
	}

	printf("Random connectivity test:\n");
	{
		# if defined(PROFILE)
			# define miluMovementCount() 1000LU
		# elif defined(NDEBUG)
			# define miluMovementCount() 10000000LU
		# else
			# define miluMovementCount() 10LU
		# endif

		enum {
			iMaximumDirectionCount = 44
		};

		Cell_tzc Cell_mConstruct(azcCell, iMaximumDirectionCount + 1);

		REGISTER char cDirectionCount = 0;
		for (; cDirectionCount <= iMaximumDirectionCount; ++cDirectionCount) {
			printf("Resolution %d, %lu iterations, starting at \"%s\"...\n", cDirectionCount, miluMovementCount(), mrzcASCIIDirections(azcCell));
			{
				REGISTER clock_t const tClock = clock();

				REGISTER int long unsigned iluMovementCount = miluMovementCount();
				for (; iluMovementCount > 0; --iluMovementCount) {
					REGISTER Modulo7_te const eDirection = Modulo7_meRandomNonZero();
					Cell_feToAdjacent(azcCell, eDirection);

					# if !defined(NDEBUG)
						printf(" + %d = \"%s\"\n", eDirection, mrzcASCIIDirections(azcCell));
					# endif
				}

				{
					REGISTER double const dSeconds = ((double)(clock() - tClock) / CLOCKS_PER_SEC);
					printf(" Total: %g seconds.\n", dSeconds);
				}
			}
			while (!Cell_fbToChild(azcCell, Modulo7_meRandom()));
		}
	}

	return 1;
}
