/* TODO: Index:

- Tighten up hexagonal logic.
	- Logic for stepping off tessellation?
		- See about choosing such that AddDirection is streamlined (i.e. don't require the 0 < iuPentagonCount check).
	- Test iteration for hexagonal (vs. icosahedral).
	- Add addition tests to cover all hexagonal cases.

- More tests.
	- Add tests to cover multi-directional addition cases.

- Allow for automatic dynamic allocation so that indexes larger than the index buffer can be used.
	- Add a pointer member to the end of Index struct, which points to a "dynamic string" (array of char, containing size, capacity, etc).
	- Manage this in functions so that it is automatically used.  Allows for infinitely sized indexes, which are still fast in 
	the normal case.
*/

# include "../Headers/index.h"
# include "Core/Headers/file.h"
# include <assert.h>
# include <time.h>
# include <string.h>

/* Forward declaration. */
static Direction_te feAddDirection(
	REGISTER Size_tiu const iuDirectionCount,
	REGISTER DirectionString_tzc rzcDirections[mReferenceConst(iuDirectionCount? iuDirectionCount: 1)],
	REGISTER Size_tiu riuPentagonCount[mReferenceConst(1)],
	REGISTER Direction_te eDirection
);

/* Get the lattice of the current index. */
static INLINE Lattice_tb fbLattice(
	REGISTER Size_tiu const iuDirectionCount
) {
	return Size_fbOdd(iuDirectionCount);
}

/* Sets the direction to center, and returns the adjusted pentagon count. */
static INLINE Size_tiu fiuSetDirectionToCenter(
	REGISTER Size_tiu const iuDirectionCount,
	REGISTER DirectionString_tzc rzcDirections[mReferenceConst(iuDirectionCount)],
	REGISTER Size_tiu iuPentagonCount
) {
	assert(iuDirectionCount > 0);
	mStrongAssert(rzcDirections);

	/* Set to center. */
	DirectionString_fSetDirection(rzcDirections + iuDirectionCount - 1, Direction_i111);

	/* If the parent was a pentagon, this is now a pentagon.
	Set the pentagon count to the offset of the next vertex direction (or the null terminator offset).
	*/
	if (iuPentagonCount == iuDirectionCount) do {
		++iuPentagonCount;
	} while (Direction_i111 == DirectionString_fiDirection(rzcDirections + iuPentagonCount - 1));
	return iuPentagonCount;
}

/* Sets the direction to the specified vertex direction, and returns the adjusted pentagon count. */
static INLINE Size_tiu fiuSetDirectionToVertex(
	REGISTER Size_tiu const iuDirectionCount,
	REGISTER DirectionString_tzc rzcDirections[mReferenceConst(iuDirectionCount)],
	REGISTER Size_tiu const iuPentagonCount,
	REGISTER Direction_te const eVertexDirection
) {
	assert(iuDirectionCount > 0);
	mStrongAssert(rzcDirections);
	assert(Direction_i111 != eVertexDirection);

	/* Set to vertex direction. */
	DirectionString_fSetDirection(rzcDirections + iuDirectionCount - 1, eVertexDirection);

	/* If this is now a hexagon, return differing pentagon count accordingly. */
	if (iuPentagonCount > iuDirectionCount && iuDirectionCount > 3) return iuDirectionCount;
	return iuPentagonCount;
}

/* If this returns false, the index refers to a pentagonal entity. */
static INLINE Boolean_tb fbHexagon(
	REGISTER Size_tiu const iuDirectionCount,
	REGISTER Size_tiu const iuPentagonCount
) {
	/* Note that pentagon count for a pentagon index is (resolution + 1), since index 0 (resolution 0) is a pentagon. */
	return (iuPentagonCount <= iuDirectionCount);
}

/* Returns the underlap direction proceeding from the index. */
static INLINE Direction_te feLocalUnderlapDirection(
	REGISTER Size_tiu const iuDirectionCount,
	REGISTER DirectionString_tzc const rzcDirections[mReferenceConst(iuDirectionCount)],
	REGISTER Size_tiu const iuPentagonCount
) {
	assert(rzcDirections);
	assert(iuDirectionCount > 0);

	{
		Direction_te eDirection = Direction_i111;
		switch (iuDirectionCount) {
		default:
			if (fbHexagon(iuDirectionCount, iuPentagonCount)) return Direction_i111;
		case 3:
			mbVerify(DirectionString_fbGetDirection(rzcDirections + 2, &eDirection));
			if (Direction_i111 != eDirection) return eDirection;
		case 2:
			mbVerify(DirectionString_fbGetDirection(rzcDirections + 1, &eDirection));
			if (Direction_i111 != eDirection) return eDirection;
		case 1:
			mbVerify(DirectionString_fbGetDirection(rzcDirections, &eDirection));
			if (Direction_i111 != eDirection) {
				assert(Index_iRootUnderlapDirection == eDirection);
				return Direction_feNegative(eDirection);
			}
		case 0:
			return Index_iRootUnderlapDirection;
		}
	}
}

/* Alternates between choice of neighbour according to lattice to choose the neighbour closest to the global direction.
This function can be used to just get the adjacent sibling, in the "overlap" direction, of any vertex.
*/
static INLINE Direction_te feLocalOverlapDirection(
	REGISTER Lattice_tb const bLattice,
	REGISTER Direction_te const eLocalUnderlapDirection
) {
	return bLattice? Direction_feNegativeDouble(eLocalUnderlapDirection): Direction_feNegativeHalf(eLocalUnderlapDirection);
}

/* Returns the direction between two adjacent vertex addends, given the sum of the addends. */
static INLINE Direction_te feDirectionBetweenAdjacentAddends(
	REGISTER Lattice_tb const bAddendLattice,
	REGISTER Direction_te const eSumDirection
) {
	return bAddendLattice? Direction_feNegative(eSumDirection): Direction_feDouble(eSumDirection);
}

/* Resolution 2 icosahedral vertex children overlap partially.  This returns the normalized grandchildren as a result. */
static INLINE DirectionSet_tc fcNormalizedIcosahedralResolution2VertexGrandchildren(
	REGISTER Direction_te eIndexDirection
) {
	assert(Direction_i111 != eIndexDirection);
	
	eIndexDirection = Direction_feNegative(eIndexDirection);
	return DirectionSet_fcInsert(
		DirectionSet_fcInsert(DirectionSet_fcCenterOnly(), eIndexDirection), 
		feLocalOverlapDirection(0, eIndexDirection)
	);
}

/* Return the set of normalized icosahedral children. */
static INLINE DirectionSet_tc fcNormalizedChildDirections(
	REGISTER Size_tiu const iuDirectionCount,
	REGISTER DirectionString_tzc const rzcDirections[mReferenceConst(iuDirectionCount)],
	REGISTER Size_tiu const iuPentagonCount
) {
	assert(rzcDirections);

	/* If it's a vertex index, the only child is the centroid. */
	if (0 < iuDirectionCount && Direction_i111 != DirectionString_fiDirection(rzcDirections + iuDirectionCount - 1)) return DirectionSet_fcCenterOnly();

	/* Deal with special icosahedral cases. */
	if (0 < iuPentagonCount) {
		/* A temporary direction holder for efficiency. */
		Direction_te eDirection;

		/* Special cases for low resolutions. */
		switch (iuDirectionCount) {
		case 3:
			/* 7x7 has two vertex children: -x, and overlap(-x). */
			mbVerify(DirectionString_fbGetDirection(rzcDirections + 1, &eDirection));
			if (Direction_i111 != eDirection) {
				assert(Direction_i111 == DirectionString_fiDirection(rzcDirections + 2));
				return fcNormalizedIcosahedralResolution2VertexGrandchildren(eDirection);
			}
			break;
		case 2:
			/* Resolution 2 south index has no vertex children. */
			mbVerify(DirectionString_fbGetDirection(rzcDirections, &eDirection));
			if (Direction_i111 != eDirection) {
				assert(Direction_i111 == DirectionString_fiDirection(rzcDirections + 1));
				assert(eDirection == Index_iRootUnderlapDirection);
				return DirectionSet_fcCenterOnly();
			}
			break;
		case 0:
			/* Resolution 0 only has one normalized vertex child. */
			return DirectionSet_fcInsert(DirectionSet_fcCenterOnly(), Index_iRootUnderlapDirection);
		}
		/* If there is a local underlap, include all vertex children except that one. */
		eDirection = feLocalUnderlapDirection(iuDirectionCount, rzcDirections, iuPentagonCount);
		if (Direction_i111 != eDirection) return DirectionSet_fcRemove(DirectionSet_fcFull(), eDirection);
	}

	/* Include all possible children. */
	return DirectionSet_fcFull();
}

/* Helper function.  Assumes the index direction is a vertex, and not in the local underlap. */
static INLINE Boolean_tb fbIcosahedronResolution4LongitudinalUnderlap(
	REGISTER Direction_te const eGPIndexDirection,
	REGISTER Direction_te const eIndexDirection
) {
	return (eGPIndexDirection != Direction_i111) && (
		!DirectionSet_fbContains(fcNormalizedIcosahedralResolution2VertexGrandchildren(eGPIndexDirection), eIndexDirection)
	);
}

