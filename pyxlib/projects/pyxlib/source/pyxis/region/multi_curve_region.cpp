/******************************************************************************
multi_curve_region.cpp

begin		: 2010-10-04
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "multi_curve_region.h"

#include "pyxis/derm/index.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/snyder_projection.h"


PYXPointer<IRegion> PYXMultiCurveRegion::clone() const
{
	return PYXMultiCurveRegion::create(*this);
}

PYXBoundingCircle PYXMultiCurveRegion::getBoundingCircle() const
{
	return m_boundingCircle;
}

//! distance in radians
double PYXMultiCurveRegion::getDistanceToBorder(const PYXCoord3DDouble & location,double errorThreshold) const
{
	return m_arcs.getDistanceTo(location);	
}


bool PYXMultiCurveRegion::isPointContained(const PYXCoord3DDouble & location,double errorThreshold = 0) const
{
	return false;
}

PYXRegion::CellIntersectionState PYXMultiCurveRegion::intersects(const PYXIcosIndex & index, bool asTile) const
{
	switch(getVisitor()->intersects(index))
	{
	case knIntersectionComplete:
		return PYXRegion::knComplete;
	case knIntersectionPartial:
		return PYXRegion::knPartial;
	case knIntersectionNone:
		return PYXRegion::knNone;
	default:
		PYXTHROW(PYXException,"Unexpected value for PYXRegion::CellIntersectionState");
	}
}

PYXRegion::CellIntersectionState PYXMultiCurveRegion::intersects(const PYXBoundingCircle & circle,double errorThreshold) const
{
	switch(getVisitor()->intersects(circle,errorThreshold))
	{
	case knIntersectionComplete:
		return PYXRegion::knComplete;
	case knIntersectionPartial:
		return PYXRegion::knPartial;
	case knIntersectionNone:
		return PYXRegion::knNone;
	default:
		PYXTHROW(PYXException,"Unexpected value for PYXRegion::CellIntersectionState");
	}
}

PYXMultiCurveRegion::PYXMultiCurveRegion()
{

}

//! Copy constuctor
PYXMultiCurveRegion::PYXMultiCurveRegion(const PYXMultiCurveRegion & multicurve) : m_boundingCircle(multicurve.m_boundingCircle), m_arcs(multicurve.m_arcs)
{
}


PYXMultiCurveRegion::PYXMultiCurveRegion(const std::vector<PYXPointer<PYXCurveRegion>> & regions):
	m_boundingCircle(),
	m_arcs(regions)
{
	for (std::vector<PYXPointer<PYXCurveRegion>>::const_iterator curve = regions.begin(); curve != regions.end(); ++curve)
	{
		m_boundingCircle += (*curve)->getBoundingCircle();
	}
}

PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXMultiCurveRegion & region)
{
	buffer << region.m_arcs;
	return buffer;
}

PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXMultiCurveRegion> & region)
{
	region = PYXMultiCurveRegion::create();
	buffer >> region->m_arcs;
	region->m_boundingCircle = region->m_arcs.getBoundingCircle();
	return buffer;
}

PYXPointer<IRegionVisitor> PYXMultiCurveRegion::getVisitor() const 
{
	return Visitor::create(this);
}

void PYXMultiCurveRegion::serializeVisitor(PYXWireBuffer & buffer,const PYXPointer<IRegionVisitor> & visitor) const
{
	Visitor * curveVisitor = dynamic_cast<Visitor*>(visitor.get());

	if (curveVisitor == NULL)
	{
		PYXTHROW(PYXException,"Unsupported format of visitor");
	}

	curveVisitor->serialize(buffer);
}

PYXPointer<IRegionVisitor> PYXMultiCurveRegion::deserializeVisitor(PYXWireBuffer & buffer) const
{
	return Visitor::create(this,buffer);
}

int PYXMultiCurveRegion::getCurveCount() const
{
	return m_arcs.getSegmentsCount();
}

int PYXMultiCurveRegion::getCurveVerticesCount(unsigned int index) const
{
	return m_arcs.getSegmentSize(index) + 1;
}

PYXCoord3DDouble PYXMultiCurveRegion::getCurveVertex(unsigned int curveIndex,unsigned int vertexIndex) const
{
	return m_arcs.getSegmentVertex(curveIndex,vertexIndex);
}

//VISITOR

///////////////////////////////////////////////////////////////////
// PYXMultiCurveRegion::Visitor class
///////////////////////////////////////////////////////////////////

PYXMultiCurveRegion::Visitor::Visitor(const PYXPointer<const PYXMultiCurveRegion> & curve) : 
	m_curve(curve)
{
	m_chunks.push_back(Range<int>::createClosedClosed(0,m_curve->m_arcs.size()-1));
}

PYXMultiCurveRegion::Visitor::Visitor(const PYXPointer<const PYXMultiCurveRegion> & curve, std::vector<Range<int>> & chunks) : m_curve(curve)
{
	std::swap(m_chunks,chunks);
}

PYXMultiCurveRegion::Visitor::Visitor(const Visitor & other) : m_curve(other.m_curve), m_chunks(other.m_chunks)
{
}

PYXMultiCurveRegion::Visitor::Visitor(const PYXPointer<const PYXMultiCurveRegion> & curve, PYXWireBuffer & buffer) : m_curve(curve)
{

	int count = 0;
	int offset = 0;
	buffer >> PYXCompactInteger(count);
	for(int i=0;i<count;i++)
	{
		int min,max,delta;
		buffer >> PYXCompactInteger(delta); offset += delta;
		min = offset;
		buffer >> PYXCompactInteger(delta); offset += delta;
		max = offset;
		m_chunks.push_back(RangeInt::createClosedClosed(min,max));
	}
}

void PYXMultiCurveRegion::Visitor::serialize(PYXWireBuffer & buffer)
{

	int count = (int)m_chunks.size();
	buffer << PYXCompactInteger(count);
	int offset = 0;
	for(int i=0;i<count;i++)
	{
		Range<int> & range = m_chunks[i];

		int delta = (range.min-offset);
		buffer << PYXCompactInteger(delta);
		delta = (range.max-range.min);
		buffer << PYXCompactInteger(delta);

		offset = range.max;
	}
}

int PYXMultiCurveRegion::Visitor::getChunksSize() const
{
	int count = 0;
	for(unsigned int i=0;i<m_chunks.size();++i)
	{
		const Range<int> & range = m_chunks[i];
		count += range.max - range.min + 1;
	}
	return count;
}

bool PYXMultiCurveRegion::Visitor::isOptimal() const
{
	return getChunksSize() < knOptimalSize;
}

PYXPointer<IRegionVisitor> PYXMultiCurveRegion::Visitor::trim(const PYXIcosIndex & index) const
{
	std::vector<Range<int>> new_chunks;

	double distanceThreshold = PYXIcosMath::UnitSphere::calcCellCircumRadius(index);

	PYXCoord3DDouble cellCenter;
	SnyderProjection::getInstance()->pyxisToXYZ(index, &cellCenter);
	PYXBoundingCircle cellCircle (cellCenter,distanceThreshold);

	if (!m_curve->getBoundingCircle().intersects(cellCircle))
	{
		return 0;
	}
	else if (m_curve->getBoundingCircle().getRadius()*4 < distanceThreshold)
	{
		new_chunks = m_chunks;
		return PYXNEW(Visitor,m_curve,new_chunks);
	}

	m_curve->m_arcs.findIntersectingArcs(m_chunks,cellCircle,new_chunks);

	if (new_chunks.empty())
	{
		return 0;
	}
	else 
	{
		return PYXNEW(Visitor,m_curve,new_chunks);
	}
}

PYXPointer<PYXInnerTileIntersectionIterator> PYXMultiCurveRegion::Visitor::getInnerTileIterator(const PYXInnerTile & tile) const
{
	return Iterator::create(this,tile);
}

PYXInnerTileIntersection PYXMultiCurveRegion::Visitor::intersects(const PYXBoundingCircle & circle,double errorThreshold) const
{
	for(unsigned int i=0;i<m_chunks.size();++i)
	{
		const Range<int> & range = m_chunks[i];

		for(int arcIndex=range.min;arcIndex<=range.max;++arcIndex)
		{
			const SphereMath::GreatCircleArc & arc = m_curve->m_arcs.getArc(arcIndex);

			if (arc.distanceTo(circle.getCenter()) < circle.getRadius())
			{
				return knIntersectionPartial;
			}
		}
	}

	return knIntersectionNone;
}

PYXInnerTileIntersection PYXMultiCurveRegion::Visitor::intersects(const PYXIcosIndex & cell) const
{
	double distanceThreshold = PYXIcosMath::UnitSphere::calcCellCircumRadius(cell);

	PYXCoord3DDouble cellCenter;
	SnyderProjection::getInstance()->pyxisToXYZ(cell, &cellCenter);

	return intersects(PYXBoundingCircle(cellCenter,distanceThreshold));	
}

bool PYXMultiCurveRegion::Visitor::isPointContained( const PYXCoord3DDouble & location,double errorThreshold /*= 0*/ ) const
{
	return false;
}
///////////////////////////////////////////////////////////////////
// PYXMultiCurveRegion::Visitor::Iterator class
///////////////////////////////////////////////////////////////////

