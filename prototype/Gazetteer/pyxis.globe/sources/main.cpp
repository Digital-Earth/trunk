#include "pyxis/globe/test.hpp"

int main(int argc, char * const argv[])
{
	(void)argc;
	(void)argv;

	if (Pyxis::Globe::Test())
	{
		return EXIT_SUCCESS;
	}
	assert(0 && "The tests did not all pass.");
	return EXIT_FAILURE;
}