/* Pans to the neighbour in the given direction (multiplied first by the factor), and returns the adjusted factor. */
static Direction_te feAddDirectionWithFactor(
	REGISTER Size_tiu const iuDirectionCount,
	REGISTER DirectionString_tzc rzcDirections[mReferenceConst(iuDirectionCount? iuDirectionCount: 1)],
	REGISTER Size_tiu riuPentagonCount[mReferenceConst(1)],
	REGISTER Direction_te eDirection,
	REGISTER Direction_te const eFactor
) {
	assert(rzcDirections);

	eDirection = Direction_feProduct(eDirection, eFactor);
	{
		REGISTER Direction_te const eNewDirection = feAddDirection(iuDirectionCount, rzcDirections, riuPentagonCount, eDirection);
		if (eNewDirection != eDirection) return Direction_feProduct(eFactor, Direction_feQuotient(eNewDirection, eDirection));
	}
	return eFactor;
}

/* Pans to the neighbour in the given direction, and returns the adjusted direction. */
static Direction_te feAddDirection(
	REGISTER Size_tiu const iuDirectionCount,
	REGISTER DirectionString_tzc rzcDirections[mReferenceConst(iuDirectionCount? iuDirectionCount: 1)],
	REGISTER Size_tiu riuPentagonCount[mReferenceConst(1)],
	REGISTER Direction_te eDirection
) {
	assert(rzcDirections);
	mStrongAssert(riuPentagonCount);
	assert(Direction_fbValid(eDirection));

	if (Direction_i111 == eDirection) return Direction_i111;
	switch (iuDirectionCount) {
	case 1:
		if (0 < *riuPentagonCount) {
			assert(DirectionString_fiDirection(rzcDirections) == Direction_i111 || DirectionString_fiDirection(rzcDirections) == Index_iRootUnderlapDirection);
			DirectionString_fSetDirection(rzcDirections, (DirectionString_fiDirection(rzcDirections) == Direction_i111)? Index_iRootUnderlapDirection: Direction_i111);
			return Direction_feReverse(eDirection);
		}
	default:
		/* Sanity check for switch statement. */
		assert(0 == *riuPentagonCount || 1 < iuDirectionCount);
		{
			Direction_te eIndexDirection = Direction_i111;
			mbVerify(DirectionString_fbGetDirection(rzcDirections + iuDirectionCount - 1, &eIndexDirection));

			if (eDirection == eIndexDirection) /* Straight out from vertex */ {
				if (1 < iuDirectionCount) {
					/* If about to move into local underlap, pan to adjacent overlap instead (only possible in resolutions 2 and 3). */
					if (iuDirectionCount < 4 && 0 < *riuPentagonCount && (
						feLocalUnderlapDirection(iuDirectionCount, rzcDirections, *riuPentagonCount) == eDirection
					)) return feAddDirection(iuDirectionCount, rzcDirections, riuPentagonCount, 
						feLocalOverlapDirection(fbLattice(iuDirectionCount), eDirection)
					);

					/* Pan grandparent, then set direction to opposite to prepare for local pan. */
					eDirection = Direction_feNegative(feAddDirection(iuDirectionCount - 2, rzcDirections, riuPentagonCount, eDirection));

					/* If we're headed into a longitudinal underlap, adjust. */
					if (4 == iuDirectionCount && 0 < *riuPentagonCount) {
						Direction_te eGPIndexDirection = Direction_i111;
						mbVerify(DirectionString_fbGetDirection(rzcDirections + 1, &eGPIndexDirection));
						if (fbIcosahedronResolution4LongitudinalUnderlap(eGPIndexDirection, eDirection)) {
							REGISTER Lattice_tb const bOverlapLattice = (feLocalOverlapDirection(0, eDirection) == eGPIndexDirection);
							DirectionString_fSetDirection(rzcDirections + iuDirectionCount - 1, eIndexDirection = feLocalOverlapDirection(bOverlapLattice, eDirection));
							return feLocalOverlapDirection(bOverlapLattice, eIndexDirection);
						}
					}
				}

				/* Pan local. */
				*riuPentagonCount = fiuSetDirectionToCenter(iuDirectionCount, rzcDirections, *riuPentagonCount);
				return Direction_feNegative(feAddDirection(iuDirectionCount, rzcDirections, riuPentagonCount, eDirection));
			}

			if (Direction_fiuBitSum(eIndexDirection) == Direction_fiuBitSum(eDirection)) /* Adjacent in from vertex */ {
				/* Cache the current index direction. */
				REGISTER Direction_te const eOldIndexDirection = eIndexDirection;

				/* Set the index direction to the sum, and update eIndexDirection for use in resolution 4 underlap correction below. */
				DirectionString_fSetDirection(rzcDirections + iuDirectionCount - 1, eIndexDirection = Direction_feSum(eIndexDirection, eDirection));

				/* If we're in the local underlap direction, keep going. */
				if (0 < *riuPentagonCount && feLocalUnderlapDirection(iuDirectionCount - 1, rzcDirections, *riuPentagonCount) == eIndexDirection) {
					eDirection = Direction_feNegative(eOldIndexDirection);
					DirectionString_fSetDirection(rzcDirections + iuDirectionCount - 1, eIndexDirection = Direction_feSum(eIndexDirection, eDirection));
				}
			} else if (Direction_i111 == eIndexDirection) /* From center */ {
				if (0 < *riuPentagonCount) {
					/* If about to move into the local underlap, adjust direction to go to adjacent overlap instead. */
					if (feLocalUnderlapDirection(iuDirectionCount, rzcDirections, *riuPentagonCount) == eDirection) {
						eDirection = feLocalOverlapDirection(fbLattice(iuDirectionCount), eDirection);
					}

					/* Handle stepping off of the tessellation for low resolutions.  These are cases that the "vertex parent" code
					below this isn't designed to handle, so we must get them out of the way first.
					*/
					if (iuDirectionCount < 4) {
						/* From south pole. */
						if (DirectionString_fiDirection(rzcDirections) != Direction_i111) {
							assert(DirectionString_fiDirection(rzcDirections) == Index_iRootUnderlapDirection);
							eDirection = feAddDirection(1, rzcDirections, riuPentagonCount, eDirection);

							if (2 == iuDirectionCount) return Direction_feProduct(eDirection, 
								feAddDirectionWithFactor(2, rzcDirections, riuPentagonCount,
									feLocalOverlapDirection(0, Direction_feNegative(eDirection)), 
									Direction_i001
								)
							);
							return feLocalOverlapDirection(1, Direction_feProduct(eDirection,
								feAddDirectionWithFactor(2, rzcDirections, riuPentagonCount,
									Direction_feNegative(eDirection), 
									Direction_i001
								)
							));
						}
						/* From any other pentagonal vertex centroid.  The only possibility here is in resolution 3. */
						{
							Direction_te eR2Direction = Direction_i111;
							mbVerify(DirectionString_fbGetDirection(rzcDirections + 1, &eR2Direction));
							if (Direction_i111 != eR2Direction) {
								assert(3 == iuDirectionCount);

								if (feLocalOverlapDirection(1, eR2Direction) == eDirection) /* To south pole. */ {
									DirectionString_fSetDirection(rzcDirections + 1, Direction_i111);
									return feAddDirection(1, rzcDirections, riuPentagonCount, eR2Direction);
								}
								if (feLocalOverlapDirection(1, eDirection) == eR2Direction) /* To neighbour. */ {
									DirectionString_fSetDirection(rzcDirections + 1, Direction_i111);
									return feLocalOverlapDirection(0, feAddDirection(2, rzcDirections, riuPentagonCount, eDirection));
								}
							}
						}
					}
				}

				/* If the parent is a vertex, effectively pan from the parent's centroid sibling and correct outward. */
				if (1 < iuDirectionCount) {
					REGISTER Size_tiu const iuPDirectionCount = iuDirectionCount - 1;
					Direction_te aeAddends[2];
					mbVerify(DirectionString_fbGetDirection(rzcDirections + iuPDirectionCount - 1, aeAddends));
					if (Direction_i111 != aeAddends[0]) {
						/* Process the addend with the same bit sum as eDirection first, unless the other addend is -eDirection. */
						REGISTER Boolean_tb bFirstDirection = (Direction_fiuBitSum(eDirection) == Direction_fiuBitSum(
							aeAddends[1] = feLocalOverlapDirection(fbLattice(iuDirectionCount), aeAddends[0])
						));
						if (Direction_fbOppositeVertices(aeAddends[!bFirstDirection], eDirection)) bFirstDirection = !bFirstDirection;

						/* Set the parent index to center. */
						*riuPentagonCount = fiuSetDirectionToCenter(iuPDirectionCount, rzcDirections, *riuPentagonCount);

						/* Set the index to the 1st addend. */
						*riuPentagonCount = fiuSetDirectionToVertex(iuDirectionCount, rzcDirections, *riuPentagonCount, aeAddends[bFirstDirection]);

						/* Add the direction, then the 2nd addend.  Return the direction adjusted by the accumulated rotation factor. */
						return Direction_feProduct(eDirection, 
							feAddDirectionWithFactor(iuDirectionCount, rzcDirections, riuPentagonCount, aeAddends[!bFirstDirection], 
								feAddDirectionWithFactor(iuDirectionCount, rzcDirections, riuPentagonCount, eDirection, Direction_i001)
							)
						);
					}
				}

				/* Set the direction, and update eIndexDirection for use in resolution 4 underlap correction below. */
				*riuPentagonCount = fiuSetDirectionToVertex(iuDirectionCount, rzcDirections, *riuPentagonCount, eDirection);
				eIndexDirection = eDirection;
			} else {
				if (!Direction_fbOppositeVertices(eIndexDirection, eDirection)) /* Adjacent out from vertex */ {
					/* Get the direction in which to pan the parent (and possibly adjust eDirection).
					If icosahedral resolution 2, there are some special rules for south pole connectivity.
					*/
					REGISTER Direction_te const eParentDirection = (
						(2 == iuDirectionCount && 0 < *riuPentagonCount && feLocalOverlapDirection(0, eIndexDirection) == eDirection)?
						(eDirection = eIndexDirection):
						feDirectionBetweenAdjacentAddends(fbLattice(iuDirectionCount), Direction_feSum(eIndexDirection, eDirection))
					);

					/* Pan the parent, adjusting the direction accordingly, and fall through to set this to center. */
					eDirection = Direction_feProduct(eDirection,
						feAddDirectionWithFactor(iuDirectionCount - 1, rzcDirections, riuPentagonCount, eParentDirection, Direction_i001)
					);

					/* If coming in on the local underlap, adjust direction accordingly. */
					if (4 == iuDirectionCount && 0 < *riuPentagonCount) {
						Direction_te eGPDirection = Direction_i111;
						mbVerify(DirectionString_fbGetDirection(rzcDirections + 1, &eGPDirection));
						if (Direction_feNegative(eDirection) == eGPDirection) eDirection = feLocalOverlapDirection(0, eDirection);
					}
				}

				/* To center. */
				*riuPentagonCount = fiuSetDirectionToCenter(iuDirectionCount, rzcDirections, *riuPentagonCount);
				return eDirection;
			}

			/* If we've landed in a longitudinal underlap, adjust direction and keep going from center. */
			if (4 == iuDirectionCount && 0 < *riuPentagonCount) {
				Direction_te eGPIndexDirection = Direction_i111;
				mbVerify(DirectionString_fbGetDirection(rzcDirections + 1, &eGPIndexDirection));
				if (fbIcosahedronResolution4LongitudinalUnderlap(eGPIndexDirection, eIndexDirection)) {
					return feLocalOverlapDirection((feLocalOverlapDirection(0, eIndexDirection) == eGPIndexDirection),
						Direction_feProduct(eDirection,
							feAddDirectionWithFactor(4, rzcDirections, riuPentagonCount, eIndexDirection, Direction_i001)
						)
					);
				}
			}
		}
		return eDirection;
	case 0:
		return Direction_i111;
	}
}

