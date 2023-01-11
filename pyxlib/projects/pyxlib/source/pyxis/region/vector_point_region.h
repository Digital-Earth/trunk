#ifndef PYXIS__REGION__VECTOR_POINT_REGION_H
#define PYXIS__REGION__VECTOR_POINT_REGION_H
/******************************************************************************
vector_point_region.h

begin		: 2010-11-18
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/region/region.h"
#include "pyxis/derm/index.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/sphere_math.h"
#include "pyxis/utility/wire_buffer.h"


#include <vector>

class PYXLIB_DECL PYXVectorPointRegion : public PYXVectorRegion
{
//PYXRegion API
public:
	virtual PYXPointer<IRegion> clone() const;

	virtual int getVerticesCount() const
	{
		return 1;
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

	virtual PYXBoundingCircle getBoundingCircle() const;

//members
private:
	PYXCoord3DDouble m_point;


private:
	class Visitor : public IRegionVisitor
	{
		friend class PYXVectorPointRegion;
	private:
		class Iterator : public PYXInnerTileIntersectionIterator
		{
		public:
			static PYXPointer<Iterator> create(const PYXPointer<const PYXVectorPointRegion::Visitor> & visitor,const PYXInnerTile & tile)
			{
				return PYXNEW(Iterator,visitor,tile);
			}

			Iterator(const PYXPointer<const PYXVectorPointRegion::Visitor> & visitor,const PYXInnerTile & tile);

			virtual const PYXInnerTile & getTile() const { return m_currentTile; }
			virtual const PYXInnerTileIntersection & getIntersection() const { return m_currentIntersection; }
			virtual bool end() const;
			virtual void next();

		private:
			PYXPointer<const PYXVectorPointRegion::Visitor> m_visitor;
			PYXInnerTile m_tile;
			std::vector<PYXIcosIndex> m_candidates;
			size_t m_candidateIndex;

			PYXInnerTile m_currentTile;
			PYXInnerTileIntersection m_currentIntersection;

			void findIntersection();
			void findNextInnerTile(PYXIcosIndex & index);
		};

		Visitor(const PYXPointer<const PYXVectorPointRegion> & point);
		Visitor(const PYXPointer<const PYXVectorPointRegion> & point, PYXWireBuffer & buffer);
		Visitor(const Visitor & other);

	public:
		virtual bool isPointContained(const PYXCoord3DDouble & location,double errorThreshold = 0) const { return false; }

		static PYXPointer<Visitor> create(const PYXPointer<const PYXVectorPointRegion> & point,PYXWireBuffer & buffer)
		{
			return PYXNEW(Visitor,point,buffer);
		}

		static PYXPointer<Visitor> create(const PYXPointer<const PYXVectorPointRegion> & point)
		{
			return PYXNEW(Visitor,point);
		}

		static PYXPointer<Visitor> create(const Visitor & other)
		{
			return PYXNEW(Visitor,other);
		}

	public: //IRegionVisitor
		virtual bool isOptimal() const { return true; }

		virtual PYXPointer<IRegionVisitor> trim(const PYXIcosIndex & index) const
		{
			if (m_point->intersects(index,false) == PYXRegion::knNone)
			{
				return 0;
			}
			return const_cast<Visitor*>(this);
		}

		virtual PYXPointer<PYXInnerTileIntersectionIterator> getInnerTileIterator(const PYXInnerTile & tile) const;

		virtual PYXInnerTileIntersection intersects(const PYXBoundingCircle & circle,double errorThreshold = 0) const
		{
			if (m_point->intersects(circle,errorThreshold) == PYXRegion::knNone)
			{
				return knIntersectionNone;
			}
			return knIntersectionPartial;
		}

		virtual PYXInnerTileIntersection intersects(const PYXIcosIndex & cell) const
		{
			if (m_point->intersects(cell,false) == PYXRegion::knNone)
			{
				return knIntersectionNone;
			}
			return knIntersectionPartial;
		}

	private:
		PYXPointer<const PYXVectorPointRegion> m_point;
	};

public:

	static PYXPointer<PYXVectorPointRegion> create(const PYXCoord3DDouble & point)
	{
		return PYXNEW(PYXVectorPointRegion,point);
	}

	static PYXPointer<PYXVectorPointRegion> create(const PYXVectorPointRegion & point)
	{
		return PYXNEW(PYXVectorPointRegion,point);
	}

	//! Constructs a point region from a PYXCoord3DDouble 
	explicit PYXVectorPointRegion(PYXCoord3DDouble const & point);

	const PYXCoord3DDouble & getPoint() const { return m_point; }
};

PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXVectorPointRegion & region);
PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXVectorPointRegion> & region);

#endif // guard
