# include "../Headers/cell.h"
# include <stdlib.h>
# include <time.h>
# include <stdio.h>

static Boolean_tb fbTest(void) {
	/* Random seeding. */
	srand((int unsigned const)time(0));

	if (!Cell_fbTest()) return 0;
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

	# if !defined(PROFILE)
		/* Prevents the console window from disappearing until a key is pressed. */
		printf("Press enter to finish...");
		printf("%c", getc(stdin));
	# endif

	return EXIT_SUCCESS;
}