/* Writes the index to the stream. */
static INLINE Boolean_tb fbWrite(
	REGISTER Size_tiu const iuDirectionCount,
	REGISTER DirectionString_tzc const rzcDirections[mReferenceConst(iuDirectionCount? iuDirectionCount: 1)],
	REGISTER File_ts rsOutput[mReferenceConst(1)] /* Already open for writing */
) {
	mStrongAssert(rzcDirections);
	assert(rsOutput);

	if (0 == iuDirectionCount) return File_fbWrite(rsOutput, '0');
	return DirectionString_fbWrite(rzcDirections, rsOutput);
}

/* Reads a series of "instructions" for moving the supplied index.  Instructions include:
* 1-6: down to vertex child
* 7: down to centroid child
* 0: up to parent
* +: add the next index to the previous.
*/
static Boolean_tb fbParse(
	REGISTER File_ts rsInput[mReferenceConst(1)] /* Already open for reading */,
	REGISTER File_ts rsOutput[mReferenceConst(1)] /* Already open for writing */,
	REGISTER Boolean_tb const bIcosahedron
) {
	assert(rsInput);
	assert(rsOutput);

	{
		Index_ts sIndex = Index_mInitializer(0);
		mbVerify(Index_fbInitialize(&sIndex, bIcosahedron, ""));

		for (; ; ) {
			REGISTER int iDirection = Index_fiRead(&sIndex, rsInput);
			if ('+' == iDirection) {
				Index_ts sAddend = Index_mInitializer(0);
				Index_ts sSum = Index_mInitializer(0);
				mbVerify(Index_fbInitialize(&sSum, bIcosahedron, ""));

				/* Read the next index and get the next character to process. */
				iDirection = Index_fiRead(&sAddend, rsInput);

				/* Add the indexes. */
				mbVerify(Index_fbAdd(&sSum, &sIndex, &sAddend));

				/* Copy the sum into 'sIndex'. */
				sIndex = sSum;
			}
			switch (iDirection) {
			case '0': /* Decimate. */
				Index_fbDecrement(&sIndex);
				break;
			case '\n': /* Write the index. */
				if (!fbWrite(sIndex.iuDirectionCount, sIndex.azcDirections, rsOutput) || !File_fbWrite(rsOutput, '\n')) return mbVerify(0);
				break;
			default:
				return 1;
			}
		}
	}
}

/* Helper for Index_fbIterate. */
static Boolean_tb fbIterateVertices(
	REGISTER Boolean_tb const bDepthFirst,
	REGISTER Index_ts rsIndex[mReferenceConst(1)],
	REGISTER DirectionSet_tc const cChildDirectionSet,
	REGISTER Size_tiu const iuDirectionCount,
	REGISTER Boolean_tb (* const fbAction)(Index_ts const rsIndex[mReference(1)], void * pData),
	REGISTER void * const pData
) {
	mStrongAssert(rsIndex);
	assert(fbAction);

	{
		REGISTER Direction_te eDirection;
		for (eDirection = Direction_i001; eDirection < Direction_i111; ++eDirection) {
			/* If this is a normalized vertex direction, iterate. */
			if (DirectionSet_fbContains(cChildDirectionSet, eDirection)) {
				/* Set to the vertex direction, iterate, and return to center. */
				rsIndex->iuPentagonCount = fiuSetDirectionToVertex(rsIndex->iuDirectionCount, rsIndex->azcDirections, rsIndex->iuPentagonCount, eDirection);
				if (!Index_fbIterate(bDepthFirst, rsIndex, iuDirectionCount, fbAction, pData)) return 0;
				rsIndex->iuPentagonCount = fiuSetDirectionToCenter(rsIndex->iuDirectionCount, rsIndex->azcDirections, rsIndex->iuPentagonCount);
			}
		}
	}
	return 1;
}

/* Pan randomly the desired number of times, and time it. */
static INLINE double fdSecondsToAddRandomDirections(
	REGISTER Index_ts rsIndex[mReferenceConst(1)],
	REGISTER int unsigned iuPanCount
) {
	assert(rsIndex);

	{
		#if !defined(NDEBUG)
			File_ts sOutput = File_mInitializer();
			mbVerify(File_fbOutput(&sOutput));
		# endif
		{
			REGISTER clock_t const tClock = clock();

			for (; iuPanCount > 0; --iuPanCount) {
				Index_feAddDirection(rsIndex, Direction_feRandom(0));

				# if !defined(NDEBUG)
					mbVerify(Index_fbWrite(rsIndex, &sOutput));
					printf("\n");
				# endif
			}
			return (double)(clock() - tClock) / CLOCKS_PER_SEC; 
		}
	}
}

/* Test connectivities of index. */
static Boolean_tb fbTestIndex(
	REGISTER Index_ts const rsIndex[mReferenceConst(1)],
	REGISTER Boolean_tb const bWrite
) {
	mStrongAssert(rsIndex);

	# if defined(NDEBUG)
		(void)bWrite;
	# else
		if (bWrite) {
			/* Write the index. */
			File_ts sOutput = File_mInitializer();
			mbVerify(File_fbOutput(&sOutput));
			mbVerify(Index_fbWrite(rsIndex, &sOutput));
			printf("\n");
		}
	# endif

	/* If the resolution is greater than 2, test connectivities. */
	if (rsIndex->iuDirectionCount > 2) {
		REGISTER Direction_te eDirection;
		Index_ts sIndex = *rsIndex; /* Make a non-const copy of the index. */

		/* Move straight out in each direction and back. */
		for (eDirection = Direction_i001; eDirection < Direction_i111; ++eDirection) {
			/* Move in the direction. */
			REGISTER Direction_te eNewDirection = Index_feAddDirection(&sIndex, eDirection);

			/* Move back and verify. */
			eNewDirection = Index_feAddDirection(&sIndex, Direction_feNegative(eNewDirection));
			if (!Index_fbEquivalent(&sIndex, rsIndex)) return mbVerify(0);
		}

		# if !defined(NDEBUG)
			{
				REGISTER Lattice_tb const bLattice = fbLattice(rsIndex->iuDirectionCount);

				/* Move in triangles, from each possible direction, and ensure that we always end up where we started. */
				for (eDirection = Direction_i001; eDirection < Direction_i111; ++eDirection) {
					/* Move in the direction. */
					REGISTER Direction_te eNewDirection = Index_feAddDirection(&sIndex, eDirection);

					/* Move to the adjacent twice, and verify that we have reached the original index. */
					eNewDirection = Index_feAddDirection(&sIndex, Direction_feNegative(feLocalOverlapDirection(bLattice, eNewDirection)));
					eNewDirection = Index_feAddDirection(&sIndex, Direction_feNegative(feLocalOverlapDirection(bLattice, eNewDirection)));
					if (!Index_fbEquivalent(&sIndex, rsIndex)) return mbVerify(0);
				}
			}
		# endif
	}

	return 1;
}

