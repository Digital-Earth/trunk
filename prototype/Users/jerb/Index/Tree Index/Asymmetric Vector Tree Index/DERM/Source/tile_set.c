/* TODO:

- Write fbIntersection.
	- Just implement basic boolean result now; ignore the other "out" parameters.

- Stack it up against PYXTileSet and record results on Wiki page.

- Write fRemove (which takes aggregation into account, adding siblings as necessary).

- Expand fbIntersection.

- Optimization:
	- Implement things *not* by iterator if there is a savings.

- Test with ANSI and C99.
*/

# include "tile_tree.h"
# include "../Headers/tile.h"
# include "../Headers/tile_set.h"
# include <time.h>

static Boolean_tb fbWriteAction(
	Index_ts const rsTile[mReference(1)],
	void * psFile
) {
	mStrongAssert(rsTile);
	assert(mbIs(psFile, File_ts *));
	
	{
		File_ts * const psOutputFile = (File_ts * const)psFile;
		mbVerify(Index_fbWrite(rsTile, psOutputFile));
		mbVerify(File_fbWrite(psOutputFile, '\n'));
	}
	return 1;
}

static Boolean_tb fbIncrementCountAction(
	Index_ts const rsTile[mReference(1)],
	void * rCount
) {
	mStrongAssert(rCount);
	assert(mbIs(rCount, Size_tiu *));

	(void)rsTile;
	++*(Size_tiu * const)rCount;
	return 1;
}

static Boolean_tb fbTestSaveAndLoad(
	REGISTER TileSet_ts const rsTileSet[mReferenceConst(1)]
) {
	mStrongAssert(rsTileSet);

	{
		REGISTER Boolean_tb bSuccess = 0;
		File_ts sFile = File_mInitializer();
		if (!mbVerify(File_fbOpenTemporary(&sFile))) return 0;
		for (; ; ) {
			if (!mbVerify(TileSet_fbSave(rsTileSet, &sFile))) break;
			if (!mbVerify(File_fbRewind(&sFile))) break;
			{
				TileSet_ts sLoadedTileSet = TileSet_mInitializer(0);
				if (!mbVerify(TileSet_fbLoad(&sLoadedTileSet, &sFile))) break;
				if (!mbVerify(TileSet_fbEquivalent(rsTileSet, &sLoadedTileSet))) break;
			}
			bSuccess = 1;
			break;
		}
		return mbVerify(File_fbClose(&sFile)) && bSuccess;
	}
}

static Boolean_tb fbTestInsert(
	REGISTER TileSet_ts rsTileSet[mReferenceConst(1)],
	REGISTER char const rzcTile[mReferenceConst(1)],
	REGISTER Size_tiu iuExpectedTileCount,
	REGISTER char const * const rrzcExpectedTiles[mReference(iuExpectedTileCount? iuExpectedTileCount: 1)],
	REGISTER File_ts rsOutput[mReferenceConst(1)]
) {
	mStrongAssert(rsTileSet);
	mStrongAssert(rzcTile);
	mStrongAssert(rrzcExpectedTiles);

	{
		/* Create the tile. */
		Index_ts sTile = Tile_mInitializer(1);
		mbVerify(Tile_fbInitialize(&sTile, 1, rzcTile));
		
		/* Insert the tile into the tile set. */
		if (!TileSet_fbInsert(rsTileSet, &sTile)) return mbVerify(0);
		
		/* Write the contents of the tile set. */
		TileSet_fbIterate(rsTileSet, fbWriteAction, rsOutput);

		/* Confirm tile count. */
		if (iuExpectedTileCount != TileSet_fiuCount(rsTileSet)) return mbVerify(0);

		/* Confirm tile containment. */
		for (; iuExpectedTileCount > 0; ++rrzcExpectedTiles, --iuExpectedTileCount) {
			Index_ts sExpectedTile = Tile_mInitializer(1);
			mbVerify(Tile_fbInitialize(&sExpectedTile, 1, *rrzcExpectedTiles));
			if (!TileSet_fbContains(rsTileSet, &sExpectedTile)) return mbVerify(0);
		}

		/* Save, load and confirm. */
		if (!fbTestSaveAndLoad(rsTileSet)) return mbVerify(0);
	}
	
	return 1;
}

/*
TileSet
*/

