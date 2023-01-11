/******************************************************************************
vector_geometry2.cpp

begin		: 2012-05-10
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/vector_geometry2.h"

// pyxlib includes
#include "pyxis/geometry/exceptions.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/geometry/vector_geometry.h"
#include "pyxis/geometry/geometry_intersection_utils.h"
#include "pyxis/region/circle_region.h"
#include "pyxis/derm/child_iterator.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/utility/coord_lat_lon.h"
#include "pyxis/utility/sphere_math.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/bit_utils.h"
#include "pyxis/region/curve_region.h"
#include "pyxis/region/region.h"





// standard includes
#include <cfloat>
#include <stack>
#include <queue>


//Enable this to debug serialization issues
//#define ADD_DEBUG_CHECK_INSIDE_SERIALIZED_TREE

//depth 6 tile have 1261 cells. However, raster a geometry into depth 6 we seldom get 1261 cells.
//on average, raster 6+3 resolutions deep will get us around 1000 tiles.
//A tradeoff between speed and details
const int SMALL_ENOUGH_TILE_DEPTH = 9;

const int MAX_TILE_COLLECTION_SIZE_TO_PERFORM_PER_TILE_INTERSECTION = 50;

// TEST
Tester<PYXVectorGeometry2> tester();


/////////////////////////////////////////////////////
// PYXVectorGeometry2
/////////////////////////////////////////////////////

PYXPointer<PYXGeometry> PYXVectorGeometry2::intersection(const PYXGeometry& geometry, bool bCommutative) const
{
	const PYXTile* const pTile = dynamic_cast<const PYXTile*>(&geometry);
	if (pTile != NULL)
	{
		return intersection(*pTile);
	}

	//TODO: see if we can make this faster
	initTileCollection();
	return m_tileCollection->intersection(geometry);
}

PYXPointer<PYXGeometry> PYXVectorGeometry2::intersection(const PYXTileCollection& collection) const
{
	//TODO: see if we can make this faster
	initTileCollection();
	return m_tileCollection->intersection(collection);
}

PYXPointer<PYXGeometry> PYXVectorGeometry2::intersection(const PYXTile& tile) const
{
	generateDermIndex();

	auto tileCollection = PYXTileCollection::create();
		
	for(auto & innerTile : PYXInnerTile::createInnerTiles(tile))
	{
		for(auto innerTileIterator = getInnerTileIterator(innerTile);!innerTileIterator->end();innerTileIterator->next())
		{
			if (innerTileIterator->getIntersection() != knIntersectionNone)
			{
				tileCollection->addTile(innerTileIterator->getTile().asTile());
			}
		};
	}

	return tileCollection;
}

PYXPointer<PYXGeometry> PYXVectorGeometry2::intersection(const PYXCell& cell) const
{
	//TODO: see if we can make this faster
	initTileCollection();
	return m_tileCollection->intersection(cell);
}

bool PYXVectorGeometry2::contains(const PYXGeometry& geometry) const
{
	const PYXCell* const pCell = dynamic_cast<const PYXCell*>(&geometry);
	if (pCell != nullptr)
	{
		generateDermIndex();

		bool result = m_serialized->intersects(pCell->getIndex()) == knIntersectionComplete;

#ifndef NDEBUG
		PYXPointer<PYXInnerTileIntersectionIterator> it = getInnerTileIterator(PYXInnerTile(pCell->getIndex(),pCell->getIndex().getResolution()));
		assert((it->getIntersection() == knIntersectionComplete) == result);
#endif
		return result;
	}

	
	const PYXTile* const pTile = dynamic_cast<const PYXTile*>(&geometry);
	if (pTile != nullptr)
	{
		generateDermIndex();
		
		for(auto & tile : PYXInnerTile::createInnerTiles(*pTile))
		{
			if (m_serialized->intersects(tile.getRootIndex()) != knIntersectionComplete)
			{
				return false;
			}
		}

		return true;
	}

	//small tile collection are easy to check
	const PYXTileCollection* const pTileCollection = dynamic_cast<const PYXTileCollection*>(&geometry);
	if (pTileCollection != nullptr && pTileCollection->getGeometryCount()<100)
	{
		generateDermIndex();
		
		auto iterator = pTileCollection->getTileIterator();
		while(!iterator->end())
		{			
			for(auto & tile : PYXInnerTile::createInnerTiles(*iterator->getTile()))
			{
				if (m_serialized->intersects(tile.getRootIndex()) != knIntersectionComplete)
				{
					return false;
				}
			}
			iterator->next();
		}

		return true;
	}
	
	//try to use the vector regions to speed things up
	PYXVectorRegion * otherVectorRegion = nullptr;
	PYXVectorRegion * vectorRegion = dynamic_cast<PYXVectorRegion*>(m_region.get());

	const PYXVectorGeometry * const vectorGeom = dynamic_cast<const PYXVectorGeometry*>(&geometry);
	if (vectorGeom != nullptr)
	{
		otherVectorRegion = vectorGeom->getRegion().get();
	}

	const PYXVectorGeometry2* const vectorGeom2 = dynamic_cast<const PYXVectorGeometry2*>(&geometry);
	if (vectorGeom2 != nullptr)
	{
		PYXPointer<IRegion> otherRegion = vectorGeom2->getRegion();
		otherVectorRegion = dynamic_cast<PYXVectorRegion*>(otherRegion.get());
	}

	if (vectorRegion != 0 && otherVectorRegion != 0)
	{
		PYXBoundingCircle circle = vectorRegion->getBoundingCircle();
		PYXBoundingCircle otherCircle = otherVectorRegion->getBoundingCircle();

		if (!circle.intersects(otherCircle))
		{
			return false;
		}

		if (otherCircle.contains(circle))
		{
			return false;
		}

		if (vectorRegion->getVerticesCount() < 100)
		{
			if (vectorRegion->intersects(otherCircle) == knIntersectionComplete)
			{
				return true;
			}
		}

		PYXTileCollection raster;
		geometry.copyTo(&raster,otherCircle.estimateResolutionFromRadius(otherCircle.getRadius()));
		assert(raster.getGeometryCount() < 10);

		if (contains(raster))
		{
			return true;
		}
	}	

	//TODO: see if we can make this faster
	initTileCollection();
	return m_tileCollection->contains(geometry);
}


bool PYXVectorGeometry2::intersects(const PYXGeometry& geometry, bool bCommutative) const
{	

	const PYXCell* const pCell = dynamic_cast<const PYXCell*>(&geometry);
	if (pCell != nullptr)
	{
		generateDermIndex();

		bool result = m_serialized->intersects(pCell->getIndex()) != knIntersectionNone;

#ifndef NDEBUG
		PYXPointer<PYXInnerTileIntersectionIterator> it = getInnerTileIterator(PYXInnerTile(pCell->getIndex(),pCell->getIndex().getResolution()));
		assert((it->getIntersection() != knIntersectionNone) == result);
#endif
		return result;
	}

	
	const PYXTile* const pTile = dynamic_cast<const PYXTile*>(&geometry);
	if (pTile != nullptr)
	{
		generateDermIndex();
		
		for(auto & tile : PYXInnerTile::createInnerTiles(*pTile))
		{
			if (m_serialized->intersects(tile.getRootIndex()) != knIntersectionNone)
			{
				return true;
			}
		}

		return false;
	}

	//small tile collection are easy to check
	const PYXTileCollection* const pTileCollection = dynamic_cast<const PYXTileCollection*>(&geometry);
	if (pTileCollection != nullptr)
	{
		//optimization for point regions
		auto point = dynamic_cast<const PYXVectorPointRegion *>(m_region.get());
		if (point)
		{
			PYXIcosIndex index;
			SnyderProjection::getInstance()->xyzToPYXIS(point->getPoint(),&index,pTileCollection->getCellResolution());			
			return pTileCollection->intersects(index);
		}

		generateDermIndex();

		//our geometry is smaller than the tile collection resolution. it better to rasterize  ourself.
		if (PYXBoundingCircle::estimateResolutionFromRadius(getBoundingCircle().getRadius()) >= pTileCollection->getCellResolution())
		{
			PYXTileCollection overview;
			copyTo(&overview,pTileCollection->getCellResolution());
			return overview.intersects(*pTileCollection);
		}
		
		//estimate small enough resolution
		auto tileCount = pTileCollection->getGeometryCount();

		if (tileCount < MAX_TILE_COLLECTION_SIZE_TO_PERFORM_PER_TILE_INTERSECTION) 
		{
			auto iterator = pTileCollection->getTileIterator();
			while(!iterator->end())
			{			
				for(auto & tile : PYXInnerTile::createInnerTiles(*iterator->getTile()))
				{
					if (m_serialized->intersects(tile.getRootIndex()) != knIntersectionNone)
					{
						return true;
					}
				}
				iterator->next();
			}
			return false;
		}

		auto candidates = PYXGeometryIntersectionUtils::createSmallOverview(geometry);
		candidates->setCellResolution(geometry.getCellResolution());		
		return PYXGeometryIntersectionUtils::intersectsAtTiles(*candidates, *this, geometry);
	}

	auto otherCircle = geometry.getBoundingCircle();
	auto thisCircle = getBoundingCircle();

	if (!thisCircle.intersects(otherCircle)) 
	{
		return false;
	}

	//if other geometry is a circle...
	if (PYXGeometryIntersectionUtils::isCircle(geometry)) 
	{
		auto vectorRegion = dynamic_cast<const PYXVectorRegion*>(m_region.get());
		if (vectorRegion) 
		{
			return vectorRegion->intersects(otherCircle) != PYXRegion::knNone;
		}
	}

	//if this geometry is a circle
	if (PYXGeometryIntersectionUtils::isCircle(*this))
	{
		auto vectorRegion = dynamic_cast<const PYXVectorRegion*>(PYXGeometryIntersectionUtils::extractRegion(geometry).get());
		if (vectorRegion)
		{
			return vectorRegion->intersects(thisCircle) != PYXRegion::knNone;
		}
	}

	generateDermIndex();

	PYXPointer<PYXTileCollection> tiles;
	if (thisCircle.getRadius() < otherCircle.getRadius()) 
	{
		tiles = PYXGeometryIntersectionUtils::createSmallOverview(*this);
	}
	else 
	{
		tiles = PYXGeometryIntersectionUtils::createSmallOverview(geometry);
	}

	//intersects resolutions is +SMALL_ENOUGH_TILE_DEPTH deep than overview resolution
	tiles->setCellResolution(std::min(PYXMath::knMaxAbsResolution,tiles->getCellResolution() + SMALL_ENOUGH_TILE_DEPTH));

	return PYXGeometryIntersectionUtils::intersectsAtTiles(*tiles, *this, geometry);
}

int PYXVectorGeometry2::getCellResolution() const
{
	return m_nResolution;
}

void PYXVectorGeometry2::setCellResolution(int nCellResolution)
{
	assert(0 <= nCellResolution && nCellResolution < PYXMath::knMaxAbsResolution);
	m_nResolution = nCellResolution;
}

PYXPointer<PYXIterator> PYXVectorGeometry2::getIterator() const
{
	initTileCollection();
	return m_tileCollection->getIterator();
}

PYXPointer<PYXGeometry> PYXVectorGeometry2::clone() const
{
	return create(*this);
}

void PYXVectorGeometry2::copyTo(PYXTileCollection* pTileCollection) const
{
	copyTo(pTileCollection, getCellResolution());
}

void PYXVectorGeometry2::copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const
{
	generateDermIndex();

	assert(pTileCollection != 0);
	pTileCollection->clear();
	for(PYXPrimeInnerTileIterator prime; !prime.end(); prime.next())
	{
		for(PYXPointer<PYXInnerTileIntersectionIterator> iterator=
			m_serialized->getIterator(PYXInnerTile(prime.getIndex(),nTargetResolution));
			!iterator->end();
			iterator->next())
		{
			if(iterator->getIntersection()!=knIntersectionNone)
			{
				pTileCollection->addTile(iterator->getTile().asTile());
			}
		}
	}
}


void PYXVectorGeometry2::test()
{
	RegionIndexTree::Test();

	// Create a Region
	std::vector<PYXCoord3DDouble> vertices;
	PYXCoord3DDouble coord;
	SnyderProjection::getInstance()->pyxisToXYZ(PYXIcosIndex("1-04030205"),&coord);
	vertices.push_back(coord);
	SnyderProjection::getInstance()->pyxisToXYZ(PYXIcosIndex("1-04000204"),&coord);
	vertices.push_back(coord);
	PYXPointer<PYXCurveRegion> curve = PYXCurveRegion::create(vertices);
	//Create the Tree
	PYXPointer<RegionIndexTree> tree= RegionIndexTree::create(curve);
	//Serialize Tree
	PYXStringWireBuffer buffer;
	tree->serialize(buffer);
	//Read
	PYXPointer<RegionIndexSerialized> serialized= RegionIndexSerialized::create(curve,buffer);
	//intersection
	PYXInnerTileIntersection result= serialized->intersects(PYXIcosIndex("1-04030205"));
	assert(result==knIntersectionPartial&&"Intersection result is wrong");

	result= serialized->intersects(PYXIcosIndex("1-04000204"));
	assert(result==knIntersectionPartial&&"Intersection result is wrong");

	result= serialized->intersects(PYXIcosIndex("1-04030004"));
	assert(result==knIntersectionNone&&"Intersection result is wrong");

	//iterator
	for(PYXPointer<PYXInnerTileIntersectionIterator> iterator1=serialized->getIterator(PYXInnerTile(PYXIcosIndex("1-04"),6));
		!iterator1->end();
		iterator1->next())
	{
		PYXInnerTile tile= iterator1->getTile();
		PYXInnerTileIntersection intersection = iterator1->getIntersection();
	}
}

void PYXVectorGeometry2::generateDermIndex() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	if (!m_serialized)
	{
		//Create the Tree
		PYXPointer<RegionIndexTree> tree= RegionIndexTree::create(m_region);
		//Serialize Tree
		PYXStringWireBuffer buffer;
		tree->serialize(buffer);
		//Read
		m_serialized= RegionIndexSerialized::create(m_region,buffer);
	}
}

void PYXVectorGeometry2::initTileCollection() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	if(!m_tileCollection)
	{
		PYXPointer<PYXTileCollection> tileCollection = PYXTileCollection::create();
		copyTo(tileCollection.get());
		m_tileCollection= tileCollection;
	}
}

void PYXVectorGeometry2::deserialize( PYXWireBuffer & buffer )
{
	PYXConstBufferSlice chunk;
	buffer >> m_nResolution;
	buffer >> chunk;
	m_region = PYXRegionSerializer::deserialize(chunk);
	buffer >> chunk;
	m_serialized = RegionIndexSerialized::create(m_region, PYXConstWireBuffer(chunk));
	
	m_tileCollection.reset();
}

void PYXVectorGeometry2::serialize( std::basic_ostream< char>& out ) const
{
	PYXStringWireBuffer buffer;
	serialize(buffer);
	std::string str = buffer.toString();
	size_t size = str.size();
	out.write((char *)&size,sizeof(size));
	out.write((const char *)str.c_str(),size);
}

void PYXVectorGeometry2::serialize( PYXWireBuffer & out ) const
{
	generateDermIndex();

	out << m_nResolution;
	out << PYXRegionSerializer::serialize(*(m_serialized->m_region));
	out << *m_serialized->m_buffer.getBuffer();
}

PYXPointer<PYXInnerTileIntersectionIterator> PYXVectorGeometry2::getInnerTileIterator( const PYXInnerTile & tile ) const
{
	generateDermIndex();

	return m_serialized->getIterator(tile);
}

PYXBoundingCircle PYXVectorGeometry2::getBoundingCircle() const
{
	PYXVectorRegion * vectorRegion = dynamic_cast<PYXVectorRegion*>(m_region.get());
	if (vectorRegion != 0)
	{
		return vectorRegion->getBoundingCircle();
	}
	initTileCollection();
	return m_tileCollection->getBoundingCircle();
}

//! Get the bounding box for this geometry.
void PYXVectorGeometry2::getBoundingRects(const ICoordConverter* coordConvertor,
	PYXRect2DDouble* pRect1,
	PYXRect2DDouble* pRect2) const
{
	PYXTileCollection collection;
	auto boundingCircle = getBoundingCircle();
	auto resolution = std::min(boundingCircle.estimateResolutionFromRadius(boundingCircle.getRadius())+5,getCellResolution());
	copyTo(&collection,resolution);
	collection.getBoundingRects(coordConvertor, pRect1, pRect2);
}



/////////////////////////////////////////////////////
// Region Index Tree
/////////////////////////////////////////////////////

PYXVectorGeometry2::RegionIndexTree::RegionIndexTree(const PYXPointer<IRegion> & region): m_region(region)
{
	PYXPointer<IRegionVisitor> visitor= region->getVisitor();
	for(PYXPrimeInnerTileIterator prime; !prime.end(); prime.next())
	{
		prime.getIndex();
		PYXPointer<IRegionVisitor> subVisitor=	visitor->trim(prime.getIndex());
		if(subVisitor)
		{
			m_root[prime.getPosition()].reset(new RegionIndexTreeNode(prime.getIndex(),subVisitor ));
			m_root[prime.getPosition()]->initialize();
		}
	}
}


void PYXVectorGeometry2::RegionIndexTree::serialize(PYXWireBuffer & buffer)
{
	int validRootsCount=0;
	for(int i=0; i<PYXPrimeInnerTileIterator::totalCount; i++)
	{
		if(m_root[i])
		{
			validRootsCount++;
		}
	}
	//write number of roots
	buffer<<validRootsCount;
	int startPos= buffer.pos();

#ifdef ADD_DEBUG_CHECK_INSIDE_SERIALIZED_TREE
	int childPosition=startPos+validRootsCount*10;//10 is the size of a root node (the node code +index + child offset)
#else
	int childPosition=startPos+validRootsCount*6;//6 is the size of a root node (the node code +index + child offset)
#endif


	int rootPosition=buffer.pos();
	buffer.setPos(childPosition,PYXWireBuffer::expandIfNeeded);
	for(int i=0; i<PYXPrimeInnerTileIterator::totalCount; i++)
	{
		if(m_root[i])
		{
			//write each root
			childPosition=buffer.pos();
			buffer.setPos(rootPosition,PYXWireBuffer::expandIfNeeded);
			m_root[i]->write(buffer,knPartialCoverRoot,childPosition);

			//write tree of each root
			rootPosition=buffer.pos();
			buffer.setPos(childPosition,PYXWireBuffer::expandIfNeeded);
			serializeBlock( *m_root[i],0,buffer);
		}
	}
}

void PYXVectorGeometry2::RegionIndexTree::serializeBlock(RegionIndexTreeNode & root,int baseResolution,PYXWireBuffer & buffer)
{
	std::stack<boost::shared_ptr<RegionIndexTreeNode>> stack;
	std::queue<boost::shared_ptr<RegionIndexTreeNode>> queue;
	std::queue<int> queuePos;
	boost::shared_ptr<RegionIndexTreeNode> children[7];
	
	bool hasChildren = false;
	for(PYXInnerChildIterator childItr(root.getIndex());!childItr.end();childItr.next())
	{
		PYXPointer<IRegionVisitor> trimmed = root.getVisitor()->trim(childItr.getIndex());


		if(trimmed != NULL)
		{
			//add child
			int childIndex=childItr.getIndex().getSubIndex().getLastDigit();
			root.setChild(childIndex);
			children[childIndex].reset(new RegionIndexTreeNode(childItr.getIndex() ,trimmed));
			hasChildren = true;
		}		
	}

	// PUSH  CHILDREN in reverse order
	for (int i=6; i>=0; i--)
	{
		if (children[i])
		{
			stack.push(children[i]);
		}
	}

	//Part1: Writing the tree nodes for this block in DFS order

	while (stack.size() > 0)
	{

		boost::shared_ptr<RegionIndexTreeNode> node = stack.top();
		stack.pop();
		boost::shared_ptr<RegionIndexTreeNode> children[7];
		hasChildren=false;

		if(	 node->getVisitor() &&
			!node->getVisitor()->isOptimal() &&
			node->getIndex().getResolution() < (PYXMath::knMaxAbsResolution - 4)) // make sure we don't continue trim to much
		{
			
			for(PYXInnerChildIterator childItr(node->getIndex());!childItr.end();childItr.next())
			{
				PYXPointer<IRegionVisitor> trimmed = node->getVisitor()->trim(childItr.getIndex());
				if(trimmed != NULL)
				{
					//add child
					int childIndex=childItr.getIndex().getSubIndex().getLastDigit();
					node->setChild(childIndex);
					children[childIndex].reset(new RegionIndexTreeNode(childItr.getIndex() ,trimmed));
					hasChildren = true;
				}		
			}
			//we don't need to keep not optimal visitors for nodes that have no children...
			if (!hasChildren)
			{
				node->clearVisitor();
			}
		}

		if (node->isInside())
		{			
			node->write(buffer,knCompleteInside);
		}
		else if (node->getChildrenCount()>0)
		{ 
			// there is no visitor for this node
			if (node->getIndex().getResolution() - baseResolution < m_subTreeDepth)
			{
				node->write(buffer,knPartialCover);

				for (int i=6;i>=0;i-- )
				{
					if (children[i])
					{
						stack.push(children[i]);
					}
				}
			}
			else
			{
				// it is a connection
				queue.push(node);
				queuePos.push(buffer.pos());
				node->write(buffer,knPartialCoverConnection);
			}
		}
		else
		{
			//Write the node + visitor (sometimes, we don't have a visitor)
			node->write(buffer,knPartialCoverTerminal,m_region);
		}
	}

	//Part2:
	//  1) write all sub-blocks.
	//  2) update the connection address after writing each sub-block

	while (0 < queue.size())
	{  //go back to every connection and fix it!
		boost::shared_ptr<RegionIndexTreeNode> connection = queue.front();
		int connectionPos = (int)queuePos.front();

		queue.pop();
		queuePos.pop();
		int currentPos = buffer.pos();
		buffer.setPos(connectionPos,PYXWireBuffer::expandIfNeeded);
		connection->write(buffer,knPartialCoverConnection,currentPos);
		buffer.setPos(currentPos,PYXWireBuffer::expandIfNeeded);
		//write every child of the connection
		serializeBlock(*connection, connection->getIndex().getResolution(), buffer);
	}
}




//void PYXVectorGeometry2::RegionIndexTree::serializeBlock(RegionIndexTreeNode & root,int baseResolution,PYXWireBuffer & buffer)
//{
//
//	std::stack<RegionIndexTreeNode*> stack;
//	std::queue<RegionIndexTreeNode*> queue;
//	std::queue<int> queuePos;
//
//	//////////////////////////////////////////////////////////////////////////
//	bool hasChildren = false;
//	if(	 root.getVisitor() &&
//		!root.getVisitor()->isOptimal())
//	{
//		for(PYXInnerChildIterator childItr(root.getIndex());!childItr.end();childItr.next())
//		{
//			PYXPointer<IRegionVisitor> trimmed = root.getVisitor()->trim(childItr.getIndex());
//			if(trimmed != NULL)
//			{
//				//add child
//				int childIndex=childItr.getIndex().getSubIndex().getLastDigit();
//				root.m_children[childIndex].reset(new RegionIndexTreeNode(childItr.getIndex() ,trimmed));
//				hasChildren = true;
//
//				//stack.push(root.m_children[childIndex].get());
//			}		
//		}
//
//		for (int i=6; i>=0; i--)
//		{
//			if (root.m_children[i])
//			{
//				stack.push(root.m_children[i].get());
//			}
//		}
//	}
//	else
//	{
//
//	}
//
//	//root.getVisitor().reset();
//	//////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//	//Part1: Writing the tree nodes for this block in DFS order
//
//	while (stack.size() > 0)
//	{
//
//		RegionIndexTreeNode * node = stack.top();
//		stack.pop();
//
//		if (node->m_cellIsInside)
//		{			
//			node->write(buffer,knCompleteInside);
//		}
//		else 
//		{ 
//			// initialize children & see if it has any
//
//			bool hasChildren = false;
//			if(	 //node->getVisitor() &&
//				!node->getVisitor()->isOptimal())
//			{
//				for(PYXInnerChildIterator childItr(node->getIndex());!childItr.end();childItr.next())
//				{
//					PYXPointer<IRegionVisitor> trimmed = node->getVisitor()->trim(childItr.getIndex());
//					if(trimmed != NULL)
//					{
//						//add child
//						int childIndex=childItr.getIndex().getSubIndex().getLastDigit();
//						node->m_children[childIndex].reset(new RegionIndexTreeNode(childItr.getIndex() ,trimmed));
//						hasChildren = true;
//					}		
//				}
//			}
//
//
//			if(!hasChildren)
//			{
//				//Write the node + visitor
//				node->write(buffer,knPartialCoverTerminal,m_region);
//			}
//			else if ( node->getIndex().getResolution() - baseResolution < m_subTreeDepth)
//			{
//
//				for (int i=6; i>=0; i--)
//				{
//					if (node->m_children[i])
//					{
//						stack.push(node->m_children[i].get());
//					}
//				}
//				node->write(buffer,knPartialCover);
//			}
//			else
//			{
//				// it is a connection
//				queue.push(node);
//				queuePos.push(buffer.pos());
//				node->write(buffer,knPartialCoverConnection);
//			}
//		}
//
//	}
//
//	//Part2:
//	//  1) write all sub-blocks.
//	//  2) update the connection address after writing each sub-block
//
//	while (0 < queue.size())
//	{  //go back to every connection and fix it!
//		RegionIndexTreeNode * connection = queue.front();
//		int connectionPos = (int)queuePos.front();
//
//		queue.pop();
//		queuePos.pop();
//		int currentPos = buffer.pos();
//		buffer.setPos(connectionPos,PYXWireBuffer::expandIfNeeded);
//		connection->write(buffer,knPartialCoverConnection,currentPos);
//		buffer.setPos(currentPos,PYXWireBuffer::expandIfNeeded);
//		//write every child of the connection
//		serializeBlock(*connection, connection->getIndex().getResolution(), buffer);
//	}
//}
//
//
//

void PYXVectorGeometry2::RegionIndexTree::Test()
{
	//TODO: add tests

}


/////////////////////////////////////////////////////
// Region Index Tree Node
/////////////////////////////////////////////////////

void PYXVectorGeometry2::RegionIndexTreeNode::initialize()
{

	bool hasChildren = false;
	for(PYXInnerChildIterator childItr(m_index);!childItr.end();childItr.next())
	{
		PYXPointer<IRegionVisitor> trimmed = m_visitor->trim(childItr.getIndex());
		if(trimmed != NULL)
		{
			//add child
			int childIndex=childItr.getIndex().getSubIndex().getLastDigit();
			setChild(childIndex);
			hasChildren = true;
		}		
	}

}


void PYXVectorGeometry2::RegionIndexTreeNode::write(PYXWireBuffer & buffer,const NodeType & type,int childOffset ,const PYXPointer<IRegion> & region)
{

#ifdef ADD_DEBUG_CHECK_INSIDE_SERIALIZED_TREE
	int debug_start=buffer.pos();
	int debug_check=666;
	buffer << debug_check;

	if (type != knPartialCoverRoot )
	{
		buffer << m_index.toString();
	}
#endif

	if ( type == knPartialCoverConnection)
	{
		m_code |= 128;
	}

	buffer<<m_code;

	assert((!isInside()||m_code==128)&&"If node is completely inside it should not have children");

	if (type ==knPartialCoverRoot)
	{
		buffer << (unsigned char) PYXPrimeInnerTileIterator::getPrimeInnerTileRootPosition(m_index);
		buffer << childOffset;
	}

	if (type == knPartialCoverConnection)
	{
		buffer << childOffset;
		assert(m_code > 128 && "Connection type is wrong!");
	}
	else if (type == knPartialCoverTerminal)
	{
		// If node is partial but none of the children intersect then we dont write the visitor.
		if (m_visitor)
		{
			// 1 - mark that there is visitor comming right after
			buffer << (unsigned char)(1);
			region->serializeVisitor(buffer,m_visitor);
		}
		else 
		{
			// 0 - mark that there is no visitor
			buffer << (unsigned char)(0);
		}
		assert(m_code==0 && "Terminal type is wrong!");
	}
}

std::auto_ptr<PYXVectorGeometry2::RegionIndexTreeNode>  PYXVectorGeometry2::RegionIndexTreeNode::createRoot(PYXWireBuffer & buffer)
{
#ifdef ADD_DEBUG_CHECK_INSIDE_SERIALIZED_TREE
	int debug_check;
	buffer>>debug_check;
	assert(debug_check==666&&"Not a good place to read a Root");
#endif

	std::auto_ptr<RegionIndexTreeNode> returnNode(new RegionIndexTreeNode());
	unsigned char code;
	buffer >> code;
	unsigned char rootPosition;
	buffer >> rootPosition;

	for (int i = 0; i < 7; i++)
	{
		if((code & 1)==1)
		{
			returnNode->setChild(i);
		}
		code = (unsigned char)(code >> 1);
	}
	returnNode->m_index=PYXPrimeInnerTileIterator::getPrimeInnerTileRootByPosition(rootPosition);

	buffer >> returnNode->m_childrenOffset;
	returnNode->type = knPartialCoverRoot;
	returnNode->m_address = buffer.pos();
	return returnNode;
}

void PYXVectorGeometry2::RegionIndexTreeNode::deseralizeNode(const PYXPointer<IRegion> & region, PYXWireBuffer & buffer,const PYXIcosIndex & index)
{
#ifdef ADD_DEBUG_CHECK_INSIDE_SERIALIZED_TREE
	int debug_check;
	buffer>>debug_check;
	assert(debug_check==666&&"Not a good place to read a node");

	std::string indexString;
	buffer >> indexString;
	setIndex(indexString);

	if (!index.isNull())
	{
		assert(getIndex()==index);
	}

#endif
	setIndex(index);
	unsigned char tmp;
	buffer >> tmp;
	if (tmp == 0)// terminal node!
	{
		type = knPartialCoverTerminal;
		//chcek if we have a visitor...
		buffer >> tmp;
		if (tmp != 0)
		{
			m_visitor = region->deserializeVisitor(buffer,index);
		}
		else 
		{
			m_visitor.reset();
		}
	}
	else if (tmp == 128)//Complete Inside Node
	{
		type = knCompleteInside;
	}
	else
	{
		m_code = tmp & 0x7F; //clear 8bit.
		/*
		for (int i = 0; i < 7; i++)
		{
			if((tmp & 1)==1)
			{
				setChild(i);
			}
			tmp = (unsigned char)(tmp >> 1);
		}
		*/
		if ((tmp & BitUtils::knBit8) != 0)
		{
			//connection node
			type = knPartialCoverConnection;
			buffer >> m_childrenOffset;
		}
		else
		{
			//Ordinary node
			type = knPartialCover;
		}
	}

	m_address = buffer.pos();
}

