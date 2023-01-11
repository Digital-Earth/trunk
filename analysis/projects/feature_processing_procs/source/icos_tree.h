#ifndef ICOS_TREE_H
#define ICOS_TREE_H
/******************************************************************************
icos_tree.h

begin		: Dec 15, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_feature_processing_procs.h"
#include "pyxis/utility/numeric_histogram.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/utility/wire_buffer.h"

#include "boost/scoped_ptr.hpp"
#include "boost/bind.hpp"

template<typename T>
/*! 
Represents a sparse collection of PYXIS cells with associated data T, aggregated to lower resolutions.
*/
class IcosTree
{
public:
	/*! 
	A tree node containing a T and up to 7 child nodes corresponding to the children of a PYXIS cell.
	*/
	class Bin
	{
	public:
		Bin* 	m_childBins[7];
		T		m_value;

		Bin() : m_value()
		{
			for(int i=0;i<7;i++) 
			{
				m_childBins[i] = 0;
			}
		}

		~Bin()
		{
			for(int i=0;i<7;i++)
			{
				delete m_childBins[i];
			}
		}

		int getChildCount() const 
		{
			int result = 0;
			for(int i=0;i<7;i++) 
			{
				if (m_childBins[i] != NULL)
				{
					result++;
				}
			}
			return result;
		}

		unsigned char getChildrenBitmap() const 
		{
			unsigned char  result = 0;
			for(int i=0;i<7;i++) 
			{
				if (m_childBins[i] != NULL)
				{
					result+=1<<i;
				}
			}
			return result;
		}

		Bin * getSingleChildOrNull()
		{
			Bin * result = 0;

			for(int i=0;i<7;i++) 
			{
				if (m_childBins[i] != NULL)
				{
					if (result == NULL)
						result = m_childBins[i];
					else
						return NULL;
				}
			}
			return result;
		}

		int removeChildBins()
		{
			int result = 0;

			for(int i=0;i<7;i++) 
			{
				if (m_childBins[i] != NULL)
				{
					delete m_childBins[i];
					m_childBins[i] = NULL;
					result++;
				}
			}

			return result;
		}

		int childrenCount()
		{
			int result = 0;

			for(int i=0;i<7;i++) 
			{
				if (m_childBins[i] != NULL)
				{
					result++;
				}
			}

			return result;
		}
	};

private:
	static const int MAXROOTS = 32;
	std::vector<Bin*> m_rootBins;

public:
	IcosTree() : m_rootBins(MAXROOTS,NULL)
	{
	}

	void clear()
	{
		for(int i=0;i<MAXROOTS;++i)
		{
			delete m_rootBins[i];
			m_rootBins[i] = 0;
		}
	}

	~IcosTree()
	{
		clear();
	}

	int getRootIndex(const PYXIcosIndex & index) const
	{
		if (index.isVertex())
		{
			return index.getPrimaryResolution() - index.knFirstVertex;
		}
		else
		{
			return index.getPrimaryResolution() - index.kcFaceFirstChar + index.knLastVertex - index.knFirstVertex + 1;
		}
	}

	PYXIcosIndex getRootIcosIndex(int index) const
	{
		PYXIcosIndex result;

		if (index < PYXIcosIndex::knLastVertex)
		{
			result.setPrimaryResolution(index + PYXIcosIndex::knFirstVertex);
		}
		else 
		{
			result.setPrimaryResolution(index - PYXIcosIndex::knLastVertex + PYXIcosIndex::kcFaceFirstChar);
		}
		return result;
	}

	const Bin * getNode(const PYXIcosIndex & index) const
	{
		int rootIndex = getRootIndex(index);
		if (m_rootBins[rootIndex] == NULL)
		{
			return NULL;
		}
		Bin * bin = m_rootBins[rootIndex];
		const PYXIndex & subIndex = index.getSubIndex();
		int digit = 0;

		while(digit < subIndex.getDigitCount() && bin != NULL)
		{
			int childIndex = subIndex.getDigit(digit);
			bin = bin->m_childBins[childIndex];
			++digit;
		}

		return bin;
	}

