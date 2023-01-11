/******************************************************************************
cml_utils.cpp

begin		: 2007-09-04
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#include "cml_utils.h"

#include "pyxis/utility/sphere_math.h"
#include "pyxis/derm/snyder_projection.h"

// standard includes
#include <limits>
#include <cassert>

/*!
The direction must be normalized.
\param p	Origin of ray (point).
\param d	Direction of ray (normalized vector).
\return The time of intersection (can be negative), or
		std::numeric_limits<double>::infinity() if no intersection.
*/
double intersectRayWithUnitSphere(const vec3& p, const vec3& d)
{
#if 1
	// Algebraic method. See http://www.devmaster.net/wiki/Ray-sphere_intersection
	double B = cml::dot(p, d);
	double C = cml::dot(p, p) - 1;
	double D = B*B - C;
	return D > 0 ? -B - sqrt(D) : std::numeric_limits<double>::infinity();
#else
	// Geometric method. See http://www.devmaster.net/wiki/Ray-sphere_intersection
	// TODO problems getting this working...
	vec3 oc(-p);
	double l2oc = cml::dot(oc, oc);

	if (l2oc < 1)
	{
		// Ray starts inside of unit sphere.
		return std::numeric_limits<double>::infinity();
	}
	else
	{
		double tca = cml::dot(oc, d);
		if (tca < 0)
		{
			// Points away from sphere.
			return std::numeric_limits<double>::infinity();
		}
		double l2hc = (1 - l2oc) / cml::dot(d, d) + tca*tca;
		return l2hc > 0 ? tca - sqrt(l2hc) : std::numeric_limits<double>::infinity();
	}
#endif
}


vec3 CmlConvertor::toVec3(const float * origin,const float * coord)
{
	return vec3(static_cast<double>(origin[0])+coord[0],
				static_cast<double>(origin[1])+coord[1],
				static_cast<double>(origin[2])+coord[2]);
}

void CmlConvertor::fromVec3(const vec3 & vector,float * origin,float * coord)
{
	//convert to float - we would have rounding error
	origin[0] = static_cast<float>(vector[0]);
	origin[1] = static_cast<float>(vector[1]);
	origin[2] = static_cast<float>(vector[2]);

	//make coord match the rounding error
	fromVec3WithOrigin(vector,origin,coord);
}

void CmlConvertor::fromVec3WithOrigin(const vec3 & vector,const vec3 & origin,float * coord)
{
	coord[0] = static_cast<float>(vector[0]-origin[0]);
	coord[1] = static_cast<float>(vector[1]-origin[1]);
	coord[2] = static_cast<float>(vector[2]-origin[2]);

	//TODO: make sure we are currect - the assert now working so great
	//assert(vec3(origin[0]+coord[0],origin[1]+coord[1],origin[2]+coord[2])==vector && "Failed to convert double vector into two floats vector");
}

void CmlConvertor::fromVec3WithOrigin(const vec3 & vector,const float * origin,float * coord)
{
	coord[0] = static_cast<float>(vector[0]-origin[0]);
	coord[1] = static_cast<float>(vector[1]-origin[1]);
	coord[2] = static_cast<float>(vector[2]-origin[2]);

	//TODO: make sure we are currect - the assert now working so great
	//assert(toVec3(origin,coord)==vector && "Failed to convert double vector into two floats vector");
}

vec3 CmlConvertor::toVec3(const CoordLatLon & latlon)
{
	return toVec3(SphereMath::llxyz(latlon));
}

vec3 CmlConvertor::toVec3(const PYXIcosIndex & index)
{
	PYXCoord3DDouble coord;
	SnyderProjection::getInstance()->pyxisToXYZ(index, &coord);

	return toVec3(coord);
}

PYXCoord3DDouble CmlConvertor::toPYXCoord3D(const CoordLatLon & latlon)
{
	return SphereMath::llxyz(latlon);
}

PYXCoord3DDouble CmlConvertor::toPYXCoord3D(const PYXIcosIndex & index)
{
	PYXCoord3DDouble coord;
	SnyderProjection::getInstance()->pyxisToXYZ(index, &coord);

	return coord;	
}

CoordLatLon CmlConvertor::toLatLon(const vec3 & vector)
{
	return toLatLon(toPYXCoord3D(vector));
}

CoordLatLon CmlConvertor::toLatLon(const PYXCoord3DDouble & coord)
{
	PYXCoord3DDouble normalizedCoord(coord);
	normalizedCoord.normalize();

	return SphereMath::xyzll(normalizedCoord);
}

CoordLatLon CmlConvertor::toLatLon(const PYXIcosIndex & index)
{
	CoordLatLon latlon;
	SnyderProjection::getInstance()->pyxisToNative(index,&latlon);

	return latlon;
}

PYXIcosIndex CmlConvertor::toPYXIcosIndex(const vec3 & vector,const int & resolution)
{
	return toPYXIcosIndex(toLatLon(vector),resolution);	
}

PYXIcosIndex CmlConvertor::toPYXIcosIndex(const CoordLatLon & latlon,const int & resolution)
{
	PYXIcosIndex root;			
	SnyderProjection::getInstance()->nativeToPYXIS(latlon, &root, resolution);

	return root;
}

PYXIcosIndex CmlConvertor::toPYXIcosIndex(const PYXCoord3DDouble & coord,const int & resolution)
{
	return toPYXIcosIndex(toLatLon(coord),resolution);
}