std::auto_ptr<PYXVectorGeometry2::RegionIndexTreeNode> PYXVectorGeometry2::RegionIndexTreeNode::create(const PYXPointer<IRegion> & region, PYXWireBuffer & buffer,const PYXIcosIndex & index)
{
	std::auto_ptr<RegionIndexTreeNode> returnNode(new RegionIndexTreeNode());

	returnNode->deseralizeNode(region,buffer,index);

	/*
#ifdef ADD_DEBUG_CHECK_INSIDE_SERIALIZED_TREE
	int debug_check;
	buffer>>debug_check;
	assert(debug_check==666&&"Not a good place to read a node");

	std::string indexString;
	buffer >> indexString;
	returnNode->setIndex(indexString);

#endif
	returnNode->setIndex(index);
	unsigned char tmp;
	buffer >> tmp;
	if (tmp == 0)// terminal node!
	{
		returnNode->type = knPartialCoverTerminal;
		//chcek if we have a visitor...
		buffer >> tmp;
		if (tmp != 0)
		{
			returnNode->m_visitor = region->deserializeVisitor(buffer,index);
		}
	}
	else if (tmp == 128)//Complete Inside Node
	{
		returnNode->type = knCompleteInside;
	}
	else
	{
		for (int i = 0; i < 7; i++)
		{
			if((tmp & 1)==1)
			{
				returnNode->setChild(i);
			}
			tmp = (unsigned char)(tmp >> 1);
		}
		if (tmp == 1)
		{
			//connection node
			returnNode->type = knPartialCoverConnection;
			buffer >> returnNode->m_childrenOffset;
		}
		else
		{
			//Ordinary node
			returnNode->type = knPartialCover;
		}
	}

	returnNode->m_address = buffer.pos();
	*/
	return returnNode;
}

