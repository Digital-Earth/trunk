#pragma once
#ifndef VIEW_MODEL__CAMERA_H
#define VIEW_MODEL__CAMERA_H
/******************************************************************************
camera.h

begin		: 2007-07-27
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"

// view model includes
#include "cml_utils.h"

#include "pyxis/derm/index.h"
#include "pyxis/utility/sphere_math.h"

// standard includes
#include <string>

/*!
Camera object is implemented only for Orbital mode at this time.

The Camera model is defined as follows:
1.	Sphere Point - a point on the unit-sphere (LatLon + Heading angle).
2.	Altitude - height we add to the "Sphere Point" - which become the camera “Target point”
3.	Tilt & Range - tilt the target point and move backward to the right range of the camera.

Note that the Altitude and Range values are in meters, and Tilt is in degrees

In OpenGL, the camera has x right, y up, and is looking down negative z.


TODO:
- Should change the angle representations from degrees to radians.
- Should change the distance representations from metres to raw units (earth radius, or metre?).

*/
//! A camera.
class VIEW_MODEL_API Camera
{
public:

	enum
	{
		knModeOrbital = 1
	};

public:

	Camera() :
		m_fovy(39), // 38 is too "close", 39 is slightly too "far", but OK for now
		m_aspect(1.0),
		m_nearz(0.000001),
		m_farz(11),
		m_altitude(0),
		m_tilt(0),
		m_range(SphereMath::knEarthRadius*2),
		m_nMode(1)
	{
		/*
		m_eye[0] = 0;
		m_eye[1] = 0;
		m_eye[2] = 0;
		*/
		m_center[0] = 0;
		m_center[1] = 0;
		m_center[2] = -1;
		/*
		m_up[0] = 0;
		m_up[1] = 1;
		m_up[2] = 0;
		*/

		// TODO need proper "default position"
		// NOTE default no-rotation quaternion is (0,0,0,1)
		vec3 axis(0, 0, 0);
		cml::quaternion_rotation_axis_angle(m_rot, axis, 0.0); // cml::constantsd::pi() * 2/3
	}

public:

	std::string toString() const;

	std::string getPerspectiveString() const;
	std::string getEyeString() const;
	std::string getCenterString() const;
	std::string getUpString() const;

public:

	double getFovy() const
	{
		return m_fovy;
	}

	double getAspect() const
	{
		return m_aspect;
	}

	double getNearz() const
	{
		return m_nearz;
	}

	double getFarz() const
	{
		return m_farz;
	}

#ifndef SWIG //swig ignore

	/*void getEye(double* eye) const
	{
		eye[0] = m_eye[0];
		eye[1] = m_eye[1];
		eye[2] = m_eye[2];
	}*/

	void getCenter(double* center) const
	{
		center[0] = m_center[0];
		center[1] = m_center[1];
		center[2] = m_center[2];
	}

	/*
	void getUp(double* up) const
	{
		up[0] = m_up[0];
		up[1] = m_up[1];
		up[2] = m_up[2];
	}*/

	vec3 getCenter() const;
	vec3 getEye() const;
	vec3 getUp() const;
	vec3 getLook() const;
#endif

	// Heading is in degrees for now, probably should be rad
	double getHeading() const;

	void setHeading(double hdg);

public:

	void setFovy(double fovy)
	{
		m_fovy = fovy;
	}

	void setAspect(double aspect)
	{
		m_aspect = aspect;
	}

	void setNearz(double nearz)
	{
		m_nearz = nearz;
	}

	void setFarz(double farz)
	{
		m_farz = farz;
	}

	/*void setEye(double* eye)
	{
		m_eye[0] = eye[0];
		m_eye[1] = eye[1];
		m_eye[2] = eye[2];
	}*/

#ifndef SWIG //swig ignore
	void setCenter(double* center)
	{
		m_center[0] = center[0];
		m_center[1] = center[1];
		m_center[2] = center[2];
	}
#endif

	/*
	void setUp(double* up)
	{
		m_up[0] = up[0];
		m_up[1] = up[1];
		m_up[2] = up[2];
	}*/

public:

	/*
	void dolly(double dist);

	void azimuth(double deg);

	void zenith(double deg);

	void orbit(double degazi, double degzen);

	void pitch(double deg);
	void yaw(double deg);
	void roll(double deg);

	void rotate(double deg);
	*/

public:

	int getMode() const
	{
		return m_nMode;
	}

	void setMode(int nMode)
	{
		m_nMode = nMode;
	}

	bool isModeOrbital() const
	{
		return m_nMode == knModeOrbital;
	}

public:

#ifndef SWIG //swig ignore
	void calcFrustumPlanes(double planes[6][4]) const;
#endif

public:

	double getOrbitalAltitude() const
	{
		return m_altitude;
	}

	double getOribitalEyeAltitiude() const
	{		
		return (getEye().length() - 1) * SphereMath::knEarthRadius;
	}

