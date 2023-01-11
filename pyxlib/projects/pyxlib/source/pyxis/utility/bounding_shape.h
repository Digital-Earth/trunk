#ifndef PYXIS__UTILITY__BOUNDING_SHAPE_H
#define PYXIS__UTILITY__BOUNDING_SHAPE_H
/******************************************************************************
bounding_shape.h

begin		: 2011-01-22
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/math_utils.h"
#include "pyxis/utility/coord_3d.h"
#include "pyxis/utility/sphere_math.h"
#ifdef INSTANCE_COUNTING
#include "pyxis/utility/instance_counter.h"
#endif

/*! PYXBoundingShape interface class

PYXBoundingShape is the base interface to work with bounding shapes.

a bounding shape can provide two queries
1. canIntersects(ray) - return true if bounding shape intersect the given GreateCircleArc
2. canBeCloser(location,distance) - return true if the bounding shape can be closer
3. getBoundingArea() - return the area of the bounding shape

4. contains(location) - return true if containing the given location (TODO)
*/
class PYXLIB_DECL PYXBoundingShape
#ifdef INSTANCE_COUNTING
	: protected InstanceCounter
#endif
{
public:
	virtual bool canIntersects(const SphereMath::GreatCircleArc & ray) const = 0;

	virtual bool canBeCloser(const PYXCoord3DDouble & location,double distance) const = 0;

	virtual double getBoundingArea() const = 0;
};


//! PYXBoundingGreatCircleArc - bounding buffer around a GreatCircleArc
class PYXLIB_DECL PYXBoundingGreatCircleArc : public PYXBoundingShape
{
private:
	SphereMath::GreatCircleArc	m_arc;
	double						m_radius;

public:
	PYXBoundingGreatCircleArc(const SphereMath::GreatCircleArc	& arc,double radius) 
		:	m_arc(arc),
			m_radius(radius)
	{
	}	

	PYXBoundingGreatCircleArc(const PYXBoundingGreatCircleArc & other) 
		:	m_arc(other.m_arc),
			m_radius(other.m_radius)
	{
	}

public:
	PYXBoundingGreatCircleArc & operator = (const PYXBoundingGreatCircleArc & other)
	{
		if (this == &other)
		{
			return *this;
		}

		m_arc = other.m_arc;
		m_radius = other.m_radius;

		return *this;
	}

public:
	const SphereMath::GreatCircleArc & getGreatCircleArc() const { return m_arc; }
	double getRadius() const { return m_radius; }

//PYXBoundingShape API
public:
	virtual bool canIntersects(const SphereMath::GreatCircleArc & ray) const
	{
		return	ray.intersects(m_arc) ||
				ray.distanceTo(m_arc.getPointA()) < m_radius || 
				ray.distanceTo(m_arc.getPointB()) < m_radius ||
				m_arc.distanceTo(ray.getPointA()) < m_radius ||
				m_arc.distanceTo(ray.getPointB()) < m_radius;
	}

	virtual bool canBeCloser(const PYXCoord3DDouble & location,double distance) const
	{
		return m_arc.distanceTo(location)-m_radius < distance;
	}

	virtual double getBoundingArea() const
	{
		//this equation calculate the area over a plane. and not on the sphere. so, big bounding arc will have errors.
		return (SphereMath::distanceBetween(m_arc.getPointA(),m_arc.getPointB())+m_radius*MathUtils::kfPI)*m_radius;
	}
};

//! PYXBoundingCircle - bounding buffer around a location on earth
class PYXLIB_DECL PYXBoundingCircle : public PYXBoundingShape
{
private:
	PYXCoord3DDouble 	m_center;
	double 				m_radius;

public:
	static PYXBoundingCircle global()
	{
		return PYXBoundingCircle(PYXCoord3DDouble(1,0,0),MathUtils::kfPI);
	}

	PYXBoundingCircle() : m_center(),m_radius(0)
	{
	}

	PYXBoundingCircle(const PYXCoord3DDouble & center,double radius) 
		:	m_center(center),
			m_radius(radius)
	{
	}

	PYXBoundingCircle(const SphereMath::GreatCircleArc & arc,double radius)
		:	m_radius(radius+SphereMath::distanceBetween(arc.getPointA(),arc.getPointB())*0.5)
	{
		m_center = m_center.pointAlongLine(arc.getPointA(),arc.getPointB(),0.5);
		m_center.normalize();
	}

	PYXBoundingCircle(const PYXBoundingCircle & other) 
		:	m_center(other.m_center),
			m_radius(other.m_radius)
	{
	}

public:
	//!estimate a resolution based on a radius of a bounding circle
	static int estimateResolutionFromRadius(double radius);
	
public:
	PYXBoundingCircle & operator = (const PYXBoundingCircle & other)
	{
		if (this == &other)
		{
			return *this;
		}

		m_center = other.m_center;
		m_radius = other.m_radius;

		return *this;
	}