	Bin * getNodeOrCreate(const PYXIcosIndex & index)
	{
		int rootIndex = getRootIndex(index);
		if (m_rootBins[rootIndex] == NULL)
		{
			m_rootBins[rootIndex] = new Bin();
		}
		Bin * bin = m_rootBins[rootIndex];
		const PYXIndex & subIndex = index.getSubIndex();
		int digit = 0;

		while(digit < subIndex.getDigitCount() && bin != NULL)
		{
			int childIndex = subIndex.getDigit(digit);
			if (bin->m_childBins[childIndex] == NULL)
			{
				bin->m_childBins[childIndex] = new Bin();
			}
			bin = bin->m_childBins[childIndex];
			++digit;
		}

		return bin;
	}

	T & operator[](const PYXIcosIndex & index)
	{
		return getNodeOrCreate(index)->m_value;
	}

	const T & operator[](const PYXIcosIndex & index) const
	{
		const Bin * bin = getNode(index);
		if (bin == NULL)
		{
			PYXTHROW(PYXException,"node wasn't found");
		}

		return bin->m_value;
	}

public:
	/*!
	Call the supplied function for all nodes in the tree in a depth first manner.
	*/
	void visitAll(const boost::function<void(const PYXIcosIndex & index,T & value)> & function)
	{
		PYXIcosIndex index;
		for(int i=0;i<MAXROOTS;++i)
		{
			index = getRootIcosIndex(i);

			if (m_rootBins[i] != NULL)
			{
				visitAll(function,index,m_rootBins[i]);
			}
		}
	}

private:
	void visitAll(const boost::function<void(const PYXIcosIndex & index,T & value)> & function,PYXIcosIndex & index,Bin * bin)
	{
		function(index,bin->m_value);

		for(int i=0;i<7;++i)
		{
			Bin * child = bin->m_childBins[i];

			if (child != NULL)
			{
				index.getSubIndex().appendDigit(i);
				visitAll(function,index,child);
				index.getSubIndex().stripRight();;
			}
		}
	}

public:
	/*!
	Call the supplied visit function for all nodes in the tree that satisfy the supplied where function
	in a depth first manner.
	*/
	void visitWhere(	const boost::function<void(const PYXIcosIndex & index,T & value)> & visit_function,
						const boost::function<bool(const PYXIcosIndex & index,T & value)> & where_function	)
	{
		PYXIcosIndex index;
		for(int i=0;i<MAXROOTS;++i)
		{
			index = getRootIcosIndex(i);

			if (m_rootBins[i] != NULL)
			{
				visitWhere(visit_function,where_function,index,m_rootBins[i]);
			}
		}
	}

private:

