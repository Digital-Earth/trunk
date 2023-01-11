#pragma once
#ifndef VIEW_MODEL__CAMERA_ANIMATOR_H
#define VIEW_MODEL__CAMERA_ANIMATOR_H
/******************************************************************************
camera_animator.h

begin		: 2010-01-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "camera.h"
#include "animation.h"
#include "cml_utils.h"

class ViewOpenGLThread;

class CameraAnimator : public PYXObject
{
public:
	CameraAnimator() : m_animation(),m_context(NULL),m_usingTerrainModel(false) { setDefaultSettings(); };
	CameraAnimator(ViewOpenGLThread & context) : m_animation(),m_context(&context),m_usingTerrainModel(true) { setDefaultSettings(); };

	virtual ~CameraAnimator() {};

protected:
	ViewOpenGLThread * m_context;

public:
	bool hasContext() const { return m_context != NULL; }
	ViewOpenGLThread & getContext() { return *m_context; }
	void setContext(ViewOpenGLThread & context) { m_context = &context; }

protected:
	//! Current camera.
	Camera m_cam;

	//! Last valid camera. this is used to recover from invalid animations/setCamera.
	Camera m_lastValidCamera;

	//! Start and end Camera position for animation.
	Camera m_camFrom;
	Camera m_camTo;	

	double m_middleRange;
	double m_middleRangeFactor;

	vec3   m_pointerLoction;
	bool   m_zoomToPointer;

	//! used as cache we no new data 
	double m_curEyeElevation;

	void animateCamera();
	void adjustCameraToPointer();
	bool forceCameraAboveTerrain();
	void tweakCameraNearFarPlane();

	void decayRotation();

	void validateCamera();
	
public:
	Camera & getCamera() { return m_cam; }
	const Camera & getCamera() const { return m_cam; }

	//! set the camera position without animating
	void setCamera(const Camera & camera);

	//! make camera animation to the right frame
	bool updateCamera();

protected:
	PYXPointer<LinearWithEasingAnimation<float>> m_animation;	

	bool m_cameraHasChanged;

	quat m_rotation;
	double m_stopDecayFactor;
	bool m_isDrifting;

	double m_zoomFactorDrifting;
	bool m_isZoomDrifting;

	bool m_usingTerrainModel;
	//! fast animation time in milliseconds.
	int m_fastAnimationTime; 
	//! long animation time in milliseconds.
	int m_longAnimationTime;


	//! used to force camera not to change altitude
	bool m_lockCameraAltitude;
	double m_lockedAltitude;

	//! start to animate
	void startAnimation(int timeInMilliseconds);

public:
	bool isMoving() const { return isAnimating() || isDrifting() || isZoomDrifting(); };
	void stop();

	bool isAnimating() const { return m_animation; }
	void stopAnimation();

	bool isDrifting() const { return m_isDrifting; }
	void stopDrifting() { m_isDrifting = false; }

	bool isZoomDrifting() const { return m_isZoomDrifting; }
	void stopZoomDrifting() { m_isZoomDrifting = false; }

	const bool & isUsingTerrainModel() const { return m_usingTerrainModel; }
	void setUsingTerrainModel(const bool & use) { m_usingTerrainModel = use; }	

	const bool & getLockCameraAltitude() const { return m_lockCameraAltitude; }
	void setLockCameraAltitude(const bool & lock) { m_lockCameraAltitude = lock; m_lockedAltitude = m_cam.getOrbitalAltitude(); }	

	const int & getFastAnimationTime() const { return m_fastAnimationTime; };
	const int & getLongAnimationTime() const { return m_longAnimationTime; };
	void setFastAnimationTime(const int & time) { m_fastAnimationTime = time; };
	void setLongAnimationTime(const int & time) { m_longAnimationTime = time; };


protected:
	double m_cameraMinRange;
	double m_cameraMaxRange;

	double m_cameraMinTilt;
	double m_cameraMaxTilt;

	double m_cameraRotateSpeed;
	double m_cameraTiltSpeed;

	double m_cameraZoomSomeFactor;

	PYXPointer<FeatureIterator> m_tripIterator;
	
public: 
	const double & getCameraMinRange() const { return m_cameraMinRange; }
	const double & getCameraMaxRange() const { return m_cameraMaxRange; }

	const double & getCameraMinTilt() const { return m_cameraMinTilt; }
	const double & getCameraMaxTilt() const { return m_cameraMaxTilt; }

	const double & getCameraRotateSpeed() const { return m_cameraRotateSpeed; }
	const double & getCameraTiltSpeed() const { return m_cameraTiltSpeed; }

	const double & getCameraZoomSomeFactor() const { return m_cameraZoomSomeFactor; }

public:
	void setDefaultSettings();

public:
	double getCurrentEyeElevation() const { return m_curEyeElevation; };
	double calcAppropriateRange(const Camera& cam, int nHeight, double metres) const;
	double getZoomLogScale() const;

	void setAspect(double aspect);

public:
	void zoomInSome();
	void zoomOutSome();
	void zoomInSomePointer();
	void zoomOutSomePointer();

	//! zoom to a log scale between camera minRange and camera maxRange. if zoomLogScale=0.0 - zoom to minRange. if zoomLogScale=1.0 - zoom to maxRange
	void zoomToLogScale(const double & zoomLogScale);

	void orbit(double azi, double zen);
	void tilt(double deg);
	void rotate(double deg);
	void tiltAndRotate(double tiltDeg,double rotateDeg);

	void goToCamera(const Camera& cam,int timeInMilliseconds);
	void goToCamera(const Camera& cam);
	void goToIndex(const PYXIcosIndex & index,int timeInMilliseconds);
	void goToIndex(const PYXIcosIndex & index);

	void rotate(const quat & rotation);
	void drift(const quat & rotation);

	void zoomDrift(const double & zoomFactor);

	void tripAnimation(const PYXPointer<FeatureIterator> & features);

	void startTripStep();

protected:
	Notifier m_cameraChangeNotifier;

public:
	Notifier & getCameraChangeNotifier() { return m_cameraChangeNotifier; }
};


class CameraChangeEvent : public NotifierEvent
{
public:
	explicit CameraChangeEvent(CameraAnimator & animator) : m_animator(animator) {}
	static PYXPointer<CameraChangeEvent> create(CameraAnimator & animator) { return PYXNEW(CameraChangeEvent,animator); }

	virtual ~CameraChangeEvent() {}

protected:
	CameraAnimator & m_animator;

public:
	CameraAnimator & getAnimator() { return m_animator; }
	Camera & getCamera() { return m_animator.getCamera(); }
};

#endif
