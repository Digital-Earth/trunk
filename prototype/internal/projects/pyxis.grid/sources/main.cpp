#include <cstdlib>
#include "pyxis/grid/dodecahedral/test.hpp"

int main(int argc, char * const argv[])
{
	(void)argc;
	(void)argv;

	if (!Pyxis::Grid::Dodecahedral::Test())
	{
		assert(0 && "The tests did not all pass.");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
