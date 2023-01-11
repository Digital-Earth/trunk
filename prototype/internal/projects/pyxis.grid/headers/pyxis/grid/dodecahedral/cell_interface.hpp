#if !defined(PYXIS__GRID__DODECAHEDRAL__CELL_INTERFACE)
#define PYXIS__GRID__DODECAHEDRAL__CELL_INTERFACE

namespace Pyxis
{
	namespace Grid
	{
		namespace Dodecahedral
		{
			// A cell.
			struct CellInterface;

			// A mutable cell.
			struct MutableCellInterface;
		}
	}
}

#include "pyxis/grid/dodecahedral/direction.hpp"
#include "pyxis/grid/dodecahedral/tile_interface.hpp"

// A cell.
struct Pyxis::Grid::Dodecahedral::CellInterface :
virtual TileInterface
{
	typedef char NeighbourCount;
	typedef char OverlapCount;
	typedef char UnderlapCount;

	typedef char NeighbourOffset;
	typedef char OverlapOffset;
	typedef char UnderlapOffset;

	// Convert overlap/neighbour/underlap offset to direction.
	virtual Direction getOverlapDirection(
		OverlapOffset const overlapOffset) const = 0;
	virtual Direction getNeighbourDirection(
		NeighbourOffset const neighbourOffset) const = 0;
	virtual Direction getUnderlapDirection(
		UnderlapOffset const underlapOffset) const = 0;

	// Convert direction to overlap/neighbour/underlap offset.
	virtual OverlapOffset * getOverlapOffset(
		OverlapOffset & result, Direction const direction) const = 0;
	virtual NeighbourOffset * getNeighbourOffset(
		NeighbourOffset & result, Direction const direction) const = 0;
	virtual UnderlapOffset * getUnderlapOffset(
		UnderlapOffset & result, Direction const direction) const = 0;

	// Returns true if the cell is contained by a cell at the prior resolution.
	virtual bool getIsContained() const = 0;

	// Returns the number of neighbours at the same resolution.
	virtual NeighbourCount getNeighbourCount() const = 0;

	// Returns the number of intersecting cells at the prior resolution.
	virtual UnderlapCount getUnderlapCount() const = 0;
	
	// Returns the number of intersecting cells at the next resolution.
	virtual OverlapCount getOverlapCount() const = 0;
};

// A mutable cell.
struct Pyxis::Grid::Dodecahedral::MutableCellInterface :
virtual CellInterface
{
	// Steps to a neighbour cell at the same resolution.
	// Returns the neighbour offset for the return trip.
	// Throws if the neighbour offset is invalid.
	virtual NeighbourOffset stepToNeighbour(NeighbourOffset const neighbourOffset) = 0;

	// Steps to an intersecting cell at the next resolution.
	// Returns the underlap offset for the return trip.
	// Throws on resolution overflow, or if the vertex is invalid.
	virtual UnderlapOffset stepToOverlap(OverlapOffset const overlapOffset) = 0;

	// Steps to an intersecting cell at the prior resolution.
	// Returns the overlap offset for the return trip.
	// Throws on resolution underflow, or if the vertex is invalid.
	virtual OverlapOffset stepToUnderlap(UnderlapOffset const underlapOffset) = 0;

	// Steps to the largest contained cell at a higher resolution.
	// Returns false if there is no contained cell.
	virtual bool stepToContained() = 0;

	// Steps to the smallest contained cell at a lower resolution.
	// Returns false if there is no container cell.
	virtual bool stepToContainer() = 0;
};
			
#endif