PYXMultiCurveRegion::Visitor::Iterator::Iterator(const PYXPointer<const PYXMultiCurveRegion::Visitor> & visitor,const PYXInnerTile & tile) : m_visitor(visitor), m_tile(tile)
{
	m_arcs.reserve(m_visitor->getChunksSize());
	for(unsigned int i=0;i<m_visitor->m_chunks.size();++i)
	{
		const Range<int> & range = m_visitor->m_chunks[i];
		m_visitor->m_curve->m_arcs.getArcs(range.min,range.max+1,m_arcs);
	}

	m_index = m_tile.getRootIndex();

	findIntersection();
}

void PYXMultiCurveRegion::Visitor::Iterator::findIntersection()
{
	PYXCoord3DDouble cellCenter;
	SnyderProjection::getInstance()->pyxisToXYZ(m_index, &cellCenter);

	double minDistance = 10000;

	for(unsigned int i=0; i <m_arcs.size(); ++i)
	{
		double distance = m_arcs[i].distanceTo(cellCenter);
		if (minDistance > distance)
		{
			minDistance = distance;
		}
	}

	//find the largest tile that have a singular answer (complete outside in this case)
	while (PYXIcosMath::UnitSphere::calcCellCircumRadius(m_index) >= minDistance && m_index.getResolution() < m_tile.getCellResolution())
	{
		m_index.incrementResolution();
	}

	m_currentTile = PYXInnerTile(m_index,m_tile.getCellResolution());

	if (PYXIcosMath::UnitSphere::calcCellCircumRadius(m_index) < minDistance)
	{
		
			m_currentIntersection = knIntersectionNone;
		
	}
	else 
	{
		m_currentIntersection = knIntersectionPartial;
	}
}

void PYXMultiCurveRegion::Visitor::Iterator::findNextInnerTile(PYXIcosIndex & index)
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

const PYXInnerTile & PYXMultiCurveRegion::Visitor::Iterator::getTile() const
{
	return m_currentTile;
}

const PYXInnerTileIntersection & PYXMultiCurveRegion::Visitor::Iterator::getIntersection() const
{
	return m_currentIntersection;
}

bool PYXMultiCurveRegion::Visitor::Iterator::end() const
{
	return m_index.isNull();
}

void PYXMultiCurveRegion::Visitor::Iterator::next()
{
	findNextInnerTile(m_index);
	if (!m_index.isNull())
	{
		findIntersection();
	}
}