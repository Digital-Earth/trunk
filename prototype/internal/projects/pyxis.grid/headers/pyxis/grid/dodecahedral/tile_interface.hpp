#if !defined(PYXIS__GRID__DODECAHEDRAL__TILE_INTERFACE)
#define PYXIS__GRID__DODECAHEDRAL__TILE_INTERFACE

namespace Pyxis
{
	namespace Grid
	{
		namespace Dodecahedral
		{
			// A tile.
			struct TileInterface;
		}
	}
}

#include "pyxis/grid/dodecahedral/resolution.hpp"
#include "pyxis/pointee.hpp"

struct Pyxis::Grid::Dodecahedral::TileInterface :
virtual Pointee
{
	// Returns the resolution of the tile.
	virtual Resolution getResolution() const = 0;
};

#endif
