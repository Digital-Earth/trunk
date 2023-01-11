/******************************************************************************
multi_polygon_region.cpp

begin		: 2010-10-04
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "multi_polygon_region.h"
#include "pyxis/derm/index.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/snyder_projection.h"


PYXPointer<IRegion> PYXMultiPolygonRegion::clone() const
{
	return PYXMultiPolygonRegion::create(*this);
}

PYXBoundingCircle PYXMultiPolygonRegion::getBoundingCircle() const
{

	return m_boundingCircle;

}
//! distance in radians
double PYXMultiPolygonRegion::getDistanceToBorder(const PYXCoord3DDouble & location,double errorThreshold) const
{	
	PYXPointer<Visitor> visitor = boost::dynamic_pointer_cast<Visitor>(getVisitor());	
	if (visitor)
	{
		return visitor->getDistanceToBorder(location,errorThreshold);
	}	
	return 0;
}


bool PYXMultiPolygonRegion::isPointContained(const PYXCoord3DDouble & location,double errorThreshold = 0) const
{
	PYXPointer<Visitor> visitor = boost::dynamic_pointer_cast<Visitor>(getVisitor());	
	if (visitor)
	{
		return visitor->isPointContained(location,errorThreshold);
	}
	return false;
}

PYXRegion::CellIntersectionState PYXMultiPolygonRegion::intersects(const PYXIcosIndex & index, bool asTile) const
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

PYXRegion::CellIntersectionState PYXMultiPolygonRegion::intersects(const PYXBoundingCircle & circle,double errorThreshold) const
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

//! get exterior point location - valid only after seting up an exterior ring
const PYXCoord3DDouble & PYXMultiPolygonRegion::getExteriorPoint() const
{
	return m_exteriorPoint;
}

const bool PYXMultiPolygonRegion::getExteriorPointContained() const
{
	return m_exteriorPointContained != 0;
}

PYXMultiPolygonRegion::PYXMultiPolygonRegion()
{

}

//! Copy constuctor
PYXMultiPolygonRegion::PYXMultiPolygonRegion(const PYXMultiPolygonRegion & multiPolygon) : 
	m_boundingCircle(multiPolygon.m_boundingCircle),
	m_arcs(multiPolygon.m_arcs),
	m_exteriorPoint(multiPolygon.m_exteriorPoint),
	m_exteriorPointContained(multiPolygon.m_exteriorPointContained)
{
}


PYXMultiPolygonRegion::PYXMultiPolygonRegion(const std::vector<PYXPointer<PYXCurveRegion>> & regions):
m_boundingCircle(),
m_arcs(regions)
{
	// exterior point
	if (m_exteriorPoint.equal(PYXCoord3DDouble(0,0,0)))
	{
		//set a default - and it will work for small enough polygons (polygons smaller then half of earth)
		setExteriorPoint(regions[0]->getVertex(0).cross(regions[0]->getVertex(1)),false);
	}
	// bounding circle
	for (unsigned int i= 0 ; i<regions.size();i++)
	{
		m_boundingCircle += regions[i]->getBoundingCircle();
	}
}

void PYXMultiPolygonRegion::setExteriorPoint(const PYXCoord3DDouble & point,bool isContained)
{
	m_exteriorPointContained = (int)isContained;
	m_exteriorPoint = point;
	m_exteriorPoint.normalize();
}

PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXMultiPolygonRegion & region)
{
	buffer << region.m_boundingCircle.getCenter();
	buffer << region.m_boundingCircle.getRadius();
	buffer << region.m_arcs;
	return buffer;
}

PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXMultiPolygonRegion> & region)
{
	PYXCoord3DDouble circleCenter;
	double radius;

	region = PYXMultiPolygonRegion::create();

	buffer >> circleCenter;
	buffer >> radius;
	region->m_boundingCircle = PYXBoundingCircle(circleCenter,radius);

	PYXCoord3DDouble farAway = circleCenter;
	farAway.negate();
	region->setExteriorPoint(farAway,false);

	buffer >> region->m_arcs;
	return buffer;
}


PYXPointer<IRegionVisitor> PYXMultiPolygonRegion::getVisitor() const 
{
	return Visitor::create(this);
}

void PYXMultiPolygonRegion::serializeVisitor(PYXWireBuffer & buffer,const PYXPointer<IRegionVisitor> & visitor) const
{
	Visitor * polygonVisitor = dynamic_cast<Visitor*>(visitor.get());

	if (polygonVisitor == NULL)
	{
		PYXTHROW(PYXException,"Unsupported format of visitor");
	}

	polygonVisitor->serialize(buffer);
}
PYXPointer<IRegionVisitor> PYXMultiPolygonRegion::deserializeVisitor(PYXWireBuffer & buffer,const PYXIcosIndex & index) const
{
	return Visitor::create(this,buffer,index);
}

int PYXMultiPolygonRegion::getRingsCount() const
{
	return m_arcs.getSegmentsCount();
}

int PYXMultiPolygonRegion::getRingVerticesCount(unsigned int index) const
{
	return m_arcs.getSegmentSize(index) + 1;
}

PYXCoord3DDouble PYXMultiPolygonRegion::getRingVertex(unsigned int ringIndex,unsigned int vertexIndex) const
{
	return m_arcs.getSegmentVertex(ringIndex,vertexIndex);
}

//VISITOR

///////////////////////////////////////////////////////////////////
// PYXMultiPolygonRegion::Visitor class
///////////////////////////////////////////////////////////////////

PYXMultiPolygonRegion::Visitor::Visitor(const PYXPointer<const PYXMultiPolygonRegion> & polygon) : 
m_polygon(polygon), 
m_rayOrigin (polygon->getExteriorPoint()),
m_rayOriginContained (polygon->getExteriorPointContained() ? 1 : 0)
{
	m_chunks.push_back(Range<int>::createClosedClosed(0,m_polygon->m_arcs.size()-1));
}

PYXMultiPolygonRegion::Visitor::Visitor(const PYXPointer<const PYXMultiPolygonRegion> & polygon, std::vector<Range<int>> & chunks ,const PYXCoord3DDouble & rayOrigin, bool isOriginContained) : m_polygon(polygon), m_rayOrigin(rayOrigin), m_rayOriginContained(isOriginContained?1:0)
{
	std::swap(m_chunks,chunks);
}


PYXMultiPolygonRegion::Visitor::Visitor(const Visitor & other) : m_polygon(other.m_polygon), m_chunks(other.m_chunks), m_rayOrigin(other.m_rayOrigin), m_rayOriginContained(other.m_rayOriginContained)
{
}

PYXMultiPolygonRegion::Visitor::Visitor(const PYXPointer<const PYXMultiPolygonRegion> & polygon, PYXWireBuffer & buffer,const PYXIcosIndex & index) : m_polygon(polygon)
{
	buffer >> PYXCompactInteger(m_rayOriginContained);
	assert((m_rayOriginContained==0||m_rayOriginContained==1)&&"Problem reading ray origin");

	if (!index.isNull())
	{
		SnyderProjection::getInstance()->pyxisToXYZ(index, &m_rayOrigin);
	}
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

void PYXMultiPolygonRegion::Visitor::serialize(PYXWireBuffer & buffer)
{
	assert(isOptimal());
	buffer << PYXCompactInteger(m_rayOriginContained);

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

int PYXMultiPolygonRegion::Visitor::getChunksSize() const
{
	int count = 0;
	for(unsigned int i=0;i<m_chunks.size();++i)
	{
		const Range<int> & range = m_chunks[i];
		count += range.max - range.min + 1;
	}
	return count;
}

bool PYXMultiPolygonRegion::Visitor::isOptimal() const
{
	return getChunksSize() < knOptimalSize;
}

PYXPointer<IRegionVisitor> PYXMultiPolygonRegion::Visitor::trim(const PYXIcosIndex & index) const
{
	std::vector<Range<int>> new_chunks;

	double distanceThreshold = PYXIcosMath::UnitSphere::calcCellCircumRadius(index);

	PYXCoord3DDouble cellCenter;
	SnyderProjection::getInstance()->pyxisToXYZ(index, &cellCenter);

	PYXBoundingCircle cellCircle (cellCenter,distanceThreshold);

	if(!m_polygon->getBoundingCircle().intersects(cellCircle))
	{
		//"MulltiPolygon Trim ended, not even close"
		return 0;
	}
	m_polygon->m_arcs.findIntersectingArcs(m_chunks,cellCircle,new_chunks);

	if (new_chunks.empty())
	{
		if(isPointContained(cellCenter))
		{
			return PYXCompletelyInsideVisitor::create();
		}
		return 0;
	}
	else 
	{
		return PYXNEW(Visitor,m_polygon,new_chunks,cellCenter,isPointContained(cellCenter));
	}
}

PYXPointer<PYXInnerTileIntersectionIterator> PYXMultiPolygonRegion::Visitor::getInnerTileIterator(const PYXInnerTile & tile) const
{
	return Iterator::create(this,tile);
}


PYXInnerTileIntersection PYXMultiPolygonRegion::Visitor::intersects(const PYXBoundingCircle & circle,double errorThreshold) const
{
	for(unsigned int i=0;i<m_chunks.size();++i)
	{
		const Range<int> & range = m_chunks[i];

		for(int arcIndex=range.min;arcIndex<=range.max;++arcIndex)
		{
			const SphereMath::GreatCircleArc & arc = m_polygon->m_arcs.getArc(arcIndex);

			if (arc.distanceTo(circle.getCenter()) < circle.getRadius())
			{
				return knIntersectionPartial;
			}			
		}
	}

	if (isPointContained(circle.getCenter()))
	{
		return knIntersectionComplete;
	}

	return knIntersectionNone;
}

PYXInnerTileIntersection PYXMultiPolygonRegion::Visitor::intersects(const PYXIcosIndex & cell) const
{
	double distanceThreshold = PYXIcosMath::UnitSphere::calcCellCircumRadius(cell);

	PYXCoord3DDouble cellCenter;
	SnyderProjection::getInstance()->pyxisToXYZ(cell, &cellCenter);

	return intersects(PYXBoundingCircle(cellCenter,distanceThreshold));	
}

double PYXMultiPolygonRegion::Visitor::getDistanceToBorder( const PYXCoord3DDouble & location,double errorThreshold /*= 0*/ ) const
{	
	return m_polygon->m_arcs.getDistanceTo(m_chunks,location);	
}

