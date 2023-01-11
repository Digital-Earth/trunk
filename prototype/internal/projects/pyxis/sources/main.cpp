#include "pyxis/test.hpp"

int main(int argc, char * const argv[])
{
	if (Pyxis::Test())
	{
		return EXIT_SUCCESS;
	}
	assert(0 && "The tests did not all pass.");
	return EXIT_FAILURE;
}
