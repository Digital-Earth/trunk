# include "Core/Headers/boolean.h"
# include "Core/Headers/core.h"
# include "../Headers/direction_set.h"
# include "../Headers/index.h"
# include "../Headers/tile_set.h"
# include "../Headers/cell.h"
# include "../Headers/cell_set.h"
# include <stdlib.h>
# include <time.h>

static Boolean_tb fbTest(void) {
	/* Random seeding. */
	srand((int unsigned const)time(0));

	if (!Direction_fbTest()) return 0;
	if (!DirectionSet_fbTest()) return 0;
	if (!TileSet_fbTest()) return 0;
	if (!Index_fbTest()) return 0;
	return 1;
}

int main(
	REGISTER int const iArgumentCount /* Doesn't include the null terminator */,
	REGISTER char const * const rzpzcArguments[mReferenceConst(iArgumentCount + 1)]
) {
	/* Unused. */
	(void)iArgumentCount;
	(void)rzpzcArguments;

	if (!fbTest()) return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