	void visitWhere(const boost::function<void(const PYXIcosIndex & index,T & value)> & visit_function,const boost::function<bool(const PYXIcosIndex & index,T & value)> & where_function,PYXIcosIndex & index,Bin * bin)
	{
		if (where_function(index,bin->m_value))
		{
			visit_function(index,bin->m_value);
		}

		for(int i=0;i<7;++i)
		{
			Bin * child = bin->m_childBins[i];

			if (child != NULL)
			{
				index.getSubIndex().appendDigit(i);
				visitWhere(visit_function,where_function,index,child);
				index.getSubIndex().stripRight();;
			}
		}
	}

public:
	/*!
	Call the supplied function for all nodes in the tree with a given resolution.
	*/
	void visitResolution(const boost::function<void(const PYXIcosIndex & index,T & value)> & function,int resolution)
	{
		visitWhere(function,boost::bind(&IcosTree::isResolution,_1,_2,resolution));
	}

private:
	static bool isResolution(const PYXIcosIndex & index,T & value,int resolution)
	{
		return index.getResolution() == resolution;
	}

public:
	/*!
	While the visitor's visit method returns true, visit all nodes in the tree in a depth first manner.
	Call the visitor's generate method when a node does not exist and call the visitor's postVisit method
	after a node's child nodes have been visited. Nodes created by the visitor's generate method are visited.
	*/
	template<typename Visitor>
	void visit(Visitor & visitor)
	{
		PYXIcosIndex index;
		for(int i=0;i<MAXROOTS;++i)
		{
			index = getRootIcosIndex(i);

			if (m_rootBins[i] == NULL)
			{
				m_rootBins[i] = visitor.generate(index);
			}

			if (m_rootBins[i] != NULL)
			{
				if (visitor.visit(index,m_rootBins[i]))
				{
					visit(visitor,index,m_rootBins[i]);
					visitor.postVisit(index,m_rootBins[i]);
				}
			}
		}
	}

private:
	template<typename Visitor>
	void visit(Visitor & visitor,PYXIcosIndex & index,Bin* bin)
	{
		if (index.hasVertexChildren())
		{
			PYXMath::eHexDirection direction;
			if (PYXIcosMath::getCellGap(index,&direction))
			{
				for(int i=0;i<7;++i)
				{
					if (i == (int)direction)
						continue;

					index.getSubIndex().appendDigit(i);

					Bin * child = bin->m_childBins[i];

					if (child == NULL)
					{
						child = bin->m_childBins[i] = visitor.generate(index);
					}

					if (child != NULL)
					{
						if (visitor.visit(index,child ))
						{
							visit(visitor,index,child );
							visitor.postVisit(index,child );
						}
					}

					index.getSubIndex().stripRight();
				}
			}
			else 
			{
				for(int i=0;i<7;++i)
				{
					index.getSubIndex().appendDigit(i);

					Bin * child = bin->m_childBins[i];

					if (child == NULL)
					{
						child = bin->m_childBins[i] = visitor.generate(index);
					}

					if (child != NULL)
					{
						if (visitor.visit(index,child ))
						{
							visit(visitor,index,child );
							visitor.postVisit(index,child );
						}
					}

					index.getSubIndex().stripRight();
				}
			}
		}
		else
		{
			index.getSubIndex().appendDigit(0);

			Bin * child = bin->m_childBins[0];

			if (child == NULL)
			{
				child = bin->m_childBins[0] = visitor.generate(index);
			}

			if (child != NULL)
			{
				if (visitor.visit(index,child))
				{
					visit(visitor,index,child);
					visitor.postVisit(index,child);
				}
			}

			index.getSubIndex().stripRight();
		}
	}

public:
	/*!
	While the visitor's visit method returns true, visit all nodes in the tree, and in a supplied tree
	in a depth first manner. Call the visitor's generate method when a node does not exist and call the
	visitor's postVisit method after a node's child nodes have been visited. Nodes created by the visitor's
	generate method are visited.
	*/
	template<typename Visitor>
	void zipVisit(Visitor & visitor,const IcosTree<T> & other)
	{
		PYXIcosIndex index;
		for(int i=0;i<MAXROOTS;++i)
		{
			index = getRootIcosIndex(i);

			if (m_rootBins[i] == NULL)
			{
				m_rootBins[i] = visitor.generate(index,other.m_rootBins[i]);
			}

			if (m_rootBins[i] != NULL)
			{
				if (visitor.visit(index,m_rootBins[i],other.m_rootBins[i]))
				{
					zipVisit(visitor,index,m_rootBins[i],other.m_rootBins[i]);
					visitor.postVisit(index,m_rootBins[i],other.m_rootBins[i]);
				}
			}
		}
	}

private:
	template<typename Visitor>
	void zipVisit(Visitor & visitor,PYXIcosIndex & index,Bin* bin,Bin * other)
	{
		if (index.hasVertexChildren())
		{
			PYXMath::eHexDirection direction;
			if (PYXIcosMath::getCellGap(index,&direction))
			{
				for(int i=0;i<7;++i)
				{
					if (i == (int)direction)
						continue;

					index.getSubIndex().appendDigit(i);

					Bin * child = bin->m_childBins[i];
					Bin * otherChild = other != NULL ? other->m_childBins[i] : NULL;

					if (child == NULL)
					{
						child = bin->m_childBins[i] = visitor.generate(index, otherChild );
					}

					if (child != NULL)
					{
						if (visitor.visit(index,child,otherChild ))
						{
							zipVisit(visitor,index,child,otherChild);
							visitor.postVisit(index,child,otherChild);
						}
					}

					index.getSubIndex().stripRight();
				}
			}
			else 
			{
				for(int i=0;i<7;++i)
				{
					index.getSubIndex().appendDigit(i);

					Bin * child = bin->m_childBins[i];
					Bin * otherChild = other != NULL ? other->m_childBins[i] : NULL;

					if (child == NULL)
					{
						child = bin->m_childBins[i] = visitor.generate(index, otherChild );
					}

					if (child != NULL)
					{
						if (visitor.visit(index,child,otherChild ))
						{
							zipVisit(visitor,index,child,otherChild);
							visitor.postVisit(index,child,otherChild);
						}
					}

					index.getSubIndex().stripRight();
				}
			}
		}
		else {
			index.getSubIndex().appendDigit(0);

			Bin * child = bin->m_childBins[0];
			Bin * otherChild = other != NULL ? other->m_childBins[0] : NULL;
			if (child == NULL)
			{
				child = bin->m_childBins[0] = visitor.generate(index, otherChild );
			}

			if (child != NULL)
			{
				if (visitor.visit(index,child,otherChild ))
				{
					zipVisit(visitor,index,child,otherChild);
					visitor.postVisit(index,child,otherChild);
				}
			}

			index.getSubIndex().stripRight();
		}
	}

private:	
	struct LimitResolutionVisitor
	{
		int m_resolution;

