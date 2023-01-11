# include "Core/Headers/flexible_struct.h"
# include "../Headers/tile.h"
# include "tile_tree.h"
# include <limits.h>

/* Conventions in this file:
-	"Node directions" are directions that have been collapsed for storage in a node (i.e. v7 -> v).
	Directions not specified as "node directions" have not been collapsed.
*/

enum {iuChildCount = 7};

struct TileTree_ts {
	/* Child nodes.  Subscript = direction - 1. */
	TileTree_ts * apsNodes[iuChildCount];

	/* A null-terminated string, containing node directions which are collapsed: v7 -> v, where v is a vertex. */
	DirectionString_tzc FlexibleStruct_mDeclaration();
};

/* Returns 1 if all children are null. */
static INLINE Boolean_tb fbLeaf(
	REGISTER TileTree_ts const * const rsNode
) {
	mStrongAssert(rsNode);

	{
		REGISTER TileTree_ts * const * rpsChild = rsNode->apsNodes;
		REGISTER TileTree_ts * const * const rpsChildPastEnd = rpsChild + iuChildCount; /* Safe to point one past end. */
		for (; rpsChild < rpsChildPastEnd; ++rpsChild) if (*rpsChild) return 0;
	}
	return 1;
}

/* Returns 1 if the node is full. */
static INLINE Boolean_tb fbFull(
	REGISTER TileTree_ts const * const rsNode
) {
	mStrongAssert(rsNode);

	return (0 == *FlexibleStruct_mrMember(*rsNode) && fbLeaf(rsNode));
}

/* Move all child node pointers from one array to the other. */
static void fMoveChildNodes(
	REGISTER TileTree_ts * rpsSourceChildNodes[mReference(iuChildCount)],
	REGISTER TileTree_ts * rpsDestinationChildNodes[mReference(iuChildCount)]
) {
	mStrongAssert(rpsSourceChildNodes);
	mStrongAssert(rpsDestinationChildNodes);

	{
		REGISTER int unsigned iuChildOffset = 0;
		for (; iuChildOffset < iuChildCount; ++iuChildOffset) {
			/* Each line below dereferences the pointer, does the assignment, then increments the pointer. */
			*rpsDestinationChildNodes++ = *rpsSourceChildNodes;
			*rpsSourceChildNodes++ = 0;
		}
	}
}

/* Returns the child array offset corresponding to a child direction. */
static int unsigned fiuChildOffset(
	REGISTER Direction_te const eChildDirection
) {
	mStrongAssert(Direction_fbValid(eChildDirection));
	
	return eChildDirection - 1;
}

/* Deallocates the child nodes and sets each pointer to null. */
static void fDeallocateAndZeroChildNodes(
	REGISTER TileTree_ts * rrsNode[mReference(1)]
) {
	mStrongAssert(rrsNode && *rrsNode);

	rrsNode = (*rrsNode)->apsNodes;
	{
		REGISTER TileTree_ts * const * const rpsChildPastEnd = rrsNode + iuChildCount; /* Safe to point one past end. */
		for (; rrsNode < rpsChildPastEnd; ++rrsNode) if (*rrsNode) TileTree_fEmpty(rrsNode);
	}
}

/* Resizes the direction array of the node to the specified size.  If node is null, allocates.
Sets the terminator, but leaves any new bytes preceding it uninitialized.
*/
static Boolean_tb fbReallocate(
	REGISTER TileTree_ts * rpsNode[mReferenceConst(1)],
	REGISTER Size_tiu const iuNodeDirectionCount /* Number of node directions the node should hold. */
) {
	mStrongAssert(rpsNode);

	/* Reallocate the node.  Include space for the null terminator. */
	{
		/* Determine whether we will need to initialize afterward. */
		REGISTER Boolean_tb const bInitialize = (0 == *rpsNode);

		/* Reallocate. Cache the allocation size so that we don't pass an expression to the macro. */
		{
			REGISTER Size_tiu const iuCharCount = iuNodeDirectionCount + 1;
			if (!FlexibleStruct_mbReallocate(*rpsNode, TileTree_ts, iuCharCount)) return 0;
		}
		mStrongAssert(*rpsNode);
		
		/* If the node was null, we need to initialize child pointers to null. */
		if (bInitialize) {
			REGISTER TileTree_ts * * rpsChild = (*rpsNode)->apsNodes;
			REGISTER TileTree_ts * const * const rpsChildPastEnd = rpsChild + iuChildCount; /* Safe to point one past end. */
			for (; rpsChild < rpsChildPastEnd; ++rpsChild) *rpsChild = 0;
		}
	}
	mStrongAssert(*rpsNode);

	/* Set the null terminator and return. */
	FlexibleStruct_mrMember(**rpsNode)[iuNodeDirectionCount] = 0;
	return 1;
}

