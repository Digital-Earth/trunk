# if !defined(HEADERS__TILE)
	# define HEADERS__TILE

	/* This file contains an interface for treating an index as a tile.
	A tile is a collection of all unrestricted-depth descendants of a cell.
	A tile is represented by an index.
	A tile whose index ends in a vertex (i.e. not Direction_i111) is equivalent
	to the tile for the incremented index (i.e. appending Direction_i111).
	The latter form is the normalized form.
	*/

	# include "index.h"

	/* An initializer for a stack instance of a tile.  Note that the argument can only be a compile-time constant. */
	# define Tile_mInitializer(bIcosahedron) Index_mInitializer(bIcosahedron)

	static INLINE Boolean_tb Tile_fbNormalize(
		REGISTER Index_ts rsIndex[mReferenceConst(1)]
	) {
		return (Index_fbCentroid(rsIndex) || Index_fbIncrement(rsIndex));
	}

	/* Initialize the tile by initializing, then normalizing, the index. */
	static INLINE Boolean_tb Tile_fbInitialize(
		REGISTER Index_ts rsIndex[mReferenceConst(1)],
		REGISTER Boolean_tb const bIcosahedron,
		REGISTER char const rzcDirections[mReference(1)]
	) {
		return Index_fbInitialize(rsIndex, bIcosahedron, rzcDirections) && Tile_fbNormalize(rsIndex);
	}

	static INLINE Boolean_tb Tile_fbIncrement(
		REGISTER Index_ts rsIndex[mReferenceConst(1)]
	) {
		return (Index_fbCentroid(rsIndex) || Index_fbIncrement(rsIndex)) && Index_fbIncrement(rsIndex);
	}
	
	static INLINE Boolean_tb Tile_fbDecrement(
		REGISTER Index_ts rsIndex[mReferenceConst(1)]
	) {
		return Index_fbDecrement(rsIndex) && (Index_fbCentroid(rsIndex) || Index_fbDecrement(rsIndex));
	}

#endif
