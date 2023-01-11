#ifndef PYXIS__REGION__MULTI_POLYGON_REGION_H
#define PYXIS__REGION__MULTI_POLYGON_REGION_H
/******************************************************************************
multi_polygon_region.h

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
This class implements a set of spherical polygons on the surface of the reference
sphere.

The polygons are build from collection arcs.

*/
//! TODO: this is class is not been tested!
class PYXLIB_DECL PYXMultiPolygonRegion : public PYXVectorRegion
{
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
	
	virtual PYXPointer<IRegionVisitor> deserializeVisitor(PYXWireBuffer & buffer) const
	{
		PYXTHROW_NOT_IMPLEMENTED();
		return deserializeVisitor(buffer, PYXIcosIndex()); // index here is not important as we should never get here!
	};

	virtual PYXPointer<IRegionVisitor> deserializeVisitor(PYXWireBuffer & buffer,const PYXIcosIndex & index) const;
private:
	class Visitor : public IRegionVisitor
	{
		friend class PYXMultiPolygonRegion;
	private:
		class Iterator : public PYXInnerTileIntersectionIterator
		{
		public:
			static PYXPointer<Iterator> create(const PYXPointer<const PYXMultiPolygonRegion::Visitor> & visitor,const PYXInnerTile & tile)
			{
				return PYXNEW(Iterator,visitor,tile);
			}

			Iterator(const PYXPointer<const PYXMultiPolygonRegion::Visitor> & visitor,const PYXInnerTile & tile);
			
			virtual const PYXInnerTile & getTile() const;
			virtual const PYXInnerTileIntersection & getIntersection() const;
			virtual bool end() const;
			virtual void next();

		private:
			PYXPointer<const PYXMultiPolygonRegion::Visitor> m_visitor;
			PYXInnerTile m_tile;
			PYXIcosIndex m_index;

			PYXInnerTile m_currentTile;
			PYXInnerTileIntersection m_currentIntersection;

			std::vector<SphereMath::GreatCircleArc> m_arcs;

			bool isPointContained(const PYXCoord3DDouble & location) const;			
			void findIntersection();
			void findNextInnerTile(PYXIcosIndex & index);
		};
	private:
		static const int knOptimalSize = 10;

		//help constructor that takes the chunks
		Visitor(const PYXPointer<const PYXMultiPolygonRegion> & curve, std::vector<Range<int>> & chunks,const PYXCoord3DDouble & rayOrigin, bool isOriginContained);
		Visitor(const PYXPointer<const PYXMultiPolygonRegion> & curve, PYXWireBuffer & buffer,const PYXIcosIndex & index);
 
		void serialize(PYXWireBuffer & buffer);

	public:
		virtual double getDistanceToBorder( const PYXCoord3DDouble & location,double errorThreshold = 0 ) const;
		virtual bool isPointContained(const PYXCoord3DDouble & location,double errorThreshold = 0) const;		

		static PYXPointer<Visitor> create(const PYXPointer<const PYXMultiPolygonRegion> & polygon,PYXWireBuffer & buffer,const PYXIcosIndex & index)
		{
			return PYXNEW(Visitor, polygon, buffer, index);
		}

		static PYXPointer<Visitor> create(const PYXPointer<const PYXMultiPolygonRegion> & polygon)
		{
			return PYXNEW(Visitor,polygon);
		}

		static PYXPointer<Visitor> create(const Visitor & other)
		{
			return PYXNEW(Visitor,other);
		}

		Visitor(const PYXPointer<const PYXMultiPolygonRegion> & curve);
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
		PYXPointer<const PYXMultiPolygonRegion> m_polygon;
		std::vector<Range<int>> m_chunks;
		int m_rayOriginContained;
		PYXCoord3DDouble m_rayOrigin;
	};



//Polygon manipulations
public:

	//! get exterior point location - valid only after seting up an exterior ring
	const PYXCoord3DDouble & getExteriorPoint() const;

	const bool getExteriorPointContained() const;

//consturtors 
public:

	//static consturtor
	static PYXPointer<PYXMultiPolygonRegion> create()
	{
		return PYXNEW(PYXMultiPolygonRegion);
	}

	//static copy consturtor
	static PYXPointer<PYXMultiPolygonRegion> create(PYXMultiPolygonRegion multiPolygon )
	{
		return PYXNEW(PYXMultiPolygonRegion,multiPolygon);
	}

	static PYXPointer<PYXMultiPolygonRegion> create(const std::vector< PYXPointer<PYXCurveRegion> > &  regions )
	{
		return PYXNEW(PYXMultiPolygonRegion,regions);
	}
	

	//! Constructs an empty polygon region
	explicit PYXMultiPolygonRegion();

	//! Copy constuctor
	PYXMultiPolygonRegion::PYXMultiPolygonRegion(const PYXMultiPolygonRegion & multiPolygon);

	explicit PYXMultiPolygonRegion(const std::vector< PYXPointer<PYXCurveRegion> > & regions);

	//! Convenience constructor; constructs a polygon region with external ring
	explicit PYXMultiPolygonRegion(const std::vector<PYXCoord3DDouble> vertices);


public:
	int getRingsCount() const;
	
	int getRingVerticesCount(unsigned int index) const;

	PYXCoord3DDouble getRingVertex(unsigned int ringIndex,unsigned int vertexIndex) const;

private:

	//Memberes
private:
	
	ArcBuffer m_arcs;
	PYXBoundingCircle m_boundingCircle;
	PYXCoord3DDouble m_exteriorPoint;
	int m_exteriorPointContained;
	void setExteriorPoint(const PYXCoord3DDouble & point,bool isContained);

public:
	friend PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXMultiPolygonRegion & region);
	friend PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXMultiPolygonRegion> & region);
};

PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXMultiPolygonRegion & region);
PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXMultiPolygonRegion> & region);

#endif // guard