/* Test the addition of two direction strings as indexes. */
static Boolean_tb fbTestAddition(
	REGISTER Boolean_tb const bIcosahedral,
	REGISTER char const rzcAugend[mReferenceConst(1)],
	REGISTER char const rzcAddend[mReferenceConst(1)],
	REGISTER char const rzcExpectedSum[mReferenceConst(1)]
) {
	assert(rzcAugend);
	assert(rzcAddend);
	mStrongAssert(rzcExpectedSum);

	{
		Index_ts sAugend = Index_mInitializer(0);
		Index_ts sAddend = Index_mInitializer(0);
		Index_ts sSum = Index_mInitializer(0);
		if (!Index_fbInitialize(&sAugend, bIcosahedral, rzcAugend)) return mbVerify(0);
		if (!Index_fbInitialize(&sAddend, 0, rzcAddend)) return mbVerify(0);
		if (!Index_fbInitialize(&sSum, bIcosahedral, "")) return mbVerify(0);
		if (!Index_fbAdd(&sSum, &sAugend, &sAddend)) return mbVerify(0);
		if (0 != strcmp(sSum.azcDirections, rzcExpectedSum)) return mbVerify(0);
	}
	return 1;
}

/* Used by test. */
static Boolean_tb fbIterationTestAction(
	REGISTER Index_ts const rsIndex[mReference(1)],
	REGISTER void * pData
) {
	mStrongAssert(rsIndex);
	
	/* Unused. */
	(void)pData;

	/* Test the index. */
	return fbTestIndex(rsIndex, 0);
}

/*
Index
*/

/* Initializes the index.
Note that the directions passed in must be in numeric format; stops when an invalid character is reached.
Returns false if there were too many directions too fit, resulting in an index 
that represents a truncation.
*/
Boolean_tb Index_fbInitialize(
	REGISTER Index_ts rsIndex[mReferenceConst(1)],
	REGISTER Boolean_tb const bIcosahedron,
	REGISTER char const rzcDirections[mReference(1)]
) {
	mStrongAssert(rsIndex);
	mStrongAssert(rzcDirections);

	rsIndex->iuPentagonCount = (bIcosahedron? 1: 0);
	rsIndex->iuDirectionCount = 0;
	*(rsIndex->azcDirections) = 0;
	for (; Direction_fbValid(*rzcDirections); ++rzcDirections) {
		if (!Index_fbIncrement(rsIndex)) return 0;
		Index_feAddDirection(rsIndex, *rzcDirections);
	}
	return 1;
}

/* Copies the index. */
void Index_fCopy(
	REGISTER Index_ts const rsSource[mReferenceConst(1)],
	REGISTER Index_ts rsDestination[mReferenceConst(1)]
) {
	mStrongAssert(rsSource);
	mStrongAssert(rsDestination);
	
	*rsDestination = *rsSource;
}

/* Returns the number of pentagon ancestors of the index, including the index itself. */
Size_tiu Index_fiuPentagonCount(
	REGISTER Index_ts const rsIndex[mReferenceConst(1)]
) {
	mStrongAssert(rsIndex);

	return rsIndex->iuPentagonCount;
}

/* Returns true if the index is icosahedral, or false if hexagonal. */
Boolean_tb Index_fbIcosahedron(
	REGISTER Index_ts const rsIndex[mReferenceConst(1)]
) {
	return (0 < Index_fiuPentagonCount(rsIndex));
}

/* Returns the direction count. */
Size_tiu Index_fiuDirectionCount(
	REGISTER Index_ts const rsIndex[mReferenceConst(1)]
) {
	mStrongAssert(rsIndex);

	return rsIndex->iuDirectionCount;
}

/* Returns a string containing the numeric directions. */
DirectionString_tzc const * Index_frzcDirections(
	REGISTER Index_ts const rsIndex[mReferenceConst(1)]
) {
	mStrongAssert(rsIndex);

	return rsIndex->azcDirections;
}

/* Returns the lattice. */
Lattice_tb Index_fbLattice(
	REGISTER Index_ts const rsIndex[mReferenceConst(1)]
) {
	mStrongAssert(rsIndex);

	return fbLattice(rsIndex->iuDirectionCount);
}

/* Returns true if the index describes a centroid, or false if it's a vertex. */
Boolean_tb Index_fbCentroid(
	REGISTER Index_ts const rsIndex[mReferenceConst(1)]
) {
	mStrongAssert(rsIndex);

	return (0 == rsIndex->iuDirectionCount || Direction_i111 == DirectionString_fiDirection(rsIndex->azcDirections + rsIndex->iuDirectionCount - 1));
}

/* Returns whether the index describes a hexagonal entity. */
Boolean_tb Index_fbHexagon(
	REGISTER Index_ts const rsIndex[mReferenceConst(1)]
) {
	mStrongAssert(rsIndex);

	return fbHexagon(rsIndex->iuDirectionCount, rsIndex->iuPentagonCount);
}

/* Pans the index in any direction, and returns the adjusted direction. */
Direction_te Index_feAddDirection(
	REGISTER Index_ts rsIndex[mReferenceConst(1)],
	REGISTER Direction_te const eDirection
) {
	mStrongAssert(rsIndex);
	assert(Direction_fbValid(eDirection));

	return feAddDirection(rsIndex->iuDirectionCount, rsIndex->azcDirections, &(rsIndex->iuPentagonCount), eDirection);
}

/* Add to indexes. */
Boolean_tb Index_fbAdd(
	REGISTER Index_ts rsSum[mReferenceConst(1)],
	REGISTER Index_ts const rsAugend[mReference(1)],
	REGISTER Index_ts const rsAddend[mReference(1)]
) {
	mStrongAssert(rsSum);
	mStrongAssert(rsAugend);
	mStrongAssert(rsAddend);

	/* Ensure that the augend is the big one. */
	if (rsAugend->iuDirectionCount < rsAddend->iuDirectionCount) {
		REGISTER Index_ts const * const psAugend = rsAugend;
		rsAugend = rsAddend;
		rsAddend = psAugend;
	}

	/* Iterate through the augend and pan the result in each direction.
	If there is a corresponding direction in the addend, pan there too.
	*/
	{
		REGISTER Size_tiu iuAugendDirectionOffset = 0;
		REGISTER Size_tiu iuAddendDirectionOffset = 0;
		REGISTER Direction_te eFactor = Direction_i001;

		/* Reset the sum index. */
		rsSum->iuDirectionCount = 0;
		
		while (iuAugendDirectionOffset < rsAugend->iuDirectionCount) {
			/* Descend to centroid child. */
			if (!Index_fbIncrement(rsSum)) return 0;

			/* Add augend direction. */
			++iuAugendDirectionOffset;
			eFactor = feAddDirectionWithFactor(iuAugendDirectionOffset, rsSum->azcDirections, &(rsSum->iuPentagonCount),
				DirectionString_fiDirection(rsAugend->azcDirections + iuAugendDirectionOffset - 1), eFactor
			);

			/* Add addend direction, if there is one. */
			if ((rsAugend->iuDirectionCount - iuAugendDirectionOffset) < rsAddend->iuDirectionCount) {
				++iuAddendDirectionOffset;
				eFactor = feAddDirectionWithFactor(iuAugendDirectionOffset, rsSum->azcDirections, &(rsSum->iuPentagonCount),
					DirectionString_fiDirection(rsAddend->azcDirections + iuAddendDirectionOffset - 1), eFactor
				);
			}
		}
	}

	return 1;
}

/* Decimates the index. */
Boolean_tb Index_fbDecrement(
	REGISTER Index_ts rsIndex[mReferenceConst(1)]
) {
	mStrongAssert(rsIndex);
	mStrongAssert(Index__MAXIMUM_DIRECTION_COUNT >= rsIndex->iuDirectionCount);
	assert(Index__MAXIMUM_DIRECTION_COUNT + 1 == Size_miuArrayElementCount(rsIndex->azcDirections));

	if (0 == rsIndex->iuDirectionCount) return 0;
	if (!Index_fbHexagon(rsIndex)) {
		assert(rsIndex->iuPentagonCount == rsIndex->iuDirectionCount + 1);
		--(rsIndex->iuPentagonCount);
	}
	rsIndex->azcDirections[--(rsIndex->iuDirectionCount)] = 0;
	return 1;
}