/* Appends the directions to the node. */
static Boolean_tb fbAppendNodeDirections(
	REGISTER TileTree_ts * rpsNode[mReferenceConst(1)],
	REGISTER DirectionString_tzc const rzcNodeDirections[mReference(1)]
) {
	mStrongAssert(rpsNode);
	mStrongAssert(rzcNodeDirections);

	{
		/* Get the existing node direction count, which will be used to prevent overwriting everything. */
		REGISTER Size_tiu const iuNodeDirectionCount = *rpsNode? DirectionString_fiuCount(FlexibleStruct_mrMember(**rpsNode)): 0;

		/* The number of node directions to append. */
		REGISTER Size_tiu const iuExtraNodeDirectionCount = DirectionString_fiuCount(rzcNodeDirections);

		/* Allocate/reallocate the node accordingly. */
		if (!fbReallocate(rpsNode, iuNodeDirectionCount + iuExtraNodeDirectionCount)) return 0;

		/* Copy the array. */
		DirectionString_fCopySection(
			FlexibleStruct_mrMember(**rpsNode) + iuNodeDirectionCount,
			rzcNodeDirections,
			iuExtraNodeDirectionCount
		);
	}
	return 1;
}

/* Advance to the next node direction, skipping vertex centroid children. */
static DirectionString_tzc const * frzcNextNodeDirection(
	REGISTER DirectionString_tzc const rzcDirections[mReference(1)] /* Non-empty normalized index directions. */
) {
	mStrongAssert(rzcDirections && *rzcDirections);

	if (
		Direction_i111 != DirectionString_fiDirection(rzcDirections) &&
		0 == *++rzcDirections /* Advance past vertex */
	) return rzcDirections;
	assert(Direction_i111 == DirectionString_fiDirection(rzcDirections) /* Normalized as index */);
	return ++rzcDirections; /* Advance past centroid. */
}

/* Collapse the tile directions to node directions, and append. */
static Boolean_tb fbAppendDirections(
	REGISTER TileTree_ts * rpsNode[mReferenceConst(1)],
	REGISTER DirectionString_tzc const rzcDirections[mReference(1)] /* Normalized index directions. */
) {
	mStrongAssert(rpsNode);
	mStrongAssert(rzcDirections);

	{
		/* Get the existing node direction count, which will be used to prevent overwriting everything. */
		REGISTER Size_tiu const iuNodeDirectionCount = *rpsNode? DirectionString_fiuCount(FlexibleStruct_mrMember(**rpsNode)): 0;

		/* The node directions to append. */
		DirectionString_tzc azcNodeDirections[Index__MAXIMUM_DIRECTION_COUNT + 1] = {0};
		
		/* Populate the node direction array, and count the nodes. */
		REGISTER Size_tiu iuExtraNodeDirectionCount = 0;
		for (; *rzcDirections; ++iuExtraNodeDirectionCount) {
			if (Index__MAXIMUM_DIRECTION_COUNT < iuExtraNodeDirectionCount) return 0; /* Too many directions. */
			azcNodeDirections[iuExtraNodeDirectionCount] = *rzcDirections;
			rzcDirections = frzcNextNodeDirection(rzcDirections);
		}

		/* Allocate/reallocate the node accordingly. */
		if (!fbReallocate(rpsNode, iuNodeDirectionCount + iuExtraNodeDirectionCount)) return 0;

		/* Copy the array. */
		DirectionString_fCopySection(
			FlexibleStruct_mrMember(**rpsNode) + iuNodeDirectionCount,
			azcNodeDirections,
			iuExtraNodeDirectionCount
		);
	}
	return 1;
}