	double getOribitalUnitAltitude() const;	

	double getOrbitalTilt() const
	{
		return m_tilt;
	}

	double getOrbitalRange() const
	{
		return m_range;
	}

	//! Get PYXIcosIndex on a UnitSphere that the camera is targeting
	PYXIcosIndex getOrbitalTargetIndex(int nResolution) const;

	//! Get the CoordLatLon on a UnitSphere that the camera is targeting. LatLon are in geocentric (not WGS84)
	CoordLatLon getOrbitalTargetLatLon() const;

	//! Get PYXIcosIndex on a UnitSphere below the camera eye
	PYXIcosIndex getOrbitalEyeIndex(int nResolution) const;

	//! Get CoordLatLon on a UnitSphere below the camera eye. LatLon are in geocentric (not WGS84)
	CoordLatLon getOrbitalEyeLatLon() const;

	void setOrbitalAltitude(double altitude)
	{
		m_altitude = altitude;
	}

	//!Adjust Altitude and Range to keep camera eye at the same elevation
	void adjustOrbitalAltitudeAndRange(double newAlt,double minRange);

	void setOrbitalTilt(double tilt)
	{
		m_tilt = tilt;
	}

	void setOrbitalRange(double range)
	{
		m_range = range;
	}

#ifndef SWIG //swig ingnore
	// TEMP
	quat getOrbitalRotation() const
	{
		return m_rot;
	}
	void setOrbitalRotation(const quat& rot)
	{
		m_rot = rot;
	}
#endif

public:

#ifndef SWIG //swig ingnore
	void getModelViewMatrix(mat4& m) const;

	void getProjectionMatrix(mat4& m) const;
#endif

public:

	// TODO need to be able to set camera from euler angles (lon(z) lat(y) hdg(x)), alt/tlt/rng, etc.

private:

	void recalcUp();

private:

	// Projection.
	double m_fovy;
	double m_aspect;
	double m_nearz;
	double m_farz;

	// TODO um use a union?

	// Modelview.
	//double m_eye[3];
	vec3 m_center;
	//double m_up[3];

	// Orbital representation
	quat m_rot;
	double m_altitude;
	double m_tilt;
	double m_range;

	// Mode 0 is eye/center/up vector.
	// Mode 1 is orbital: quaternion rotation/orientation, altitude, tilt, and range.
	int m_nMode;

};

/*!
CameraFrustum - Helper class to check if a point/sphere is inisde the camera Frustum

Note, the CameraFrustum accept two input types:
1. PYXIcosIndex - that is been transform into a point or a sphere containing the specified cell
2. vec3 - a unit-sphere coordinate location. if a radius is specified it's in raw-units (not meters)

*/
//! CameraFrustum - Helper class to check if a point/sphere is inisde the camera Frustum
class VIEW_MODEL_API CameraFrustum
{
public:
	static const double kfFrustumFactor;

	enum CameraFrustumBoundary
	{
		knClassOutside = 0,
		knClassBoundary = 1,
		knClassInside = 2	
	};

protected:
	double m_planes[6][4];
	vec3 m_planes_normals[6];
	double m_planes_d[6];

	vec3 m_look;
	vec3 m_eye;
	double m_fovy;

public:
	CameraFrustum(const Camera & camera);
	virtual ~CameraFrustum();

	CameraFrustumBoundary classifyPoint(const vec3 & point,const double & radius);
	CameraFrustumBoundary classifyIndex(const PYXIcosIndex & index);

	bool isPointOutside(const vec3 & point,const double & radius);
	bool isIndexOutside(const PYXIcosIndex & index);

	inline bool isPointVisible(const vec3 & point,const double & radius) 
	{
		return !isPointOutside(point,radius);
	}

	inline bool isIndexVisible(const PYXIcosIndex & index)
	{
		return !isIndexOutside(index);
	}

	double getSizeOnScreen(const int viewPortHeight,const vec3 & point,const double & radius);
};


VIEW_MODEL_API void lerp(const Camera& cam0, const Camera& cam1, double t, Camera& cam);

VIEW_MODEL_API vec3 projectPointToUnitSphere(const Camera& cam, double ndx, double ndy);

//! Convert Camera into Camera settings. the LatLon are in WGS84 datum.
VIEW_MODEL_API void camToSettings(const Camera& cam, double& lat, double& lon, double& hdg, double& alt, double& tlt, double& rng);

//! Creaate a Camera from Camera settings. the LatLon are in WGS84 datum.
VIEW_MODEL_API void camFromSettings(Camera& cam, double lat, double lon, double hdg, double alt, double tlt, double rng);

#ifndef SWIG
VIEW_MODEL_API void camToCookieStr(const Camera& cam, std::string& str);
#endif

VIEW_MODEL_API void camFromCookieStr(Camera& cam, const std::string& str);

#endif
