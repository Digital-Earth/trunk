# if !defined(HEADERS__TILE_SET)
	# define HEADERS__TILE_SET

	# include "Core/Headers/boolean.h"
	# include "Core/Headers/core.h"
	# include "Core/Headers/size.h"
	# include "index.h"

	/* A set of tiles. */
	typedef struct {
		/* The tile tree that stores all the tiles in the set.  If this is null, the set is empty. */
		struct TileTree_ts * psTree;

		/* Whether or not the tile set is icosahedral.  
		Important for aggregation: we don't wait around for underlaps (local or otherwise) before aggregating.
		*/
		char /* Boolean_tb */ cIcosahedron;
	} TileSet_ts;

	# define TileSet_mInitializer(bIcosahedron) {0, !!(bIcosahedron)}

	void TileSet_fInitialize(
		REGISTER TileSet_ts rsTileSet[mReferenceConst(1)],
		REGISTER Boolean_tb const bIcosahedron
	);

	/* Returns true if this contains icosahedral tiles, or false if they are hexagonal. */
	static INLINE Boolean_tb TileSet_fbIcosahedron(
		REGISTER TileSet_ts const rsTileSet[mReferenceConst(1)]
	) {
		mStrongAssert(rsTileSet);
		
		return rsTileSet->cIcosahedron;
	}

	static INLINE Boolean_tb TileSet_fbEmpty(
		REGISTER TileSet_ts const rsTileSet[mReferenceConst(1)]
	) {
		mStrongAssert(rsTileSet);
		
		return (0 == rsTileSet->psTree);
	}

	/* Count the root indexes in the tile set. */
	Size_tiu TileSet_fiuCount(
		REGISTER TileSet_ts const rsTileSet[mReferenceConst(1)]
	);

	/* Returns 1 if the set contains the index; 0 if not. */
	Boolean_tb TileSet_fbContains(
		REGISTER TileSet_ts const rsTileSet[mReferenceConst(1)],
		REGISTER Index_ts const rsTile[mReferenceConst(1)] /* If wrong type for tile set (icosahedral/hexagonal), converted. */
	);

	/* Returns true if the two are equivalent. */
	Boolean_tb TileSet_fbEquivalent(
		REGISTER TileSet_ts const rsLeft[mReferenceConst(1)],
		REGISTER TileSet_ts const rsRight[mReferenceConst(1)]
	);

	/* For each root index in the tile set,
	call fbAction({index}, pData), and stop iteration if it returns false.
	Returns true if fbAction returned true in every case.
	*/
	Boolean_tb TileSet_fbIterate(
		REGISTER TileSet_ts const rsTileSet[mReferenceConst(1)],
		REGISTER Boolean_tb (* const fbAction)(Index_ts const rsTile[mReference(1)], void * pData),
		REGISTER void * const pData
	);

	/* Returns 0 if failed. */
	Boolean_tb TileSet_fbInsert(
		REGISTER TileSet_ts rsTileSet[mReferenceConst(1)],
		REGISTER Index_ts const rsTile[mReferenceConst(1)] /* If wrong type for tile set (icosahedral/hexagonal), converted. */
	);

	/* Returns 0 if failed. */
	Boolean_tb TileSet_fbRemove(
		REGISTER TileSet_ts rsTileSet[mReferenceConst(1)],
		REGISTER Index_ts const rsTile[mReferenceConst(1)] /* If wrong type for tile set (icosahedral/hexagonal), converted. */
	);

	/* Returns 0 if failed. */
	Boolean_tb TileSet_fbLoad(
		REGISTER TileSet_ts rsTileSet[mReferenceConst(1)],
		REGISTER File_ts rsInput[mReferenceConst(1)] /* Binary stream open for reading. */
	);

	/* Returns 0 if failed. */
	Boolean_tb TileSet_fbSave(
		REGISTER TileSet_ts const rsTileSet[mReferenceConst(1)],
		REGISTER File_ts rsOutput[mReferenceConst(1)] /* Binary stream open for writing. */
	);

	/* Returns 0 if failed. */
	Boolean_tb TileSet_fbTest(void);

# endif
