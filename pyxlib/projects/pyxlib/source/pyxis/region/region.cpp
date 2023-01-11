/******************************************************************************
region.cpp

begin		: 2010-11-15
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "region.h"

#include "pyxis/geometry/inner_tile.h"

#include "pyxis/derm/index.h"
#include "pyxis/derm/iterator.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/snyder_projection.h"

#include "float.h"

////////////////////////////////////////////////////////////////////////////////
// PYXVectorRegion default implementation
////////////////////////////////////////////////////////////////////////////////

PYXRegion::CellIntersectionState PYXVectorRegion::intersects(const PYXIcosIndex & index, bool asTile) const
{
	double threshold = (asTile ?
		PYXIcosMath::UnitSphere::calcTileCircumRadius(index) :
		PYXIcosMath::UnitSphere::calcCellCircumRadius(index));

	PYXCoord3DDouble location;
	SnyderProjection::getInstance()->pyxisToXYZ(index, &location);

	return intersects(PYXBoundingCircle(location,threshold),threshold/10);
}

PYXRegion::CellIntersectionState PYXVectorRegion::intersects(const PYXBoundingCircle & circle,double errorThreshold) const
{
	if (getDistanceToBorder(circle.getCenter(), errorThreshold) <= circle.getRadius())
	{
		return PYXRegion::knPartial;
	}
	if (isPointContained(circle.getCenter(), errorThreshold))
	{
		return PYXRegion::knComplete;
	}
	return PYXRegion::knNone;
}

///////////////////////////////////////////////////////////////////////////////
// PYXVectorRegionVisitor default implementation
////////////////////////////////////////////////////////////////////////////////


PYXVectorRegionVisitor::PYXVectorRegionVisitor(const PYXPointer<const PYXVectorRegion> & region) : m_region(region)
{
}

PYXPointer<IRegionVisitor> PYXVectorRegionVisitor::trim(const PYXIcosIndex & index) const
{
	switch(m_region->intersects(index,false))
	{
	case PYXRegion::knNone:
		return 0;
	case PYXRegion::knComplete:
		return PYXCompletelyInsideVisitor::create();
	case PYXRegion::knPartial:
		return const_cast<PYXVectorRegionVisitor*>(this);
	default:
		PYXTHROW(PYXException,"Unexpected value for PYXRegion::CellIntersectionState");
	}
}

PYXPointer<PYXInnerTileIntersectionIterator> PYXVectorRegionVisitor::getInnerTileIterator(const PYXInnerTile & tile) const
{
	switch(m_region->intersects(tile.getRootIndex(),false))
	{
	case PYXRegion::knNone:
		return PYXInnerTileIntersectionIterator::SingleTileIterator::create(tile,knIntersectionNone);
	case PYXRegion::knComplete:
		return PYXInnerTileIntersectionIterator::SingleTileIterator::create(tile,knIntersectionComplete);
	case PYXRegion::knPartial:
		return Iterator::create(m_region,tile);
	default:
		PYXTHROW(PYXException,"Unexpected value for PYXRegion::CellIntersectionState");
	}
}

PYXInnerTileIntersection PYXVectorRegionVisitor::intersects(const PYXBoundingCircle & circle,double errorThreshold) const
{
	switch(m_region->intersects(circle,errorThreshold))
	{
	case PYXRegion::knNone:
		return knIntersectionNone;
	case PYXRegion::knComplete:
		return knIntersectionComplete;
	case PYXRegion::knPartial:
		return knIntersectionPartial;
	default:
		PYXTHROW(PYXException,"Unexpected value for PYXRegion::CellIntersectionState");
	}
}

PYXInnerTileIntersection PYXVectorRegionVisitor::intersects(const PYXIcosIndex & cell) const
{
	switch(m_region->intersects(cell,false))
	{
	case PYXRegion::knNone:
		return knIntersectionNone;
	case PYXRegion::knComplete:
		return knIntersectionComplete;
	case PYXRegion::knPartial:
		return knIntersectionPartial;
	default:
		PYXTHROW(PYXException,"Unexpected value for PYXRegion::CellIntersectionState");
	}
}

PYXVectorRegionVisitor::Iterator::Iterator(const PYXPointer<const PYXVectorRegion> & region,const PYXInnerTile & tile) : m_region(region), m_tile(tile)
{
	m_index = m_tile.getRootIndex();

	findIntersection();
}

const PYXInnerTile & PYXVectorRegionVisitor::Iterator::getTile() const 
{
	return m_currentTile;
}

const PYXInnerTileIntersection & PYXVectorRegionVisitor::Iterator::getIntersection() const
{
	return m_currentIntersection;
}

bool PYXVectorRegionVisitor::Iterator::end() const
{
	return m_index.isNull();
}

void PYXVectorRegionVisitor::Iterator::next()
{
	findNextInnerTile(m_index);
	if (!m_index.isNull())
	{
		findIntersection();
	}
}

void PYXVectorRegionVisitor::Iterator::findIntersection()
{
	PYXCoord3DDouble cellCenter;
	SnyderProjection::getInstance()->pyxisToXYZ(m_index, &cellCenter);

	double minDistance = m_region->getDistanceToBorder(cellCenter);

	//find the largest tile that have a singular answer (complete outside in this case)
	while (PYXIcosMath::UnitSphere::calcCellCircumRadius(m_index) >= minDistance && m_index.getResolution() < m_tile.getCellResolution())
	{
		m_index.incrementResolution();
	}

	m_currentTile = PYXInnerTile(m_index,m_tile.getCellResolution());
	if (PYXIcosMath::UnitSphere::calcCellCircumRadius(m_index) < minDistance )
	{
		m_currentIntersection = m_region->isPointContained(cellCenter) ? knIntersectionComplete : knIntersectionNone;
	}
	else 
	{
		m_currentIntersection = knIntersectionPartial;
	}
}

void PYXVectorRegionVisitor::Iterator::findNextInnerTile(PYXIcosIndex & index)
{
	PYXIndex & subIndex = index.getSubIndex();

	if (subIndex.getDigitCount()==0)
	{
		index.reset();
		return;
	}

	unsigned int lastDigit = subIndex.getLastDigit();

	if (lastDigit == 0)
	{		
		if (m_tile.getCellResolution()==index.getResolution())
		{
			subIndex.stripRight();
			findNextInnerTile(index);
			return;
		}
		else if (index.isPentagon())
		{
			if (index.isNorthern())
			{
				subIndex.appendDigit(2);
			}
			else
			{
				subIndex.appendDigit(1);
			}
		}
		else
		{
			subIndex.appendDigit(1);
		}
	}
	else if (lastDigit < 6)
	{
		subIndex.stripRight();
		lastDigit++;
		PYXMath::eHexDirection gap = PYXMath::knDirectionZero;
		if (PYXIcosMath::getCellGap(index, &gap))
		{
			if (lastDigit == gap)
			{
				lastDigit++;
			}
		}

		subIndex.appendDigit(lastDigit);
	}
	else
	{
		if (subIndex.getDigitCount()<2)
		{
			index.reset();
			return;
		}
		else 
		{
			subIndex.stripRight();
			subIndex.stripRight();
			findNextInnerTile(index);
			return;
		}
	}
	if (!m_tile.asTile().getRootIndex().isAncestorOf(m_index))
	{
		index.reset();
	}
}

///////////////////////////////////////////////////////////////////////////////
// PYXVectorBufferRegion default implementation
////////////////////////////////////////////////////////////////////////////////

PYXVectorBufferRegion::PYXVectorBufferRegion(const PYXPointer<PYXVectorRegion> & region,double bufferRadius)
	:	m_region(region),
		m_bufferRadius(bufferRadius)
{
	PYXVectorBufferRegion * bufferRegion = dynamic_cast<PYXVectorBufferRegion*>(m_region.get());
	if (bufferRegion != 0)
	{
		m_region = bufferRegion->m_region;
		m_bufferRadius += bufferRegion->m_bufferRadius;
	}
}

PYXVectorBufferRegion::PYXVectorBufferRegion(const PYXPointer<PYXVectorBufferRegion> & region,double bufferRadius)
	:	m_region(region->m_region),
		m_bufferRadius(bufferRadius + region->m_bufferRadius)
{
}

PYXPointer<IRegion> PYXVectorBufferRegion::clone() const
{
	return PYXVectorBufferRegion::create(m_region,m_bufferRadius);
}

double PYXVectorBufferRegion::getDistanceToBorder(const PYXCoord3DDouble & location,double errorThreshold) const
{
	double distance = m_region->getDistanceToBorder(location,errorThreshold);

	if (m_region->isPointContained(location,errorThreshold))
	{
		//we are inisde the region - so the buffer is away from the location.
		return abs(distance+m_bufferRadius);
	}
	else
	{
		//we are outside the region - so the buffer is toward the location.
		return abs(distance-m_bufferRadius);
	}
}

bool PYXVectorBufferRegion::isPointContained(const PYXCoord3DDouble & location,double errorThreshold) const
{
	if (m_region->isPointContained(location,errorThreshold))
	{
		if (m_bufferRadius > 0)
		{
			//positive buffer - the location must be containted
			return true;
		}
		//we are inisde the region - so the buffer is away from the location.
		return m_region->getDistanceToBorder(location,errorThreshold) + m_bufferRadius > 0;
	}
	else
	{
		//we are outside the region - so the buffer is toward the location.
		if (m_bufferRadius < 0)
		{
			//negative buffer - the location must be outside the region
			return false;
		}

		return m_region->getDistanceToBorder(location,errorThreshold) - m_bufferRadius < 0;
	}
}

PYXBoundingCircle PYXVectorBufferRegion::getBoundingCircle() const
{
	PYXBoundingCircle circle = m_region->getBoundingCircle();

	return PYXBoundingCircle(circle.getCenter(),circle.getRadius()+m_bufferRadius);
}

////////////////////////////////////////////////////////////////////////////////
// PYXCollectionVectorRegion default implementation
////////////////////////////////////////////////////////////////////////////////

PYXCollectionVectorRegion::PYXCollectionVectorRegion(const std::vector<PYXPointer<PYXVectorRegion>> & regions)
	: m_regions(regions)
{
	findBoundingCircle();
}

PYXPointer<IRegion> PYXCollectionVectorRegion::clone() const
{
	return PYXCollectionVectorRegion::create(m_regions);
}

double PYXCollectionVectorRegion::getDistanceToBorder(const PYXCoord3DDouble & location,double errorThreshold) const
{
	if (m_regions.size()==0)
	{
		PYXTHROW(PYXException,"PYXCollectionVectorRegions empty");
	}

	std::vector<PYXPointer<PYXVectorRegion>>::const_iterator it = m_regions.begin();

	double minDistance = (**it).getDistanceToBorder(location,errorThreshold);

	for(++it;it != m_regions.end(); ++it)
	{
		double distance = (**it).getDistanceToBorder(location,errorThreshold);

		if (minDistance > distance)
		{
			minDistance = distance;
		}
	}

	return minDistance;
}

bool PYXCollectionVectorRegion::isPointContained(const PYXCoord3DDouble & location,double errorThreshold) const
{
	for(std::vector<PYXPointer<PYXVectorRegion>>::const_iterator it = m_regions.begin();it != m_regions.end(); ++it)
	{
		if ((**it).isPointContained(location,errorThreshold))
		{
			return true;
		}
	}

	return false;
}

PYXBoundingCircle PYXCollectionVectorRegion::getBoundingCircle() const
{
	return m_boundingCircle;
}

void PYXCollectionVectorRegion::findBoundingCircle()
{
	m_boundingCircle = PYXBoundingCircle();

	for(std::vector<PYXPointer<PYXVectorRegion>>::iterator it = m_regions.begin();it != m_regions.end(); ++it)
	{
		m_boundingCircle += (*it)->getBoundingCircle();
	}
}