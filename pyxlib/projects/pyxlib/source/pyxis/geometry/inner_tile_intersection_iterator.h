#ifndef PYXIS__GEOMETRY__INNER_TILE_INTERSECTION_ITERATOR_H
#define PYXIS__GEOMETRY__INNER_TILE_INTERSECTION_ITERATOR_H
/******************************************************************************
tile_intersection_iterator.h

begin		: 2012-05-14
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/geometry/inner_tile.h"
#include "pyxis/utility/abstract_iterator.h"

enum PYXInnerTileIntersection
{
	knIntersectionNone,
	knIntersectionPartial,
	knIntersectionComplete
};

class PYXLIB_DECL PYXInnerTileIntersectionIterator : public PYXAbstractIterator, public PYXObject
{
public:
	virtual const PYXInnerTile & getTile() const = 0;

	virtual const PYXInnerTileIntersection & getIntersection() const = 0;

public:
	class SingleTileIterator;

public:
	//! Utility function to check if 2 PYXInnerTileIntersectionIterator intersect
	static bool intersects(PYXInnerTileIntersectionIterator & a,PYXInnerTileIntersectionIterator & b);

	//! Find the next tile that has intersection, aka getIntersection() != None
	virtual void nextTileWithIntersection();
};

class PYXInnerTileIntersectionIterator::SingleTileIterator : public PYXInnerTileIntersectionIterator 
{
public:
	static PYXPointer<SingleTileIterator> create(const PYXInnerTile & tile,PYXInnerTileIntersection intersection)
	{
		return PYXNEW(SingleTileIterator,tile,intersection);
	}

	SingleTileIterator(const PYXInnerTile & tile,PYXInnerTileIntersection intersection)
		: m_tile(tile), m_intersection(intersection), m_ended(false)
	{
	}

	virtual const PYXInnerTile & getTile() const { return m_tile; }

	virtual const PYXInnerTileIntersection & getIntersection() const { return m_intersection; }

	virtual bool end() const { return m_ended; }

	virtual void next()
	{
		m_ended = true;
	}

private:
	PYXInnerTile m_tile;
	PYXInnerTileIntersection m_intersection;
	bool m_ended;
};

#endif // guard
