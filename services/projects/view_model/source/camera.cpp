/******************************************************************************
camera.cpp

begin		: 2007-07-27
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#include "camera.h"

// pyxlib includes
#include "pyxis/derm/wgs84.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/utility/sphere_math.h"

// standard includes
#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>

#pragma warning(push)
#pragma warning(disable: 4482) 

std::string Camera::toString() const
{
	char buf[256];

	if (isModeOrbital())
	{
		vec3 eye = getEye();
		vec3 center = getCenter();
		vec3 up = getUp();

		sprintf_s(buf,sizeof(buf),"(%6.3f,%6.3f,%6.3f,%6.3f) (%6.3f,%6.3f,%6.3f) (%6.3f,%6.3f,%6.3f) (%6.3f,%6.3f,%6.3f)",
		m_fovy, m_aspect, m_nearz, m_farz,
		eye[0], eye[1], eye[2],
		center[0], center[1], center[2],
		up[0], up[1], up[2]);
	}
	else 
	{
		PYXTHROW(PYXException,"unsupported camera mode");
	}

	std::string str(buf);
	return str;
}

std::string Camera::getPerspectiveString() const
{
	char buf[128];
	sprintf_s(buf, sizeof(buf), "(%6.3f,%6.3f,%6.3f,%6.3f)", m_fovy, m_aspect, m_nearz, m_farz);
	return buf;
}

std::string Camera::getEyeString() const
{
	char buf[128];
	vec3 eye = getEye();
	sprintf_s(buf, sizeof(buf), "(%6.3f,%6.3f,%6.3f)", eye[0], eye[1], eye[2]);
	return buf;
}

std::string Camera::getCenterString() const
{
	char buf[128];
	sprintf_s(buf, sizeof(buf), "(%6.3f,%6.3f,%6.3f)", m_center[0], m_center[1], m_center[2]);
	return buf;
}

std::string Camera::getUpString() const
{
	char buf[32];
	vec3 up = getUp();
	sprintf_s(buf, sizeof(buf),  "(%6.3f,%6.3f,%6.3f)", up[0], up[1], up[2]);
	return buf;
}

vec3 Camera::getCenter() const
{
	return m_center;
}

vec3 Camera::getEye() const
{
	vec3 v;

	if (isModeOrbital())
	{
		v.zero();

		mat4 m;
		getModelViewMatrix(m);
		m.inverse();

		v = cml::transform_point(m, v);
	}

	return v;
}

vec3 Camera::getUp() const
{
	vec3 v;

	if (isModeOrbital())
	{
		v.cardinal(1);

		mat4 m;
		getModelViewMatrix(m);
		m.inverse();

		v = cml::transform_vector(m, v);
	}

	return v;
}

vec3 Camera::getLook() const
{
	vec3 v;

	if (isModeOrbital())
	{
		v.zero();
		v[2] = -1;

		mat4 m;
		getModelViewMatrix(m);
		m.inverse();

		v = cml::transform_vector(m, v);
	}

	return v;
}

double Camera::getOribitalUnitAltitude() const
{
	return m_altitude/SphereMath::knEarthRadius;
}

CoordLatLon Camera::getOrbitalTargetLatLon() const
{
	double a0, a1, a2;
	quat e;
	cml::quaternion_rotation_euler(e, -cml::constantsd::pi_over_2(), -cml::constantsd::pi_over_2(), 0.0, cml::EulerOrder::euler_order_xyz);
	quat c = getOrbitalRotation();
	quat q = cml::quaternion_rotation_difference(e, c);
	cml::quaternion_to_euler(q, a0, a1, a2, cml::EulerOrder::euler_order_yxz);

	return CoordLatLon(a1, -a0);
}

PYXIcosIndex Camera::getOrbitalTargetIndex(int nResolution) const
{		
	PYXIcosIndex cellIndex;
		
	SnyderProjection::getInstance()->nativeToPYXIS(getOrbitalTargetLatLon(),&cellIndex,nResolution);		

	return cellIndex;
}

CoordLatLon Camera::getOrbitalEyeLatLon() const
{
	vec3 eye = getEye();
	eye /= eye.length();

	PYXCoord3DDouble xyz(eye[0],eye[1],eye[2]);	
	xyz.normalize();
			
	return SphereMath::xyzll(xyz);
}

PYXIcosIndex Camera::getOrbitalEyeIndex(int nResolution) const
{
	PYXIcosIndex cellIndex;
		
	SnyderProjection::getInstance()->nativeToPYXIS(getOrbitalEyeLatLon(),&cellIndex,nResolution);		

	return cellIndex;
}

void Camera::adjustOrbitalAltitudeAndRange(double newAlt,double minRange)
{
	double alt_delta = newAlt - m_altitude;
	
	m_altitude = newAlt;

	double range_delta = - alt_delta / cos(cml::rad(m_tilt));

	if (m_range + range_delta > minRange)
	{
		m_range += range_delta;
	} else 
	{
		m_range = minRange;
	}
}

double Camera::getHeading() const
{
	double hdg;

	if (isModeOrbital())
	{
		mat4 m;
		cml::matrix_rotation_quaternion(m, m_rot);

		vec3 v(0, 0, 1);
		v = cml::transform_vector(m, v);

		v[2] = 0; // project to xy plane

		// TODO need special handling for poles (singularity)

		hdg = cml::deg(cml::signed_angle(vec3(0, 1, 0), v, vec3(0, 0, 1)));

		if (hdg < 0)
		{
			hdg += 360;
		}
	}

	return hdg;
}

void Camera::setHeading(double hdg)
{
	if (isModeOrbital())
	{
		// TODO this is quite inefficient

		double lat, lon, dummy, alt, tlt, rng;
		camToSettings(*this, lat, lon, dummy, alt, tlt, rng);
		camFromSettings(*this, lat, lon, hdg, alt, tlt, rng);
	}
}

/*
Not working. m_eye,m_up is broken

void Camera::dolly(double dist)
{
	vec3_alias eye(m_eye);
	vec3_alias center(m_center);
	vec3 n(center - eye);
	n.normalize();
	eye += (n*dist);
#if 0
	// TEMP reset near and far dist
	n = center - eye;
	double l = n.length();
	m_farz = l;
	m_nearz = (l-1)/2;
#endif
}

void Camera::azimuth(double deg)
{
	vec3_alias eye(m_eye);
	vec3_alias center(m_center);
	vec3_alias up(m_up);

	vec3 n(center - eye);

	mat4 m;
	mat4 t;

	cml::matrix_translation(m, center);
	cml::matrix_rotation_axis_angle(t, up, cml::rad(deg));
	m *= t;
	cml::matrix_translation(t, -center);
	m *= t;

	eye = cml::transform_point(m, eye);

	recalcUp();
}

void Camera::zenith(double deg)
{
	vec3_alias eye(m_eye);
	vec3_alias center(m_center);
	vec3_alias up(m_up);

	vec3 n(center - eye);
	vec3 u(cml::unit_cross(up, n));

	mat4 m;
	mat4 t;

	cml::matrix_translation(m, center);
	cml::matrix_rotation_axis_angle(t, -u, cml::rad(deg));
	m *= t;
	cml::matrix_translation(t, -center);
	m *= t;

	eye = cml::transform_point(m, eye);

	recalcUp();
}

void Camera::orbit(double degazi, double degzen)
{
	// TODO code this directly for optimization
	azimuth(degazi);
	zenith(degzen);
}

void Camera::pitch(double deg)
{
	vec3_alias eye(m_eye);
	vec3_alias center(m_center);
	vec3_alias up(m_up);

	vec3 n(center - eye);
	vec3 u(cml::unit_cross(up, n));

	mat4 m;
	mat4 t;

	cml::matrix_translation(m, eye);
	cml::matrix_rotation_axis_angle(t, -u, cml::rad(deg));
	m *= t;
	cml::matrix_translation(t, -eye);
	m *= t;

	center = cml::transform_point(m, center);

	recalcUp();
}

void Camera::yaw(double deg)
{
	vec3_alias eye(m_eye);
	vec3_alias center(m_center);
	vec3_alias up(m_up);

	vec3 n(center - eye);

	mat4 m;
	mat4 t;

	cml::matrix_translation(m, eye);
	cml::matrix_rotation_axis_angle(t, up, cml::rad(deg));
	m *= t;
	cml::matrix_translation(t, -eye);
	m *= t;

	center = cml::transform_point(m, center);

	recalcUp();
}

void Camera::roll(double deg)
{
	vec3_alias eye(m_eye);
	vec3_alias center(m_center);
	vec3_alias up(m_up);

	vec3 n(center - eye);
	n.normalize();

	mat4 m;

	cml::matrix_rotation_axis_angle(m, n, cml::rad(deg));

	up = cml::transform_point(m, up);
}

void Camera::rotate(double deg)
{
	if (isModeOrbital())
	{
		// TODO rotate
	}
}
*/