/* Truncates the node. */
static Boolean_tb fbTruncate(
	REGISTER TileTree_ts * rpsNode[mReferenceConst(1)],
	REGISTER Size_tiu const iuNodeDirectionCount
) {
	mStrongAssert(rpsNode);

	if (!fbReallocate(rpsNode, iuNodeDirectionCount)) return 0;
	fDeallocateAndZeroChildNodes(rpsNode);
	return 1;
}

/* Splits the node.
The direction count is the number of directions to keep on this node.
If nothing to split off, an assertion is thrown.
Returns the new child node, or 0 if failed.
*/
static TileTree_ts * fpsSplit(
	REGISTER TileTree_ts * rrsNode[mReferenceConst(1)],
	REGISTER Size_tiu const iuNodeDirectionCount
) {
	mStrongAssert(rrsNode && *rrsNode);
	mStrongAssert(DirectionString_fiuCount(FlexibleStruct_mrMember(**rrsNode)) > iuNodeDirectionCount); /* Need something to split off. */

	{
		/* Get a pointer to the end chunk of the direction array that will be dropped. */
		REGISTER DirectionString_tzc const * const rzcNodeDirectionsToDrop = FlexibleStruct_mrMember(**rrsNode) + iuNodeDirectionCount;
		mStrongAssert(*rzcNodeDirectionsToDrop);
		{
			/* Grab the direction that will be used as the child offset. */
			REGISTER int unsigned const iuChildOffset = fiuChildOffset(*rzcNodeDirectionsToDrop);
			mStrongAssert(iuChildOffset < iuChildCount);
			{
				/* Allocate the child. */
				TileTree_ts * psChild = 0;
				if (!fbAppendNodeDirections(&psChild, rzcNodeDirectionsToDrop + 1 /* Exclude the child offset bit */)) return 0;
				mStrongAssert(psChild);

				/* Reallocate this node to be smaller. */
				if (!fbReallocate(rrsNode, iuNodeDirectionCount)) {
					/* Clean up and return failure. */
					TileTree_fEmpty(&psChild);
					return 0;
				}
				mStrongAssert(*rrsNode);

				/* Move the children of this node into the child's child array, set the child, and return. */
				fMoveChildNodes((*rrsNode)->apsNodes, psChild->apsNodes);
				return (*rrsNode)->apsNodes[iuChildOffset] = psChild;
			}
		}
	}
}

enum {iTerminatorOffset = 7};

static char unsigned fcuSetTerminator(
	REGISTER DirectionSet_tc const cDirectionSet
) {
	return Bit_miOn(cDirectionSet, iTerminatorOffset);
}

static DirectionSet_tc fcClearTerminator(
	REGISTER char unsigned cuByte
) {
	cuByte = (char unsigned const)Bit_miOff(cuByte, iTerminatorOffset);
	assert(cuByte <= SCHAR_MAX);
	return (DirectionSet_tc const)cuByte;
}

static Boolean_tb fbTerminator(
	REGISTER char unsigned const cuByte
) {
	return Bit_mbOn(cuByte, iTerminatorOffset);
}

