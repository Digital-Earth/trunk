/******************************************************************************
curve_region.cpp

begin		: 2010-10-18
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "curve_region.h"
#include "region_serializer.h"

#include "pyxis/derm/index.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/snyder_projection.h"

#include "pyxis/utility/bounding_shape.h"
#include "pyxis/utility/tester.h"

///////////////////////////////////////////////////////////////////
// PYXCurveRegion::CurveChunk class
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// PYXCurveRegion class
///////////////////////////////////////////////////////////////////

Tester<PYXCurveRegion> gTester;

void PYXCurveRegion::test()
{
	//[idans] disabled until distanceToBorder for curve would be implemented
	/*
	std::vector<PYXCoord3DDouble> vertices;

	PYXCoord3DDouble coord;
	SnyderProjection::getInstance()->pyxisToXYZ(PYXIcosIndex("1-0000"),&coord);
	vertices.push_back(coord);
	SnyderProjection::getInstance()->pyxisToXYZ(PYXIcosIndex("1-04030205"),&coord);
	vertices.push_back(coord);

	PYXPointer<PYXCurveRegion> curve = PYXCurveRegion::create(vertices);


	PYXPointer<IRegionVisitor> visitor = curve->getVisitor();
	PYXPointer<PYXInnerTileIntersectionIterator> iterator = visitor->getInnerTileIterator(PYXInnerTile(PYXIcosIndex("1"),11));

	while(!iterator->end())
	{
		PYXRegion::CellIntersectionState curveResult = curve->intersects(iterator->getTile().getRootIndex(),false);
		PYXInnerTileIntersection iteratorResult = iterator->getIntersection();

		if (curveResult == PYXRegion::knNone)
		{
			TEST_ASSERT(iteratorResult == knIntersectionNone);
		}
		if (curveResult == PYXRegion::knPartial)
		{
			TEST_ASSERT(iteratorResult == knIntersectionPartial);
		}
		if (curveResult == PYXRegion::knComplete)
		{
			TEST_ASSERT(iteratorResult == knIntersectionComplete);
		}
		iterator->next();
	}
	*/
}

PYXCurveRegion::PYXCurveRegion()
{
}

//! Copy a curve region
PYXCurveRegion::PYXCurveRegion(const PYXCurveRegion & curve)
	:	m_nodes(curve.m_nodes),
		m_boundingCircle(curve.m_boundingCircle)
{
}

//! Convenience constructor; constructs a curve region for two 3D coordinate.
PYXCurveRegion::PYXCurveRegion(PYXCoord3DDouble const & pointA,PYXCoord3DDouble const & pointB)
{
	m_nodes.push_back(pointA);
	m_nodes.push_back(pointB);
	m_boundingCircle = PYXBoundingCircle(pointA,0) + PYXBoundingCircle(pointB,0);
}

//! Convenience constructor; constructs a curve region for list of points
PYXCurveRegion::PYXCurveRegion(std::vector<PYXCoord3DDouble> const & points,bool shouldCloseCurve)
{
	m_nodes.reserve(points.size());
	m_nodes.push_back(points[0]);
	m_boundingCircle = PYXBoundingCircle(points[0],0);

	for(unsigned int i=1;i<points.size();i++)
	{
		if (!m_nodes.back().equal(points[i]))
		{
			m_nodes.push_back(points[i]);
			m_boundingCircle += PYXBoundingCircle(points[i],0);

		}
	}

	if (shouldCloseCurve)
	{
		closeCurve();
	}
}

bool PYXCurveRegion::isClosed() const
{
	return m_nodes.front().equal(m_nodes.back());
}

void PYXCurveRegion::closeCurve()
{
	if (!isClosed())
	{
		m_nodes.push_back(m_nodes.front());
	}
}

PYXPointer<IRegion> PYXCurveRegion::clone() const
{
	return PYXCurveRegion::create(*this);
}

//! distance in radians
double PYXCurveRegion::getDistanceToBorder(const PYXCoord3DDouble & location,double errorThreshold) const
{
	double minDistance = std::numeric_limits<double>::max();
	for(unsigned int i = 0;i<m_nodes.size()-1;++i)
	{
		SphereMath::GreatCircleArc arc(m_nodes[i],m_nodes[i+1]);

		double distance = arc.distanceTo(location);

		if (distance < minDistance)
		{
			minDistance = distance;
		}
	}
	return minDistance;
}

