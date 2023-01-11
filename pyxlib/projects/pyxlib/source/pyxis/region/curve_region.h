#ifndef PYXIS__REGION__CURVE_REGION_H
#define PYXIS__REGION__CURVE_REGION_H
/******************************************************************************
curve_region.h

begin		: 2010-07-12
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/region/region.h"

#include "pyxis/derm/index.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/sphere_math.h"
#include "pyxis/utility/range.h"
#include "pyxis/utility/wire_buffer.h"

#include <boost/scoped_ptr.hpp>
#include <vector>

//! TODO: this class has not been tested!
class PYXLIB_DECL PYXCurveRegion : public PYXVectorRegion
{

public: //PYXRegion API
	virtual PYXPointer<IRegion> clone() const;

	virtual int getVerticesCount() const
	{
		return m_nodes.size();
	}

	virtual PYXPointer<IRegionVisitor> getVisitor() const;

	virtual void serializeVisitor(PYXWireBuffer & buffer,const PYXPointer<IRegionVisitor> & visitor) const;

	virtual PYXPointer<IRegionVisitor> deserializeVisitor(PYXWireBuffer & buffer) const;

	//! distance in radians
	virtual double getDistanceToBorder(const PYXCoord3DDouble & location,double errorThreshold = 0) const;

	virtual bool isPointContained(const PYXCoord3DDouble & location,double errorThreshold = 0) const
	{
		return false;
	}
	
	virtual CellIntersectionState intersects(const PYXIcosIndex & index, bool asTile=false) const;

	virtual CellIntersectionState intersects(const PYXBoundingCircle & circle,double errorThreshold = 0) const;

	virtual PYXBoundingCircle getBoundingCircle() const;

private:
	typedef std::vector<PYXCoord3DDouble> NodesVector;
	
	class Visitor : public IRegionVisitor
	{
		friend class PYXCurveRegion;

	private:
		class Iterator : public PYXInnerTileIntersectionIterator
		{
		public:
			static PYXPointer<Iterator> create(const PYXPointer<const PYXCurveRegion::Visitor> & visitor,const PYXInnerTile & tile)
			{
				return PYXNEW(Iterator,visitor,tile);
			}

			Iterator(const PYXPointer<const PYXCurveRegion::Visitor> & visitor,const PYXInnerTile & tile);

			virtual const PYXInnerTile & getTile() const;
			virtual const PYXInnerTileIntersection & getIntersection() const;
			virtual bool end() const;
			virtual void next();

		private:
			PYXPointer<const PYXCurveRegion::Visitor> m_visitor;
			PYXInnerTile m_tile;
			PYXIcosIndex m_index;
			PYXInnerTile m_currentTile;
			PYXInnerTileIntersection m_currentIntersection;
			std::vector<SphereMath::GreatCircleArc> m_arcs;
		private:
			void findIntersection();
			void findNextInnerTile(PYXIcosIndex & index);
		};

	private:
		static const int knOptimalSize = 10;
		PYXPointer<const PYXCurveRegion> m_curve;
		std::vector<Range<int>> m_chunks;

		//help constructor that takes the chunks
		Visitor(const PYXPointer<const PYXCurveRegion> & curve, std::vector<Range<int>> & chunks);
		Visitor(const PYXPointer<const PYXCurveRegion> & curve, PYXWireBuffer & buffer);

		static PYXPointer<Visitor> create(const PYXPointer<const PYXCurveRegion> & curve,PYXWireBuffer & buffer)
		{
			return PYXNEW(Visitor,curve,buffer);
		}

		void serialize(PYXWireBuffer & buffer);

	public:
		static PYXPointer<Visitor> create(const PYXPointer<const PYXCurveRegion> & curve)
		{
			return PYXNEW(Visitor,curve);
		}

		static PYXPointer<Visitor> create(const Visitor & other)
		{
			return PYXNEW(Visitor,other);
		}

		Visitor(const PYXPointer<const PYXCurveRegion> & curve);
		Visitor(const Visitor & other);

	public: //IRegionVisitor
		virtual bool isOptimal() const;

		virtual PYXPointer<IRegionVisitor> trim(const PYXIcosIndex & index) const;

		virtual PYXPointer<PYXInnerTileIntersectionIterator> getInnerTileIterator(const PYXInnerTile & tile) const;

		virtual PYXInnerTileIntersection intersects(const PYXIcosIndex & cell) const;

		virtual PYXInnerTileIntersection intersects(const PYXBoundingCircle & circle,double errorThreshold = 0) const;

	public:
		int getChunksSize() const;
	};

	//members
private:
	NodesVector						m_nodes;
	PYXBoundingCircle				m_boundingCircle;

private:
	void buildChunks();

public:

	static PYXPointer<PYXCurveRegion> create()
	{
		return PYXNEW(PYXCurveRegion);
	}

	static PYXPointer<PYXCurveRegion> create(const PYXCurveRegion & curve)
	{
		return PYXNEW(PYXCurveRegion,curve);
	}

	static PYXPointer<PYXCurveRegion> create(PYXCoord3DDouble const & pointA,PYXCoord3DDouble const & pointB)
	{
		return PYXNEW(PYXCurveRegion,pointA,pointB);
	}

	static PYXPointer<PYXCurveRegion> create(std::vector<PYXCoord3DDouble> const & points,bool closeCurve = false)
	{
		return PYXNEW(PYXCurveRegion,points,closeCurve);
	}

	//! Constructs a empty curve region 
	explicit PYXCurveRegion();

	//! Copy a curve region
	explicit PYXCurveRegion(const PYXCurveRegion & curve);

	//! Convenience constructor; constructs a curve region for two 3D coordinate.
	explicit PYXCurveRegion(PYXCoord3DDouble const & pointA,PYXCoord3DDouble const & pointB);

	//! Convenience constructor; constructs a curve region for list of points
	explicit PYXCurveRegion(std::vector<PYXCoord3DDouble> const & points,bool closeCurve = false);

//curve operations:
public:
	void closeCurve();

	bool isClosed() const;

	const PYXCoord3DDouble & getVertex(unsigned int index) const { return m_nodes[index]; }

public:
	friend PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXCurveRegion & region);
	friend PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXCurveRegion> & region);
public:
	static void test();

};

PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXCurveRegion & region);
PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXCurveRegion> & region);

#endif // guard