	//find bounding circle between two circles.
	PYXBoundingCircle & operator +=(const PYXBoundingCircle & other)
	{
		PYXBoundingCircle oldCircle = *this;
		if (isEmpty())
		{
			*this = other;
			return *this;
		}

		if (other.isEmpty())
		{
			return *this;
		}

		double distanceBetweenCenters = SphereMath::distanceBetween(other.m_center,m_center);

		if (m_radius > distanceBetweenCenters+other.m_radius)
		{
			//we already cover the other circle
			return *this;
		}
		if (other.m_radius > distanceBetweenCenters+m_radius)
		{
			//the other circle cover us complete
			*this = other;
			return *this;
		}

		//the radius of the union circle
		double newRadius = (m_radius+distanceBetweenCenters+other.m_radius)/2;

		//fix numerical issues
		if (newRadius<m_radius)
		{
			newRadius=m_radius;
		}

		// Check if we need to shift the center little bit.
		// If the points are equal within a given tolerance and
		// the distance between centers is less than 1/1000th of the newRadius - skip this step
		if (!other.m_center.equal(m_center) && (distanceBetweenCenters > newRadius / 1000))
		{
			PYXCoord3DDouble oldCenter = m_center;
			//shift the center little bit.
			m_center = SphereMath::GreatCircleArc(m_center,other.m_center).pointAlongArc(std::max(0.0,std::min(1.0,(newRadius-m_radius)/distanceBetweenCenters)));

			double diff = (SphereMath::distanceBetween(m_center,oldCenter)+m_radius)-newRadius;

			//fix numerical issues
			if (diff>0)
			{
				newRadius += diff;
			}

			diff = (SphereMath::distanceBetween(m_center,other.m_center)+other.m_radius)-newRadius;

			if (diff>0)
			{
				newRadius += diff;
			}
		}

		m_radius = newRadius;

		//fix numeric issues
		while(! contains(oldCircle) || ! contains(other))
		{
			m_radius *= 1.001;
		}		

		return *this;
	}

	//add the PYXBoundingGreatCircleArc into current bounding circle
	PYXBoundingCircle & operator +=(const PYXBoundingGreatCircleArc & arc)
	{
		*this += PYXBoundingCircle(arc.getGreatCircleArc().getPointA(),arc.getRadius());
		*this += PYXBoundingCircle(arc.getGreatCircleArc().getPointB(),arc.getRadius());

		return *this;
	}
	
	const PYXBoundingCircle operator +(const PYXBoundingCircle & other)
	{
		PYXBoundingCircle result = *this;
		result += other;

		return result;
	}

	const PYXBoundingCircle operator +(const PYXBoundingGreatCircleArc & arc)
	{
		PYXBoundingCircle result = *this;
		result += arc;

		return result;
	}

	//return true if the given PYXBoundingCircle is completely contained by this circle.
	bool contains(const PYXCoord3DDouble & location) const
	{
		return (SphereMath::distanceBetween(m_center,location) <= m_radius);
	}

	//return true if the given PYXBoundingCircle is completely contained by this circle.
	bool contains(const PYXBoundingCircle & other) const
	{
		if (other.m_radius > m_radius)
		{
			return false;
		}

		return (SphereMath::distanceBetween(m_center,other.m_center)+other.m_radius <= m_radius);
	}

	//return true if the given PYXBoundingCircle intersects with circle.
	bool intersects(const PYXBoundingCircle & other) const
	{
		return (SphereMath::distanceBetween(m_center,other.m_center) < m_radius+other.m_radius);
	}

public:
	//return the center of the bounding circle
	const PYXCoord3DDouble & getCenter() const { return m_center; }
	
	//return the buffer radius around the location
	double getRadius() const { return m_radius; }
	
	//return true if the bounding circle is empty (no location on earth)
	bool isEmpty() const { return m_radius == 0 && m_center.equal(PYXCoord3DDouble()); }

	//PYXBoundingShape API
public:
	virtual bool canIntersects(const SphereMath::GreatCircleArc & ray) const
	{
		return ray.distanceTo(m_center) <= m_radius;
	}

	virtual bool canBeCloser(const PYXCoord3DDouble & location,double distance) const
	{
		return SphereMath::distanceBetween(location,m_center)-m_radius < distance;
	}

	virtual double getBoundingArea() const
	{
		if (m_radius>=MathUtils::kfPI)
		{
			return 4*m_radius*m_radius*MathUtils::kfPI;
		}
		else 
		{
			return 2*m_radius*MathUtils::kfPI * ( 1 - cos(m_radius));
		}
	}
};


#endif // guard