/* The caller must make *absolutely sure* that the read buffer is the required size. */
static Boolean_tb fbLoad(
	REGISTER TileTree_ts * rpsNode[mReferenceConst(1)],
	REGISTER File_ts rsInput[mReferenceConst(1)], /* Binary stream open for reading. */
	REGISTER DirectionString_tzc rzcNodeDirections[mReferenceConst(Index__MAXIMUM_DIRECTION_COUNT + 1)] /* The read buffer. */
) {
	mStrongAssert(rpsNode);
	mStrongAssert(rsInput);
	mStrongAssert(FileMode_fbBinary(File_feMode(rsInput)));
	mStrongAssert(rzcNodeDirections);

	*rpsNode = 0; /* Initialize the out argument. */
	{
		/* The number of node directions in the array. */
		REGISTER Size_tiu iuNodeDirectionCount = 0;

		/* Initialize the read buffer by setting the null terminator. */
		*rzcNodeDirections = 0;

		/* A node cannot be bigger than an index. */
		while (iuNodeDirectionCount <= Index__MAXIMUM_DIRECTION_COUNT) {
			/* Read the next unsigned char. */
			char unsigned cuByte = 0;
			if (!File_fbRead(rsInput, &cuByte)) return 0;

			/* If the byte is null, then something is incorrect in the stream.  This is a trap representation. */
			if (0 == cuByte) return 0;

			{
				/* Determine whether the byte has the terminator set. */
				REGISTER Boolean_tb const bTerminator = fbTerminator(cuByte);

				/* Get the direction set. */
				REGISTER DirectionSet_tc const cDirectionSet = fcClearTerminator(cuByte);

				if (!DirectionSet_fbSingleton(cDirectionSet)) {
					if (!DirectionSet_fbEmpty(cDirectionSet)) {
						/* Create the node. */
						if (!fbAppendNodeDirections(rpsNode, rzcNodeDirections)) return 0;
						mStrongAssert(*rpsNode);

						/* If it's not a leaf... */
						if (!DirectionSet_fbFull(cDirectionSet)) {
							REGISTER TileTree_ts * * rpsChild = (*rpsNode)->apsNodes;
							REGISTER Direction_te eChildDirection = Direction_i001;

							/* If the byte has the terminator, this is an optimization to indicate that each child is full. */
							if (bTerminator) {
								/* Create full children (i.e. with no nodes or children). */
								for (; ; ++eChildDirection, ++rpsChild) {
									if (DirectionSet_fbContains(cDirectionSet, eChildDirection)) {
										if (!fbReallocate(rpsChild, 0)) return 0;
										assert(*rpsChild); /* The child pointer shouldn't be null. */
									}
									if (Direction_i111 == eChildDirection) break;
								}
							} else {
								/* Load each child. */
								for (; ; ++eChildDirection, ++rpsChild) {
									if (DirectionSet_fbContains(cDirectionSet, eChildDirection)) {
										if (!fbLoad(rpsChild, rsInput, rzcNodeDirections)) return 0;
										assert(*rpsChild); /* The child pointer shouldn't be null. */
									}
									if (Direction_i111 == eChildDirection) break;
								}
							}
						}
					}

					/* Now that we've processed the children, we're done. */
					return 1;
				}

				{					
					/* Get the single direction from the set. */
					REGISTER Direction_te const eDirection = DirectionSet_fiLowest(cDirectionSet);
					assert(Direction_fbValid(eDirection));

					/* Append it to the local array. */
					DirectionString_fSetDirection(rzcNodeDirections + iuNodeDirectionCount, eDirection);
				}
				/* Set the new null terminator and increment the count. */
				rzcNodeDirections[++iuNodeDirectionCount] = 0;

				/* If we've hit the terminator, create the node and return. */
				if (bTerminator) return fbAppendNodeDirections(rpsNode, rzcNodeDirections);
			}
		}
	}
	return 0;
}

/*
TileTree
*/

/* Deallocates the tree and sets its pointer to null. */
void TileTree_fEmpty(
	REGISTER TileTree_ts * rpsTree[mReferenceConst(1)]
) {
	mStrongAssert(rpsTree);

	if (*rpsTree) {
		fDeallocateAndZeroChildNodes(rpsTree);
		mbVerify(FlexibleStruct_mbDeallocateAndZero(*rpsTree));
	}
}