PYXRegion::CellIntersectionState PYXCurveRegion::intersects(const PYXIcosIndex & index, bool asTile) const
{
	double distanceThreshold = asTile? PYXIcosMath::UnitSphere::calcTileCircumRadius(index) : PYXIcosMath::UnitSphere::calcCellCircumRadius(index);

	PYXCoord3DDouble cellCenter;
	SnyderProjection::getInstance()->pyxisToXYZ(index, &cellCenter);

	if (getDistanceToBorder(cellCenter, distanceThreshold / 10) < distanceThreshold)
	{
		return PYXRegion::knPartial;
	}

	return PYXRegion::knNone;
}

PYXRegion::CellIntersectionState PYXCurveRegion::intersects(const PYXBoundingCircle & circle,double errorThreshold) const
{
	if (getDistanceToBorder(circle.getCenter(), errorThreshold) < circle.getRadius())
	{
		return PYXRegion::knPartial;
	}

	return PYXRegion::knNone;
}


PYXBoundingCircle PYXCurveRegion::getBoundingCircle() const
{
	return m_boundingCircle;
}

PYXPointer<IRegionVisitor> PYXCurveRegion::getVisitor() const
{
	return Visitor::create(this);
}

void PYXCurveRegion::serializeVisitor(PYXWireBuffer & buffer,const PYXPointer<IRegionVisitor> & visitor) const
{
	Visitor * cruveVisitor = dynamic_cast<Visitor*>(visitor.get());

	if (cruveVisitor == NULL)
	{
		PYXTHROW(PYXException,"Unsupported format of visitor");
	}

	cruveVisitor->serialize(buffer);
}

PYXPointer<IRegionVisitor> PYXCurveRegion::deserializeVisitor(PYXWireBuffer & buffer) const
{
	return Visitor::create(this,buffer);
}

///////////////////////////////////////////////////////////////////
// PYXCurveRegion::Visitor class
///////////////////////////////////////////////////////////////////

PYXCurveRegion::Visitor::Visitor(const PYXPointer<const PYXCurveRegion> & curve) : m_curve(curve)
{
	m_chunks.push_back(Range<int>::createClosedClosed(0,m_curve->getVerticesCount()-2));
}

PYXCurveRegion::Visitor::Visitor(const PYXPointer<const PYXCurveRegion> & curve, std::vector<Range<int>> & chunks) : m_curve(curve)
{
	std::swap(m_chunks,chunks);
}


PYXCurveRegion::Visitor::Visitor(const Visitor & other) : m_curve(other.m_curve), m_chunks(other.m_chunks)
{
}

PYXCurveRegion::Visitor::Visitor(const PYXPointer<const PYXCurveRegion> & curve, PYXWireBuffer & buffer) : m_curve(curve)
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

void PYXCurveRegion::Visitor::serialize(PYXWireBuffer & buffer)
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

int PYXCurveRegion::Visitor::getChunksSize() const
{
	int count = 0;
	for(unsigned int i=0;i<m_chunks.size();++i)
	{
		const Range<int> & range = m_chunks[i];
		count += range.max - range.min + 1;
	}
	return count;
}

bool PYXCurveRegion::Visitor::isOptimal() const
{
	return getChunksSize() < knOptimalSize;
}

PYXPointer<IRegionVisitor> PYXCurveRegion::Visitor::trim(const PYXIcosIndex & index) const
{
	std::vector<Range<int>> new_chunks;

	double distanceThreshold = PYXIcosMath::UnitSphere::calcCellCircumRadius(index);

	PYXCoord3DDouble cellCenter;
	SnyderProjection::getInstance()->pyxisToXYZ(index, &cellCenter);

	if (!m_curve->getBoundingCircle().intersects(PYXBoundingCircle(cellCenter,distanceThreshold)))
	{
		return 0;
	}
	else if (m_curve->getBoundingCircle().getRadius()*4 < distanceThreshold)
	{
		new_chunks = m_chunks;
		return PYXNEW(Visitor,m_curve,new_chunks);
	}

	for(unsigned int i=0;i<m_chunks.size();++i)
	{
		const Range<int> & range = m_chunks[i];

		for(int arcIndex=range.min;arcIndex<=range.max;++arcIndex)
		{
			SphereMath::GreatCircleArc arc(m_curve->m_nodes[arcIndex],m_curve->m_nodes[arcIndex+1]);

			if (arc.distanceTo(cellCenter) < distanceThreshold)
			{
				if (!new_chunks.empty() && new_chunks.back().max == arcIndex-1)
				{
					new_chunks.back().max = arcIndex;
				}
				else 
				{
					new_chunks.push_back(Range<int>(arcIndex));
				}
			}
		}
	}

	if (new_chunks.empty())
	{
		return 0;
	}
	else 
	{
		return PYXNEW(Visitor,m_curve,new_chunks);
	}
}