bool PYXMultiPolygonRegion::Visitor::isPointContained( const PYXCoord3DDouble & location,double errorThreshold /*= 0*/ ) const
{
	assert((m_rayOriginContained==1 ||m_rayOriginContained==0) && "Invalid RayOriginContained");
	if (location.equal( m_rayOrigin))
	{
		return m_rayOriginContained == 1;
	}

	SphereMath::GreatCircleArc ray (m_rayOrigin,location);
	int intersectionCount= m_rayOriginContained + m_polygon->m_arcs.countIntersectionWithRay(m_chunks,ray);

	return intersectionCount%2==1;
}

///////////////////////////////////////////////////////////////////
// PYXMultiPolygonRegion::Visitor::Iterator class
///////////////////////////////////////////////////////////////////

PYXMultiPolygonRegion::Visitor::Iterator::Iterator(const PYXPointer<const PYXMultiPolygonRegion::Visitor> & visitor,const PYXInnerTile & tile) : m_visitor(visitor), m_tile(tile)
{
	m_arcs.reserve(m_visitor->getChunksSize());
	for(unsigned int i=0;i<m_visitor->m_chunks.size();++i)
	{
		const Range<int> & range = m_visitor->m_chunks[i];
		m_visitor->m_polygon->m_arcs.getArcs(range.min,range.max+1,m_arcs);
	}

	m_index = m_tile.getRootIndex();
	findIntersection();
}

