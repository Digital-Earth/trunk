#ifndef PYXIS__REGION__MULTI_CURVE_REGION_H
#define PYXIS__REGION__MULTI_CURVE_REGION_H
/******************************************************************************
multi_curve_region.h

begin		: 2012-06-12
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/region/region.h"
#include "pyxis/region/curve_region.h"
#include "pyxis/region/arc_buffer.h"

#include "pyxis/utility/bounding_circle_spatial_set.h"
#include "pyxis/utility/wire_buffer.h"
#include "region_serializer.h"

#include <vector>

/*
This class implements a set of spherical curves on the surface of the reference
sphere.

The curves are build from collection arcs.

*/
//! TODO: this is class is not been tested!
class PYXLIB_DECL PYXMultiCurveRegion : public PYXVectorRegion
{
	friend class Visitor;

	//PYXRegion API
public:
	virtual PYXPointer<IRegion> clone() const;

	virtual int getVerticesCount() const
	{
		return m_arcs.size();
	}

	virtual PYXPointer<IRegionVisitor> getVisitor() const;

	//! distance in radians
	virtual double getDistanceToBorder(const PYXCoord3DDouble & location,double errorThreshold = 0) const;

	virtual bool isPointContained( const PYXCoord3DDouble & location,double errorThreshold /*= 0*/ ) const;

	virtual CellIntersectionState intersects(const PYXIcosIndex & index, bool asTile = false) const;

	virtual CellIntersectionState intersects(const PYXBoundingCircle & circle,double errorThreshold = 0) const;

	virtual PYXBoundingCircle getBoundingCircle() const;

	virtual void serializeVisitor(PYXWireBuffer & buffer,const PYXPointer<IRegionVisitor> & visitor) const;

	virtual PYXPointer<IRegionVisitor> deserializeVisitor(PYXWireBuffer & buffer) const;

private:
	class Visitor : public IRegionVisitor
	{
		friend class PYXMultiCurveRegion;
	private:
		class Iterator : public PYXInnerTileIntersectionIterator
		{
		public:
			static PYXPointer<Iterator> create(const PYXPointer<const PYXMultiCurveRegion::Visitor> & visitor,const PYXInnerTile & tile)
			{
				return PYXNEW(Iterator,visitor,tile);
			}

			Iterator(const PYXPointer<const PYXMultiCurveRegion::Visitor> & visitor,const PYXInnerTile & tile);

			virtual const PYXInnerTile & getTile() const;
			virtual const PYXInnerTileIntersection & getIntersection() const;
			virtual bool end() const;
			virtual void next();

		private:
			PYXPointer<const PYXMultiCurveRegion::Visitor> m_visitor;
			PYXInnerTile m_tile;
			PYXIcosIndex m_index;

			PYXInnerTile m_currentTile;
			PYXInnerTileIntersection m_currentIntersection;

			std::vector<SphereMath::GreatCircleArc> m_arcs;

			void findIntersection();
			void findNextInnerTile(PYXIcosIndex & index);
		};
	private:
		static const int knOptimalSize = 10;

		//help constructor that takes the chunks
		Visitor(const PYXPointer<const PYXMultiCurveRegion> & curve, std::vector<Range<int>> & chunks);
		Visitor(const PYXPointer<const PYXMultiCurveRegion> & curve, PYXWireBuffer & buffer);

		void serialize(PYXWireBuffer & buffer);

	public:
		virtual bool isPointContained(const PYXCoord3DDouble & location,double errorThreshold = 0) const;

		static PYXPointer<Visitor> create(const PYXPointer<const PYXMultiCurveRegion> & curve,PYXWireBuffer & buffer)
		{
			return PYXNEW(Visitor,curve,buffer);
		}

		static PYXPointer<Visitor> create(const PYXPointer<const PYXMultiCurveRegion> & curve)
		{
			return PYXNEW(Visitor,curve);
		}

		static PYXPointer<Visitor> create(const Visitor & other)
		{
			return PYXNEW(Visitor,other);
		}

		Visitor(const PYXPointer<const PYXMultiCurveRegion> & curve);
		Visitor(const Visitor & other);

	public: //IRegionVisitor
		virtual bool isOptimal() const;

		virtual PYXPointer<IRegionVisitor> trim(const PYXIcosIndex & index) const;

		virtual PYXPointer<PYXInnerTileIntersectionIterator> getInnerTileIterator(const PYXInnerTile & tile) const;

		virtual PYXInnerTileIntersection intersects(const PYXIcosIndex & cell) const;

		virtual PYXInnerTileIntersection intersects(const PYXBoundingCircle & circle,double errorThreshold = 0) const;

	public:
		int getChunksSize() const;

	private:
		PYXPointer<const PYXMultiCurveRegion> m_curve;
		std::vector<Range<int>> m_chunks;
	};

	//consturtors 
public:

	//static consturtor
	static PYXPointer<PYXMultiCurveRegion> create()
	{
		return PYXNEW(PYXMultiCurveRegion);
	}

	//static copy consturtor
	static PYXPointer<PYXMultiCurveRegion> create(PYXMultiCurveRegion multicurve )
	{
		return PYXNEW(PYXMultiCurveRegion,multicurve);
	}

	static PYXPointer<PYXMultiCurveRegion> create(const std::vector< PYXPointer<PYXCurveRegion> > & regions )
	{
		return PYXNEW(PYXMultiCurveRegion,regions);
	}


	//! Constructs an empty curve region
	explicit PYXMultiCurveRegion();

	//! Copy constuctor
	PYXMultiCurveRegion::PYXMultiCurveRegion(const PYXMultiCurveRegion & multicurve);

	explicit PYXMultiCurveRegion(const std::vector< PYXPointer<PYXCurveRegion> > & regions);

	//! Convenience constructor; constructs a polygon region with external ring
	explicit PYXMultiCurveRegion(const std::vector<PYXCoord3DDouble> vertices);

public:
	int getCurveCount() const;
	
	int getCurveVerticesCount(unsigned int index) const;

	PYXCoord3DDouble getCurveVertex(unsigned int curveIndex,unsigned int vertexIndex) const;

private:

	//Memberes
private:

	ArcBuffer m_arcs;
	PYXBoundingCircle m_boundingCircle;

public:
	friend PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXMultiCurveRegion & region);
	friend PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXMultiCurveRegion> & region);
};

PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXMultiCurveRegion & region);
PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXMultiCurveRegion> & region);

#endif // guard