void Camera::calcFrustumPlanes(double planes[6][4]) const
{
	mat4 p;
	getProjectionMatrix(p);

	mat4 m;
	getModelViewMatrix(m);

	// NOTE use code like this to create modelview matrix from eye/center/up representation
	//cml::matrix_look_at_RH(m, m_eye[0], m_eye[1], m_eye[2], m_center[0], m_center[1], m_center[2], m_up[0], m_up[1], m_up[2]);

	// Apparently, order is left, right, bottom, top, near, far
	cml::extract_frustum_planes(m, p, planes, cml::z_clip_neg_one);
}

/*
void Camera::recalcUp()
{
	vec3_alias eye(m_eye);
	vec3_alias center(m_center);
	vec3_alias up(m_up);
	vec3 n(center - eye);
	vec3 u(cml::cross(up, n));
	up = cml::unit_cross(n, u);
}
*/

void Camera::getModelViewMatrix(mat4& m) const
{
	// TODO if we just want the inverse matrix, we could build it directly instead of inverting this.
	// Also if we just want the translation or rotation bits, we can build them directly.

	if (isModeOrbital())
	{
		mat4 t; // temporary matrix

		cml::matrix_translation(m, 0.0, 0.0, -m_range/SphereMath::knEarthRadius);                // range
		// no need to multiply
		cml::matrix_rotation_axis_angle(t, vec3(-1, 0, 0), cml::rad(m_tilt)); // tilt
		m *= t;
		cml::matrix_translation(t, 0.0, 0.0, -1 - m_altitude/SphereMath::knEarthRadius);            // altitude
		m *= t;
		cml::matrix_rotation_quaternion(t, m_rot);                           // orientation
		m *= t;
	}
	else
	{
		assert(false);
	}
}

