#ifndef PYXIS__GEOMETRY__VECTOR_GEOMETRY2_H
#define PYXIS__GEOMETRY__VECTOR_GEOMETRY2_H
/******************************************************************************
vector_geometry2.h

begin		: 2012-05-10
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/index.h"
#include "pyxis/geometry/cell.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/geometry/inner_tile.h"
#include "pyxis/geometry/inner_tile_intersection_iterator.h"
#include "pyxis/region/region.h"
#include "pyxis/region/vector_point_region.h"
#include "pyxis/region/curve_region.h"
#include "pyxis/region/region_serializer.h"
#include "pyxis/utility/wire_buffer.h"

#include "boost/scoped_ptr.hpp"
#include "boost/ptr_container/ptr_deque.hpp"

//#include <stack>
/*
This class represents a region using a hibrid data structure.
data structure has a hexagonal tree index. 
The tree leaves point to a portion of the region.
Once down to a portion of a region "Visitors" will go geometrics oprations locally on that part

*/
//TODO : - move to a multi-buffer. A const buffer that has many type of regions combined
//TODO : - remove Index from node. A node should not have an index, index is calculated while traversing the tree.
//TODO :- skip tree creation. create nodes directly on the serialized version

class PYXLIB_DECL PYXVectorGeometry2 : public PYXGeometry
{
private:


	//
	enum NodeType
	{
		//AB..B -> this is the format of the code byte used to serialize the node
		//A is the first bit on left and B's are the other bits that represent 7 children
		//x..x represents that at least one of children bit is 1
		//0..0 represents that all children bit are zero
		knPartialCover, //0x..x
		knCompleteInside, //10..0
		knPartialCoverConnection, //1x..x
		knPartialCoverTerminal,//00..0
		// Roots are formated in 2 bytes first byte is the index and second byte represents the children
		knPartialCoverRoot,
		
	};

	/////////////////////////////////////////
	// Region Index Tree Node
	/////////////////////////////////////////
	class RegionIndexTreeNode
#ifdef INSTANCE_COUNTING
		: protected InstanceCounter
#endif
	{
	public: //Functions
		void initialize();
		RegionIndexTreeNode(const PYXIcosIndex & index, const PYXPointer<IRegionVisitor> & visitor ):m_index(index),m_visitor(visitor)
			{
				m_code = 0;
				PYXCompletelyInsideVisitor * inside= dynamic_cast<PYXCompletelyInsideVisitor*>(visitor.get());
				if(inside)
				{
					m_code |= 128;
				}
			}
		RegionIndexTreeNode()
		{
			m_code =0;
		}

		void deseralizeNode(const PYXPointer<IRegion> & region, PYXWireBuffer & buffer,const PYXIcosIndex & index);
		
		void write(PYXWireBuffer & buffer,const NodeType & type)
		{
			PYXPointer<IRegion> nullPointer; write(buffer,type,0,nullPointer);
		}
		void write(PYXWireBuffer & buffer,const NodeType & type,int childOffset )
		{
			PYXPointer<IRegion> nullPointer; write(buffer,type,childOffset,nullPointer);
		}
		void write(PYXWireBuffer & buffer,const NodeType & type,const PYXPointer<IRegion> & region)
		{
			 write(buffer,type,0,region);
		}
		void write(PYXWireBuffer & buffer,const NodeType & type,int childOffset ,const PYXPointer<IRegion> & region);

		static std::auto_ptr<PYXVectorGeometry2::RegionIndexTreeNode> create(const PYXPointer<IRegion> & region, PYXWireBuffer & buffer,const PYXIcosIndex & index);
		static std::auto_ptr<PYXVectorGeometry2::RegionIndexTreeNode> createRoot(PYXWireBuffer & buffer);
		static std::auto_ptr<PYXVectorGeometry2::RegionIndexTreeNode> createSkipChildren(const PYXPointer<IRegion> & region, PYXWireBuffer & buffer,const PYXIcosIndex & index);
	   
		const int numberOfChildrenBefore (int childIndex);
		const int getChildrenCount();
		const PYXIcosIndex & getIndex() const { return m_index; } 
		void setIndex(const PYXIcosIndex & index){m_index= index;}
		const PYXPointer<IRegionVisitor> & getVisitor() const { return m_visitor; }
		void clearVisitor() { m_visitor.reset(); }
		const bool isInside()
		{
			return (m_code )==128;
		}
		void setChild(int i)
		{
			m_code |= (1<<i);
		}
		bool getChild(int i)
		{
			return (m_code >> i & 1)==1;
		}
		static void Test();

	public:	//Members

		NodeType type;
		int m_address;
		int m_childrenOffset;

	private:
		PYXIcosIndex m_index;
		PYXPointer<IRegionVisitor> m_visitor;
		unsigned char m_code;

	};