std::auto_ptr<PYXVectorGeometry2::RegionIndexTreeNode> PYXVectorGeometry2::RegionIndexTreeNode::createSkipChildren(const PYXPointer<IRegion> & region, PYXWireBuffer & buffer,const PYXIcosIndex & index)
{
	std::auto_ptr<RegionIndexTreeNode> cellreader(RegionIndexTreeNode::create(region,buffer,index));

	if (cellreader->type == knPartialCover)
	{
		PYXIcosIndex emptyIndex;

		for (int i=0;i<7;i++)
		{
			if (cellreader->getChild(i)) //BitUtils::...
			{
				createSkipChildren(region,buffer,emptyIndex); // we dont really care what index we pass here but we cant use null
			}
		}
	}
	return cellreader;
}




const int PYXVectorGeometry2::RegionIndexTreeNode::numberOfChildrenBefore(int childIndex)
{
	int ret = 0;
	for (int i = 0; i < childIndex; i++)
	{
		if (getChild(i))
		{
			ret++;
		}
	}
	return ret;
}




const int PYXVectorGeometry2::RegionIndexTreeNode::getChildrenCount()  
{
	return numberOfChildrenBefore(7);
}

void PYXVectorGeometry2::RegionIndexTreeNode::Test()
{

	// Create a Region
	std::vector<PYXCoord3DDouble> vertices;
	PYXCoord3DDouble coord;
	SnyderProjection::getInstance()->pyxisToXYZ(PYXIcosIndex("1-0000"),&coord);
	vertices.push_back(coord);
	SnyderProjection::getInstance()->pyxisToXYZ(PYXIcosIndex("1-04030205"),&coord);
	vertices.push_back(coord);
	PYXPointer<PYXCurveRegion> curve = PYXCurveRegion::create(vertices);

	// Writing and Create Tests
	PYXPointer<IRegion> nullPointer;
	{
		{
			RegionIndexTreeNode node;
			PYXStringWireBuffer buffer;

			// Ordinary Node
			node.setChild(0);
			node.setChild(3);
			node.setChild(4);
			node.setChild(6);

			node.write(buffer,knPartialCover);
			buffer.setPos(0);
			assert(buffer.size()==1 && "Ordinary node is serlialized longer than expected!");
			unsigned char code;
			buffer >> code;
			assert(code==89 && "Wrong ordinary code!");

			buffer.setPos(0);
			std::auto_ptr <RegionIndexTreeNode> readNode= RegionIndexTreeNode::create(nullPointer,buffer,PYXIcosIndex());
			assert(readNode->getChild(0)&&"Child is not read correctly");
			assert(readNode->getChild(3)&&"Child is not read correctly");
			assert(readNode->getChild(4)&&"Child is not read correctly");
			assert(readNode->getChild(6)&&"Child is not read correctly");
			assert(readNode->getChildrenCount()==4 && "Children are not read correctly");
			assert(readNode->numberOfChildrenBefore(4)==2 && "Children are not read correctly");
			assert(readNode->type==knPartialCover && "Children are not read correctly");
		}

		// Terminal Node
		{
			RegionIndexTreeNode node;
			PYXStringWireBuffer buffer;
			unsigned char  code=255;
			node.m_visitor=curve->getVisitor();

			buffer.setPos(14,PYXWireBuffer::expandIfNeeded);
			node.write(buffer,knPartialCoverTerminal,0,curve);
			buffer.setPos(14);

#ifdef ADD_DEBUG_CHECK_INSIDE_SERIALIZED_TREE
			int debug_sixsixisix;
			buffer >>debug_sixsixisix;
			assert(debug_sixsixisix==666&& "!");
#endif
			buffer >> code;
			assert(code==0 && "Wrong Terminal code!");

			buffer.setPos(0);
			std::auto_ptr <RegionIndexTreeNode> readNode= RegionIndexTreeNode::create(curve,buffer,PYXIcosIndex());
			assert(readNode->getChildrenCount()==0 && "Children are not read correctly");
			assert(readNode->numberOfChildrenBefore(4)==0 && "Children are not read correctly");
			assert(readNode->type==knPartialCoverTerminal && "Children are not read correctly");

		}
		// Connection Node
		{
			RegionIndexTreeNode node;
			PYXStringWireBuffer buffer;
			unsigned char  code=255;
			int	childrenOffset=0;

			node.setChild(1);
			node.setChild(5);


			node.write(buffer,knPartialCoverConnection,123);
			buffer.setPos(0);
			buffer >> code;
			buffer >> childrenOffset;
			assert(buffer.size()==5 &&"Wrong connection node size!");
			assert(code==128+34 && "Wrong Connection code!");
			assert(childrenOffset==123 && "Wrong ChildrenOffset !");

			buffer.setPos(0);
			std::auto_ptr <RegionIndexTreeNode> readNode= RegionIndexTreeNode::create(nullPointer,buffer,PYXIcosIndex());
			assert(readNode->getChildrenCount()==2 && "Children are not read correctly");
			assert(readNode->numberOfChildrenBefore(4)==1 && "Children are not read correctly");
			assert(readNode->m_childrenOffset==123 && "Children are not read correctly");
			assert(readNode->type==knPartialCoverConnection && "Children are not read correctly");

		}
		// Root Node
		{
			RegionIndexTreeNode node;
			PYXStringWireBuffer buffer;
			unsigned char  code=255;
			unsigned char indexCode;
			int	childrenOffset=0;

			node.setChild(1);
			node.setChild(5);

			node.m_index= PYXPrimeInnerTileIterator::getPrimeInnerTileRootByPosition(66);

			node.write(buffer,knPartialCoverRoot,123);
			buffer.setPos(0);
			buffer >> code;
			buffer >> indexCode;
			buffer >> childrenOffset;

			assert(buffer.size()==6 &&"Wrong Root node size!");
			assert(code==34 && "Wrong Root code!");
			assert(indexCode==66 && "Wrong Root Index!");
			assert(childrenOffset==123 && "Wrong ChildrenOffset !");

			buffer.setPos(0);
			std::auto_ptr <RegionIndexTreeNode> readNode= RegionIndexTreeNode::createRoot(buffer);
			assert(readNode->getChildrenCount()==2 && "Root Children are not read correctly");
			assert(readNode->m_index==node.m_index && "Root Index is not read correctly");
			assert(readNode->m_childrenOffset==123 && "Root Children offset are not read correctly");
			assert(readNode->type==knPartialCoverRoot && "Root type are not read correctly");
		}
	}

}