void Camera::getProjectionMatrix(mat4& m) const
{
	cml::matrix_perspective_yfov_RH(m, cml::rad(m_fovy), m_aspect, m_nearz, m_farz, cml::z_clip_neg_one);
}

////////////////////////////////////////////////////////////////////////////////

void lerp(const Camera& cam0, const Camera& cam1, double t, Camera& cam)
{
	assert(cam0.getMode() == cam1.getMode());

	if (cam0.isModeOrbital())
	{
		// TODO handle aspect ratio etc. (projection matrix ugh)

		cam.setOrbitalRotation(cml::slerp(cam0.getOrbitalRotation(), cam1.getOrbitalRotation(), t));
		cam.setOrbitalAltitude(cml::lerp(cam0.getOrbitalAltitude(), cam1.getOrbitalAltitude(), t));
		cam.setOrbitalTilt(cml::lerp(cam0.getOrbitalTilt(), cam1.getOrbitalTilt(), t));
		cam.setOrbitalRange(cml::lerp(cam0.getOrbitalRange(), cam1.getOrbitalRange(), t));
	}
	else
	{
		assert(false);
	}
}

/*!
\param cam	The camera to get the point of view from.
\param ndx	Normalized device coordinate x (from -1 to 1).
\param ndy	Normalized device coordinate y (from -1 to 1).
\return The intersection point, or the origin if no intersection.
*/
vec3 projectPointToUnitSphere(const Camera& cam, double ndx, double ndy)
{
	vec3 result;

	// TODO use cml when it has support for only getting viewport coords
	// TODO or use cml picking or projection directly
	double planes[6][4];
	cam.calcFrustumPlanes(planes);
	vec3 corners[4];
//	cml::get_frustum_corners(planes, corners);
	corners[0] = cml::detail::intersect_planes(planes[0], planes[2], planes[4]);
    corners[1] = cml::detail::intersect_planes(planes[1], planes[2], planes[4]);
    corners[2] = cml::detail::intersect_planes(planes[1], planes[3], planes[4]);
    corners[3] = cml::detail::intersect_planes(planes[0], planes[3], planes[4]);

	vec3 look = cml::bilerp(corners[0], corners[1], corners[3], corners[2], (ndx+1)/2, (ndy+1)/2);

	vec3 eye = cam.getEye();
	vec3 d = look - eye;
	d.normalize();

	double t = intersectRayWithUnitSphere(eye, d);

	if (t == std::numeric_limits<double>::infinity() || t < 0)
	{
		result.zero();
	}
	else
	{
		result = eye + t * d;
		result.normalize();
	}

	return result;
}

