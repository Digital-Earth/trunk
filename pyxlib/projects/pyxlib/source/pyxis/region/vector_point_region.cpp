/******************************************************************************
vector_point_region.cpp

begin		: 2010-11-18
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "vector_point_region.h"
#include "region_serializer.h"

#include "pyxis/derm/index.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/neighbour_iterator.h"

PYXPointer<IRegion> PYXVectorPointRegion::clone() const
{
	return PYXVectorPointRegion::create(*this);
}

//! distance in radians
double PYXVectorPointRegion::getDistanceToBorder(const PYXCoord3DDouble & location,double errorThreshold) const
{
	return SphereMath::distanceBetween(m_point,location);
}

PYXBoundingCircle PYXVectorPointRegion::getBoundingCircle() const
{
	return PYXBoundingCircle(m_point,0);
}

//! Constructs a point region from a PYXCoord3DDouble 
PYXVectorPointRegion::PYXVectorPointRegion(PYXCoord3DDouble const & point) : m_point(point)
{
	m_point.normalize();
}

PYXPointer<IRegionVisitor> PYXVectorPointRegion::getVisitor() const
{
	return Visitor::create(this);
}

void PYXVectorPointRegion::serializeVisitor(PYXWireBuffer & buffer,const PYXPointer<IRegionVisitor> & visitor) const
{
	//do nothing...
}

PYXPointer<IRegionVisitor> PYXVectorPointRegion::deserializeVisitor(PYXWireBuffer & buffer) const
{
	//just generate a visitor...
	return getVisitor();
}


PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXVectorPointRegion & region)
{
	buffer << region.getPoint();
	return buffer;
}

PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXVectorPointRegion> & region)
{
	PYXCoord3DDouble point;
	buffer >> point;
	region = PYXVectorPointRegion::create(point);
	return buffer;
}

PYXVectorPointRegion::Visitor::Visitor(const PYXPointer<const PYXVectorPointRegion> & point) : m_point(point)
{
}

PYXVectorPointRegion::Visitor::Visitor(const PYXPointer<const PYXVectorPointRegion> & point, PYXWireBuffer & buffer) : m_point(point)
{
}

PYXVectorPointRegion::Visitor::Visitor(const Visitor & other) : m_point(other.m_point)
{
}


PYXPointer<PYXInnerTileIntersectionIterator> PYXVectorPointRegion::Visitor::getInnerTileIterator(const PYXInnerTile & tile) const
{
	return Iterator::create(this,tile);
}

PYXVectorPointRegion::Visitor::Iterator::Iterator(const PYXPointer<const PYXVectorPointRegion::Visitor> & visitor,const PYXInnerTile & tile) : m_visitor(visitor), m_tile(tile)
{
	PYXIcosIndex deepIndex;

	//find deep resolution index (max resolution - 2 to avoid PYXMath::overflow)
	SnyderProjection::getInstance()->xyzToPYXIS(m_visitor->m_point->m_point,&deepIndex, PYXMath::knMaxAbsResolution - 2);
	deepIndex.setResolution(tile.getCellResolution());
	if (tile.hasIndex(deepIndex))
	{
		m_candidates.push_back(deepIndex);
	}

	PYXIcosIndex index;
	SnyderProjection::getInstance()->xyzToPYXIS(m_visitor->m_point->m_point,&index, tile.getCellResolution());
	if (index != deepIndex && tile.hasIndex(index))
	{
		m_candidates.push_back(index);

		if (m_candidates.size()>1) 
		{
			if (m_candidates[1] < m_candidates[0]) {
				std::swap(m_candidates[0],m_candidates[1]);
			}
		}
	}

	if (m_candidates.size()>0)
	{		
		m_candidateIndex = 0;
		m_currentTile = PYXInnerTile(m_candidates[ m_candidateIndex ], m_tile.getCellResolution());
		m_currentIntersection = knIntersectionPartial;
	}
	else 
	{
		//dummey index...
		m_candidates.push_back(PYXIcosIndex());
		m_candidateIndex = 0;
		m_currentTile = m_tile;
		m_currentIntersection = knIntersectionNone;
	}
}

bool PYXVectorPointRegion::Visitor::Iterator::end() const
{
	return m_candidateIndex == m_candidates.size();
}

void PYXVectorPointRegion::Visitor::Iterator::next()
{
	if (end())
	{
		return;
	}
	m_candidateIndex++;

	if (!end())
	{
		m_currentTile = PYXInnerTile(m_candidates[ m_candidateIndex ], m_tile.getCellResolution());
		m_currentIntersection = knIntersectionPartial;
	}
}
