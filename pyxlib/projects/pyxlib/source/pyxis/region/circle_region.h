#ifndef PYXIS__REGION__CIRCLE_REGION_H
#define PYXIS__REGION__CIRCLE_REGION_H
/******************************************************************************
circle_region.h

begin		: 2010-12-01
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

class PYXLIB_DECL PYXCircleRegion : public PYXVectorRegion
{
//PYXRegion API
public:
	virtual PYXPointer<IRegion> clone() const;

	virtual int getVerticesCount() const
	{
		return 1;
	}

	virtual PYXPointer<IRegionVisitor> getVisitor() const
	{
		return PYXVectorRegionVisitor::create(this);
	}
	
	virtual void serializeVisitor(PYXWireBuffer & buffer,const PYXPointer<IRegionVisitor> & visitor) const
	{
		return;
	}

	virtual PYXPointer<IRegionVisitor> deserializeVisitor(PYXWireBuffer & buffer) const
	{
		return PYXVectorRegionVisitor::create(this);
	}

	//! distance in radians
	virtual double getDistanceToBorder(const PYXCoord3DDouble & location,double errorThreshold = 0) const;

	virtual bool isPointContained(const PYXCoord3DDouble & location,double errorThreshold = 0) const;

	virtual PYXBoundingCircle getBoundingCircle() const;

//members
private:
	//! circle center
	PYXCoord3DDouble m_center;

	//! radius of the circle in arc radians
	double m_radius;

public:

	//! Constructs a circle region from a center and radius in radians
	static PYXPointer<PYXCircleRegion> create(const PYXCoord3DDouble & center,double radius)
	{
		return PYXNEW(PYXCircleRegion,center,radius);
	}

	static PYXPointer<PYXCircleRegion> create(const PYXIcosIndex & cell, bool asTile)
	{
		return PYXNEW(PYXCircleRegion,cell,asTile);
	}

	static PYXPointer<PYXCircleRegion> create(const PYXCircleRegion & circle)
	{
		return PYXNEW(PYXCircleRegion,circle);
	}

	explicit PYXCircleRegion(const PYXIcosIndex & cell, bool asTile);

	explicit PYXCircleRegion(const PYXCircleRegion & circle);

	//! Constructs a circle region from a center and radius in radians
	explicit PYXCircleRegion(PYXCoord3DDouble const & centerm, double radius);

	const PYXCoord3DDouble & getCenter() const { return m_center; }

	const double & getRadius() const { return m_radius; }
};

PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXCircleRegion & region);
PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXCircleRegion> & region);

#endif // guard
