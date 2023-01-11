# if !defined(HEADERS__CELL_SET)
	# define HEADERS__CELL_SET

	# include "cell.h"
	# include "tile_set.h"

	/* A set of cells at a specific resolution.
	If a cell at a lower resolution is added, all its descendants at the resolution are added.
	If a cell at a higher resolution is added, its ancestor at the resolution is added.
	*/
	typedef struct {
		/* The tile set that it is implemented in terms of. */
		TileSet_ts sTileSet;

		/* The resolution. */
		Size_tiu iuDirectionCount;
	} CellSet_ts;

	/* TODO: Fill in interface. */

#endif