		LimitResolutionVisitor(int resolution) : m_resolution(resolution) 
		{}

		Bin * generate(const PYXIcosIndex & index) { return NULL; }
		bool visit(const PYXIcosIndex & index,Bin * bin)
		{ 
			if (index.getResolution() == m_resolution)
			{
				bin->removeChildBins();
				return false;
			}
			return true;
		}
		void postVisit(const PYXIcosIndex & index,Bin * bin) {};
	};

public:
	/*!
	Prune the tree to nodes of a given resolution.
	*/
	void limitTreeResolution(int resolution)
	{
		visit(LimitResolutionVisitor(resolution));
	}

private:
	struct CountVisitor
	{
		int count;

		CountVisitor() : count(0) 
		{}

		Bin * generate(const PYXIcosIndex & index) { return NULL; }
		bool visit(const PYXIcosIndex & index,Bin * bin) { count++; return true; }
		void postVisit(const PYXIcosIndex & index,Bin * bin) {};
	};

public:
	int size()
	{
		CountVisitor result;

		visit(result);

		return result.count;
	}

private:
	struct CountResolutionVisitor
	{
		int count;
		int m_resolution;

		CountResolutionVisitor(int resolution) : count(0), m_resolution(resolution)
		{}

		Bin * generate(const PYXIcosIndex & index) { return NULL; }
		bool visit(const PYXIcosIndex & index,Bin * bin) 
		{
			if (index.getResolution() == m_resolution)
			{
				count++;
			}
			return index.getResolution() < m_resolution;
		}
		void postVisit(const PYXIcosIndex & index,Bin * bin) {};
	};

public:
	/*!
	Get the number of tree nodes at a given resolution.
	*/
	int sizeOfResolution(int resolution)
	{
		CountResolutionVisitor result(resolution);

		visit(result);

		return result.count;
	}
};


template<typename T>
class PYXSeralizedIcosTree
{
private:
	struct CellInfo
	{
		unsigned char childrenBitmap; 
		T value;

		CellInfo() : childrenBitmap(0)
		{
		}
	};

	std::vector<CellInfo> m_data;
	static const int MAXROOTS = 32;

public:
	PYXSeralizedIcosTree()
	{
	}


private:
	template<typename TSource>
	struct CopyTreeVisitor
	{
		std::vector<CellInfo> & m_data;
		void (*m_convert)(const TSource & source,T & dest);

		CopyTreeVisitor(std::vector<CellInfo> & data,void (*covert_function)(const TSource & source,T & dest)) : m_data(data), m_convert(covert_function)
		{
		}

		typename IcosTree<TSource>::Bin * generate(const PYXIcosIndex & index) {
			if (index.getResolution() == 1)
			{
				m_data.push_back(CellInfo());
				m_data.back().childrenBitmap = 0;
			}
			return NULL; 
		}

		bool visit(const PYXIcosIndex & index,typename IcosTree<TSource>::Bin * bin) {
			m_data.push_back(CellInfo());
			m_data.back().childrenBitmap = bin->getChildrenBitmap();
			(*m_convert)(bin->m_value,m_data.back().value);
			return true;
		}
		void postVisit(const PYXIcosIndex & index,typename IcosTree<TSource>::Bin * bin) {};
	};
public:

	template<typename TSource>
	PYXSeralizedIcosTree(IcosTree<TSource> & tree,void (*covert_function)(const TSource & source,T & dest))
	{
		tree.visit(CopyTreeVisitor<TSource>(m_data,covert_function));
	}

private:
	static void justCopy(const T & source,T & dest)
	{
		dest = source;
	}

public:
	PYXSeralizedIcosTree(IcosTree<T> & tree)
	{
		tree.visit(CopyTreeVisitor<TSource>(m_data,justCopy));
	}

public:
	template<typename TSource>
	void generateFrom(IcosTree<TSource> & tree,void (*covert_function)(const TSource & source,T & dest))
	{
		m_data.clear();
		tree.visit(CopyTreeVisitor<TSource>(m_data,covert_function));
	}