/* Insert the source index directions into the node and possibly children. */
Boolean_tb TileTree_fbInsert(
	REGISTER TileTree_ts * rpsTree[mReferenceConst(1)],
	REGISTER DirectionString_tzc const rzcDirections[mReference(1)], /* Normalized index directions. */
	REGISTER Index_ts rsTile[mReferenceConst(1)] /* Update as we go so that we can determine normalized children for aggregation. */
) {
	mStrongAssert(rpsTree);
	mStrongAssert(rzcDirections);

	/* If the tree is null, allocate it from the source directions (collapsing them) and return. */
	if (0 == *rpsTree) return fbAppendDirections(rpsTree, rzcDirections);
	{
		/* Iterate to the end of the node directions.  If we find a difference, split.
		Keep the index referring to the parent so that we can get its normalized child directions.
		*/
		REGISTER DirectionString_tzc const * rzcNodeDirections = FlexibleStruct_mrMember(**rpsTree);
		for (; *rzcNodeDirections; ++rzcNodeDirections) {
			/* Compare to source. */
			if (*rzcNodeDirections != *rzcDirections) {
				/* If at the end of the source, adding an ancestor. */
				if (0 == *rzcDirections) return fbTruncate(rpsTree, rzcNodeDirections - FlexibleStruct_mrMember(**rpsTree));

				/* Not the end of the node directions; split. */
				if (0 == fpsSplit(rpsTree, rzcNodeDirections - FlexibleStruct_mrMember(**rpsTree))) return 0;

				/* Break to add child. */				
				break;
			}

			/* Advance the tile index. */
			if (!Tile_fbIncrement(rsTile)) return 0;
			Index_feAddDirection(rsTile, *rzcDirections);
			Tile_fbNormalize(rsTile); /* If there is no room to add Direction_i111, don't worry about it. */

			/* Advance source direction pointer. */
			rzcDirections = frzcNextNodeDirection(rzcDirections);
		}
		assert(0 == *rzcNodeDirections);

		/* If this line is reached, and there's more in 'rzcDirections', deal with child.
		The index is the parent.
		*/
		if (*rzcDirections) {
			/* Get a reference to the child pointer. */
			REGISTER TileTree_ts * * const rpsChild = (*rpsTree)->apsNodes + fiuChildOffset(*rzcDirections);
			
			/* If the child exists, call insert on it and return. */
			if (*rpsChild) {
				/* Must advance index first, to point to the child. */
				if (!Tile_fbIncrement(rsTile)) return 0;
				Index_feAddDirection(rsTile, *rzcDirections);
				Tile_fbNormalize(rsTile); /* If there is no room to add Direction_i111, don't worry about it. */

				/* Call insert on the child. */
				return TileTree_fbInsert(rpsChild, frzcNextNodeDirection(rzcDirections), rsTile);
			}

			/* Deal with special cases before creating the child. */
			{
				REGISTER DirectionSet_tc const cAllChildren = Index_fcNormalizedChildDirections(rsTile);
				REGISTER DirectionSet_tc cAbsentChildren = cAllChildren;
				REGISTER Boolean_tb bAllChildrenFull = (0 == *frzcNextNodeDirection(rzcDirections));
				
				{
					REGISTER TileTree_ts * const * rpsPresentChild = (*rpsTree)->apsNodes;
					REGISTER Direction_te eChildDirection = Direction_i001;
					for (; ; ++eChildDirection, ++rpsPresentChild) {
						if (*rpsPresentChild) {
							assert(DirectionSet_fbContains(cAbsentChildren, eChildDirection));
							cAbsentChildren = DirectionSet_fcRemove(cAbsentChildren, eChildDirection);
							bAllChildrenFull = bAllChildrenFull && fbFull(*rpsPresentChild);
						}
						if (Direction_i111 == eChildDirection) break;
					}
				}

				/* If there are no children, append the remaining source directions to the node (collapsing each) and return. */
				if (cAbsentChildren == cAllChildren) return fbAppendDirections(rpsTree, rzcDirections);

				/* If all children are present (including new child) and full, aggregate. */
				if (bAllChildrenFull && DirectionSet_fbEmpty(DirectionSet_fcRemove(cAbsentChildren, *rzcDirections))) {
					fDeallocateAndZeroChildNodes(rpsTree);
					return 1;
				}
			}

			/* Create the child and return. */
			return fbAppendDirections(rpsChild, frzcNextNodeDirection(rzcDirections));
		}
	}

	/* Remove children and return. */
	fDeallocateAndZeroChildNodes(rpsTree);
	return 1;
}

Boolean_tb TileTree_fbContains(
	REGISTER TileTree_ts const * const psTree,
	REGISTER DirectionString_tzc const rzcDirections[mReference(1)]
) {
	mStrongAssert(rzcDirections);

	/* If the tree is null (i.e. a null child or root), not contained.  Different than an empty tree. */
	if (0 == psTree) return 0;

	/* Iterate through non-null node directions.  If we find a mismatch in the node (including null terminator in source), return 0. */
	{
		REGISTER DirectionString_tzc const * rzcNodeDirections = FlexibleStruct_mrMember(*psTree);
		while (*rzcNodeDirections) {
			if (*rzcNodeDirections++ != *rzcDirections) return 0;
			rzcDirections = frzcNextNodeDirection(rzcDirections);
		}
	}

	/* This is either an ancestor, or the same (if tile direction is null); contained. */
	if (fbLeaf(psTree)) return 1;

	/* We've reached the end of the tile directions, but this isn't a leaf: not contained. */
	if (0 == *rzcDirections) return 0;

	/* Return whether the child contains it. */
	return TileTree_fbContains(psTree->apsNodes[fiuChildOffset(*rzcDirections)], frzcNextNodeDirection(rzcDirections));
}