/* Zooms the index to its centroid child. */
Boolean_tb Index_fbIncrement(
	REGISTER Index_ts rsIndex[mReferenceConst(1)]
) {
	mStrongAssert(rsIndex);
	mStrongAssert(Index__MAXIMUM_DIRECTION_COUNT >= rsIndex->iuDirectionCount);
	assert(Index__MAXIMUM_DIRECTION_COUNT + 1 == Size_miuArrayElementCount(rsIndex->azcDirections));

	if (Index__MAXIMUM_DIRECTION_COUNT == rsIndex->iuDirectionCount) return 0;

	rsIndex->azcDirections[++(rsIndex->iuDirectionCount)] = 0;
	rsIndex->iuPentagonCount = fiuSetDirectionToCenter(rsIndex->iuDirectionCount, rsIndex->azcDirections, rsIndex->iuPentagonCount);
	return 1;
}

/* True if the indices are equivalent. */
Boolean_tb Index_fbEquivalent(
	REGISTER Index_ts const rsIndexLeft[mReferenceConst(1)],
	REGISTER Index_ts const rsIndexRight[mReferenceConst(1)]
) {
	mStrongAssert(rsIndexLeft);
	mStrongAssert(rsIndexRight);

	return (
		rsIndexLeft->iuPentagonCount == rsIndexRight->iuPentagonCount &&
		rsIndexLeft->iuDirectionCount == rsIndexRight->iuDirectionCount &&
		0 == strcmp(rsIndexLeft->azcDirections, rsIndexRight->azcDirections)
	);
}

/* Returns the set of normalized child directions for the index. */
DirectionSet_tc Index_fcNormalizedChildDirections(
	REGISTER Index_ts const rsIndex[mReferenceConst(1)]
) {
	mStrongAssert(rsIndex);

	return fcNormalizedChildDirections(rsIndex->iuDirectionCount, rsIndex->azcDirections, rsIndex->iuPentagonCount);
}

/* For each index, calls fbAction({the index}, pData). */
Boolean_tb Index_fbIterate(
	REGISTER Boolean_tb const bDepthFirst,
	REGISTER Index_ts rsIndex[mReferenceConst(1)],
	REGISTER Size_tiu const iuDirectionCount,
	REGISTER Boolean_tb (* const fbAction)(Index_ts const rsIndex[mReference(1)], void * pData),
	REGISTER void * const pData
) {
	mStrongAssert(rsIndex);
	mStrongAssert(fbAction);

	/* If we want indexes shallower than the given index, return 0. */
	if (iuDirectionCount < rsIndex->iuDirectionCount) return 0;

	/* If the index is of the desired direction count, perform the action on it. */
	if (iuDirectionCount == rsIndex->iuDirectionCount) return fbAction(rsIndex, pData);

	{
		/* Get the set of child directions.  Do this before incrementing. */
		REGISTER DirectionSet_tc const cChildDirectionSet = fcNormalizedChildDirections(
			rsIndex->iuDirectionCount, rsIndex->azcDirections, rsIndex->iuPentagonCount
		);
		assert(DirectionSet_fbContains(cChildDirectionSet, Direction_i111));

		/* Increment the index. */
		if (Index_fbIncrement(rsIndex)) {
			/* Call on centroid and each vertex. */
			REGISTER Boolean_tb const bResult = bDepthFirst? (
				Index_fbIterate(bDepthFirst, rsIndex, iuDirectionCount, fbAction, pData) && 
				fbIterateVertices(bDepthFirst, rsIndex, cChildDirectionSet, iuDirectionCount, fbAction, pData)
			): (
				fbIterateVertices(bDepthFirst, rsIndex, cChildDirectionSet, iuDirectionCount, fbAction, pData) &&
				Index_fbIterate(bDepthFirst, rsIndex, iuDirectionCount, fbAction, pData)
			);
			
			/* Decrement to restore index. */
			mbVerify(Index_fbDecrement(rsIndex));
			
			/* Return the result. */
			return bResult;
		}
	}
	return 0;
}

/* Writes the index to the stream. */
Boolean_tb Index_fbWrite(
	REGISTER Index_ts const rsIndex[mReferenceConst(1)],
	REGISTER File_ts rsOutput[mReferenceConst(1)] /* Already open for writing */
) {
	mStrongAssert(rsIndex);
	assert(rsOutput);

	return fbWrite(rsIndex->iuDirectionCount, rsIndex->azcDirections, rsOutput);
}

/* Appends the string directions in the stream to the index (which is not initialized first), and returns the next character read. */
int Index_fiRead(
	REGISTER Index_ts rsIndex[mReferenceConst(1)],
	REGISTER File_ts rsInput[mReferenceConst(1)] /* Already open for reading. */
) {
	assert(rsInput);
	mStrongAssert(rsIndex);

	for (; ; ) {
		char unsigned cuChar = 0;

		/* If nothing could be read, return EOF. */	
		if (!File_fbRead(rsInput, &cuChar)) return EOF;

		/* If invalid character or EOF reached, we're done reading the index.  Return the character. */
		if (cuChar < '1' || cuChar > '7') return cuChar;

		/* Descend to centroid child and add the direction. */
		if (!Index_fbIncrement(rsIndex)) return cuChar; /* Too big for the index. */
		feAddDirection(rsIndex->iuDirectionCount, rsIndex->azcDirections, &(rsIndex->iuPentagonCount), cuChar - '0');
	}
}

/* Assumes random number generator has been properly seeded. */
void Index_fRandom(
	REGISTER Index_ts rsIndex[mReferenceConst(1)]
) {
	mStrongAssert(rsIndex);

	{
		/* Get the maximum count of new directions. */
		REGISTER Size_tiu iuExtraDirectionCount = Index__MAXIMUM_DIRECTION_COUNT - Index_fiuDirectionCount(rsIndex);

		/* Randomize. */
		if (1 < iuExtraDirectionCount) iuExtraDirectionCount = rand() % (iuExtraDirectionCount + 1);

		/* Loop and append random numbers. */
		for (; 0 < iuExtraDirectionCount; --iuExtraDirectionCount) {
			mbVerify(Index_fbIncrement(rsIndex));
			Index_feAddDirection(rsIndex, Direction_feRandom(1));
		}
	}
}