/////////////////////////////////////////////////////
// Region Index Serialized
/////////////////////////////////////////////////////

PYXVectorGeometry2::RegionIndexSerialized::RegionIndexSerialized(const PYXPointer<IRegion> & region, PYXWireBuffer & buffer) : m_region(region), m_buffer(buffer)
{
	m_buffer.setPos(0);
}

PYXInnerTileIntersection PYXVectorGeometry2::RegionIndexSerialized::intersects(const PYXIcosIndex& index) const
{
	PYXConstWireBuffer buffer=PYXConstWireBuffer(m_buffer);
	buffer.setPos(0);
	std::auto_ptr<RegionIndexTreeNode> node;
	int indexLength= index.getSubIndex().getDigitCount();

	//	reading the roots
	int rootCount;
	buffer >> rootCount;


	PYXIcosIndex  rootindex( PYXPrimeInnerTileIterator::getPrimeInnerTileRoot(index));
	for (int i=0;i<rootCount;i++)
	{
		node= RegionIndexTreeNode::createRoot(buffer);
		if(node->getIndex()==rootindex )
		{
			break;	
		}
	}

	if(node->getIndex()!=rootindex )
	{
		return knIntersectionNone;
	}

	
	if (node->getIndex()==index)
	{
		//this is a primeRoot we test against. we need check if all its children are completey inside
		int completeChildren = index.isPentagon()?6:7;
		int childCount = node->getChildrenCount();
		if (childCount != completeChildren ) 
		{
			//root don't have all children - there the intersection is partial
			return knIntersectionPartial;
		}
		else 
		{
			//root have all children - make sure they all knCompleteInside
			buffer.setPos(node->m_childrenOffset);
			for(int i=0;i<childCount;++i)
			{
				PYXIcosIndex childIndex(index);
				childIndex.getSubIndex().appendDigit(i);
				node = RegionIndexTreeNode::createSkipChildren(m_region,buffer,childIndex);	

				if (node->type != knCompleteInside)
				{
					return knIntersectionPartial;
				}
			}	
			return knIntersectionComplete;
		}		
	}

	// finished reading the root the correct root is in node!
	for(int i=node->getIndex().getSubIndex().getDigitCount();i<indexLength ;i++)
	{
		if(node->type==knCompleteInside)
		{
			return knIntersectionComplete;
		}
		if(node->type==knPartialCoverTerminal)
		{
			if(node->getVisitor())
			{
				return node->getVisitor()->intersects(index);
			}
			else if (indexLength==i)
			{
				return knIntersectionPartial;
			}
			else
			{
				return knIntersectionNone;
			}
		}
		// Update to next inner tile index
		if (i < indexLength - 1 && index.getSubIndex().getDigit(i+1) != 0 )
		{
			i++;
		}
		// find the index of the node to be read!
		int indexDigit = index.getSubIndex().getDigit(i);

		if(!node->getChild(indexDigit))
		{
			//outside
			return knIntersectionNone;
		}

		//read the child
		if(node->type==knPartialCoverConnection || node->type==knPartialCoverRoot )
		{
			//connection and root children are positioned at m_childrenOffset
			buffer.setPos(node->m_childrenOffset);
		}

		PYXIcosIndex childIndex(node->getIndex());
		childIndex.getSubIndex().appendDigit(0);
		if(indexDigit>0)
		{
			childIndex.getSubIndex().appendDigit(indexDigit);
		}

		//skip all children before the child we are intereseted in
		int skips = node->numberOfChildrenBefore(indexDigit);
		PYXIcosIndex emptyIndex;
		for (int j = 0; j < skips; j++)
		{
			RegionIndexTreeNode::createSkipChildren(m_region,buffer,emptyIndex);// index here is not important
		}

		node = RegionIndexTreeNode::create(m_region,buffer,childIndex);

		// if it is the last iteration if the node exists return partial or complete. to avoid reading after end of buffer
		if (i >= indexLength - 1)
		{

			return (node->type == knCompleteInside) ? 
				knIntersectionComplete : knIntersectionPartial;
		}
	}
	assert(0 && "Should never reach here!");
	PYXTHROW(PYXException,"Should never reach here!");
}



