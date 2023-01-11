/* Copyright (c) 2007 Jason Erb

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

# include "../Headers/char_string.h"
# include "../Headers/core.h"
# include "../Headers/modulo_7.h"
# include "../Headers/modulo_7_set.h"
# include <stdlib.h>
# include <time.h>

static Boolean_tb fbTest(void) {
	/* Random seeding. */
	srand((int unsigned const)time(0));

	if (!CharString_fbTest()) return 0;
	if (!Modulo7_fbTest()) return 0;
	if (!Modulo7Set_fbTest()) return 0;
	return 1;
}

int main(
	REGISTER int iArgumentCount /* Doesn't include the null terminator */,
	REGISTER char const * const rzpzicArguments[mReference(iArgumentCount + 1)]
) {
	mAssert(0 != rzpzicArguments);

	(void)iArgumentCount;
	(void)rzpzicArguments;

	if (!fbTest()) return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