/* Tests the indexing code. */
Boolean_tb Index_fbTest(void) {
	printf("Performing index unit tests.\n");

	/* Test dynamic initialization overflow. */
	{
		Index_ts sIndex = Index_mInitializer(0);
		REGISTER Size_tiu iuDirectionCount = Index__MAXIMUM_DIRECTION_COUNT + 1;
		char azcDirections[Index__MAXIMUM_DIRECTION_COUNT + 2] = {0};

		/* Otherwise, this test won't work. */
		assert(Index__MAXIMUM_DIRECTION_COUNT + 1 < Size_miuMaximum());

		assert(Index__MAXIMUM_DIRECTION_COUNT + 1 == Size_miuArrayElementCount(sIndex.azcDirections));
		while (iuDirectionCount > 0) azcDirections[--iuDirectionCount] = 1;
		if (!Index_fbInitialize(&sIndex, 0, azcDirections + 1)) return mbVerify(0);
		if (Index_fbInitialize(&sIndex, 0, azcDirections)) return mbVerify(0);
	}

	/* Test increment past end. */
	{
		Index_ts sIndex = Index_mInitializer(0);

		assert(Index__MAXIMUM_DIRECTION_COUNT + 1 == Size_miuArrayElementCount(sIndex.azcDirections));
		while (sIndex.iuDirectionCount < Index__MAXIMUM_DIRECTION_COUNT) if (!Index_fbIncrement(&sIndex)) return mbVerify(0);
		assert(sIndex.iuDirectionCount == Index__MAXIMUM_DIRECTION_COUNT);
		if (Index_fbIncrement(&sIndex)) return mbVerify(0);
	}

	/* Test pentagon count change. */
	{
		{
			Index_ts sIndex = Index_mInitializer(0);
			mbVerify(Index_fbInitialize(&sIndex, 1, "\7\7\7\2\7\7\5\7"));
			if (Index_fiuPentagonCount(&sIndex) != 4) return mbVerify(0);

			{
				Index_ts sSum = Index_mInitializer(1);
				Index_ts sAddend = Index_mInitializer(0);
				mbVerify(Index_fbInitialize(&sAddend, 0, "\5\7\7\7\7"));
				mbVerify(Index_fbAdd(&sSum, &sIndex, &sAddend));
				if (Index_fiuPentagonCount(&sSum) != 7) return mbVerify(0);
			}
		}

		{
			Index_ts sIndex = Index_mInitializer(0);
			mbVerify(Index_fbInitialize(&sIndex, 1, "\7\1\7\4"));
			if (Index_fiuPentagonCount(&sIndex) != 4) return mbVerify(0);

			mbVerify(Index_fbDecrement(&sIndex));
			if (Index_fiuPentagonCount(&sIndex) != 4) return mbVerify(0);

			mbVerify(Index_fbDecrement(&sIndex));
			if (Index_fiuPentagonCount(&sIndex) != 3) return mbVerify(0);
		}

		{
			Index_ts sIndex = Index_mInitializer(0);
			mbVerify(Index_fbInitialize(&sIndex, 1, "\7\7\1\7"));
			if (Index_fiuPentagonCount(&sIndex) != 5) return mbVerify(0);

			Index_feAddDirection(&sIndex, 1);
			if (Index_fiuPentagonCount(&sIndex) != 4) return mbVerify(0);
		}

		{
			Index_ts sIndex = Index_mInitializer(0);
			mbVerify(Index_fbInitialize(&sIndex, 1, "\7\1"));
			if (Index_fiuPentagonCount(&sIndex) != 3) return mbVerify(0);

			mbVerify(Index_fbIncrement(&sIndex));
			if (Index_fiuPentagonCount(&sIndex) != 4) return mbVerify(0);

			mbVerify(Index_fbIncrement(&sIndex));
			if (Index_fiuPentagonCount(&sIndex) != 5) return mbVerify(0);

			{
				Direction_te eDirection = Index_feAddDirection(&sIndex, 1);
				if (Index_fiuPentagonCount(&sIndex) != 4) return mbVerify(0);
				Index_feAddDirection(&sIndex, Direction_feNegative(eDirection));
				if (Index_fiuPentagonCount(&sIndex) != 5) return mbVerify(0);
			}
		}

		/* After decrementing. */
		{
			Index_ts sIndex = Index_mInitializer(0);

			/* Initialize the index to a pentagon. */
			mbVerify(Index_fbInitialize(&sIndex, 1, "\7\1\7\7"));

			/* Verify the pentagon count. */
			if (sIndex.iuPentagonCount != 5) return mbVerify(0);

			/* Decrement, and verify the new pentagon count. */
			mbVerify(Index_fbDecrement(&sIndex));
			if (sIndex.iuPentagonCount != 4) return mbVerify(0);
		}
	}

	/* Test addition. */
	{
		/* Icosahedral. */
		{
			/* Resolution 0. */
			if (!fbTestAddition(1, "", "\1", "\5")) return mbVerify(0);
			if (!fbTestAddition(1, "", "\2", "\5")) return mbVerify(0);
			if (!fbTestAddition(1, "", "\3", "\5")) return mbVerify(0);
			if (!fbTestAddition(1, "", "\4", "\5")) return mbVerify(0);
			if (!fbTestAddition(1, "", "\5", "\5")) return mbVerify(0);
			if (!fbTestAddition(1, "", "\6", "\5")) return mbVerify(0);

			/* Resolution 1. */
			{
				/* From center. */
				if (!fbTestAddition(1, "\7", "\4", "\5")) return mbVerify(0);
				if (!fbTestAddition(1, "\7", "\6", "\5")) return mbVerify(0);
				if (!fbTestAddition(1, "\7", "\2", "\5")) return mbVerify(0);
				if (!fbTestAddition(1, "\7", "\3", "\5")) return mbVerify(0);
				if (!fbTestAddition(1, "\7", "\1", "\5")) return mbVerify(0);
				if (!fbTestAddition(1, "\7", "\5", "\5")) return mbVerify(0);

				/* From underlap. */
				if (!fbTestAddition(1, "\5", "\4", "\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\5", "\6", "\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\5", "\2", "\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\5", "\3", "\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\5", "\1", "\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\5", "\5", "\7")) return mbVerify(0);
			}

			/* Resolution 2. */
			{
				/* From south. */
				if (!fbTestAddition(1, "\5\7", "\3", "\7\3")) return mbVerify(0);
				if (!fbTestAddition(1, "\5\7", "\1", "\7\2")) return mbVerify(0);
				if (!fbTestAddition(1, "\5\7", "\5", "\7\6")) return mbVerify(0);
				if (!fbTestAddition(1, "\5\7", "\4", "\7\4")) return mbVerify(0);
				if (!fbTestAddition(1, "\5\7", "\6", "\7\1")) return mbVerify(0);
				if (!fbTestAddition(1, "\5\7", "\2", "\7\1")) return mbVerify(0);

				/* Straight out. */
				if (!fbTestAddition(1, "\7\3", "\3", "\5\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\2", "\2", "\5\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\6", "\6", "\5\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\4", "\4", "\5\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\1", "\1", "\5\7")) return mbVerify(0);

				/* Adjacent out. */
				if (!fbTestAddition(1, "\7\1", "\5", "\5\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\1", "\3", "\5\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\3", "\1", "\5\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\3", "\2", "\5\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\2", "\3", "\5\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\2", "\6", "\5\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\6", "\2", "\5\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\6", "\4", "\5\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\4", "\6", "\5\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\4", "\5", "\5\7")) return mbVerify(0);
			}

			/* Resolution 3. */
			{
				/* To center. */
				if (!fbTestAddition(1, "\7\7\2\7\5", "\5", "\7\7\7\7\2")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\7\3\7\4", "\4", "\7\7\7\7\3")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\7\1\7\6", "\6", "\7\7\7\7\1")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\7\4\7\3", "\3", "\7\7\7\7\4")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\7\6\7\1", "\1", "\7\7\7\7\6")) return mbVerify(0);

				/* From center. */
				{
					/* From 777. */
					if (!fbTestAddition(1, "\7\7\7\7\4", "\4", "\7\7\4\7\3")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\7\7\6", "\6", "\7\7\6\7\1")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\7\7\2", "\2", "\7\7\2\7\5")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\7\7\3", "\3", "\7\7\3\7\4")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\7\7\1", "\1", "\7\7\1\7\6")) return mbVerify(0);

					/* From 7p7. */
					{
						/* 717. */
						if (!fbTestAddition(1, "\7\1\7\7\4", "\4", "\7\4\7\7\5")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\1\7\7\6", "\6", "\7\7\4\7\5")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\1\7\7\2", "\2", "\7\7\1\7\5")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\1\7\7\3", "\3", "\7\3\7\7\5")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\1\7\7\5", "\5", "\5\7\7\7\3")) return mbVerify(0);

						/* 737. */
						if (!fbTestAddition(1, "\7\3\7\7\4", "\4", "\7\7\1\7\3")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\3\7\7\6", "\6", "\7\7\3\7\1")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\3\7\7\2", "\2", "\7\2\7\7\1")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\3\7\7\1", "\1", "\5\7\7\7\1")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\3\7\7\5", "\5", "\7\1\7\7\3")) return mbVerify(0);

						/* 727. */
						if (!fbTestAddition(1, "\7\2\7\7\4", "\4", "\7\7\2\7\3")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\2\7\7\6", "\6", "\7\6\7\7\3")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\2\7\7\3", "\3", "\5\7\7\7\5")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\2\7\7\1", "\1", "\7\3\7\7\2")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\2\7\7\5", "\5", "\7\7\3\7\2")) return mbVerify(0);

						/* 767. */
						if (!fbTestAddition(1, "\7\6\7\7\4", "\4", "\7\4\7\7\2")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\6\7\7\2", "\2", "\5\7\7\7\4")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\6\7\7\3", "\3", "\7\2\7\7\6")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\6\7\7\1", "\1", "\7\7\2\7\6")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\6\7\7\5", "\5", "\7\7\6\7\2")) return mbVerify(0);

						/* 747. */
						if (!fbTestAddition(1, "\7\4\7\7\6", "\6", "\5\7\7\7\6")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\4\7\7\2", "\2", "\7\6\7\7\4")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\4\7\7\3", "\3", "\7\7\6\7\4")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\4\7\7\1", "\1", "\7\7\4\7\6")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\4\7\7\5", "\5", "\7\1\7\7\4")) return mbVerify(0);
					}

					/* From 577. */
					if (!fbTestAddition(1, "\5\7\7\7\4", "\4", "\7\6\7\7\2")) return mbVerify(0);
					if (!fbTestAddition(1, "\5\7\7\7\6", "\6", "\7\4\7\7\6")) return mbVerify(0);
					if (!fbTestAddition(1, "\5\7\7\7\3", "\3", "\7\1\7\7\5")) return mbVerify(0);
					if (!fbTestAddition(1, "\5\7\7\7\1", "\1", "\7\3\7\7\1")) return mbVerify(0);
					if (!fbTestAddition(1, "\5\7\7\7\5", "\5", "\7\2\7\7\3")) return mbVerify(0);
				}

				/* Adjacent in. */
				{
					/* Clockwise. */
					if (!fbTestAddition(1, "\7\7\1\7\2", "\2", "\7\7\3\7\5")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\3\7\6", "\6", "\7\7\2\7\1")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\2\7\4", "\4", "\7\7\6\7\3")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\6\7\5", "\5", "\7\7\4\7\2")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\4\7\1", "\1", "\7\7\1\7\4")) return mbVerify(0);

					/* Counter-clockwise. */
					if (!fbTestAddition(1, "\7\7\4\7\2", "\2", "\7\7\6\7\5")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\6\7\3", "\3", "\7\7\2\7\4")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\2\7\1", "\1", "\7\7\3\7\6")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\3\7\5", "\5", "\7\7\1\7\2")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\1\7\4", "\4", "\7\7\4\7\1")) return mbVerify(0);
				}

				/* Adjacent out. */
				if (!fbTestAddition(1, "\7\7\1\7\3", "\3", "\7\3\7\7\4")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\7\3\7\1", "\1", "\7\3\7\7\6")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\7\3\7\2", "\2", "\7\2\7\7\5")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\7\2\7\3", "\3", "\7\2\7\7\4")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\7\2\7\6", "\6", "\7\6\7\7\1")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\7\6\7\2", "\2", "\7\6\7\7\5")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\7\6\7\4", "\4", "\7\4\7\7\3")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\7\4\7\6", "\6", "\7\4\7\7\1")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\7\4\7\5", "\5", "\7\1\7\7\6")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\7\1\7\5", "\5", "\7\1\7\7\2")) return mbVerify(0);

				/* Straight out (to test local underlap correction). */
				if (!fbTestAddition(1, "\7\7\1", "\1", "\7\1\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\7\3", "\3", "\7\3\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\7\2", "\2", "\7\2\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\7\6", "\6", "\7\6\7")) return mbVerify(0);
				if (!fbTestAddition(1, "\7\7\4", "\4", "\7\4\7")) return mbVerify(0);
			}

			/* Resolution 4. */
			{
				/* From center. */
				{
					/* Normal. */
					if (!fbTestAddition(1, "\7\7\7\7\7\4", "\4", "\7\7\7\4\7\3")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\7\7\7\6", "\6", "\7\7\7\6\7\1")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\7\7\7\2", "\2", "\7\7\7\2\7\5")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\7\7\7\3", "\3", "\7\7\7\3\7\4")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\7\7\7\1", "\1", "\7\7\7\1\7\6")) return mbVerify(0);

					/* Into underlap. */
					if (!fbTestAddition(1, "\7\7\7\7", "\5", "\7\7\7\1")) return mbVerify(0);
					if (!fbTestAddition(1, "\5\7\7\7", "\2", "\5\7\7\6")) return mbVerify(0);
					
					/* Vertex centroid. */
					if (!fbTestAddition(1, "\7\7\4\7\7\3", "\3", "\7\7\7\1\7\5")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\4\7\7\1", "\1", "\7\1\7\6\7\4")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\4\7\7\5", "\5", "\7\1\7\4\7\6")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\4\7", "\4", "\7\1\7\4")) return mbVerify(0);

					/* Into south hemisphere longitudinal underlap. */
					{
						/* To the side. */
						if (!fbTestAddition(1, "\7\1\7\7\7\2", "\2", "\7\3\7\5\7\1")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\3\7\7\7\6", "\6", "\7\2\7\1\7\3")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\2\7\7\7\4", "\4", "\7\6\7\3\7\2")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\6\7\7\7\5", "\5", "\7\4\7\2\7\6")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\4\7\7\7\1", "\1", "\7\1\7\4\7\5")) return mbVerify(0);

						/* To south (overlap side). */
						if (!fbTestAddition(1, "\7\3\7\7\7\1", "\1", "\5\7\7\3\7\1")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\2\7\7\7\3", "\3", "\5\7\7\1\7\5")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\6\7\7\7\2", "\2", "\5\7\7\5\7\4")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\4\7\7\7\6", "\6", "\5\7\7\4\7\6")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\1\7\7\7\5", "\5", "\5\7\7\6\7\2")) return mbVerify(0);

						/* To south (underlap side). */
						if (!fbTestAddition(1, "\7\1\7\7\7\3", "\3", "\5\7\7\3\7\2")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\3\7\7\7\2", "\2", "\5\7\7\1\7\3")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\2\7\7\7\6", "\6", "\5\7\7\5\7\1")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\6\7\7\7\4", "\4", "\5\7\7\4\7\5")) return mbVerify(0);
						if (!fbTestAddition(1, "\7\4\7\7\7\5", "\5", "\5\7\7\6\7\4")) return mbVerify(0);
					}
				}

				/* Adjacent in. */
				{
					/* Normal. */
					if (!fbTestAddition(1, "\7\7\7\2\7\4", "\4", "\7\7\7\6\7\3")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\7\4\7\2", "\2", "\7\7\7\6\7\5")) return mbVerify(0);

					/* In 5 direction. */
					if (!fbTestAddition(1, "\7\7\7\3\7\5", "\5", "\7\7\7\1\7\2")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\7\6\7\5", "\5", "\7\7\7\4\7\2")) return mbVerify(0);

					/* Into 7775. */
					if (!fbTestAddition(1, "\7\7\7\1\7\4", "\4", "\7\7\7\4\7\1")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\7\4\7\1", "\1", "\7\7\7\1\7\4")) return mbVerify(0);

					/* Across. */
					if (!fbTestAddition(1, "\7\1\7\6\7\3", "\3", "\7\3\7\5\7\5")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\3\7\4\7\2", "\2", "\7\2\7\1\7\1")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\2\7\5\7\6", "\6", "\7\6\7\3\7\3")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\6\7\1\7\4", "\4", "\7\4\7\2\7\2")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\4\7\3\7\5", "\5", "\7\1\7\4\7\4")) return mbVerify(0);

					/* To south. */
					if (!fbTestAddition(1, "\7\1\7\4\7\1", "\1", "\5\7\7\6\7\6")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\3\7\5\7\3", "\3", "\5\7\7\3\7\3")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\2\7\1\7\2", "\2", "\5\7\7\1\7\1")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\6\7\3\7\6", "\6", "\5\7\7\5\7\5")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\4\7\2\7\4", "\4", "\5\7\7\4\7\4")) return mbVerify(0);
				}
				
				/* Adjacent out. */
				{
					/* Normal. */
					if (!fbTestAddition(1, "\7\7\7\2\7\3", "\3", "\7\7\3\7\7\4")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\7\3\7\2", "\2", "\7\7\3\7\7\5")) return mbVerify(0);

					/* With gap, not into gap. */
					if (!fbTestAddition(1, "\7\7\7\4\7\5", "\5", "\7\7\4\7\7\2")) return mbVerify(0);
					
					/* With gap, into gap. */
					if (!fbTestAddition(1, "\7\7\7\1\7\5", "\5", "\7\7\4\7\7\3")) return mbVerify(0);

					/* Into 7757 underlap, from the side. */
					if (!fbTestAddition(1, "\7\1\7\4\7\6", "\6", "\7\7\4\7\7\5")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\1\7\6\7\4", "\4", "\7\7\4\7\7\1")) return mbVerify(0);

					/* North to 77p7. */
					if (!fbTestAddition(1, "\7\1\7\6\7\2", "\2", "\7\7\1\7\7\5")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\3\7\4\7\6", "\6", "\7\7\3\7\7\1")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\2\7\5\7\4", "\4", "\7\7\2\7\7\3")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\6\7\1\7\5", "\5", "\7\7\6\7\7\2")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\4\7\3\7\1", "\1", "\7\7\4\7\7\6")) return mbVerify(0);

					/* Into south hemisphere longitudinal underlap. */
					if (!fbTestAddition(1, "\7\1\7\4\7\5", "\5", "\7\4\7\7\7\1")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\3\7\5\7\1", "\1", "\7\1\7\7\7\2")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\2\7\1\7\3", "\3", "\7\3\7\7\7\6")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\6\7\3\7\2", "\2", "\7\2\7\7\7\4")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\4\7\2\7\6", "\6", "\7\6\7\7\7\5")) return mbVerify(0);

					/* From south. */
					{
						/* One way. */
						if (!fbTestAddition(1, "\5\7\7\3\7\2", "\2", "\7\1\7\7\7\3")) return mbVerify(0);
						if (!fbTestAddition(1, "\5\7\7\1\7\3", "\3", "\7\3\7\7\7\2")) return mbVerify(0);
						if (!fbTestAddition(1, "\5\7\7\5\7\1", "\1", "\7\2\7\7\7\6")) return mbVerify(0);
						if (!fbTestAddition(1, "\5\7\7\4\7\5", "\5", "\7\6\7\7\7\4")) return mbVerify(0);
						if (!fbTestAddition(1, "\5\7\7\6\7\4", "\4", "\7\4\7\7\7\5")) return mbVerify(0);

						/* The other way. */
						if (!fbTestAddition(1, "\5\7\7\3\7\1", "\1", "\7\3\7\7\7\1")) return mbVerify(0);
						if (!fbTestAddition(1, "\5\7\7\1\7\5", "\5", "\7\2\7\7\7\3")) return mbVerify(0);
						if (!fbTestAddition(1, "\5\7\7\5\7\4", "\4", "\7\6\7\7\7\2")) return mbVerify(0);
						if (!fbTestAddition(1, "\5\7\7\4\7\6", "\6", "\7\4\7\7\7\6")) return mbVerify(0);
						if (!fbTestAddition(1, "\5\7\7\6\7\2", "\2", "\7\1\7\7\7\5")) return mbVerify(0);
					}
				}
				
				/* Straight out. */
				{
					/* Out from north. */
					if (!fbTestAddition(1, "\7\7\7\1\7\1", "\1", "\7\1\7\6\7\6")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\7\3\7\3", "\3", "\7\3\7\4\7\4")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\7\2\7\2", "\2", "\7\2\7\5\7\5")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\7\6\7\6", "\6", "\7\6\7\1\7\1")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\7\7\4\7\4", "\4", "\7\4\7\3\7\3")) return mbVerify(0);

					/* In to north. */
					if (!fbTestAddition(1, "\7\1\7\6\7\6", "\6", "\7\7\7\1\7\1")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\3\7\4\7\4", "\4", "\7\7\7\3\7\3")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\2\7\5\7\5", "\5", "\7\7\7\2\7\2")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\6\7\1\7\1", "\1", "\7\7\7\6\7\6")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\4\7\3\7\3", "\3", "\7\7\7\4\7\4")) return mbVerify(0);

					/* Across longitudinal underlap. */
					if (!fbTestAddition(1, "\7\3\7\5\7\5", "\5", "\7\1\7\6\7\3")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\2\7\1\7\1", "\1", "\7\3\7\4\7\2")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\6\7\3\7\3", "\3", "\7\2\7\5\7\6")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\4\7\2\7\2", "\2", "\7\6\7\1\7\4")) return mbVerify(0);
					if (!fbTestAddition(1, "\7\1\7\4\7\4", "\4", "\7\4\7\3\7\5")) return mbVerify(0);

					/* From south. */
					if (!fbTestAddition(1, "\5\7\7\3\7\3", "\3", "\7\3\7\5\7\3")) return mbVerify(0);
					if (!fbTestAddition(1, "\5\7\7\1\7\1", "\1", "\7\2\7\1\7\2")) return mbVerify(0);
					if (!fbTestAddition(1, "\5\7\7\5\7\5", "\5", "\7\6\7\3\7\6")) return mbVerify(0);
					if (!fbTestAddition(1, "\5\7\7\4\7\4", "\4", "\7\4\7\2\7\4")) return mbVerify(0);
					if (!fbTestAddition(1, "\5\7\7\6\7\6", "\6", "\7\1\7\4\7\1")) return mbVerify(0);
				}
			}
			
			/* TODO: Add tests for multi-direction additions. */
		}

		/* Hexagonal. */
		{
			/* Resolution 0. */
			if (!fbTestAddition(0, "", "\1", "\1")) return mbVerify(0);
			if (!fbTestAddition(0, "", "\2", "\2")) return mbVerify(0);
			if (!fbTestAddition(0, "", "\3", "\3")) return mbVerify(0);
			if (!fbTestAddition(0, "", "\4", "\4")) return mbVerify(0);
			if (!fbTestAddition(0, "", "\5", "\5")) return mbVerify(0);
			if (!fbTestAddition(0, "", "\6", "\6")) return mbVerify(0);

			/* From center, and straight out. */
			if (!fbTestAddition(0, "\7\7\4", "\4", "\4\7\3")) return mbVerify(0);
			if (!fbTestAddition(0, "\7\7\6", "\6", "\6\7\1")) return mbVerify(0);
			if (!fbTestAddition(0, "\7\7\2", "\2", "\2\7\5")) return mbVerify(0);
			if (!fbTestAddition(0, "\7\7\3", "\3", "\3\7\4")) return mbVerify(0);
			if (!fbTestAddition(0, "\7\7\1", "\1", "\1\7\6")) return mbVerify(0);
			if (!fbTestAddition(0, "\7\7\5", "\5", "\5\7\2")) return mbVerify(0);

			/* To center. */
			if (!fbTestAddition(0, "\7\7\4", "\3", "\7\7\7")) return mbVerify(0);
			if (!fbTestAddition(0, "\7\7\6", "\1", "\7\7\7")) return mbVerify(0);
			if (!fbTestAddition(0, "\7\7\2", "\5", "\7\7\7")) return mbVerify(0);
			if (!fbTestAddition(0, "\7\7\3", "\4", "\7\7\7")) return mbVerify(0);
			if (!fbTestAddition(0, "\7\7\1", "\6", "\7\7\7")) return mbVerify(0);
			if (!fbTestAddition(0, "\7\7\5", "\2", "\7\7\7")) return mbVerify(0);

			/* Adjacent in. */
			{
				/* One way. */
				if (!fbTestAddition(0, "\4\7\2", "\2", "\6\7\5")) return mbVerify(0);
				if (!fbTestAddition(0, "\6\7\3", "\3", "\2\7\4")) return mbVerify(0);
				if (!fbTestAddition(0, "\2\7\1", "\1", "\3\7\6")) return mbVerify(0);
				if (!fbTestAddition(0, "\3\7\5", "\5", "\1\7\2")) return mbVerify(0);
				if (!fbTestAddition(0, "\1\7\4", "\4", "\5\7\3")) return mbVerify(0);
				if (!fbTestAddition(0, "\5\7\6", "\6", "\4\7\1")) return mbVerify(0);

				/* The other way. */
				if (!fbTestAddition(0, "\4\7\1", "\1", "\5\7\6")) return mbVerify(0);
				if (!fbTestAddition(0, "\5\7\3", "\3", "\1\7\4")) return mbVerify(0);
				if (!fbTestAddition(0, "\1\7\2", "\2", "\3\7\5")) return mbVerify(0);
				if (!fbTestAddition(0, "\3\7\6", "\6", "\2\7\1")) return mbVerify(0);
				if (!fbTestAddition(0, "\2\7\4", "\4", "\6\7\3")) return mbVerify(0);
				if (!fbTestAddition(0, "\6\7\5", "\5", "\4\7\2")) return mbVerify(0);
			}

			/* Adjacent out (one lattice). */
			{
				/* One way. */
				/* TODO: Add this test. */

				/* The other way. */
				/* TODO: Add this test. */
			}

			/* Adjacent out (other lattice). */
			{
				/* One way. */
				/* TODO: Add this test. */

				/* The other way. */
				/* TODO: Add this test. */
			}

			/* TODO: Add test for every case that the hexagonal would differ from the icosahedral. */

			/* TODO: Add tests for multi-direction additions. */
		}
	}

	/* Test iteration and connectivity. */
	{
		Index_ts const sIndex = Index_mInitializer(1);
		REGISTER Size_tiu iuDirectionCount = 0;

		for (; iuDirectionCount <= 15; ++iuDirectionCount) {
			REGISTER clock_t const tClock = clock();

			Index_ts sIndexCopy = sIndex;
			printf("Testing connectivity of all resolution %" Size_mrzcFormatter() " cells.\n", iuDirectionCount);
			mbVerify(Index_fbIterate(1, &sIndexCopy, iuDirectionCount, fbIterationTestAction, 0));
			if (!Index_fbEquivalent(&sIndex, &sIndexCopy)) return mbVerify(0);

			{
				REGISTER double const dSeconds = (double)(clock() - tClock) / CLOCKS_PER_SEC;
				printf("%g seconds.\n", dSeconds);
			}
		}
	}

	# if defined(NDEBUG)
		/* Test performance. */
		printf("Random addition of directions.\n"); 
		{
			REGISTER int unsigned const iuDirectionCount = 39;

			# if defined(PROFILE)
				REGISTER int unsigned const iuPanCount = 100000;
			# else
				REGISTER int unsigned const iuPanCount = 10000000;
			# endif

			/* Icosahedral. */
			printf("Icosahedral...\n");
			{
				Index_ts sIndex = Index_mInitializer(1);
				REGISTER int unsigned iuDirectionIndex = 0;
				for (; iuDirectionIndex <= iuDirectionCount; ++iuDirectionIndex) {
					printf("Timing %u random addition of directions at resolution %u:\n", iuPanCount, iuDirectionIndex); 

					printf("%g seconds.\n", fdSecondsToAddRandomDirections(&sIndex, iuPanCount));
					mbVerify(Index_fbIncrement(&sIndex));
				}
			}

			/* Hexagonal. */
			printf("Hexagonal...\n");
			{
				Index_ts sIndex = Index_mInitializer(0);
				REGISTER int unsigned iuDirectionIndex = 0;
				for (; iuDirectionIndex < iuDirectionCount; ++iuDirectionIndex) mbVerify(Index_fbIncrement(&sIndex));

				printf("%g seconds.\n", fdSecondsToAddRandomDirections(&sIndex, iuPanCount));
			}
		}
	# endif

	# if !defined(PROFILE)
		/* Test icosahedral addition interactively. */
		printf(
			"Icosahedral interactive testing. Type (without quotes):\n"
			"> '1' to '7' to move down to child;\n"
			"> '0' to decimate;\n"
			"> '+' to add a hexagonal index (to be entered) to the running total;\n"
			"> 'return' to conclude an addition and display an index;\n"
			"> any other character to terminate execution.\n"
		);
		
		{
			File_ts sInput = File_mInitializer();
			File_ts sOutput = File_mInitializer();
			mbVerify(File_fbInput(&sInput));
			mbVerify(File_fbOutput(&sOutput));
			if (!fbParse(&sInput, &sOutput, 1)) return 0;
		}
	# endif

	return 1;
}
