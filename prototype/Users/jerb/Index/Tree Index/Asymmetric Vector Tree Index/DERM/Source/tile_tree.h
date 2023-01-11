# if !defined(SOURCE__TILE_TREE)
	# define SOURCE__TILE_TREE

	# include "../Headers/index.h"

	/* A tree of tiles.
	A null node pointer indicates an empty tree.
	A tree must be emptied after use to prevent memory leakage.
	*/
	typedef struct TileTree_ts TileTree_ts;

	/* Deallocates the tree and sets its pointer to null, indicating an empty tree.
	This must be called after use to prevent memory leaks.
	*/
	void TileTree_fEmpty(
		REGISTER TileTree_ts * rpsTree[mReferenceConst(1)]
	);

	/* Insert normalized index directions into the tree and possibly children. */
	Boolean_tb TileTree_fbInsert(
		REGISTER TileTree_ts * rpsTree[mReferenceConst(1)],
		REGISTER DirectionString_tzc const rzcDirections[mReference(1)], /* Normalized index directions. */
		REGISTER Index_ts rsTile[mReferenceConst(1)] /* Update as we go so that we can determine normalized children for aggregation. */
	);

	/* Returns 1 if the tree contains the index specified by the directions. */
	Boolean_tb TileTree_fbContains(
		REGISTER TileTree_ts const * const psTree,
		REGISTER DirectionString_tzc const rzcDirections[mReference(1)] /* Normalized index directions. */
	);

	/* Returns true if the two are equivalent. */
	Boolean_tb TileTree_fbEquivalent(
		REGISTER TileTree_ts const * const psLeft,
		REGISTER TileTree_ts const * const psRight
	);

	/* Iterate over the tiles in the tree. */
	Boolean_tb TileTree_fbIterate(
		REGISTER TileTree_ts const * const psTree,
		REGISTER Index_ts rsTile[mReferenceConst(1)], /* Gets restored to its initial value. */
		REGISTER Boolean_tb (* const fbAction)(Index_ts const rsTile[mReference(1)], void * pData),
		REGISTER void * const pData
	);

	Boolean_tb TileTree_fbSave(
		REGISTER TileTree_ts const * const psTree,
		REGISTER File_ts rsOutput[mReferenceConst(1)] /* Binary stream open for writing. */
	);
	
	Boolean_tb TileTree_fbLoad(
		REGISTER TileTree_ts * rpsTree[mReferenceConst(1)],
		REGISTER File_ts rsInput[mReferenceConst(1)] /* Binary stream open for reading. */
	);

#endif