//! Convert Camera into Camera settings. the LatLon are in WGS84 datum.
void camToSettings(const Camera& cam, double& lat, double& lon, double& hdg, double& alt, double& tlt, double& rng)
{
	// Convert quaternion to euler angles
	double a0, a1, a2;
	quat e;
	cml::quaternion_rotation_euler(e, -cml::constantsd::pi_over_2(), -cml::constantsd::pi_over_2(), 0.0, cml::EulerOrder::euler_order_xyz);
	quat c = cam.getOrbitalRotation();
	quat q = cml::quaternion_rotation_difference(e, c);
	cml::quaternion_to_euler(q, a0, a1, a2, cml::EulerOrder::euler_order_yxz);

	// Convert geocentric radians to geodetic (WGS84) degrees
	CoordLatLon llgc(a1, -a0);
	CoordLatLon llgd = WGS84::getInstance()->toDatum(llgc);

	lon = cml::deg(llgd.lon());
	lat = cml::deg(llgd.lat());
	hdg = cml::deg(a2 < 0 ? a2 + cml::constantsd::two_pi() : a2);

	alt = cam.getOrbitalAltitude();
	tlt = cam.getOrbitalTilt();
	rng = cam.getOrbitalRange();
}

//! Creaate a Camera from Camera settings. the LatLon are in WGS84 datum.
void camFromSettings(Camera& cam, double lat, double lon, double hdg, double alt, double tlt, double rng)
{
	// Convert geodetic (WGS84) degrees to geocentric radians
	CoordLatLon llgd(cml::rad(lat), cml::rad(lon));
	CoordLatLon llgc = WGS84::getInstance()->toGeocentric(llgd);
	double a0 = -llgc.lon();
	double a1 = llgc.lat();
	double a2 = cml::rad(hdg);

	// Convert euler angles to quaternion
	quat q;
	cml::quaternion_rotation_euler(q, a0, a1, a2, cml::EulerOrder::euler_order_yxz);
	quat e;
	cml::quaternion_rotation_euler(e, -cml::constantsd::pi_over_2(), -cml::constantsd::pi_over_2(), 0.0, cml::EulerOrder::euler_order_xyz);
	q *= e;

	cam.setOrbitalRotation(q);
	cam.setOrbitalAltitude(alt);
	cam.setOrbitalTilt(tlt);
	cam.setOrbitalRange(rng);
}

void camToCookieStr(const Camera& cam, std::string& str)
{
	char buf[256];
	double lat, lon, hdg, alt, tlt, rng = 0;
	camToSettings(cam, lat, lon, hdg, alt, tlt, rng);
	sprintf_s(buf, sizeof(buf), "%.17g %.17g %.17g %.17g %.17g %.17g", lat, lon, hdg, alt, tlt, rng);
	str = buf;
}

void camFromCookieStr(Camera& cam, const std::string& str)
{
	double lat, lon, hdg, alt, tlt, rng = 0;
	int n = sscanf_s(str.c_str(), "%lf %lf %lf %lf %lf %lf", &lat, &lon, &hdg, &alt, &tlt, &rng);
	assert(n == 6);
	camFromSettings(cam, lat, lon, hdg, alt, tlt, rng);
}



///////////////////////////////////////////////////////////////////////////////////
// CameraFrustum class
///////////////////////////////////////////////////////////////////////////////////