/* Returns true if the two are equivalent. */
Boolean_tb TileTree_fbEquivalent(
	REGISTER TileTree_ts const * const psLeft,
	REGISTER TileTree_ts const * const psRight
) {
	if (psLeft == psRight) return 1;
	if (0 == psLeft || 0 == psRight) return 0;
	if (!DirectionString_fbEquivalent(FlexibleStruct_mrMember(*psLeft), FlexibleStruct_mrMember(*psRight))) return 0;
	{
		REGISTER TileTree_ts * const * rpsLeftChild = psLeft->apsNodes;
		REGISTER TileTree_ts * const * rpsRightChild = psRight->apsNodes;
		REGISTER Direction_te eChildDirection = Direction_i001;
		for (; ; ++eChildDirection, ++rpsLeftChild, ++rpsRightChild) {
			if (!TileTree_fbEquivalent(*rpsLeftChild, *rpsRightChild)) return 0;
			if (Direction_i111 == eChildDirection) break;
		}
	}
	return 1;
}

Boolean_tb TileTree_fbIterate(
	REGISTER TileTree_ts const * const psTree,
	REGISTER Index_ts rsTile[mReferenceConst(1)],
	REGISTER Boolean_tb (* const fbAction)(Index_ts const rsTile[mReference(1)], void * pData),
	REGISTER void * const pData
) {
	mStrongAssert(rsTile);

	/* If the node is null (i.e. a null child or root), nothing to iterate.  Different than an empty node. */
	if (0 == psTree) return 1;

	{
		REGISTER Boolean_tb bReturn = 1;
		{
			/* This is used to roll the index back up at the end. */
			REGISTER Size_tiu iuNodeDirectionCount = 0;

			/* Append each non-zero character in the node's directions to the tile. */
			{
				REGISTER DirectionString_tzc const * rzcNodeDirections = FlexibleStruct_mrMember(*psTree);
				for (; *rzcNodeDirections; ++rzcNodeDirections) {
					assert(Direction_fbValid(*rzcNodeDirections));

					/* Update tile and iuNodeDirectionCount. */
					if (!Tile_fbIncrement(rsTile)) return 0;
					Index_feAddDirection(rsTile, *rzcNodeDirections);
					Tile_fbNormalize(rsTile); /* If there is no room to add Direction_i111, don't worry about it. */
					++iuNodeDirectionCount;
				}
			}

			/* Iterate children, and if this is a leaf, perform the action. */
			{
				/* The direction moved by a child. If there are no non-null children, this will remain 0. */
				REGISTER int unsigned iuChildDirection = 0;

				/* Iterate through children. */
				{
					REGISTER TileTree_ts * const * rpsChildNode = psTree->apsNodes;
					REGISTER Direction_te eChildDirection = Direction_i001;
					for (; ; ++eChildDirection, ++rpsChildNode) {
						if (*rpsChildNode) {
							/* Increment down to child. */
							mbVerify(Tile_fbIncrement(rsTile));

							/* Add the direction. */
							iuChildDirection = Index_feAddDirection(rsTile, eChildDirection);
							assert((Direction_te const)iuChildDirection == eChildDirection); /* Only normalized indexes should be stored. */
							if (!Tile_fbNormalize(rsTile) || !TileTree_fbIterate(*rpsChildNode, rsTile, fbAction, pData)) {
								bReturn = 0;
								break;
							}

							/* Decrement index by one to return to the parent. */
							mbVerify(Tile_fbDecrement(rsTile));
						}
						if (Direction_i111 == eChildDirection) break;
					}
				}

				/* If there were no children, this is a leaf; perform the action. */
				if (bReturn && 0 == iuChildDirection) bReturn = fbAction(rsTile, pData);
			}

			/* Decrement index back to its initial direction count. */
			for (; iuNodeDirectionCount > 0; --iuNodeDirectionCount) mbVerify(Tile_fbDecrement(rsTile));
		}
		return bReturn;
	}
}