void PYXMultiPolygonRegion::Visitor::Iterator::findIntersection()
{
	PYXCoord3DDouble cellCenter;
	SnyderProjection::getInstance()->pyxisToXYZ(m_index, &cellCenter);

	double minDistance = 10000;

	for(unsigned int i=0; i <m_arcs.size(); ++i)
	{
		double distance = m_arcs[i].distanceTo(cellCenter);
		if (minDistance>distance)
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
		if (isPointContained(cellCenter))
		{
			m_currentIntersection = knIntersectionComplete;
		}
		else 
		{
			m_currentIntersection = knIntersectionNone;
		}
	}
	else 
	{
		m_currentIntersection = knIntersectionPartial;
	}
}

bool PYXMultiPolygonRegion::Visitor::Iterator::isPointContained(const PYXCoord3DDouble & location) const
{
	if (location.equal(m_visitor->m_rayOrigin))
	{
		return m_visitor->m_rayOriginContained == 1;
	}

	int intersectionCount = m_visitor->m_rayOriginContained;
	SphereMath::GreatCircleArc ray (m_visitor->m_rayOrigin,location);
	for(unsigned int i=0; i <m_arcs.size(); ++i)
	{
		if(ray.intersects(m_arcs[i]))
		{
			intersectionCount++;
		}
	}

	return intersectionCount%2==1;
}

void PYXMultiPolygonRegion::Visitor::Iterator::findNextInnerTile(PYXIcosIndex & index)
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

const PYXInnerTile & PYXMultiPolygonRegion::Visitor::Iterator::getTile() const
{
	return m_currentTile;
}

const PYXInnerTileIntersection & PYXMultiPolygonRegion::Visitor::Iterator::getIntersection() const
{
	return m_currentIntersection;
}

bool PYXMultiPolygonRegion::Visitor::Iterator::end() const
{
	return m_index.isNull();
}

void PYXMultiPolygonRegion::Visitor::Iterator::next()
{
	findNextInnerTile(m_index);
	if (!m_index.isNull())
	{
		findIntersection();
	}
}