	void generateFrom(IcosTree<T> & tree)
	{
		m_data.clear();
		tree.visit(CopyTreeVisitor<TSource>(m_data,justCopy));
	}

public:
	friend PYXWireBuffer & operator << (PYXWireBuffer & buffer,const PYXSeralizedIcosTree & tree);
	friend PYXWireBuffer & operator >> (PYXWireBuffer & buffer,PYXSeralizedIcosTree & tree);

private:
	int getRootIndex(const PYXIcosIndex & index) const
	{
		if (index.isVertex())
		{
			return index.getPrimaryResolution() - index.knFirstVertex;
		}
		else
		{
			return index.getPrimaryResolution() - index.kcFaceFirstChar + index.knLastVertex - index.knFirstVertex + 1;
		}
	}

	PYXIcosIndex getRootIcosIndex(int index) const
	{
		PYXIcosIndex result;

		if (index < PYXIcosIndex::knLastVertex)
		{
			result.setPrimaryResolution(index + PYXIcosIndex::knFirstVertex);
		}
		else 
		{
			result.setPrimaryResolution(index - PYXIcosIndex::knLastVertex + PYXIcosIndex::kcFaceFirstChar);
		}
		return result;
	}

public:
	void visitAll(const boost::function<void(const PYXIcosIndex & index,T & value)> & function)
	{
		std::vector<CellInfo>::iterator it = m_data.begin();
		std::vector<unsigned char> childPath;
		PYXIcosIndex index;

		for(int i=0;i<MAXROOTS && it != m_data.end();++i)
		{
			index = getRootIcosIndex(i);

			visitAll(function,index,it);
		}
	}

	void visitWhere(const boost::function<void(const PYXIcosIndex & index,T & value)> & function,
					const boost::function<bool(const PYXIcosIndex & index,T & value)> & where_function)
	{
		std::vector<CellInfo>::iterator it = m_data.begin();
		std::vector<unsigned char> childPath;
		PYXIcosIndex index;

		for(int i=0;i<MAXROOTS && it != m_data.end();++i)
		{
			index = getRootIcosIndex(i);

			visitWhere(funciton,where_function,index,it);
		}
	}

private:
	void skip(typename std::vector<CellInfo>::iterator & it)
	{
		unsigned char bitamp = it->childrenBitmap;
		++it;

		if (bitamp != 0)
		{
			for(int c =0;c<7 && it != m_data.end();++c)
			{
				if (bitmap & (1 << c) != 0)
				{
					skip(it);
				}
			}
		}
	}

	void visitAll(const boost::function<void(const PYXIcosIndex & index,T & value)> & function, PYXIcosIndex & index,typename std::vector<CellInfo>::iterator & it)
	{
		function(index,it->state);
		unsigned char bitamp = it->childrenBitmap;
		++it;

		if (bitamp != 0)
		{
			for(int c =0;c<7 && it != m_data.end();++c)
			{
				if (bitmap & (1 << c) != 0)
				{
					index.getSubIndex().appendDigit(c);
					visitAll(function,index,it->value);
					index.getSubIndex().stripRight();
				}
			}
		}
	}

	void visitWhere(const boost::function<void(const PYXIcosIndex & index,T & value)> & function, PYXIcosIndex & index,
				  							   const boost::function<bool(const PYXIcosIndex & index,T & value)> & where_function,
											   typename std::vector<CellInfo>::iterator & it)
	{
		if (where_function(index,it->state))
		{
			function(index,it->state);
			unsigned char bitamp = it->childrenBitmap;
			++it;

			if (bitamp != 0)
			{
				for(int c =0;c<7 && it != m_data.end();++c)
				{
					if (bitmap & (1 << c) != 0)
					{
						index.getSubIndex().appendDigit(c);
						visitWhere(function,where_function,index,it->value);
						index.getSubIndex().stripRight();
					}
				}
			}
		}
		else 
		{
			skip(it);
		}

	}
};

template<typename T>
PYXWireBuffer & operator << (PYXWireBuffer & buffer,const PYXSeralizedIcosTree<T> & tree)
{
	buffer << (int)tree.size();
	for(typename std::vector<PYXSeralizedIcosTree<T>::CellInfo>::iterator it = tree.m_data.begin();it != tree.m_data.end();++it)
	{
		buffer << it->childrenBitmap << it->value;
	}
}