/////////////////////////////////////////////////////
// Region Index Serialized Iterator
/////////////////////////////////////////////////////


PYXVectorGeometry2::RegionIndexSerialized::Iterator::Iterator(
	const RegionIndexSerialized & serializedIndex,
	const PYXInnerTile & tile)
	: m_buffer(serializedIndex.m_buffer),
	m_region(serializedIndex.m_region),
	m_cellResolution(tile.getCellResolution()),
	m_ended(false),
	m_innerTile(tile)
{
	findFirstNode(tile);
}

void PYXVectorGeometry2::RegionIndexSerialized::Iterator::findFirstNode(const PYXInnerTile & tile)
{
	PYXIcosIndex index = tile.getRootIndex();
	m_buffer.setPos(0);
	std::auto_ptr<RegionIndexTreeNode> node;
	int indexLength= index.getSubIndex().getDigitCount();

	//	reading the roots
	int rootCount;
	m_buffer >> rootCount;

	PYXIcosIndex rootindex( PYXPrimeInnerTileIterator::getPrimeInnerTileRoot(index));
	for (int i=0;i<rootCount;i++)
	{
		node= RegionIndexTreeNode::createRoot(m_buffer);
		if(node->getIndex()==rootindex )
		{
			break;
		}
	}

	if(node->getIndex()!=rootindex )
	{
		m_currentTile = tile;
		m_currentIntersection = knIntersectionNone;
		return;
	}

	// finished reading the root the correct root is in node!
	for(int i=node->getIndex().getSubIndex().getDigitCount();i<indexLength ;i++)
	{
		if(node->type==knCompleteInside)
		{
			m_currentTile = tile;
			m_currentIntersection = knIntersectionComplete;
			return;
		}
		if(node->type==knPartialCoverTerminal)
		{
			if(node->getVisitor())
			{
				m_geometryVisitor = node->getVisitor()->getInnerTileIterator(tile);
				m_currentTile = m_geometryVisitor->getTile();
				m_currentIntersection = m_geometryVisitor->getIntersection();
				return;
			}
			else
			{
				m_currentTile = tile;
				m_currentIntersection = knIntersectionNone;
				return;
			}
		}
		// Update to next inner tile index
		if (i < indexLength - 1 && index.getSubIndex().getDigit(i+1) != 0 )
		{
			i++;
		}
		// find the index of the node to be read!
		int indexDigit = index.getSubIndex().getDigit(i);

		if(!node->getChild(indexDigit))
		{
			//outside
			m_currentTile = tile;
			m_currentIntersection = knIntersectionNone;
			return;
		}

		//read the child
		if(node->type==knPartialCoverConnection || node->type==knPartialCoverRoot )
		{
			//connection and root children are positioned at m_childrenOffset
			m_buffer.setPos(node->m_childrenOffset);
		}

		PYXIcosIndex childIndex(node->getIndex());
		childIndex.getSubIndex().appendDigit(0);
		if(indexDigit>0)
		{
			childIndex.getSubIndex().appendDigit(indexDigit);
		}

		//skip all children before the child we are intereseted in
		int skips = node->numberOfChildrenBefore(indexDigit);
		PYXIcosIndex emptyIndex;
		for (int j = 0; j < skips; j++)
		{
			RegionIndexTreeNode::createSkipChildren(m_region,m_buffer,emptyIndex);// index here is not important
		}

		node = RegionIndexTreeNode::create(m_region,m_buffer,childIndex);
	}

	//if we got here - we found a partial node that match the tile root index
	std::auto_ptr<NodeInfo> nodeInfo(new NodeInfo());
	nodeInfo->node = node;
	nodeInfo->childIndex = 0;
	m_nodesStack.push_front(nodeInfo);

	if(!m_nodesStack.empty())
	{
		next();
	}
}