PYXPointer<PYXInnerTileIntersectionIterator> PYXCurveRegion::Visitor::getInnerTileIterator(const PYXInnerTile & tile) const
{
	return Iterator::create(this,tile);
}

PYXInnerTileIntersection PYXCurveRegion::Visitor::intersects(const PYXBoundingCircle & circle,double errorThreshold) const
{
	for(unsigned int i=0;i<m_chunks.size();++i)
	{
		const Range<int> & range = m_chunks[i];

		for(int arcIndex=range.min;arcIndex<=range.max;++arcIndex)
		{
			SphereMath::GreatCircleArc arc(m_curve->m_nodes[arcIndex],m_curve->m_nodes[arcIndex+1]);

			if (arc.distanceTo(circle.getCenter()) < circle.getRadius())
			{
				return knIntersectionPartial;
			}
		}
	}

	return knIntersectionNone;
}

PYXInnerTileIntersection PYXCurveRegion::Visitor::intersects(const PYXIcosIndex & cell) const
{
	double distanceThreshold = PYXIcosMath::UnitSphere::calcCellCircumRadius(cell);

	PYXCoord3DDouble cellCenter;
	SnyderProjection::getInstance()->pyxisToXYZ(cell, &cellCenter);	

	return intersects(PYXBoundingCircle(cellCenter,distanceThreshold));
}

///////////////////////////////////////////////////////////////////
// PYXCurveRegion::Visitor::Iterator class
///////////////////////////////////////////////////////////////////

PYXCurveRegion::Visitor::Iterator::Iterator(const PYXPointer<const PYXCurveRegion::Visitor> & visitor,const PYXInnerTile & tile) : m_visitor(visitor), m_tile(tile)
{
	for(unsigned int i=0;i<m_visitor->m_chunks.size();++i)
	{
		const Range<int> & range = m_visitor->m_chunks[i];

		for(int arcIndex=range.min;arcIndex<=range.max;++arcIndex)
		{
			m_arcs.push_back(SphereMath::GreatCircleArc(m_visitor->m_curve->m_nodes[arcIndex],m_visitor->m_curve->m_nodes[arcIndex+1]));
		}
	}

	m_index = m_tile.getRootIndex();

	findIntersection();
}

void PYXCurveRegion::Visitor::Iterator::findIntersection()
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
	m_currentIntersection = PYXIcosMath::UnitSphere::calcCellCircumRadius(m_index) < minDistance ? knIntersectionNone : knIntersectionPartial;
}

void PYXCurveRegion::Visitor::Iterator::findNextInnerTile(PYXIcosIndex & index)
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

const PYXInnerTile & PYXCurveRegion::Visitor::Iterator::getTile() const
{
	return m_currentTile;
}

const PYXInnerTileIntersection & PYXCurveRegion::Visitor::Iterator::getIntersection() const
{
	return m_currentIntersection;
}

bool PYXCurveRegion::Visitor::Iterator::end() const
{
	return m_index.isNull();
}

void PYXCurveRegion::Visitor::Iterator::next()
{
	findNextInnerTile(m_index);
	if (!m_index.isNull())
	{
		findIntersection();
	}
}

///////////////////////////////////////////////////////////////////
// PYXCurveRegion serialition
///////////////////////////////////////////////////////////////////

PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXCurveRegion & region)
{
	buffer << region.m_nodes;
	return buffer;
}

PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXCurveRegion> & region)
{
	PYXCurveRegion::NodesVector nodes;
	buffer >> nodes;
	region = PYXCurveRegion::create(nodes);
	return buffer;
}