	/////////////////////////////////////////
	// Region Index Tree
	/////////////////////////////////////////
	class RegionIndexTree : public PYXObject 
	{
	private:
		static const int m_subTreeDepth = 5;

	public:
		RegionIndexTree(const PYXPointer<IRegion> & region);
		static PYXPointer<RegionIndexTree> create(const PYXPointer<IRegion> & region)
		{
			return PYXNEW(RegionIndexTree,region);
		}
		void serialize(PYXWireBuffer & buffer);
		static void Test();

	private:
		void serializeBlock(RegionIndexTreeNode & node,int baseRez,PYXWireBuffer & buffer);
		
	private:
		boost::scoped_ptr<RegionIndexTreeNode> m_root[PYXPrimeInnerTileIterator::totalCount];
		PYXPointer<IRegion> m_region;
	};
	
	
	/////////////////////////////////////////
	// Region Index Serialized
	/////////////////////////////////////////
	/*
	This class writes a tree structure of the region
	Each tree node represent a cell and the parts of region within that cell
	Nodes have different types (see node type enum).

	//What the difference between this class and tree class
	
	//What the order of writing the nodes
	
	//What is the format of writing a node
	*/	
	class RegionIndexSerialized: public PYXObject
	{
	private:
		class Iterator : public PYXInnerTileIntersectionIterator 
		{
			struct NodeInfo
			{
				std::auto_ptr<RegionIndexTreeNode> node;
				int childIndex;
				size_t offsetReset;
			};

		
		public:
			static PYXPointer<PYXInnerTileIntersectionIterator> 
							   create(  const RegionIndexSerialized & serializedIndex,
										const PYXInnerTile & tile)
			{
				return PYXNEW(Iterator,serializedIndex,tile);
			}
			
			Iterator(const RegionIndexSerialized & serializedIndex,
					 const PYXInnerTile & tile);

		

		public: //PYXAbstractIterator API
			virtual void next();
			virtual bool end() const;

		public: //PYXInnerTileIntersectionIterator API
			virtual const PYXInnerTile & getTile() const{return m_currentTile;};
			virtual const PYXInnerTileIntersection & getIntersection() const{return m_currentIntersection;};

		private:
			void findFirstNode(const PYXInnerTile & tile);
			bool nextUsingGeometryVisitor();
			void findNextTile();
			void pop();

		private: 
			PYXConstWireBuffer m_buffer;
			PYXPointer<IRegion> m_region;

			PYXInnerTile m_currentTile;
			PYXInnerTileIntersection m_currentIntersection;
			boost::ptr_deque <NodeInfo> m_nodesStack;			

			//! GeometryIteraotr been used when iterating over a Terminal Partial Node
			PYXPointer<PYXInnerTileIntersectionIterator> m_geometryVisitor;
			int m_cellResolution;

			//! the given inner tile to iterator over
			PYXInnerTile m_innerTile;

			//! mark when the iterator has finished
			bool m_ended;
		};

	public:
		static PYXPointer<RegionIndexSerialized> create(const PYXPointer<IRegion> & region, PYXWireBuffer & buffer)
		{
			return PYXNEW(RegionIndexSerialized,region,buffer);		
		}
		RegionIndexSerialized(const PYXPointer<IRegion> & region, PYXWireBuffer & buffer);
	
	public:
		PYXInnerTileIntersection intersects(const PYXIcosIndex& index) const;
		PYXPointer<PYXInnerTileIntersectionIterator> getIterator(const PYXInnerTile & innerTile) const{return Iterator::create(*this,innerTile);}

	public:
		mutable PYXConstWireBuffer m_buffer;
		PYXPointer<IRegion> m_region;
	};

	/////////////////////////////////////////
	// PYX Vector Geometry 2
	/////////////////////////////////////////

public:

	static PYXPointer<PYXVectorGeometry2> create(std::basic_istream< char>& in)
	{
		return PYXNEW(PYXVectorGeometry2,in);
	}

	static PYXPointer<PYXVectorGeometry2> create(PYXWireBuffer & buffer)
	{
		return PYXNEW(PYXVectorGeometry2,buffer);
	}

	//! Create a geometry.
	static PYXPointer<PYXVectorGeometry2> create(const PYXPointer<IRegion> & region,int resolution)
	{
		return PYXNEW(PYXVectorGeometry2,region,resolution);
	}

	//! Create a geometry.
	static PYXPointer<PYXVectorGeometry2> createFromPoint(const PYXCoord3DDouble & point,int resolution)
	{
		return PYXNEW(PYXVectorGeometry2,PYXVectorPointRegion::create(point),resolution);
	}