/* Format:
- Each char is a direction set + a terminator bit.
- Cases:
	- Empty node = empty direction set + 1 terminator.
	- Full node = full direction set + 1 terminator.
	- Non-last node direction = direction set with 1 entry + 0 terminator
	- Last node direction = direction set with 1 entry + (children null? 1: 0) terminator
	- Children: direction set with each non-null child + (children full? 1: 0) terminator
*/
Boolean_tb TileTree_fbSave(
	REGISTER TileTree_ts const * const psTree,
	REGISTER File_ts rsOutput[mReferenceConst(1)] /* Binary stream open for writing. */
) {
	mStrongAssert(rsOutput);
	mStrongAssert(FileMode_fbBinary(File_feMode(rsOutput)));

	/* If the tree is null, write an empty set + terminator and return. */
	if (0 == psTree) return File_fbWrite(rsOutput, fcuSetTerminator(DirectionSet_fcEmpty()));

	{
		REGISTER DirectionSet_tc cChildDirectionSet = DirectionSet_fcEmpty();
		REGISTER Boolean_tb bAllChildrenFull = 1;

		/* Populate cChildDirectionSet and bAllChildrenFull. */
		{
			REGISTER TileTree_ts * const * rpsChild = psTree->apsNodes;
			REGISTER Direction_te eChildDirection = Direction_i001;
			for (; ; ++eChildDirection, ++rpsChild) {
				if (*rpsChild) {
					bAllChildrenFull = bAllChildrenFull && fbFull(*rpsChild);
					cChildDirectionSet = DirectionSet_fcInsert(cChildDirectionSet, eChildDirection);
				}
				if (Direction_i111 == eChildDirection) break;
			}
		}

		{
			/* Get a pointer to the node directions. */
			REGISTER DirectionString_tzc const * rzcNodeDirections = FlexibleStruct_mrMember(*psTree);
			if (*rzcNodeDirections) {
				REGISTER DirectionSet_tc cNodeDirectionSet; /* Guaranteed to be initialized inside the loop. */

				/* Write all but the last node direction, and set cNodeDirectionSet to hold it. */
				for (; ; ) {
					/* Populate node direction set with only the current node direction. */
					cNodeDirectionSet = DirectionSet_fcInsert(DirectionSet_fcEmpty(), *rzcNodeDirections);

					/* Advance the pointer.  If null character, break. */
					if (0 == *++rzcNodeDirections) break;

					/* Write the direction set + no terminator. */
					if (!File_fbWrite(rsOutput, cNodeDirectionSet)) return 0;
				}

				/* Write the last node direction set + terminator.  If there are no children, include the terminator and return. */
				if (DirectionSet_fbEmpty(cChildDirectionSet)) return File_fbWrite(rsOutput, fcuSetTerminator(cNodeDirectionSet));
				if (!File_fbWrite(rsOutput, cNodeDirectionSet)) return 0;
			}
		}

		/* Write child direction set.
		If there were no children, we write full to indicate that it's a leaf, including the terminator, and return.
		If all children are full, include the terminator in the child set and return.
		*/
		if (DirectionSet_fbEmpty(cChildDirectionSet)) return File_fbWrite(rsOutput, fcuSetTerminator(DirectionSet_fcFull()));
		if (bAllChildrenFull) return File_fbWrite(rsOutput, fcuSetTerminator(cChildDirectionSet));
		if (!File_fbWrite(rsOutput, cChildDirectionSet)) return 0;
	}

	/* For each non-null child, call Save. */
	{
		REGISTER TileTree_ts * const * rpsChild = psTree->apsNodes;
		REGISTER TileTree_ts * const * const rpsChildPastEnd = rpsChild + iuChildCount; /* Safe to point one past end. */
		for (; rpsChild < rpsChildPastEnd; ++rpsChild) if (*rpsChild && !TileTree_fbSave(*rpsChild, rsOutput)) return 0;
	}

	return 1;
}

Boolean_tb TileTree_fbLoad(
	REGISTER TileTree_ts * rpsTree[mReferenceConst(1)],
	REGISTER File_ts rsInput[mReferenceConst(1)] /* Binary stream open for reading. */
) {
	mStrongAssert(rpsTree);
	mStrongAssert(rsInput);
	mStrongAssert(FileMode_fbBinary(File_feMode(rsInput)));

	{
		/* An array to hold the node directions read. */
		DirectionString_tzc azcNodeDirections[Index__MAXIMUM_DIRECTION_COUNT + 1] = {0};
		return fbLoad(rpsTree, rsInput, azcNodeDirections);
	}
}