/* Must have been initialized with the initializer. */
void TileSet_fInitialize(
	REGISTER TileSet_ts rsTileSet[mReferenceConst(1)],
	REGISTER Boolean_tb const bIcosahedron
) {
	mStrongAssert(rsTileSet);

	TileTree_fEmpty(&(rsTileSet->psTree));
	rsTileSet->cIcosahedron = (char const)bIcosahedron;
}

Boolean_tb TileSet_fbInsert(
	REGISTER TileSet_ts rsTileSet[mReferenceConst(1)],
	REGISTER Index_ts const rsTile[mReferenceConst(1)] /* If wrong type for tile set (icosahedral/hexagonal), converted. */
) {
	mStrongAssert(rsTileSet);
	assert(rsTile);

	{
		/* Create the temporary tile used by fbInsert. */
		Index_ts sTile = Tile_mInitializer(0);
		if (!mbVerify(Tile_fbInitialize(&sTile, rsTileSet->cIcosahedron, ""))) return 0;

		/* If the tile set is icosahedral, and the index is hexagonal, the index must be normalized. */
		if (rsTileSet->cIcosahedron && !Index_fbIcosahedron(rsTile)) {
			Index_ts sNormalizedTile = Tile_mInitializer(1);
			if (!mbVerify(Tile_fbInitialize(&sNormalizedTile, 1, Index_frzcDirections(rsTile)))) return 0;
			return TileTree_fbInsert(&(rsTileSet->psTree), Index_frzcDirections(&sNormalizedTile), &sTile);
		}

		return TileTree_fbInsert(&(rsTileSet->psTree), Index_frzcDirections(rsTile), &sTile);
	}
}

/* Return 0 if failure, or if fbAction returns 0. */
Boolean_tb TileSet_fbIterate(
	REGISTER TileSet_ts const rsTileSet[mReferenceConst(1)],
	REGISTER Boolean_tb (* const fbAction)(Index_ts const rsTile[mReference(1)], void * pData),
	REGISTER void * const pData
) {
	mStrongAssert(rsTileSet);
	mStrongAssert(fbAction);

	/* If the tree is null, the set is empty. */
	if (!rsTileSet->psTree) return 1;

	{
		/* Create the tile that will be modified. */
		Index_ts sTile = Tile_mInitializer(0);
		mbVerify(Tile_fbInitialize(&sTile, rsTileSet->cIcosahedron, ""));

		/* Do tree iteration. */
		return TileTree_fbIterate(rsTileSet->psTree, &sTile, fbAction, pData);
	}
}

/* Count the root tiles in the tile set. */
Size_tiu TileSet_fiuCount(
	REGISTER TileSet_ts const rsTileSet[mReferenceConst(1)]
) {
	mStrongAssert(rsTileSet);
	
	{
		Size_tiu iuCount = 0;
		mbVerify(TileSet_fbIterate(rsTileSet, fbIncrementCountAction, &iuCount));
		return iuCount;
	}
}

/* Returns 1 if the set contains the index; 0 if not. */
Boolean_tb TileSet_fbContains(
	REGISTER TileSet_ts const rsTileSet[mReferenceConst(1)],
	REGISTER Index_ts const rsTile[mReferenceConst(1)]
) {
	mStrongAssert(rsTileSet);
	mStrongAssert(rsTile);

	return TileTree_fbContains(rsTileSet->psTree, Index_frzcDirections(rsTile));
}

/* Returns true if the two are equivalent. */
Boolean_tb TileSet_fbEquivalent(
	REGISTER TileSet_ts const rsLeft[mReferenceConst(1)],
	REGISTER TileSet_ts const rsRight[mReferenceConst(1)]
) {
	mStrongAssert(rsLeft);
	mStrongAssert(rsRight);

	return (rsLeft->cIcosahedron == rsRight->cIcosahedron) && TileTree_fbEquivalent(rsLeft->psTree, rsRight->psTree);
}