	//! Create a geometry.
	static PYXPointer<PYXVectorGeometry2> createFromLine(const PYXCoord3DDouble & pointA,const PYXCoord3DDouble & pointB,int resolution)
	{
		return PYXNEW(PYXVectorGeometry2,PYXCurveRegion::create(pointA,pointB),resolution);
	}

	//! Create a copy of the geometry.
	static PYXPointer<PYXVectorGeometry2> create(const PYXVectorGeometry2 & rhs)
	{
		return PYXNEW(PYXVectorGeometry2, rhs);
	}

	//! Constructor
	PYXVectorGeometry2(const PYXPointer<IRegion> & region,int resolution) : m_region(region), m_nResolution(resolution)
	{
	}

	PYXVectorGeometry2(const PYXVectorGeometry2 & rhs) : 
		m_region(rhs.m_region), m_nResolution(rhs.m_nResolution), m_serialized(rhs.m_serialized), m_tileCollection(rhs.m_tileCollection)
	{
	}

	//! Constructor to Deserialize
	PYXVectorGeometry2(std::basic_istream< char>& in)
	{
		unsigned int length;
		in.read(( char *)&length,sizeof(length));
		boost::scoped_array< char> str(new char[length]);
		in.read(str.get(),length);
		deserialize(PYXConstWireBuffer(str.get(),length));
	}

	//! Constructor to Deserialize
	PYXVectorGeometry2(PYXWireBuffer & buffer)
	{
		deserialize(buffer);
	}

	//! Destructor
	virtual ~PYXVectorGeometry2() {}

public: // I/O Methods

	//! Read the geometry from the buffer
	void deserialize(PYXWireBuffer & buffer);
	
	//! Serialize
	void serialize( std::basic_ostream< char>& out ) const;
	void serialize(PYXWireBuffer & out) const;
	
public: // PYXGeometry
	const PYXPointer<IRegion> & getRegion() const { return m_region; }

	//! Create a copy of the geometry.
	virtual PYXPointer<PYXGeometry> clone() const;
	
	//! Is the geometry empty?
	virtual bool isEmpty() const { return false; }
	
	//! Get the cell resolution.
	virtual int getCellResolution() const;

	//! Set the PYXIS resolution of cells in the geometry.
	virtual void setCellResolution(int nCellResolution);

	//! Get the intersection of this geometry and the specified geometry.
	virtual PYXPointer<PYXGeometry> intersection(const PYXGeometry& geometry, bool bCommutative = true) const;
	
	//! Get the intersection of this geometry and the specified geometry.
	PYXPointer<PYXGeometry> intersection(const PYXTileCollection& collection) const;

	//! Get the intersection of this geometry and the specified geometry.
	PYXPointer<PYXGeometry> intersection(const PYXTile& tile) const;

	//! Get the intersection of this geometry and the specified geometry.
	PYXPointer<PYXGeometry> intersection(const PYXCell& cell) const;

	//! Is the specified geometry contained by this one?
	virtual bool contains(const PYXGeometry& geometry) const;

	//! Does the specified geometry intersect with this one?
	virtual bool intersects(const PYXGeometry& geometry, bool bCommutative = true) const; 

	//! Get an iterator to the individual cells in the geometry.
	virtual PYXPointer<PYXIterator> getIterator() const;

	virtual PYXPointer<PYXInnerTileIntersectionIterator> getInnerTileIterator(const PYXInnerTile & tile) const;
	
	//! Calculate a series of PYXIS indices around a geometry.
	virtual void calcPerimeter(std::vector<PYXIcosIndex>* pVecIndex) const
	{
		PYXTHROW_NOT_IMPLEMENTED();
	}
	
	//! Calculate Bounding Circle
	virtual PYXBoundingCircle getBoundingCircle() const;

	//! Copies a representation of this geometry into a tile collection.
	virtual void copyTo(PYXTileCollection* pTileCollection) const;

	//! Copies a representation of this geometry into a tile collection at the specified resolution.
	virtual void copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const;

	//! Get the bounding box for this geometry.
	virtual void getBoundingRects(const ICoordConverter* coordConvertor,
		PYXRect2DDouble* pRect1,
		PYXRect2DDouble* pRect2) const;

	static void test();

private:
	void initTileCollection() const;

	void generateDermIndex() const;

protected:

	//Emptry constructor. to allow derived classes to create them self from streams.
	PYXVectorGeometry2() : m_region(),m_nResolution(0)
	{
	}

private:
	//! the cell resolution to use
	int m_nResolution;

	//! The real vector region
	PYXPointer<IRegion> m_region;

	//! lock modification on m_serialized and m_tileCollection
	mutable boost::recursive_mutex m_mutex;

	//! The Serialized Tree
	mutable PYXPointer<RegionIndexSerialized> m_serialized;
	
	//! A tile collection to use for intersection methods
	mutable PYXPointer<PYXTileCollection> m_tileCollection;
};



#endif // guard