template<typename T>
PYXWireBuffer & operator >> (PYXWireBuffer & buffer,PYXSeralizedIcosTree<T> & tree)
{
	int size;
	buffer >> size;
	tree.m_data.resize(size);
	for(int i=0;i<size;i++)
	{
		buffer >> tree[i]->childrenBitmap >> it->value;
	}
}

typedef PYXSeralizedIcosTree<PYXRegion::CellIntersectionState> PYXRasterdRegion;


struct CellIntersectionInfo
{
	PYXRegion::CellIntersectionState state;
	PYXCoord3DDouble coord;

	CellIntersectionInfo() : state(PYXRegion::knNone), coord()
	{
	}
};

class CellIntersectionIcosTree : public IcosTree<CellIntersectionInfo> 
{
private:
	struct AddRegionVisitor
	{
		const PYXVectorRegion & m_region;
		int m_resolution;

		AddRegionVisitor(const PYXVectorRegion & region, int resolution) : m_region(region), m_resolution(resolution)
		{

		}

		Bin * generate(const PYXIcosIndex & index)
		{
			Bin * result = new Bin();

			CoordLatLon ll;

			SnyderProjection::getInstance()->pyxisToNative(index,&ll);
			SphereMath::llxyz(ll,&result->m_value.coord );

			return result;
		}

		bool visit(const PYXIcosIndex & index,Bin * bin)
		{
			PYXRegion::CellIntersectionState result = m_region.intersects(PYXBoundingCircle(bin->m_value.coord,
														index.getResolution() < m_resolution?
															PYXIcosMath::UnitSphere::calcTileCircumRadius(index) :
															PYXIcosMath::UnitSphere::calcCellCircumRadius(index)));

			bin->m_value.state = merge(bin->m_value.state,result);

			if (bin->m_value.state == PYXRegion::knComplete)
			{
				bin->removeChildBins();
			}

			return result == PYXRegion::knPartial && index.getResolution() < m_resolution;
		}

		void postVisit(const PYXIcosIndex & index,Bin * bin) const 
		{
		}

		PYXRegion::CellIntersectionState merge(PYXRegion::CellIntersectionState a, PYXRegion::CellIntersectionState b) const
		{
			if (a == PYXRegion::knComplete || b == PYXRegion::knComplete)
				return PYXRegion::knComplete;

			if (a == PYXRegion::knPartial || b == PYXRegion::knPartial)
				return PYXRegion::knPartial ;

			return PYXRegion::knNone;
		}
	};

	struct MergeVisitor
	{
		Bin * generate(const PYXIcosIndex & index,Bin * otherBin) const
		{
			if (otherBin != NULL && otherBin->m_value.state != PYXRegion::knNone)
			{
				Bin * result = new Bin();
				result->m_value.coord = otherBin->m_value.coord;

				return result;
			}
			return NULL;
		}

		bool visit(const PYXIcosIndex & index,Bin * bin, Bin * otherBin) const
		{
			if (otherBin == NULL)
				return false;

			if (bin->m_value.state == PYXRegion::knComplete || otherBin->m_value.state == PYXRegion::knComplete)
			{
				bin->m_value.state = PYXRegion::knComplete;
				bin->removeChildBins();
				return false;
			}

			if (bin->m_value.state == PYXRegion::knPartial || otherBin->m_value.state == PYXRegion::knPartial )
			{
				bin->m_value.state = PYXRegion::knPartial;
				return true;
			}

			bin->m_value.state = PYXRegion::knNone;

			return false;
		}

		void postVisit(const PYXIcosIndex & index,Bin * bin, Bin * otherBin) const
		{
		}
	};

	struct CopyToTileCollectionVisitor
	{
		PYXTileCollection & m_geom;

		CopyToTileCollectionVisitor(PYXTileCollection & geom) : m_geom(geom)
		{
		}

		Bin * generate(const PYXIcosIndex & index) const
		{
			return NULL;
		}