/* Returns 0 if failed. */
Boolean_tb TileSet_fbLoad(
	REGISTER TileSet_ts rsTileSet[mReferenceConst(1)],
	REGISTER File_ts rsInput[mReferenceConst(1)] /* Binary stream open for reading. */
) {
	mStrongAssert(rsTileSet);
	mStrongAssert(rsInput && File_fbValid(rsInput));
	
	if (!FileMode_fbBinary(File_feMode(rsInput))) return 0;
	{
		char unsigned cuIcosahedron = 0;
		if (!File_fbRead(rsInput, &cuIcosahedron)) return 0;

		/* 'rsTileSet->cIcosahedron' is saved as 1 for false and 2 for true, to avoid null characters. */
		assert(1 == cuIcosahedron || 2 == cuIcosahedron);
		rsTileSet->cIcosahedron = (1 != cuIcosahedron);
	}
	return TileTree_fbLoad(&(rsTileSet->psTree), rsInput);
}

/* Returns 0 if failed. */
Boolean_tb TileSet_fbSave(
	REGISTER TileSet_ts const rsTileSet[mReferenceConst(1)],
	REGISTER File_ts rsOutput[mReferenceConst(1)] /* Binary stream open for writing. */
) {
	mStrongAssert(rsTileSet);
	mStrongAssert(rsOutput && File_fbValid(rsOutput));

	if (!FileMode_fbBinary(File_feMode(rsOutput))) return 0;

	/* 'rsTileSet->cIcosahedron' is saved as 1 for false and 2 for true, to avoid null characters. */
	assert(0 == rsTileSet->cIcosahedron || 1 == rsTileSet->cIcosahedron);
	return File_fbWrite(rsOutput, rsTileSet->cIcosahedron + 1) && TileTree_fbSave(rsTileSet->psTree, rsOutput);
}

