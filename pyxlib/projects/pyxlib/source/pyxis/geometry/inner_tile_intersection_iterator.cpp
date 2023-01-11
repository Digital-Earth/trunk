/******************************************************************************
inner_tile_intersection_iterator.cpp

begin		: 2012-05-14
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/inner_tile_intersection_iterator.h"


void PYXInnerTileIntersectionIterator::nextTileWithIntersection()
{
	//first move to next
	next();

	//continue to iterate until we find an interesting tile.
	while(!end() && getIntersection() == PYXInnerTileIntersection::knIntersectionNone)
	{
		next();
	}
}

//! Utility function to check if 2 PYXInnerTileIntersectionIterator intersect
bool PYXInnerTileIntersectionIterator::intersects(PYXInnerTileIntersectionIterator & a,PYXInnerTileIntersectionIterator & b) {
	while (!a.end() && !b.end()) 
	{
		if (a.getIntersection() != PYXInnerTileIntersection::knIntersectionNone &&
			b.getIntersection() != PYXInnerTileIntersection::knIntersectionNone)
		{
			if (a.getTile().intersects(b.getTile())) {
				return true;
			}
		}
		if (a.getTile().getRootIndex() < b.getTile().getRootIndex())
		{
			a.nextTileWithIntersection();
		}
		else if (b.getTile().getRootIndex() < a.getTile().getRootIndex())
		{
			b.nextTileWithIntersection();
		}
		else //same tile
		{
			a.nextTileWithIntersection();
			b.nextTileWithIntersection();
		}
	}
	return false;
}