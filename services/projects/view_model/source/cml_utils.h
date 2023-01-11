#pragma once
#ifndef VIEW_MODEL__CML_UTILS_H
#define VIEW_MODEL__CML_UTILS_H
/******************************************************************************
cml_utils.h

begin		: 2007-09-04
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"

// cml includes
#include "cml/cml.h"

#include "pyxis/derm/index.h"
#include "pyxis/utility/coord_3d.h"
#include "pyxis/utility/coord_lat_lon.h"

// cml typedefs
typedef cml::vector2d vec2;
typedef cml::vector< double, cml::external<2> > vec2_alias;
typedef cml::vector3d vec3;
typedef cml::vector< double, cml::external<3> > vec3_alias;
typedef cml::vector4d vec4;
typedef cml::vector< double, cml::external<4> > vec4_alias;
typedef cml::matrix44d_c mat4;
typedef cml::matrix< double, cml::external<4,4>, cml::col_basis, cml::col_major > mat4_alias;
typedef cml::quaterniond_p quat;

//! Intersects a ray with a unit sphere.
VIEW_MODEL_API double intersectRayWithUnitSphere(const vec3& p, const vec3& d);

class CmlConvertor
{
public:
	static vec3 toVec3(const PYXCoord3DDouble & coord) { return vec3(coord[0],coord[1],coord[2]); }
	static vec3 toVec3(const CoordLatLon & latlon);
	static vec3 toVec3(const PYXIcosIndex & index);
		
	static vec3 toVec3(const float * origin,const float * coord);
	
	static void fromVec3(const vec3 & vector,float * origin,float * coord);
	static void fromVec3WithOrigin(const vec3 & vector,const vec3 & origin,float * coord);
	static void fromVec3WithOrigin(const vec3 & vector,const float * origin,float * coord);

	static PYXCoord3DDouble toPYXCoord3D(const vec3 & coord) { return PYXCoord3DDouble(coord[0],coord[1],coord[2]); }	
	static PYXCoord3DDouble toPYXCoord3D(const CoordLatLon & latlon);
	static PYXCoord3DDouble toPYXCoord3D(const PYXIcosIndex & index);

	static PYXIcosIndex toPYXIcosIndex(const vec3 & vector,const int & resolution);
	static PYXIcosIndex toPYXIcosIndex(const CoordLatLon & latlon,const int & resolution);
	static PYXIcosIndex toPYXIcosIndex(const PYXCoord3DDouble & coord,const int & resolution);

	static CoordLatLon toLatLon(const vec3 & vector);
	static CoordLatLon toLatLon(const PYXCoord3DDouble & coord);
	static CoordLatLon toLatLon(const PYXIcosIndex & index);
};

#endif