/* Returns 0 if failed. */
Boolean_tb TileSet_fbTest(void) {
	File_ts sOutput = File_mInitializer();
	mbVerify(File_fbOutput(&sOutput));

	printf("Performing tile set unit tests.\n");
	{
		TileSet_ts sTileSet = TileSet_mInitializer(1);

		/* Test empty tile set. */
		printf("Iterating empty tile set.\n");
		TileSet_fbIterate(&sTileSet, fbWriteAction, &sOutput);
		if (!fbTestSaveAndLoad(&sTileSet)) return mbVerify(0);

		/* Test regular insertion. */
		printf("Inserting tile 71747.\n");
		{
			char const * const arzcExpectedTiles[] = {"\7\1\7\4\7"};
			if (!fbTestInsert(&sTileSet, "\7\1\7\4\7", Size_miuArrayElementCount(arzcExpectedTiles), arzcExpectedTiles, &sOutput)) return mbVerify(0);
		}

		/* Test inserting a sibling that is not the last; no aggregation should happen. */
		printf("Inserting index 7176.\n");
		{
			char const * const arzcExpectedTiles[] = {"\7\1\7\4\7", "\7\1\7\6\7"};
			if (!fbTestInsert(&sTileSet, "\7\1\7\6", Size_miuArrayElementCount(arzcExpectedTiles), arzcExpectedTiles, &sOutput)) return mbVerify(0);
		}

		/* Test sibling aggregation. */
		printf("Inserting index 7177.\n");
		{
			char const * const arzcExpectedTiles[] = {"\7\1\7"};
			if (!fbTestInsert(&sTileSet, "\7\1\7\7", Size_miuArrayElementCount(arzcExpectedTiles), arzcExpectedTiles, &sOutput)) return mbVerify(0);
		}

		/* Test adding a non-aggregating sibling. */
		printf("Inserting index 5774717.\n");
		{
			char const * const arzcExpectedTiles[] = {"\5\7\7\4\7\1\7", "\7\1\7"};
			if (!fbTestInsert(&sTileSet, "\5\7\7\4\7\1\7", Size_miuArrayElementCount(arzcExpectedTiles), arzcExpectedTiles, &sOutput)) return mbVerify(0);
		}

		/* Test splitting. */
		printf("Inserting index 577577717.\n");
		{
			char const * const arzcExpectedTiles[] = {"\5\7\7\4\7\1\7", "\5\7\7\5\7\7\7\1\7", "\7\1\7"};
			if (!fbTestInsert(&sTileSet, "\5\7\7\5\7\7\7\1\7", Size_miuArrayElementCount(arzcExpectedTiles), arzcExpectedTiles, &sOutput)) return mbVerify(0);
		}

		printf("Inserting index 577577377.\n");
		{
			char const * const arzcExpectedTiles[] = {"\5\7\7\4\7\1\7", "\5\7\7\5\7\7\7\1\7", "\5\7\7\5\7\7\3\7\7", "\7\1\7"};
			if (!fbTestInsert(&sTileSet, "\5\7\7\5\7\7\3\7\7", Size_miuArrayElementCount(arzcExpectedTiles), arzcExpectedTiles, &sOutput)) return mbVerify(0);
		}

		/* Test truncation. */
		printf("Inserting index 5.\n");
		{
			char const * const arzcExpectedTiles[] = {"\5\7", "\7\1\7"};
			if (!fbTestInsert(&sTileSet, "\5", Size_miuArrayElementCount(arzcExpectedTiles), arzcExpectedTiles, &sOutput)) return mbVerify(0);
		}

		/* Test append. */
		printf("Inserting index 577377.\n");
		{
			char const * const arzcExpectedTiles[] = {"\5\7\7\3\7\7", "\7\1\7"};
			if (!fbTestInsert(&sTileSet, "\5\7\7\3\7\7", Size_miuArrayElementCount(arzcExpectedTiles), arzcExpectedTiles, &sOutput)) return mbVerify(0);
		}

		printf("Inserting index 0.\n");
		{
			char const * const arzcExpectedTiles[] = {""};
			if (!fbTestInsert(&sTileSet, "", Size_miuArrayElementCount(arzcExpectedTiles), arzcExpectedTiles, &sOutput)) return mbVerify(0);
		}
	}

	printf("Performing randomized tests.\n");
	{
		TileSet_ts sTileSet = TileSet_mInitializer(1);
		TileSet_ts sTileSetReloaded = TileSet_mInitializer(0);
		char const * rzcFileName = File_rzcTemporaryName();

		/* If a file with that name already exists, pick a new name. */
		while (File_fbExists(rzcFileName)) rzcFileName = File_rzcTemporaryName();
		printf("File: %s\n", rzcFileName);

		/* Randomly populate the tile set. */
		{
			REGISTER Size_tiu iuIndexCount = 20000;
			REGISTER clock_t const tClock = clock();

			printf("Creating with %" Size_mrzcFormatter() " indexes...\n", iuIndexCount);
			for (; 0 < iuIndexCount; --iuIndexCount) {
				Index_ts sIndex = Index_mInitializer(1);
				Index_fRandom(&sIndex);
				if (!TileSet_fbInsert(&sTileSet, &sIndex)) return mbVerify(0);
			}
			printf("%g seconds.\n", (double)(clock() - tClock) / CLOCKS_PER_SEC);
		}

		/* Save the tile set. */
		{
			REGISTER Boolean_tb bSuccess = 0;
			File_ts sFile = File_mInitializer();
			REGISTER clock_t const tClock = clock();

			printf("Saving... ");
			if (!File_fbOpen(&sFile, rzcFileName, FileMode_iWriteBinary)) return mbVerify(0);
			while (TileSet_fbSave(&sTileSet, &sFile)) {
				bSuccess = 1;
				break;
			}
			if (!File_fbClose(&sFile)) return mbVerify(0);
			if (!bSuccess) return mbVerify(0);
			printf("%g seconds.\n", (double)(clock() - tClock) / CLOCKS_PER_SEC);
		}

		/* Load the tile set. */
		{
			REGISTER Boolean_tb bSuccess = 0;
			File_ts sFile = File_mInitializer();
			REGISTER clock_t const tClock = clock();

			printf("Loading... ");
			if (!File_fbOpen(&sFile, rzcFileName, FileMode_iReadBinary)) return mbVerify(0);
			while (TileSet_fbLoad(&sTileSetReloaded, &sFile)) {
				bSuccess = 1;
				break;
			}
			if (!File_fbClose(&sFile)) return mbVerify(0);
			if (!bSuccess) return mbVerify(0);
			printf("%g seconds.\n", (double)(clock() - tClock) / CLOCKS_PER_SEC);
		}

		/* Verify. */
		{
			REGISTER clock_t const tClock = clock();
			printf("Verifying... ");
			if (!TileSet_fbEquivalent(&sTileSet, &sTileSetReloaded)) return mbVerify(0);
			printf("%g seconds.\n", (double)(clock() - tClock) / CLOCKS_PER_SEC);
		}

		/* Delete the file. */
		mbVerify(File_fbRemove(rzcFileName));
	}

	return 1;
}
