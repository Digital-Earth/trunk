/******************************************************************************
ray.cpp

begin		: 2009-11-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "ray.h"

Ray::Ray(const vec3 & origin,const vec3 & direction) : m_origin(origin), m_direction(direction)
{
	m_direction.normalize();
}

Ray::Ray(const Camera & camera,const int & x,const int & y,const int & width, const int & height)
{
	double ndx = ((double)x / width) * 2 - 1;
	double ndy = ((double)y / height) * 2 - 1;

	// TODO use cml when it has support for only getting viewport coords
	// TODO or use cml picking or projection directly
	double planes[6][4];
	camera.calcFrustumPlanes(planes);

	vec3 corners[4];
	corners[0] = cml::detail::intersect_planes(planes[0], planes[2], planes[4]);
    corners[1] = cml::detail::intersect_planes(planes[1], planes[2], planes[4]);
    corners[2] = cml::detail::intersect_planes(planes[1], planes[3], planes[4]);
    corners[3] = cml::detail::intersect_planes(planes[0], planes[3], planes[4]);

	vec3 look = cml::bilerp(corners[0], corners[1], corners[3], corners[2], (ndx+1)/2, (ndy+1)/2);

	m_origin = camera.getEye();
	m_direction = look - m_origin;
	
	m_direction.normalize();
}

vec3 Ray::getPointFromTime(const double & time) const
{
	return m_origin + m_direction*time;
}

bool Ray::intersectsWithSphere(const vec3 & center,const double & radius) const
{
	//move sphere center to zero and change radius to 1 - and then we have a unit sphere
	vec3 newOrigin = (m_origin-center)/radius;

	// Algebraic method. See http://www.devmaster.net/wiki/Ray-sphere_intersection
	double B = cml::dot(newOrigin, m_direction);
	double C = cml::dot(newOrigin, newOrigin) - 1;
	double D = B*B - C;

	return D>0;
}

bool Ray::intersectsWithSphere(const vec3 & center,const double & radius,double & intersectionTime) const
{
	//move sphere center to zero and change radius to 1 - and then we have a unit sphere
	vec3 newOrigin = (m_origin-center)/radius;

	// Algebraic method. See http://www.devmaster.net/wiki/Ray-sphere_intersection
	double B = cml::dot(newOrigin, m_direction);
	double C = cml::dot(newOrigin, newOrigin) - 1;
	double D = B*B - C;

	if (D>0)
	{
		intersectionTime = -B - sqrt(D);
		return true;
	}

	return false;
}

bool Ray::intersectsWithSphere(const vec3 & center,const double & radius,vec3 & intersectinPoint) const
{
	double intersctionTime;
	if (intersectsWithSphere(center,radius,intersctionTime))
	{
		intersectinPoint = getPointFromTime(intersctionTime);
	}
	return false;
}



bool Ray::intersectsWithTriangle(const vec3 & v0,const vec3 & v1,const vec3 & v2) const
{
	//see http://www.lighthouse3d.com/opengl/maths/index.php?raytriint

	//float e1[3],e2[3],h[3],s[3],q[3];
	//float a,f,u,v;
	
	//vector(e1,v1,v0);
	//vector(e2,v2,v0);
	vec3 e1 = v1-v0;
	vec3 e2 = v2-v0;

	//crossProduct(h,d,e2);	
	vec3 h  = cml::cross(m_direction,e2);

	//a = innerProduct(e1,h);
	double a = cml::dot(e1,h);

	//if (a > -0.00001 && a < 0.00001)
	//	return(false);
	// Doesn't realy needed, we just assume it allwats intersecnts, However, Zero would create an exception
	if (a==0)
	{
		return false;
	}
		
	//f = 1/a;
	double f = 1/a;
	//vector(s,p,v0);
	//u = f * (innerProduct(s,h));
	vec3 s = m_origin-v0;
	double u = cml::dot(s,h) * f;
	
	//if (u < 0.0 || u > 1.0)
	//	return(false);

	if (u < 0.0 || u > 1.0)
		return(false);
	
	//crossProduct(q,s,e1);
	//v = f * innerProduct(d,q);
	vec3 q = cml::cross(s,e1);
	double v = cml::dot(m_direction,q) * f;

	//if (v < 0.0 || u + v > 1.0)
	//	return(false);

	if (v < 0.0 || u + v > 1.0)
		return(false);

	return true;	
}

bool Ray::intersectsWithTriangle(const vec3 & v0,const vec3 & v1,const vec3 & v2,double & interscetionTime) const
{
	//see http://www.lighthouse3d.com/opengl/maths/index.php?raytriint

	//float e1[3],e2[3],h[3],s[3],q[3];
	//float a,f,u,v;
	
	//vector(e1,v1,v0);
	//vector(e2,v2,v0);
	vec3 e1 = v1-v0;
	vec3 e2 = v2-v0;

	//crossProduct(h,d,e2);	
	vec3 h  = cml::cross(m_direction,e2);

	//a = innerProduct(e1,h);
	double a = cml::dot(e1,h);

	//if (a > -0.00001 && a < 0.00001)
	//	return(false);

	// Doesn't realy needed, we just assume it allwats intersecnts, However, Zero would create an exception
	if (a==0)
	{
		return false;
	}

	//f = 1/a;
	double f = 1/a;
	//vector(s,p,v0);
	//u = f * (innerProduct(s,h));
	vec3 s = m_origin-v0;
	double u = cml::dot(s,h) * f;
	
	//if (u < 0.0 || u > 1.0)
	//	return(false);

	if (u < 0.0-DBL_EPSILON || u > 1.0+DBL_EPSILON)
		return(false);
	
	//crossProduct(q,s,e1);
	//v = f * innerProduct(d,q);
	vec3 q = cml::cross(s,e1);
	double v = cml::dot(m_direction,q) * f;

	//if (v < 0.0 || u + v > 1.0)
	//	return(false);

	if (v < 0.0-DBL_EPSILON || u + v > 1.0+DBL_EPSILON)
		return(false);

	// at this stage we can compute t to find out where 
	// the intersection point is on the line
	//t = f * innerProduct(e2,q);

	interscetionTime = f * cml::dot(e2,q);

	return true;
}

bool Ray::intersectsWithTriangle(const vec3 & v0,const vec3 & v1,const vec3 & v2,vec3 & intersectinPoint) const
{
	double intersctionTime;
	if (intersectsWithTriangle(v0,v1,v2,intersctionTime))
	{
		intersectinPoint = getPointFromTime(intersctionTime);
		return true;
	}

	return false;
}

bool Ray::intersectsWithTriangle(const vec3 & v0,const vec3 & v1,const vec3 & v2,double & u,double & v) const
{
	//see http://www.lighthouse3d.com/opengl/maths/index.php?raytriint

	//float e1[3],e2[3],h[3],s[3],q[3];
	//float a,f,u,v;
	
	//vector(e1,v1,v0);
	//vector(e2,v2,v0);
	vec3 e1 = v1-v0;
	vec3 e2 = v2-v0;

	//crossProduct(h,d,e2);	
	vec3 h  = cml::cross(m_direction,e2);

	//a = innerProduct(e1,h);
	double a = cml::dot(e1,h);

	//if (a > -0.00001 && a < 0.00001)
	//	return(false);
	// Doesn't realy needed, we just assume it allwats intersecnts, However, Zero would create an exception
	if (a==0)
	{
		return false;
	}

	//f = 1/a;
	double f = 1/a;
	//vector(s,p,v0);
	//u = f * (innerProduct(s,h));
	vec3 s = m_origin-v0;
	u = cml::dot(s,h) * f;
	
	//if (u < 0.0 || u > 1.0)
	//	return(false);

	if (u < 0.0 || u > 1.0)
		return(false);
	
	//crossProduct(q,s,e1);
	//v = f * innerProduct(d,q);
	vec3 q = cml::cross(s,e1);
	v = cml::dot(m_direction,q) * f;

	//if (v < 0.0 || u + v > 1.0)
	//	return(false);

	if (v < 0.0 || u + v > 1.0)
		return(false);

	return true;
}