const double CameraFrustum::kfFrustumFactor = 1.17;

CameraFrustum::CameraFrustum(const Camera & camera) : m_eye(camera.getEye()),m_look(camera.getLook()),m_fovy(camera.getFovy())
{
	//request Frustum planes from camera
	camera.calcFrustumPlanes(m_planes);

	//create easy access array
	for(int i=0;i<6;++i)
	{
		m_planes_normals[i] = vec3_alias(m_planes[i]);
		m_planes_d[i] = m_planes[i][3];
	}
}

CameraFrustum::~CameraFrustum()
{
}

CameraFrustum::CameraFrustumBoundary CameraFrustum::classifyPoint(const vec3 & point,const double & radius)
{
	CameraFrustumBoundary nClass = knClassInside;

	for (int nPlane = 0; nPlane != 6; ++nPlane)
	{		
		double dist = cml::dot(m_planes_normals[nPlane], point) + m_planes_d[nPlane];

		if (dist < -radius)
		{
			return knClassOutside;
		}
		else if (dist <= radius)
		{
			nClass = knClassBoundary;
		}
	}

	return nClass;
}

CameraFrustum::CameraFrustumBoundary CameraFrustum::classifyIndex(const PYXIcosIndex & index)
{
	assert(2 <= index.getResolution()); // not sure exactly what the lower limit is but 2 sounds good
	
	vec3 p(CmlConvertor::toVec3(index));

	// TODO this calculation can probably be hoisted when we need to deal with
	// vertex children differently at a higher level of the algorithm (i.e. the caller)
	// TODO switched to just a cell (not a tile)
	// TODO move this into pyxlib
	int nRes = index.getResolution();
	double cellradius = PYXMath::calcCircumRadius(index.hasVertexChildren() ? nRes - 1 : nRes) * Icosahedron::kfCentralAngle * kfFrustumFactor;
	//double cellradius = PYXMath::calcCircumRadius(nRes) * Icosahedron::kfCentralAngle * kfFrustumFactor;

	return classifyPoint(p,cellradius);
}

bool CameraFrustum::isPointOutside(const vec3 & point,const double & radius)
{
	for (int nPlane = 0; nPlane != 6; ++nPlane)
	{
		// Plane normal.
		vec3_alias n(m_planes[nPlane]);

		// Distance to plane.
		double dist = cml::dot(n, point) + m_planes[nPlane][3];

		if (dist < -radius)
		{
			return true;
		}
	}

	return false;
}

bool CameraFrustum::isIndexOutside(const PYXIcosIndex & index)
{
	assert(2 <= index.getResolution()); // not sure exactly what the lower limit is but 2 sounds good

	vec3 p(CmlConvertor::toVec3(index));

	// TODO this calculation can probably be hoisted when we need to deal with
	// vertex children differently at a higher level of the algorithm (i.e. the caller)
	// TODO move this into pyxlib
	// TODO switched to just a cell (not a tile)
	int nRes = index.getResolution();
	//double cellradius = PYXMath::calcCircumRadius(index.hasVertexChildren() ? nRes - 1 : nRes) * Icosahedron::kfCentralAngle * kfFrustumFactor;
	double cellradius = PYXMath::calcCircumRadius(nRes) * Icosahedron::kfCentralAngle * kfFrustumFactor;

	return isPointOutside(p,cellradius);
}

double CameraFrustum::getSizeOnScreen(const int viewPortHeight,const vec3 & point,const double & radius)
{
	double dist = (m_eye - point).length();		

	double angle = atan2(radius,dist) * 180 / MathUtils::kfPI;
	
	//under the assumpation the point on the earth. check the direction of the camera and normal of the point
	vec3 normal(point);
	normal.normalize();

	vec3 pointLook(m_eye-point);
	pointLook.normalize();
	
	double normalAngleCos = abs(dot(normal,pointLook));

	//soften this calculation because it is to harsh...
	//make it sqrt...
	normalAngleCos = sqrt(normalAngleCos)*0.8+0.2;
	
	return viewPortHeight / m_fovy * angle * normalAngleCos ;
}


#pragma warning(pop)