		bool visit(const PYXIcosIndex & index,Bin * bin) const
		{
			if (bin->m_value.state == PYXRegion::knNone)
				return false;

			if (index.getResolution() == m_geom.getCellResolution())
			{
				PYXIcosIndex safe = index;

				//if the index ends with ?-?????????X0 - then we need to fix it to ?-????????????X for some reason...
				if (index.hasVertexChildren())
				{
					safe.decrementResolution();

					//it was ?-??????00
					if (safe.hasVertexChildren())
					{
						safe = index;
					}
				}
				
				m_geom.addTile(safe,m_geom.getCellResolution());
				return false;
			}
			if (bin->m_value.state == PYXRegion::knComplete)
			{
				m_geom.addTile(index,m_geom.getCellResolution());
				return false;
			}

			return true;
		}

		void postVisit(const PYXIcosIndex & index,Bin * bin) const
		{
		}
	};

	struct RemoveEmptyCellsVisitor
	{
		RemoveEmptyCellsVisitor() 
		{

		}

		Bin * generate(const PYXIcosIndex & index)
		{
			return NULL;
		}

		bool visit(const PYXIcosIndex & index,Bin * bin)
		{
			for(int i=0;i<7;i++) 
			{
				if (bin->m_childBins[i] != NULL && bin->m_childBins[i]->m_value.state == PYXRegion::knNone)
				{
					delete bin->m_childBins[i];
					bin->m_childBins[i] = NULL;
				}
			}

			return true;
		}

		void postVisit(const PYXIcosIndex & index,Bin * bin) const 
		{
		}
	};

public:
	void add(const PYXVectorRegion & region,int resolution)
	{
		visit(AddRegionVisitor(region,resolution));
	}

	void add(const CellIntersectionIcosTree & other)
	{
		zipVisit(MergeVisitor(),other);
	}

	void copyTo(PYXTileCollection & geom)
	{
		visit(CopyToTileCollectionVisitor(geom));
	}

	void copyTo(PYXSeralizedIcosTree<PYXRegion::CellIntersectionState> & region)
	{
		region.generateFrom(*this,&CellIntersectionIcosTree::covert);
	}

	void removeEmptyCells()
	{
		visit(RemoveEmptyCellsVisitor());
	}

private:
	static void covert(const CellIntersectionInfo & info, PYXRegion::CellIntersectionState & state)
	{
		state = info.state;
	}
};




class SpatialHistogram
{
private:
	class Bin
	{
	public:
		PYXBoundingCircle m_sphere;
		Bin* 			  m_childBins[7];
		float 	          m_binCount;
		float	          m_binTotalCount;

		Bin();
		~Bin();

		Bin * getSingleChildOrNull();

		int removeChildBins();

		static bool SpatialHistogram::Bin::sizeOfSphere(Bin * a,Bin * b) { return a->m_sphere.getRadius() < b->m_sphere.getRadius(); }
	};

private:
	static const int MAXROOTS = 32;
	std::vector<Bin*> m_rootBins;
	int m_binCount;

public:
	SpatialHistogram();
	~SpatialHistogram();

public:
	template<typename Visitor>
	Range<float> getFeaturesCount(Visitor & v)
	{
		Range<float> result(0);

		for(std::vector<Bin*>::iterator it = m_rootBins.begin(); it != m_rootBins.end();++it)
		{
			getFeaturesCount(*it,result,v);
		}

		return result;
	}

private:
	template<typename Visitor>
	static void getFeaturesCount(Bin * bin,Range<float> & result,Visitor & v)
	{
		if (bin == NULL)
			return;

		Bin * singleChild = bin->getSingleChildOrNull();

		if (singleChild != NULL)
		{
			getFeaturesCount(singleChild,result,v);
			return;
		}

		switch(v(bin->m_sphere))
		{
			case PYXRegion::knNone:
				return;

			case PYXRegion::knComplete:
				result.min += bin->m_binTotalCount;
				result.max += bin->m_binTotalCount;
				return;

			case PYXRegion::knPartial:
				result.max += bin->m_binCount;
				for(int i=0;i<7;i++) 
				{
					getFeaturesCount(bin->m_childBins[i],result,v);
				}
		}
	}

public:
	Range<float> getFeaturesCount(const PYXBoundingCircle & circle);
	Range<float> getFeaturesCount(const PYXVectorRegion & region);

public:
	void addFeature(const PYXIcosIndex & index,const PYXBoundingCircle & sphere);
	void limitBins(int limit);

private:
	int getRootIndex(const PYXIcosIndex & index) const;
	void addFeature(Bin * bin,const PYXIndex & index,int digit,const PYXBoundingCircle & sphere);
	void collectNoneSingleNodes(std::vector<Bin*> & noneSingleNodes,Bin * bin);	
};



#endif // guard