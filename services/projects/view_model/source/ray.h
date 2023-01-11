#pragma once
#ifndef VIEW_MODEL__RAY_H
#define VIEW_MODEL__RAY_H
/******************************************************************************
ray.h

begin		: 2009-11-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "cml_utils.h"
#include "camera.h"

/*!

Ray class is used to find intersection of a rat with Triangls and Shperes.

a Ray is defined with Origin and Direction.

NOTE: the intersects* functions return true if the the Line is intersects. if you would like to find if the Ray from the origin intersects, check if intersctionTime>0.

the intersectionTime is defined in the following parametric equation:
  P = Origin + intersectionTime * Direction.

Therefore, if intersectionTime > 0, P is a on the Direction from the Origin point - Therefore, forward!

TODO: add algined BBOX and some more intersection calculations

*/
class Ray 
{
protected:
	vec3 m_origin;
	vec3 m_direction;

public:
	//! construct a ray from a given origin and a direction
	Ray(const vec3 & origin,const vec3 & direction);

	//! construct a ray from a camera and cordinate on the screen
	Ray(const Camera & camera,const int & x,const int & y,const int & width, const int & height);

	//! construct a ray from a given ray
	Ray(const Ray & other) : m_origin(other.m_origin), m_direction(other.m_direction)
	{
	}

	//! assigment opertor
	Ray & operator=(const Ray & other)
	{
		m_origin = other.m_origin;
		m_direction = other.m_direction;
	}

	const vec3 & getOrigin() const { return m_origin; }
	const vec3 & getDirection() const { return m_direction; }

	//! get point from ray time
	vec3 getPointFromTime(const double & time) const;

	//! retun true if ray intersects with shpere
	bool intersectsWithSphere(const vec3 & center,const double & radius) const;

	//! return true if ray intersects with sphere and return intersction time
	bool intersectsWithSphere(const vec3 & center,const double & radius,double & intersectionTime) const;

	//! return true if ray intersects with sphere and return intersction point
	bool intersectsWithSphere(const vec3 & center,const double & radius,vec3 & intersectinPoint) const;

	//! return true if ray intersects with triangle
	bool intersectsWithTriangle(const vec3 & v0,const vec3 & v1,const vec3 & v2) const;

	//! return true if ray intersects with triangle and return intersction time
	bool intersectsWithTriangle(const vec3 & v0,const vec3 & v1,const vec3 & v2,double & interscetionTime) const;

	//! return true if ray intersects with triangle and return intersction point
	bool intersectsWithTriangle(const vec3 & v0,const vec3 & v1,const vec3 & v2,vec3 & intersectinPoint) const;		

	//! return true if ray intersects with triangle and return intersction u,v cordiantes: P = v0*u + v1*v + v2*(1-u-v)
	bool intersectsWithTriangle(const vec3 & v0,const vec3 & v1,const vec3 & v2,double & u,double & v) const;
};

#endif