bool PYXVectorGeometry2::RegionIndexSerialized::Iterator::end() const
{
	return m_ended;
}

bool PYXVectorGeometry2::RegionIndexSerialized::Iterator::nextUsingGeometryVisitor()
{
	m_geometryVisitor->next();
	if (!m_geometryVisitor->end())
	{
		m_currentTile = m_geometryVisitor->getTile();
		m_currentIntersection = m_geometryVisitor->getIntersection();
		//	assert(m_currentTile.getRootIndex().isDescendantOf(m_innerTile.getRootIndex())&&"jumped out of the tile!");
		return true;
	}
	else 
	{
		m_geometryVisitor.reset();
		return false;
	}
}

void PYXVectorGeometry2::RegionIndexSerialized::Iterator::next()
{
	if (m_ended)
	{
		return;
	}

	if (m_geometryVisitor)
	{
		if (nextUsingGeometryVisitor())
		{
			return;
		}
	}

	findNextTile();
}

void PYXVectorGeometry2::RegionIndexSerialized::Iterator::pop()
{
	NodeInfo & node =m_nodesStack.front();
	if (node.node->type == knPartialCoverConnection)
	{
		m_buffer.setPos(node.node->m_address);
	}
	m_nodesStack.pop_front();
}

void PYXVectorGeometry2::RegionIndexSerialized::Iterator::findNextTile()
{
	if (m_nodesStack.empty())
	{
		m_ended = true;
		return;
	}

	NodeInfo & nodeInfo = m_nodesStack.front();
	RegionIndexTreeNode * node = nodeInfo.node.get();
	bool inTraverseMode=(node->getIndex().getResolution()>= m_innerTile.getRootIndex().getResolution());

	// if cell is inside Visit cell with INSIDE
	if (node->type == knCompleteInside)
	{
		m_currentTile= PYXInnerTile((inTraverseMode)? node->getIndex():m_innerTile.getRootIndex(),m_cellResolution);
		m_currentIntersection=knIntersectionComplete;
		pop();
		return;
	}

	//If cell is terminal then do Geometric tests
	else if (node->type == knPartialCoverTerminal)
	{
		//if the node has visitor - use the visitor
		if (node->getVisitor())
		{
			m_geometryVisitor = node->getVisitor()->getInnerTileIterator(PYXInnerTile((inTraverseMode)? node->getIndex():m_innerTile.getRootIndex(),m_cellResolution));
			m_currentTile = m_geometryVisitor->getTile();
			m_currentIntersection = m_geometryVisitor->getIntersection();
		}
		else 
		{
			//node doesn't have a visitor, this mean the cell intersect - but none of this children does.
			if (node->getIndex().getResolution() == m_cellResolution)
			{
				//this is the cell - return partial
				m_currentTile = PYXInnerTile(node->getIndex(),m_cellResolution);
				m_currentIntersection=knIntersectionPartial;
			}
			else 
			{
				//we are trying to iterate over the children - and they all doesn't intersect - return none
				m_currentTile = PYXInnerTile(node->getIndex(),m_cellResolution);
				m_currentIntersection=knIntersectionNone;
			}
		}
		pop();
		return;

	}

	// if cell is the final resolution then do it!
	else if (node->getIndex().getResolution() == m_cellResolution)
	{
		m_currentTile = PYXInnerTile(node->getIndex(),m_cellResolution);
		m_currentIntersection=knIntersectionPartial;
		pop();
		return;
	}
	//we finish all the children
	else if (nodeInfo.childIndex == 7)
	{
		pop();
		findNextTile();
		return;
	}

	// if cell is less than final resolution  and it is ordinary or connection (aka - it has children)
	else if (node->getIndex().getResolution() < m_cellResolution)
	{		
		//first child
		if (nodeInfo.childIndex==0)
		{
			// go to the children
			if(node->type==knPartialCoverConnection||node->type==knPartialCoverRoot)
			{
				nodeInfo.offsetReset=m_buffer.pos();
				m_buffer.setPos(node->m_childrenOffset);
			}
		}
		PYXIcosIndex index(node->getIndex());
		index.getSubIndex().appendDigit(0);
		if(nodeInfo.childIndex>0)
		{
			index.getSubIndex().appendDigit(nodeInfo.childIndex);
		}
		if ( node->getChild(nodeInfo.childIndex))
		{
			std::auto_ptr<RegionIndexTreeNode> tmp;
			bool needToSkip = false;
			if(index.getResolution() < m_cellResolution)
			{
				tmp = RegionIndexTreeNode::create(m_region,m_buffer,index);
				needToSkip = tmp->type != knPartialCoverConnection;
			}
			else
			{
				tmp = RegionIndexTreeNode::createSkipChildren(m_region,m_buffer,index);
			}

#ifdef ADD_DEBUG_CHECK_INSIDE_SERIALIZED_TREE
			assert(tmp->getIndex() == index);
#endif

			if(tmp->getIndex().getResolution()<=m_cellResolution &&
				(inTraverseMode || PYXInnerTile::isAncestorOf(index,m_innerTile.getRootIndex())))
			{
				std::auto_ptr<NodeInfo> newNodeInfo(new NodeInfo());
				newNodeInfo->node = tmp;
				newNodeInfo->childIndex = 0;
				m_nodesStack.push_front(newNodeInfo);
			}
			else if (needToSkip)
			{
				PYXIcosIndex emptyIndex;
				for(int skipCount = tmp->getChildrenCount();skipCount>0;skipCount--)
				{
					tmp = RegionIndexTreeNode::createSkipChildren(m_region,m_buffer,emptyIndex);
				}
			}
			nodeInfo.childIndex++;
			findNextTile();
		}
		else if(!node->getChild(nodeInfo.childIndex))
		{
			nodeInfo.childIndex++;
			if(index.getResolution()<=m_cellResolution)
			{
				m_currentTile = PYXInnerTile((inTraverseMode)? index:m_innerTile.getRootIndex(),m_cellResolution);
				m_currentIntersection=knIntersectionNone;
			}
			else
			{
				findNextTile();
			}
		}
	}
	else
	{
		assert(0 && "we should never get here");
		PYXTHROW(PYXException,"we should never get here");
	}
}


