/******************************************************************************
circle_region.cpp

begin		: 2010-12-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "circle_region.h"
#include "region_serializer.h"

#include "pyxis/derm/index.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/snyder_projection.h"



PYXPointer<IRegion> PYXCircleRegion::clone() const
{
	return PYXCircleRegion::create(*this);
}

//! distance in radians
double PYXCircleRegion::getDistanceToBorder(const PYXCoord3DDouble & location,double errorThreshold) const
{
	return abs(SphereMath::distanceBetween(m_center,location)-m_radius);
}

bool PYXCircleRegion::isPointContained(const PYXCoord3DDouble & location,double errorThreshold) const
{
	return SphereMath::distanceBetween(m_center,location) < m_radius;
}

PYXBoundingCircle PYXCircleRegion::getBoundingCircle() const
{
	return PYXBoundingCircle(m_center,m_radius);
}

PYXCircleRegion::PYXCircleRegion(const PYXCircleRegion & circle) : m_center(circle.m_center), m_radius(circle.m_radius)
{
}

//! Constructs a point region from a PYXCoord3DDouble 
PYXCircleRegion::PYXCircleRegion(PYXCoord3DDouble const & center, double radius) :  m_center(center), m_radius(radius)
{
}

PYXCircleRegion::PYXCircleRegion(const PYXIcosIndex & cell,bool asTile)
{	
	SnyderProjection::getInstance()->pyxisToXYZ(cell, &m_center);

	m_radius = (asTile ?
		PYXIcosMath::UnitSphere::calcTileCircumRadius(cell) :
		PYXIcosMath::UnitSphere::calcCellCircumRadius(cell));

}


PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXCircleRegion & region)
{
	buffer << region.getCenter() << region.getRadius();
	return buffer;
}

PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXCircleRegion> & region)
{
	PYXCoord3DDouble center;
	double radius;
	buffer >> center >> radius;
	region = PYXCircleRegion::create(center,radius);
	return buffer;
}