/******************************************************************************
multi_vertex_region.cpp

begin		: 2012-06-14
copyright	: (C) 2012 by the PYXIS innovation inc.
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

#include "arc_buffer.h"

#define USE_BOUNDING_SPHERE_SPEEDUP

SphereMath::GreatCircleArc ArcBuffer::getArc( int index ) const
{
	{
		boost::recursive_mutex::scoped_lock lock(m_cacheMutex);
		if (!m_arcsCache.empty())
		{
			return m_arcsCache[index];
		}
	}

	PYXConstWireBuffer countBuffer(m_counts);
	PYXConstWireBuffer vertexBuffer(m_vertices);

	int count = 0;
	int totalCount =0;
	size_t position=0;

	countBuffer >> count;
	while (index >= totalCount + count)
	{
		totalCount+=count;
		position+=count+1;
		countBuffer >> count;
	}
	position+= index-totalCount;

	vertexBuffer.setPos(position * VERTEX_SIZE);
	PYXCoord3DDouble first;
	PYXCoord3DDouble second;

	vertexBuffer >> first;
	vertexBuffer >> second;
	return SphereMath::GreatCircleArc( first,second,false);
}

const void ArcBuffer::getArcs( int first , int until ,std::vector<SphereMath::GreatCircleArc> & result) const
{
	PYXConstWireBuffer countBuffer(m_counts);
	PYXConstWireBuffer vertexBuffer(m_vertices);
	int size=until-first;
	int count = 0;
	int totalCount =0;
	size_t position=0;
	countBuffer >> count;
	while (first >= totalCount + count)
	{
		totalCount+=count;
		position+=count + 1;
		countBuffer >> count;
	}
	position+= first-totalCount;

	vertexBuffer.setPos(position * VERTEX_SIZE);
	PYXCoord3DDouble firstPoint;
	PYXCoord3DDouble secondPoint;

	vertexBuffer >> secondPoint;
	for (int i = first ;i<until;i++)
	{
		if(i >= totalCount + count)//reached the end of the current segment go to next segment
		{
			totalCount += count;
			countBuffer >> count;
			vertexBuffer >> firstPoint;
		}
		else // inside the segment just read the second point
		{
			firstPoint=secondPoint;
		}
		vertexBuffer >> secondPoint;
		result.push_back(SphereMath::GreatCircleArc( firstPoint,secondPoint,false));
	}
}

int ArcBuffer::getSegmentsCount() const
{
	return m_counts.size() / sizeof(int);
}

int ArcBuffer::getSegmentSize(int index) const
{
	PYXConstWireBuffer countBuffer(m_counts);
	countBuffer.setPos(index * sizeof(int));
	int segmentSize;
	countBuffer >> segmentSize;
	return segmentSize;
}

PYXCoord3DDouble ArcBuffer::getSegmentVertex(int segmentIndex, int vertexIndex) const
{
	PYXConstWireBuffer countBuffer(m_counts);
	int offset = 0;
	for (int segment = 0; segment < segmentIndex; segment++)
	{
		int count;
		countBuffer >> count;
		offset += count + 1;
	}

	PYXConstWireBuffer vertexBuffer(m_vertices);
	vertexBuffer.setPos((offset + vertexIndex) * VERTEX_SIZE);
	PYXCoord3DDouble point;
	vertexBuffer >> point;
	return point;
}

PYXBoundingCircle ArcBuffer::getBoundingCircle() const
{
	PYXBoundingCircle result;

	PYXConstWireBuffer vertexBuffer(m_vertices);

	PYXCoord3DDouble point;

	for(unsigned int i=0;i<m_vertices.size()/VERTEX_SIZE;i++)
	{
		vertexBuffer >> point;
		result += PYXBoundingCircle(point,0);
	}

	return result;
}

void ArcBuffer::add( const PYXPointer<PYXCurveRegion> & ring ,PYXStringWireBuffer & countBuffer, PYXStringWireBuffer & vertexBuffer)
{
	unsigned int size = ring->getVerticesCount();
	countBuffer << (int)(size-1);
	for(unsigned int i = 0 ; i < size; ++i)
	{
		vertexBuffer << (ring->getVertex(i));
	}
	m_arcsCache.clear();
}

ArcBuffer::ArcBuffer(const std::vector<PYXPointer<PYXCurveRegion>> & regions)
{
	PYXStringWireBuffer countBuffer;
	PYXStringWireBuffer vertexBuffer;

	for (std::vector<PYXPointer<PYXCurveRegion>>::const_iterator curve = regions.begin(); curve != regions.end(); ++curve)
	{
		add(*curve ,countBuffer,vertexBuffer);
	}

	m_counts = *(countBuffer.getBuffer());
	m_vertices = *(vertexBuffer.getBuffer());
}

ArcBuffer::ArcBuffer(const ArcBuffer & arcBuffer) : m_counts(arcBuffer.m_counts), m_vertices(arcBuffer.m_vertices)
{
}

ArcBuffer & ArcBuffer::operator = (const ArcBuffer & arcBuffer)
{
	{
		boost::recursive_mutex::scoped_lock lock(m_cacheMutex);
		m_arcsCache.clear();
		m_counts = arcBuffer.m_counts;
		m_vertices = arcBuffer.m_vertices;
	}
	
	return *this;
}

const int ArcBuffer::size() const
{
	int count= m_counts.size()/sizeof(int);
	int tmp;
	int size=0;
	PYXConstWireBuffer buffer(m_counts);

	for (int i = 0 ; i < count; i++)
	{
		buffer >> tmp;
		size += tmp;
	}
	return size;
}

void ArcBuffer::findIntersectingArcs(const std::vector<Range<int>> & ranges, const PYXBoundingCircle & circle, std::vector<Range<int>> & result) const
{
	populateArcsCache();

	result.clear();

	const PYXCoord3DDouble & center = circle.getCenter();
	double distanceThreshold = circle.getRadius();

	for(unsigned int i=0;i<ranges.size();++i)
	{
		const Range<int> & range = ranges[i];

		for(int arcIndex = range.min; arcIndex <= range.max; ++arcIndex)
		{
#ifdef USE_BOUNDING_SPHERE_SPEEDUP
			//negative checks speed up
			if (arcIndex % 10 == 0)
			{
				if (!circle.intersects(m_arcsBoundingCircleCache[arcIndex/10]))
				{
					arcIndex += 10 - 1; //++arcIndex will happen in the for line...
					continue;
				}
				else if (arcIndex + 9 < range.max && 
						 circle.contains(m_arcsBoundingCircleCache[arcIndex/10]))
				{
					if (!result.empty() && result.back().max == arcIndex-1)
					{
						result.back().max = arcIndex + 9;
					}
					else
					{
						result.push_back(Range<int>::createClosedClosed(arcIndex,arcIndex+9));
					}
					arcIndex += 10 - 1; //++arcIndex will happen in the for line...
					continue;
				}
			}
#endif

			const SphereMath::GreatCircleArc & arc = m_arcsCache[arcIndex];

			if (arc.distanceTo(center) < distanceThreshold)
			{			
				if (!result.empty() && result.back().max == arcIndex-1)
				{
					result.back().max = arcIndex;
				}
				else
				{
					result.push_back(Range<int>(arcIndex));
				}
			}			
		}
	}
}

void ArcBuffer::populateArcsCacheIfNeeded(const std::vector<Range<int>> & ranges) const
{
	boost::recursive_mutex::scoped_lock lock(m_cacheMutex);
	//if we don't have cache...
	if (m_arcsCache.empty())
	{
		int rangeSize = 0;
		for(unsigned int i=0;i<ranges.size();++i)
		{
			rangeSize += ranges[i].max-ranges[i].min+1;
		}

		//large range - populate cache
		if (rangeSize>=20)
		{
			populateArcsCache();
		}
	}	
}

int ArcBuffer::countIntersectionWithRay(const std::vector<Range<int>> & ranges, const SphereMath::GreatCircleArc & ray) const
{
	populateArcsCacheIfNeeded(ranges);

	int intersectionCount=0;

	if (m_arcsCache.empty())
	{
		for(unsigned int i=0;i<ranges.size();++i)
		{
			const Range<int> & range = ranges[i];

			for(int arcIndex=range.min;arcIndex<=range.max;++arcIndex)
			{
				if(ray.intersects(getArc(arcIndex)))
				{
					intersectionCount++;
				}
			}
		}		
	}
	else 
	{
		//if we got here - the cache is populated...
		for(unsigned int i=0;i<ranges.size();++i)
		{
			const Range<int> & range = ranges[i];

			for(int arcIndex=range.min;arcIndex<=range.max;++arcIndex)
			{
				if(ray.intersects(m_arcsCache[arcIndex]))
				{
					intersectionCount++;
				}
			}
		}
	}

	return intersectionCount;
}

double ArcBuffer::getDistanceTo(const PYXCoord3DDouble & location) const
{
	std::vector<Range<int>> chunks;
	chunks.push_back(Range<int>::createClosedClosed(0,size()-1));
	return getDistanceTo(chunks,location);
}

double ArcBuffer::getDistanceTo(const std::vector<Range<int>> & ranges, const PYXCoord3DDouble & location) const
{
	double minDistance = std::numeric_limits<double>::max();

	populateArcsCacheIfNeeded(ranges);

	if (m_arcsCache.empty())
	{
		for(unsigned int i=0;i<ranges.size();++i)
		{
			const Range<int> & range = ranges[i];

			for(int arcIndex=range.min;arcIndex<=range.max;++arcIndex)
			{
				double distance = getArc(arcIndex).distanceTo(location);

				if (distance < minDistance)
				{
					minDistance = distance;
				}				
			}
		}
	}
	else
	{
		//if we got here - the cache is populated...
		for(unsigned int i=0;i<ranges.size();++i)
		{
			const Range<int> & range = ranges[i];

			for(int arcIndex=range.min;arcIndex<=range.max;++arcIndex)
			{
				double distance = m_arcsCache[arcIndex].distanceTo(location);

				if (distance < minDistance)
				{
					minDistance = distance;
				}					
			}
		}
	}

	return minDistance;
}

void ArcBuffer::populateArcsCache() const
{
	boost::recursive_mutex::scoped_lock lock(m_cacheMutex);

	if (m_arcsCache.empty())
	{
		int count = size();
		m_arcsCache.reserve(count);
		getArcs(0, count, m_arcsCache);


#ifdef USE_BOUNDING_SPHERE_SPEEDUP		
		for(int i=0;i<count;i+=10)
		{
			int max = std::min(i+10,count);
			PYXBoundingCircle boundingCircle(m_arcsCache[i],0);
			for(int j=i+1;j<max;j++)
			{
				boundingCircle += PYXBoundingCircle(m_arcsCache[j],0);
			}

			m_arcsBoundingCircleCache.push_back(boundingCircle);
		}
#endif		
	}
}


PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const ArcBuffer & arcs)
{
	buffer << arcs.m_counts <<arcs.m_vertices;

	return buffer;
}
PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer, ArcBuffer & arcs)
{
	buffer >> arcs.m_counts >> arcs.m_vertices;

	assert(arcs.size() == arcs.m_vertices.size()/arcs.VERTEX_SIZE-(arcs.m_counts.size()/4)); // 4 is the size of each arcs count (int) written.
	return buffer;
}


////! create a CurveChunk from node[start] to node[end].
//PYXCurveRegion::CurveChunk::CurveChunk(PYXCurveRegion & curve,unsigned int start,unsigned int end)
//	:	m_curve(curve),
//		m_start(start),
//		m_end(end),
//		m_arc(curve.m_nodes[start],curve.m_nodes[end]),
//		m_distance(0),
//		m_error(0)
//{
//	calcDistanceAndError();
//}
//
////! create a CurveChunk from node[start] to node[end] with a given approximating arc.
//PYXCurveRegion::CurveChunk::CurveChunk(PYXCurveRegion & curve,unsigned int start,unsigned int end,const SphereMath::GreatCircleArc & arc)
//	:	m_curve(curve),
//		m_start(start),
//		m_end(end),
//		m_arc(arc),
//		m_distance(0),
//		m_error(0)
//{
//	calcDistanceAndError();
//}
//
////! Copy constructor
//PYXCurveRegion::CurveChunk::CurveChunk(const CurveChunk & other)
//	:	m_curve(other.m_curve),
//		m_start(other.m_start),
//		m_end(other.m_end),
//		m_arc(other.m_arc),
//		m_distance(other.m_distance),
//		m_error(other.m_error),
//		m_subChunks(other.m_subChunks)
//{
//}
//
//PYXCurveRegion::CurveChunk & PYXCurveRegion::CurveChunk::operator = (const CurveChunk & other)
//{
//	assert(&m_curve == &other.m_curve);
//	m_start = other.m_start;
//	m_end = other.m_end;
//	m_arc = other.m_arc;
//	m_distance = other.m_distance;
//	m_error = other.m_error;
//	m_subChunks = other.m_subChunks;
//
//	return *this;
//}
//
//void PYXCurveRegion::CurveChunk::calcDistanceAndError()
//{
//	m_distance = m_curve.m_nodesDistances[m_end]-m_curve.m_nodesDistances[m_start];
//	m_error = 0;
//
//	for(unsigned int it = m_start;it != m_end;++it)
//	{
//		if (it>m_start)
//		{
//			double nodeError = m_arc.distanceTo(m_curve.m_nodes[it]);
//			if (m_error < nodeError)
//			{
//				m_error = nodeError;
//			}
//		}
//	}
//}
//
//bool PYXCurveRegion::CurveChunk::addNextNodes(double errorThreshold,unsigned int maxEndNode)
//{	
//	if (m_end < m_curve.m_nodes.size()-1 && m_end<maxEndNode)
//	{
//		int oldEnd = m_end;
//		++m_end;
//
//		while (m_end < m_curve.m_nodes.size()-1 && m_end<maxEndNode)
//		{
//			double distanceToCompleteGreateCricle = abs( acos( m_arc.getArcNormal().dot(m_curve.m_nodes[m_end])) - MathUtils::kf90Rad );
//
//			if (distanceToCompleteGreateCricle > errorThreshold)
//			{
//				break;
//			}
//			else
//			{
//				m_end++;
//			}
//		}
//
//		if (m_curve.m_nodes[m_start].equal(m_curve.m_nodes[m_end]))
//		{
//			if (m_end-m_start>1)
//			{
//				m_end--;
//			}
//			else
//			{
//				//This should not have happen
//				m_end = oldEnd;
//				return false;
//			}
//		}
//		if (m_end != oldEnd)
//		{
//			m_arc = SphereMath::GreatCircleArc(m_curve.m_nodes[m_start],m_curve.m_nodes[m_end]);
//			calcDistanceAndError();
//			return true;
//		}
//	}
//	return false;
//}
//
//bool PYXCurveRegion::CurveChunk::removeLastNode()
//{
//	if (m_end > m_start+1)
//	{
//		int oldEnd = m_end;
//		--m_end;
//
//		if (m_curve.m_nodes[m_start].equal(m_curve.m_nodes[m_end]))
//		{
//			if (m_end-m_start>1)
//			{
//				m_end--;
//			}
//			else
//			{
//				//This should not have happen.
//				m_end = oldEnd;
//				return false;
//			}
//		}
//
//		if (m_end != oldEnd)
//		{
//			m_arc = SphereMath::GreatCircleArc(m_curve.m_nodes[m_start],m_curve.m_nodes[m_end]);			
//			calcDistanceAndError();
//			return true;
//		}
//	}
//	return true;
//}
//
//void PYXCurveRegion::CurveChunk::findSubChunks()
//{
//	int chuckCreationCount = 0;
//	int distanceCount = 0;
//	findSubChunks(chuckCreationCount,distanceCount);
//
//	/*
//	if (getArcsCount()>1000)
//	{
//		TRACE_INFO("findSubChunks : nodes : " << getArcsCount() << "subChunks created " << chuckCreationCount << " distanceCalls: " << distanceCount);
//	}
//	*/
//}
//
//void PYXCurveRegion::CurveChunk::findSubChunks(int & chuckCreationCount,int & distanceCount)
//{
//	if (getArcsCount() == 1)
//	{
//		return;
//	}
//
//	if (getArcsCount() < 10)
//	{
//		//don't bother with small chunks.
//		m_subChunks.reserve(getArcsCount()-1);
//		for(unsigned int i=m_start;i<m_end;i++)
//		{
//			chuckCreationCount++;
//			m_subChunks.push_back(CurveChunk(m_curve,i,i+1));
//		}
//		return;
//	}
//
//	//big curves just divided by 10. lets not worry to much about the details.. for now
//	if (getArcsCount() > 100)
//	{
//		//get total distance
//		double totalDistance = m_curve.m_nodesDistances[m_end]-m_curve.m_nodesDistances[m_start];
//		double chunkDistance = totalDistance/10;
//
//		unsigned int subChunkStart = m_start;
//		unsigned int subChunkEnd = subChunkStart+1;
//
//		while(subChunkEnd < m_end)
//		{
//			while (m_curve.m_nodesDistances[subChunkEnd]-m_curve.m_nodesDistances[subChunkStart] < chunkDistance && subChunkEnd < m_end)
//			{
//				subChunkEnd++;
//			}
//
//			//this is a special case (that happen). if previous loop contain the whole set, this happen if the last node is very very big (90% of the distance).
//			if (subChunkStart == m_start && subChunkEnd == m_end)
//			{
//				//in this case. make sure we have two sub chunks: the whole "small" arcs as one group, and the last "big" arc as the second group.
//				subChunkEnd--;
//			}
//
//			if (m_curve.m_nodes[subChunkEnd].equal(m_curve.m_nodes[subChunkStart]))
//			{
//				if (subChunkEnd-subChunkStart>1)
//				{
//					subChunkEnd--;
//				}
//				else
//				{
//					PYXTHROW(PYXException,"Two neighboor points are the same, this should never happen");
//				}
//			}
//
//			m_subChunks.push_back(CurveChunk(m_curve,subChunkStart,subChunkEnd));
//			distanceCount += subChunkEnd-subChunkStart;
//			chuckCreationCount++;
//
//			subChunkStart = subChunkEnd;
//			subChunkEnd = subChunkStart+1;
//		}
//
//		//make sure we got everything...
//		if (subChunkStart < m_end)
//		{
//			m_subChunks.push_back(CurveChunk(m_curve,subChunkStart,m_end));
//			distanceCount += m_end-subChunkStart;
//			chuckCreationCount++;
//		}
//	}
//	else
//	{
//		//now we do the really fine job...
//		unsigned int subChunkStart = m_start;
//		unsigned int subChunkEnd = subChunkStart+1;
//		
//		double errorThreshold = getDistance()/50;
//
//		//if our chunk is better then the error threshold, then set error threshold to half the error.
//		if (m_error < errorThreshold)
//		{
//			errorThreshold = m_error/2;
//		}
//
//		while(subChunkEnd<=m_end)
//		{
//			CurveChunk subChunk(m_curve,subChunkStart,subChunkEnd);
//			chuckCreationCount++;
//
//			while(subChunk.getError() < errorThreshold && subChunk.getEndNode() != m_end)
//			{
//				if (!subChunk.addNextNodes(errorThreshold,m_end))
//				{
//					break;
//				}
//				distanceCount += subChunk.getArcsCount()-1;
//				chuckCreationCount++;
//			}
//
//			if (subChunk.getError() > errorThreshold && subChunk.getArcsCount()>1)
//			{
//				subChunk.removeLastNode();
//				distanceCount += subChunk.getArcsCount()-1;
//				chuckCreationCount++;
//			}
//
//			m_subChunks.push_back(subChunk);
//			subChunkStart = subChunk.getEndNode();
//			subChunkEnd = subChunkStart+1;
//		}
//
//		//this can happen and this is an error. if all nodes get into one group, there is no reason to go any deeper. break it down.
//		if (m_subChunks.size() == 1)
//		{
//			m_subChunks.clear();
//			m_subChunks.reserve(getArcsCount()-1);
//			for(unsigned int i=m_start;i<m_end;i++)
//			{
//				m_subChunks.push_back(CurveChunk(m_curve,i,i+1));
//				chuckCreationCount++;
//			}
//		}
//	}
//
//	//Do it on a need bases. and just on init
//	/*
//	for(std::vector<CurveChunk>::iterator it = m_subChunks.begin(); it != m_subChunks.end(); ++it)
//	{
//		it->findSubChunks(chuckCreationCount,distanceCount);
//	}
//	*/
//}
//
//void PYXCurveRegion::CurveChunk::collectNodes(PYXCurveRegion::NodesVector & usedNodes,double errorThreshold)
//{
//	if (getError() < errorThreshold || getArcsCount() == 1)
//	{
//		usedNodes.push_back(m_curve.m_nodes[m_end]);
//	}
//	else
//	{
//		if (getArcsCount()>1 && m_subChunks.empty())
//		{
//			findSubChunks();
//		}
//
//		for(std::vector<CurveChunk>::iterator it = m_subChunks.begin(); it != m_subChunks.end(); ++it)
//		{
//			it->collectNodes(usedNodes,errorThreshold);
//		}
//	}
//}
//
//void PYXCurveRegion::CurveChunk::print()
//{
//	print("",getDistance());
//}
//
//void PYXCurveRegion::CurveChunk::print(const std::string & prefix,double totalDistance)
//{
//	std::string usedPrefix;
//	if (!prefix.empty())
//	{
//		usedPrefix = prefix.substr(0,prefix.length()-3) + "+- ";
//	}
//	TRACE_INFO(usedPrefix << "Cunk: " << getArcsCount() << " nodes grouped into " << getSubChunksCount() << " nodes . errorRatio " << (getError()/getDistance())*100 << "%, distanceRatio " << (getDistance()/totalDistance)*100 << "%");
//
//	for(std::vector<CurveChunk>::iterator it = m_subChunks.begin(); it != m_subChunks.end(); ++it)
//	{
//		if (it->getArcsCount()>2)
//		{
//			it->print(prefix + "|  ",totalDistance);
//		}
//	}
//}
//
//double PYXCurveRegion::CurveChunk::distanceTo(const PYXCoord3DDouble & point, double errorThreshold)
//{
//	double minDistance = 100; //very large number. max distance is PI
//	int distanceCount = 0;
//
//	//if this is the final arc. just return it
//	if (getArcsCount()==1 || getError() < errorThreshold )
//	{
//		return m_arc.distanceTo(point);
//	}
//
//	findMinDistance(point,minDistance,errorThreshold,distanceCount);
//
//	/*
//	if (getArcsCount()>100)
//	{
//		double speedup = static_cast<double>(distanceCount)/getArcsCount()*100;
//		
//		TRACE_INFO("Nodes " << getArcsCount() << " with " << distanceCount << " checks (" << speedup << "% speedup)");
//	}
//	*/
//
//	return minDistance;
//}
//
//void PYXCurveRegion::CurveChunk::findMinDistance(const PYXCoord3DDouble & point,double & minDistance, double errorThreshold, int & distanceCount)
//{
//	if (getArcsCount()>1 && m_subChunks.empty())
//	{
//		findSubChunks();
//	}
//
//	std::vector<double> chunksDistances(m_subChunks.size());
//	for(unsigned int i = 0; i < m_subChunks.size(); ++i)
//	{
//		chunksDistances[i] = m_subChunks[i].m_arc.distanceTo(point);
//		distanceCount++;
//	}
//
//	int minIndex = 0;
//	double minChunkDistance = chunksDistances[0];
//	for(unsigned int i = 1; i < m_subChunks.size(); ++i)
//	{
//		if (minChunkDistance > chunksDistances[i])
//		{
//			minChunkDistance = chunksDistances[i];
//			minIndex = i;
//		}
//	}
//
//	if (m_subChunks[minIndex].getArcsCount() == 1 || m_subChunks[minIndex].getError() < errorThreshold )
//	{
//		//this is a arc is approximating good enough or it is the real arc. check if it the closest.
//		if (minDistance > chunksDistances[minIndex])
//		{
//			minDistance = chunksDistances[minIndex];
//		}
//	}
//	else 
//	{
//		//this is a collection of chunks.
//		if (chunksDistances[minIndex] - m_subChunks[minIndex].getError() < minDistance)
//		{
//			//the chunk can have better minimum distance, go for it.
//			m_subChunks[minIndex].findMinDistance(point,minDistance,errorThreshold,distanceCount);
//		}
//	}
//
//	unsigned int index=0;
//	for(std::vector<CurveChunk>::iterator it = m_subChunks.begin(); it != m_subChunks.end(); ++it,++index)
//	{
//		if (index==minIndex)
//		{
//			continue;
//		}
//
//		if (it->getArcsCount() == 1 || it->getError() < errorThreshold )
//		{
//			//this is a arc is approximating good enough or it is the real arc. check if it the closest.
//			if (minDistance > chunksDistances[index])
//			{
//				minDistance = chunksDistances[index];
//			}
//		}
//		else 
//		{
//			//this is a collection of chunks.
//			if (chunksDistances[index] - it->getError() < minDistance)
//			{
//				//the chunk can have better minimum distance, go for it.
//				it->findMinDistance(point,minDistance,errorThreshold,distanceCount);
//			}
//		}
//	}
//}
//
//bool PYXCurveRegion::CurveChunk::canChunkIntersects(const SphereMath::GreatCircleArc & ray) const
//{
//	return	ray.intersects(m_arc) ||
//			ray.distanceTo(m_arc.getPointA()) < getError() ||
//			ray.distanceTo(m_arc.getPointB()) < getError() ||
//			m_arc.distanceTo(ray.getPointA()) < getError() ||
//			m_arc.distanceTo(ray.getPointB()) < getError();
//}
//
//bool PYXCurveRegion::CurveChunk::canChunkBeCloserToPoint(const PYXCoord3DDouble & point, double distance) const
//{
//	return m_arc.distanceTo(point) - getError() < distance;
//}
//
//int PYXCurveRegion::CurveChunk::countIntersections(const SphereMath::GreatCircleArc & ray,bool & numericlySafe,double errorThreshold)
//{
//	int distanceCount = 0;
//	int intersectCount = 0;
//
//	int intersectionCount = countIntersections(ray,numericlySafe,errorThreshold,distanceCount,intersectCount);
//
//	/*
//	if (intersectionCount>0)
//	{
//		TRACE_INFO("intersection count was " << intersectionCount << ": " << getArcsCount() << " nodes, distanceCount = " << distanceCount << ", intersectCount = " << intersectCount);
//	}
//	*/
//
//	return intersectionCount;
//}
//
//int PYXCurveRegion::CurveChunk::countIntersections(const SphereMath::GreatCircleArc & ray, bool & numericlySafe, double errorThreshold, int & distanceCount,int & intersectCount)
//{
//	if (getArcsCount()>1 && m_subChunks.empty())
//	{
//		findSubChunks();
//	}
//
//	int intersectionCount = 0;
//	for(std::vector<CurveChunk>::iterator it = m_subChunks.begin(); it != m_subChunks.end(); ++it)
//	{
//		bool intersects = it->m_arc.intersects(ray);
//		intersectCount++;
//
//		if (it->getArcsCount() == 1 || it->getError()<errorThreshold)
//		{
//			if (intersects)
//			{
//				intersectionCount++;
//			}
//
//			if (ray.distanceTo(it->m_arc.getPointA()) <= SphereMath::knNumericEpsilon ||
//				ray.distanceTo(it->m_arc.getPointB()) <= SphereMath::knNumericEpsilon ||
//				it->m_arc.distanceTo(ray.getPointA()) <= SphereMath::knNumericEpsilon ||
//				it->m_arc.distanceTo(ray.getPointB()) <= SphereMath::knNumericEpsilon )
//			{
//				TRACE_INFO("Numeric Unsafe!!!");
//				numericlySafe = false;
//			}
//		}
//		else
//		{
//			if (intersects)
//			{
//				intersectionCount += it->countIntersections(ray,numericlySafe,errorThreshold,distanceCount,intersectCount);
//			}
//			else
//			{
//				distanceCount++;
//				if (ray.distanceTo(it->m_arc.getPointA()) < it->getError() ||
//					ray.distanceTo(it->m_arc.getPointB()) < it->getError() ||
//					it->m_arc.distanceTo(ray.getPointA()) < it->getError() ||
//					it->m_arc.distanceTo(ray.getPointB()) < it->getError())
//				{
//					intersectionCount += it->countIntersections(ray,numericlySafe,errorThreshold,distanceCount,intersectCount);
//				}
//				else
//				{
//					/*
//					if (it->countIntersectionsSafe(ray)>0)
//					{
//						TRACE_INFO("chunk was skipped - this is an error");
//					}
//					*/
//				}
//			}
//		}
//	}
//
//	return intersectionCount;
//}
//
///*
//int PYXCurveRegion::CurveChunk::countIntersectionsSafe(const SphereMath::GreatCircleArc & ray)
//{
//	int intersectionCount = 0;
//	for(std::vector<CurveChunk>::iterator it = m_subChunks.begin(); it != m_subChunks.end(); ++it)
//	{
//		if (it->getArcsCount() == 1)
//		{
//			if (it->m_arc.intersects(ray))
//			{
//				intersectionCount++;
//			}
//		}
//		else
//		{
//			intersectionCount += it->countIntersectionsSafe(ray);
//		}
//	}
//
//	return intersectionCount;
//}
//*/
//
//
//
// 
// 
/*
void PYXCurveRegion::buildChunks()
{
	//find the farthest point from point #0
	double maxDistance = 0;
	unsigned int second = 0;
	for(unsigned int i=1;i<m_nodes.size();i++)
	{
		double distance = SphereMath::distanceBetween(m_nodes[0],m_nodes[i]);
		if (distance > maxDistance)
		{
			maxDistance = distance;
			second = i;
		}
	}

	//find the farthest point from point #second.
	unsigned int first = 0;
	maxDistance = 0;
	for(unsigned int i=0;i<m_nodes.size();i++)
	{
		if (i != second)
		{
			double distance = SphereMath::distanceBetween(m_nodes[second],m_nodes[i]);
			if (distance > maxDistance)
			{
				maxDistance = distance;
				first = i;
			}
		}
	}

	//create the first chunk with an arc between "first" and "second" point - which is almost the two farthest point
	//m_chunks.reset(new CurveChunk(*this,0,m_nodes.size()-1,SphereMath::GreatCircleArc(m_nodes[first],m_nodes[second])));

	//m_chunks->findSubChunks();
}
*/