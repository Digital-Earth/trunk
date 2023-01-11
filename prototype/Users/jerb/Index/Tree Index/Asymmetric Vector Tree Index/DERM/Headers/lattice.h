# if !defined(HEADERS__LATTICE)
	# define HEADERS__LATTICE

	# include "Core/Headers/core.h"

	/* There are two lattices, labelled 0 and 1, each comprised of one of the hexagon classes. */
	typedef Boolean_tb Lattice_tb;

	static INLINE Boolean_tb Lattice_fbValid(
		REGISTER Lattice_tb const eLattice
	) {
		return (0 == eLattice || 1 == eLattice);
	}

# endif
