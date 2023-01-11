#if !defined(PYXIS__GLOBE__RASTER_INTERFACE)
#define PYXIS__GLOBE__RASTER_INTERFACE

#include "pyxis/globe/resolution.hpp"
#include "pyxis/intrusive_reference.hpp"
#include "pyxis/pointee.hpp"

namespace Pyxis
{
	template < typename Value > struct ForwardRangeInterface;

	namespace Globe
	{
		struct RasterInterface;

		class Tile;
	}
}

struct Pyxis::Globe::RasterInterface :
virtual Pointee
{
	// Returns the resolution of this raster;
	// any cells within must have this resolution.
	virtual Resolution getResolution() const = 0;

	// Returns true if empty.
	virtual bool getIsEmpty() const = 0;
	
	// Sets the raster to empty.
	virtual void setIsEmpty() = 0;

	// Returns true if full.
	virtual bool getIsFull() const = 0;

	// Returns the number of cells in the raster.
	// Not cached.
	virtual size_t getCount() const = 0;

	// Returns true if intersects.
	virtual bool getIntersects(RasterInterface const & intersectee) const = 0;

	// Returns true if the tile is contained.
	// If the tile is of the wrong resolution, returns false.
	virtual bool find(Tile const & tile) const = 0;

	// Get the tile range.
	virtual boost::intrusive_ptr< ForwardRangeInterface< Tile const & > > getTiles() const = 0;
};
			
#endif
