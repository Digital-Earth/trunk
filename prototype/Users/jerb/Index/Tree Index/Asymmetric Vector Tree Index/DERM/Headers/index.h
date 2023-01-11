# if !defined(HEADERS__INDEX)
	# define HEADERS__INDEX

	/* The indexing scheme that this code implements is described here:
	https://www.pyxisinnovation.com/pyxinternalwiki/index.php?title=Indexing_The_Sphere
	*/

	# include "Core/Headers/core.h"
	# include "Core/Headers/file.h"
	# include "Core/Headers/size.h"
	# include "direction.h"
	# include "direction_set.h"
	# include "direction_string.h"
	# include "lattice.h"

	/* Defines the number of directions that will fit in the array in the index.
	The maximum value for this is the maximum size of a char array, minus 1 (for the null terminator).
	Odd values are optimal.
	*/
	# if !defined(Index__MAXIMUM_DIRECTION_COUNT)
		# define Index__MAXIMUM_DIRECTION_COUNT 43
	# endif

	/* The underlap direction from the root centroid, which becomes the only normalized resolution 1 vertex.
	This must be idempotent for the reverse operation (i.e. left-right symmetrical; Direction_i010 or Direction_i101).
	101 was chosen because it is the only vertex direction that, like 111, contains more 1s than 0s.
	*/
	enum {Index_iRootUnderlapDirection = Direction_i101};

	/* An index. */
	typedef struct {
		Size_tiu iuPentagonCount; /* The cached number of ancestors, including self, that are pentagonal. */
		Size_tiu iuDirectionCount; /* The number of directions in the array, not including the null terminator. */
		DirectionString_tzc azcDirections[Index__MAXIMUM_DIRECTION_COUNT + 1];
	} Index_ts;

	/* An initializer for a stack instance of an index.  Note that the argument can only be a compile-time constant. */
	# define Index_mInitializer(bIcosahedron) {!!(bIcosahedron), 0, {0}}

	/* Initialize the index.
	The directions are in numeric format.
	If there are too many directions (or it is not properly null-terminated), the index will only use as many as can fit and return false.
	Also, note that since index abc = a77 + b7 + c, the string of directions is treated as a sequence of independent commands.
	*/
	Boolean_tb Index_fbInitialize(
		REGISTER Index_ts rsIndex[mReferenceConst(1)],
		REGISTER Boolean_tb const bIcosahedron,
		REGISTER char const rzcDirections[mReference(1)]
	);

	/* Copies the index. */
	void Index_fCopy(
		REGISTER Index_ts const rsSource[mReferenceConst(1)],
		REGISTER Index_ts rsDestination[mReferenceConst(1)]
	);

	/* Returns the number of pentagon ancestors of the index, including the index itself.
	0 means that it is an index of a purely hexagonal (vs. icosahedral) tessellation.
	*/
	Size_tiu Index_fiuPentagonCount(
		REGISTER Index_ts const rsIndex[mReferenceConst(1)]
	);

	/* Returns true if the index is icosahedral, or false if hexagonal. */
	Boolean_tb Index_fbIcosahedron(
		REGISTER Index_ts const rsIndex[mReferenceConst(1)]
	);

	/* Returns the direction count of the index. */
	Size_tiu Index_fiuDirectionCount(
		REGISTER Index_ts const rsIndex[mReferenceConst(1)]
	);

	/* Returns a string containing the numeric directions. */
	DirectionString_tzc const * Index_frzcDirections(
		REGISTER Index_ts const rsIndex[mReferenceConst(1)]
	);

	/* Returns the lattice that the index is on. */
	Lattice_tb Index_fbLattice(
		REGISTER Index_ts const rsIndex[mReferenceConst(1)]
	);

	/* Returns true if the index describes a centroid, or false if it's a vertex. */
	Boolean_tb Index_fbCentroid(
		REGISTER Index_ts const rsIndex[mReferenceConst(1)]
	);

	/* Returns true if the index describes a hexagonal cell, or false otherwise. */
	Boolean_tb Index_fbHexagon(
		REGISTER Index_ts const rsIndex[mReferenceConst(1)]
	);

	/* Moves to the neighbour cell in the specified direction.
	Returns the last direction travelled to get to the resulting index, which may differ from eDirection if an underlap was traversed.
	*/
	Direction_te Index_feAddDirection(
		REGISTER Index_ts rsIndex[mReferenceConst(1)],
		REGISTER Direction_te const eDirection
	);

	/* Adds two indices together. */
	Boolean_tb Index_fbAdd(
		REGISTER Index_ts rsSum[mReferenceConst(1)],
		REGISTER Index_ts const rsAugend[mReference(1)],
		REGISTER Index_ts const rsAddend[mReference(1)]
	);

	/* Zooms out to the parent, decimating the index. */
	Boolean_tb Index_fbDecrement(
		REGISTER Index_ts rsIndex[mReferenceConst(1)]
	);

	/* Zooms in to the centroid child. */
	Boolean_tb Index_fbIncrement(
		REGISTER Index_ts rsIndex[mReferenceConst(1)]
	);

	/* Returns true if the two indexes are equivalent. */
	Boolean_tb Index_fbEquivalent(
		REGISTER Index_ts const rsIndexLeft[mReferenceConst(1)],
		REGISTER Index_ts const rsIndexRight[mReferenceConst(1)]
	);

	/* Returns the set of normalized child directions for the index. */
	DirectionSet_tc Index_fcNormalizedChildDirections(
		REGISTER Index_ts const rsIndex[mReferenceConst(1)]
	);

	/* For each descendant of the given index at the given resolution,
	call fbAction({index}, pData), and stop iteration if it returns false.
	Returns true if fbAction returned true in every case.
	Note that although the index is not 'const', its value before and after the call is the same.
	If constness is required, make a non-const copy of the index prior to calling this.
	*/
	Boolean_tb Index_fbIterate(
		REGISTER Boolean_tb const bDepthFirst,
		REGISTER Index_ts rsIndex[mReferenceConst(1)],
		REGISTER Size_tiu const iuDirectionCount,
		REGISTER Boolean_tb (* const fbAction)(Index_ts const rsIndex[mReference(1)], void * pData),
		REGISTER void * const pData
	);

	/* Appends the ASCII directions in the stream to the index (which is not initialized first), and returns the next character read.
	Also, note that since index abc = a77 + b7 + c, the string of directions read is treated as a sequence of independent commands.
	*/
	int Index_fiRead(
		REGISTER Index_ts rsIndex[mReferenceConst(1)],
		REGISTER File_ts rsInput[mReferenceConst(1)] /* Already open for reading. */
	);

	/* Writes the index to the stream with directions in ASCII format. */
	Boolean_tb Index_fbWrite(
		REGISTER Index_ts const rsIndex[mReferenceConst(1)],
		REGISTER File_ts rsOutput[mReferenceConst(1)]
	);

	/* Generates a random index.  Assumes that the random number generator has been properly seeded. */
	void Index_fRandom(
		REGISTER Index_ts rsIndex[mReferenceConst(1)]
	);

	/* Tests the index code. */
	Boolean_tb Index_fbTest(void);

#endif
