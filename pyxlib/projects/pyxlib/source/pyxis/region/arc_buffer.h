#ifndef ARC_BUFFER_H
#define ARC_BUFFER_H
/******************************************************************************
curve_region.h

begin		: 2012-06-27
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/region/region.h"
#include "pyxis/region/curve_region.h"

#include "pyxis/derm/index.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/sphere_math.h"
#include "pyxis/utility/range.h"
#include "pyxis/utility/wire_buffer.h"

#include <boost/scoped_ptr.hpp>
#include <boost/shared_array.hpp>
#include <vector>
//This class is the data structure to store vertices for different regions
//Vertices are stored in a const buffer to avoid copies.
//Arcs can be accessed using getArc method that will create an arc on demand
//
//! TODO: this is class is not been tested!
class PYXLIB_DECL ArcBuffer
{
public:
	ArcBuffer(const std::vector<PYXPointer<PYXCurveRegion>> & regions);
	ArcBuffer(const ArcBuffer & arcBuffer);
	ArcBuffer(){}
	static void test();	
	
	ArcBuffer & operator = (const ArcBuffer & arcBuffer);
	
	void add( const PYXPointer<PYXCurveRegion> & ring ,PYXStringWireBuffer & countBuffer, PYXStringWireBuffer & vertexBuffer);
	SphereMath::GreatCircleArc getArc(int index) const;
	const void getArcs( int first , int until ,std::vector<SphereMath::GreatCircleArc> & result) const;
	
	int getSegmentsCount() const;
	int getSegmentSize(int index) const;
	PYXCoord3DDouble getSegmentVertex(int segmentIndex, int vertexIndex) const;

	//return number of arcs
	const int size() const;

	//get the bounding circle of all arcs
	PYXBoundingCircle getBoundingCircle() const;

	//find intersecting arcs with a circle from a given subset of arcs
	void findIntersectingArcs(const std::vector<Range<int>> & ranges, const PYXBoundingCircle & circle, std::vector<Range<int>> & result) const;

	//count intersection with a ray on a given subset of arcs
	int countIntersectionWithRay(const std::vector<Range<int>> & ranges, const SphereMath::GreatCircleArc & ray) const;

	//get distance from subset of all arcs
	double getDistanceTo(const std::vector<Range<int>> & ranges, const PYXCoord3DDouble & location) const;
	
	//get distance from all arcs
	double getDistanceTo(const PYXCoord3DDouble & location) const;
	
private:	

	PYXConstBufferSlice m_counts;
	PYXConstBufferSlice m_vertices;
	static const int VERTEX_SIZE=16; //size of a 3dcoord converted into a latlong and written in the buffer.

	mutable std::vector<SphereMath::GreatCircleArc> m_arcsCache;
	mutable std::vector<PYXBoundingCircle> m_arcsBoundingCircleCache;
	mutable boost::recursive_mutex m_cacheMutex;

	void populateArcsCacheIfNeeded(const std::vector<Range<int>> & ranges) const;
	void populateArcsCache() const;
public:
	friend PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const ArcBuffer & arcs);
	friend PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer, ArcBuffer & arcs);
};
PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const ArcBuffer & arcs);
PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer, ArcBuffer & arcs);


/*!
	CurveChunk represent a LOD chuck of a curve.

	the CurveCheck is two sided class:
	1. represent a collection of sub Chunks for finer details lookup quires.
	2. represent a single arc that describe this chuck. Moreover, it capture what error introduced by this approximation.

	the CurveRegion class uses this inner class to speed up queries.
	*/
	//class CurveChunk
	//{
	//private:
	//	PYXCurveRegion & m_curve;
	//	unsigned int m_start;
	//	unsigned int m_end;

	//	//! approximating arc
	//	SphereMath::GreatCircleArc m_arc;

	//	double m_distance;
	//	double m_error;

	//	//! finer details vector
	//	std::vector<CurveChunk> m_subChunks;

	//public:
	//	//! create a CurveChunk from node[start] to node[end].
	//	CurveChunk(PYXCurveRegion & curve,unsigned int start,unsigned int end);

	//	//! create a CurveChunk from node[start] to node[end] with a given approximating arc.
	//	CurveChunk(PYXCurveRegion & curve,unsigned int start,unsigned int end,const SphereMath::GreatCircleArc & arc);
	//	CurveChunk(const CurveChunk & other);		
	//	CurveChunk & operator = (const CurveChunk & other);

	//public:
	//	//! return the approximating arc.
	//	const SphereMath::GreatCircleArc & getArc() const { return m_arc; }

	//	//! return the real distance between starting node and end node
	//	double getDistance() const { return m_distance; }

	//	//! return the maximum approximation error
	//	double getError() const { return m_error; }

	//	//! return the ratio between the approximation error and the real distance
	//	double getErrorRatio() const { return m_error/m_distance; }

	//	//! return the amount of real arcs this chunk approximate
	//	unsigned int getArcsCount() const { return m_end-m_start; }

	//	//! return the amount of sub chucks created to better approximate this chunk
	//	unsigned int getSubChunksCount() const { return m_subChunks.size(); }

	//	//! get start node index
	//	unsigned int getStartNode() const { return m_start; }

	//	//! get end node index
	//	unsigned int getEndNode() const { return m_end; }

	//protected:
	//	void calcDistanceAndError();

	//protected:

	//	//! add nodes after the "end node" until they pas the given errorThrehsold or reached maxEndNode.
	//	bool addNextNodes(double errorThreshold,unsigned int maxEndNode);
	//	
	//	//! remove the last node from the chuck. recalculate the distance and error
	//	bool removeLastNode();

	//public:
	//	//! find sub chunks
	//	void findSubChunks();

	//	//! collect the minimal set of nodes from the curve that will not generate error more then the given errorThreshold
	//	void collectNodes(NodesVector & usedNodes,double errorThreshold);

	//	//! return the distance of the chuck to a given point with the give errorThreshold
	//	double distanceTo(const PYXCoord3DDouble & point, double errorThreshold = 0);

	//	/*! 
	//	count intersection of the given ray with this chuck, with the given errorThreshold.
	//	set numericlySafe to false if ray is "too close" to the given chunk, and result may be false.		
	//	*/
	//	//! count intersection of the given ray with this chuck, with the given errorThreshold.
	//	int countIntersections(const SphereMath::GreatCircleArc & ray,bool & numericlySafe, double errorThreshold = 0);

	//	//! print the tree of the curve chunks.
	//	void print();
	//	
	//public:
	//	//! return query without going over/creating subChunks
	//	bool canChunkIntersects(const SphereMath::GreatCircleArc & ray) const;

	//	//! return query without going over/creating subChunks
	//	bool canChunkBeCloserToPoint(const PYXCoord3DDouble & point, double distance) const;

	//private:
	//	void findSubChunks(int & chuckCreationCount,int & distanceCount);
	//	void print(const std::string & prefix,double totalDistance);
	//	void findMinDistance(const PYXCoord3DDouble & point,double & minDistance, double errorThreshold ,int & distanceCount);
	//	int countIntersections(const SphereMath::GreatCircleArc & ray,bool & numericlySafe, double errorThreshold ,int & distanceCount,int & intersectCount);
	//};

#endif // guard