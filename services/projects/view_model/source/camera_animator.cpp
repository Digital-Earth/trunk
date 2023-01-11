/******************************************************************************
camera_animator.cpp

begin		: 2010-01-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "camera_animator.h"
#include "STile.h"
#include "view_open_gl_thread.h"
#include "gl_utils.h"

#include <boost/math/special_functions/fpclassify.hpp>

#include "pyxis/utility/sphere_math.h"
#include "pyxis/utility/great_circle_arc.h"

#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/wgs84.h"

void CameraAnimator::setDefaultSettings()
{
	if (m_context != NULL)
	{
		m_usingTerrainModel = true;
	}

	m_curEyeElevation = SphereMath::knEarthRadius;

	//stop animation and drifting
	m_animation.reset();	
	m_zoomToPointer = false;
	m_lockCameraAltitude = false;

	m_fastAnimationTime = 330;
	m_longAnimationTime = 10000;

	m_cameraMinRange = 10;
	m_cameraMaxRange = 31855035; // want 63710070 but far plane clips

	m_cameraMinTilt = 0;
	m_cameraMaxTilt = 80;

	m_cameraRotateSpeed = 5;
	m_cameraTiltSpeed = 5;

	m_cameraZoomSomeFactor = 3/4.0;
}

void CameraAnimator::setCamera(const Camera & camera)
{
	//stop animation and drifting
	stop();
	
	//set camera
	m_cam = camera;

	//validate the new camera range and tilt are in the allowed ranges
	m_cam.setOrbitalRange(cml::clamp(m_cam.getOrbitalRange(),getCameraMinRange(),getCameraMaxRange()));
	m_cam.setOrbitalTilt(cml::clamp(m_cam.getOrbitalTilt(),getCameraMinTilt(),getCameraMaxTilt()));

	//We need to make sure the new camera have right clipping planes
	//to avoid clipping the earth in middle.
	tweakCameraNearFarPlane();

	m_cameraHasChanged = true;
}

void CameraAnimator::setAspect(double aspect)
{
	m_cam.setAspect(aspect);
	m_camFrom.setAspect(aspect);
	m_camTo.setAspect(aspect);
}

void CameraAnimator::startAnimation(int timeInMilliseconds)
{
	m_isDrifting = false;
	m_isZoomDrifting = false;

	//make a nice transition of camera movements...
	GreatCircleArc arc(m_camFrom.getOrbitalTargetLatLon(),m_camTo.getOrbitalTargetLatLon());

	double movingDistanceRadians = arc.getDistance() * SphereMath::knEarthRadius;
	double fromRange = m_camFrom.getOrbitalRange();
	double toRange = m_camTo.getOrbitalRange();
	double linearMiddleRange = (fromRange+toRange)/2;
	
	//if the distance that is camera is moving is larger then the range,
	//we make it zoom out in the middle
	m_middleRange = std::max(linearMiddleRange,movingDistanceRadians/2);
	
	//check if we need some smoothing on the way...
	m_middleRangeFactor = cml::clamp(1-abs(m_middleRange-linearMiddleRange)/linearMiddleRange,0.5,1.0);
	double easing = cml::clamp(m_middleRange/linearMiddleRange,1.0,2.0);

	//make sure slow animation go slow...
	if (timeInMilliseconds > 2000)
	{
		easing = 3.0;
	}

	m_animation = LinearWithEasingAnimation<float>::create(0,1,timeInMilliseconds,easing);
}

void CameraAnimator::stopAnimation()
{
	stop();	
}

void CameraAnimator::stop()
{
	m_tripIterator.reset();
	m_animation.reset();
	m_isDrifting = false;
	m_isZoomDrifting = false;
}

//! make camera animation to the right frame
bool CameraAnimator::updateCamera()
{
	bool wasChange = m_cameraHasChanged;

	if (isAnimating())
	{
		animateCamera();
		wasChange = true;
	}

	if (isDrifting())
	{
		rotate(m_rotation);
		wasChange = true;
		decayRotation();
	}

	if (isZoomDrifting())
	{
		m_cam.setOrbitalRange(cml::clamp((m_cam.getOrbitalRange() * m_zoomFactorDrifting), getCameraMinRange(), getCameraMaxRange()));
		wasChange = true;
	}

	if (hasContext() && isUsingTerrainModel())
	{
		if (forceCameraAboveTerrain())
		{
			wasChange = true;
		}
	}

	if (wasChange)
	{
		validateCamera();

		tweakCameraNearFarPlane();

		m_cameraChangeNotifier.notify(CameraChangeEvent::create(*this));
	}

	m_cameraHasChanged = false;
	return wasChange;
}

void CameraAnimator::validateCamera()
{
	auto latLon = m_cam.getOrbitalTargetLatLon();
	auto range = m_cam.getOrbitalRange();
	auto heading = m_cam.getHeading();
	auto tilt = m_cam.getOrbitalTilt();
	auto altitude = m_cam.getOrbitalAltitude();
	
	//check if camera settings are valid
	if (boost::math::isfinite(latLon.lat()) && boost::math::isfinite(latLon.lon()) &&
		boost::math::isfinite(range) && boost::math::isfinite(heading) && 
		boost::math::isfinite(tilt) && boost::math::isfinite(tilt))
	{
		//update last valid state
		m_lastValidCamera = m_cam;
	}
	else 
	{
		//add message to log
		std::string camString;
		camToCookieStr(m_cam,camString);
		TRACE_INFO("camera become invalid: " << camString); 

		//break on debug mode
		assert(false && "camera become invalid");
		
		//revert to last valid state
		m_cam = m_lastValidCamera;
		
	}
}

void CameraAnimator::decayRotation()
{
	vec3 axis;
	double angle;

	cml::quaternion_to_axis_angle(m_rotation,axis,angle);	
	if (angle > m_stopDecayFactor)
	{
		cml::quaternion_rotation_axis_angle(m_rotation,axis,angle*0.95);
	}
	else	
	{
		m_isDrifting = false;
	}
}


void CameraAnimator::animateCamera()
{
	assert(m_animation);
	double animValue = m_animation->getValue();

	lerp(m_camFrom,m_camTo,animValue ,m_cam);

	if (animValue < 0.5)
	{
		m_cam.setOrbitalRange(cml::lerp(m_camFrom.getOrbitalRange(),m_middleRange,pow(animValue*2,m_middleRangeFactor)));
	}
	else
	{
		m_cam.setOrbitalRange(cml::lerp(m_camTo.getOrbitalRange(),m_middleRange,pow((1-animValue)*2,m_middleRangeFactor)));
	}

	if (m_animation->isConstant())
	{
		if (m_tripIterator)
		{
			startTripStep();
		}
		else
		{
			std::string camString;
			camToCookieStr(m_cam,camString);
			TRACE_INFO("finish animation with camera : " << camString);
			m_animation.reset();
		}
	}

	if (m_zoomToPointer)
	{
		adjustCameraToPointer();
		m_zoomToPointer = true;
	}
}

void CameraAnimator::adjustCameraToPointer()
{
	vec3 newLocation = getContext().getPointerLocationOnUnitShpere();	
	
	quat rotation;	
	cml::quaternion_rotation_vec_to_vec(rotation, m_pointerLoction, newLocation, true);

	//m_pointerLoction = newLocation;

	rotate(rotation);
}

bool CameraAnimator::forceCameraAboveTerrain()
{
	int curResolution = getContext().getResolution();

	const double minEyeElevation = m_cam.getOrbitalRange()*0.1; //in Meteres -  * 0.1 ~= cos(85 degrees) which is the maximum tilt
	double smoothFactor = 1;

	const double SmoothMinResolution = 5;
	const double SmoothMaxResolution = 28-9;

	double eyeTerrainAltitude = getContext().getMeshElevation(m_cam.getOrbitalEyeLatLon());
	double eyeAltitude = (m_cam.getEye().length()-1)*SphereMath::knEarthRadius;
	m_curEyeElevation = eyeAltitude - eyeTerrainAltitude;

	if (curResolution > SmoothMinResolution )
	{
		if (curResolution < SmoothMaxResolution  )
		{
			smoothFactor = (curResolution - SmoothMinResolution) / ( SmoothMaxResolution - SmoothMinResolution);
		}
		double targetTerrainAltitude = getContext().getMeshElevation(m_cam.getOrbitalTargetLatLon());
		double curAltitude = m_cam.getOrbitalAltitude();
		double newAltitude;

		if (m_lockCameraAltitude)
		{
			newAltitude = m_lockedAltitude;
		}
		else
		{
			newAltitude = targetTerrainAltitude * smoothFactor;
					
			//interpolate between current altitude and wanted altitude
			newAltitude = (curAltitude*3 + newAltitude)/4;
		}

		bool eyeProblem = false;
		
		if (eyeAltitude < eyeTerrainAltitude + minEyeElevation)
		{
			double eyeAltitudeDelta = (eyeTerrainAltitude+minEyeElevation + eyeAltitude)/2 - eyeAltitude;
			
			newAltitude = m_cam.getOrbitalAltitude() + eyeAltitudeDelta;
			eyeProblem = true;
		}
		//check if the newAltitude would bring the camera eye below surface
		else if ( eyeAltitude + (newAltitude-curAltitude) < eyeTerrainAltitude + minEyeElevation)
		{
			//if so, change it to be the in the minimum eye elevation
			newAltitude = curAltitude - m_curEyeElevation + minEyeElevation;

			//and do a nice smoothing
			newAltitude = (curAltitude*3 + newAltitude)/4;

			eyeProblem = true;
		}

		if (eyeProblem || m_curEyeElevation < 500 ) //if we are under the surface or we are in "Local mode" - don't change range
		{
			//just move the camera up
			m_cam.setOrbitalAltitude( newAltitude );
			return true;
		} 
		else if (!isDrifting())
		{
			double oldRange = m_cam.getOrbitalRange();

			//adjust range and altitude
			if (abs(newAltitude-curAltitude)<m_curEyeElevation/50)
			{
				return false;
			}
			m_cam.setOrbitalAltitude( newAltitude );
			//m_cam.adjustOrbitalAltitudeAndRange( newAltitude , m_cameraMinRange );
			
			return true;
		}
	}
	return false;
}

void CameraAnimator::tweakCameraNearFarPlane()
{
	double eyeAlt = m_cam.getEye().length() - 1; // eye alt in units
	double eyeElevation = getCurrentEyeElevation() / SphereMath::knEarthRadius;

	eyeAlt = std::max(eyeAlt,eyeElevation);

	// Draw distance factor
	const int F = 4;

	// Set near plane to 0.1 eye alt so we can zoom in quite far
	m_cam.setNearz(eyeElevation * 0.1);

	double distanceToHorizon = sqrt((1+eyeAlt)*(1+eyeAlt)-1);
	m_cam.setFarz(distanceToHorizon);
}

void CameraAnimator::zoomInSome()
{
	m_camFrom = m_cam;
	m_camTo = m_cam;
	m_camTo.setOrbitalRange(cml::clamp((m_camTo.getOrbitalRange() * getCameraZoomSomeFactor()), getCameraMinRange(), getCameraMaxRange()));
	m_zoomToPointer = false;

	startAnimation(m_fastAnimationTime);
}

void CameraAnimator::zoomOutSome()
{
	m_camFrom = m_cam;
	m_camTo = m_cam;
	m_camTo.setOrbitalRange(cml::clamp((m_camTo.getOrbitalRange() / getCameraZoomSomeFactor()), getCameraMinRange(), getCameraMaxRange()));
	m_zoomToPointer = false;

	startAnimation(m_fastAnimationTime);
}

void CameraAnimator::zoomInSomePointer()
{	
	m_camFrom = m_cam;
	m_camTo = m_cam;
	m_camTo.setOrbitalRange(cml::clamp((m_camTo.getOrbitalRange() * getCameraZoomSomeFactor()), getCameraMinRange(), getCameraMaxRange()));
	m_zoomToPointer = true;

	m_pointerLoction = getContext().getPointerLocationOnUnitShpere();	

	startAnimation(m_fastAnimationTime);
}

void CameraAnimator::zoomOutSomePointer()
{
	m_camFrom = m_cam;
	m_camTo = m_cam;
	m_camTo.setOrbitalRange(cml::clamp((m_camTo.getOrbitalRange() / getCameraZoomSomeFactor()), getCameraMinRange(), getCameraMaxRange()));
	m_zoomToPointer = true;

	m_pointerLoction = getContext().getPointerLocationOnUnitShpere();	

	startAnimation(m_fastAnimationTime);
}

void CameraAnimator::zoomToLogScale(const double & zoomLogScale)
{
	m_camFrom = m_cam;
	m_camTo = m_cam;
	
	//convert from log scale to real scale
	double minLog = log10(getCameraMinRange());
	double maxLog = log10(getCameraMaxRange());
	double pos   = (maxLog-minLog)*zoomLogScale+minLog;
	double range = pow(10,pos);
	
	m_camTo.setOrbitalRange(range);
	m_zoomToPointer = false;

	startAnimation(m_fastAnimationTime);
}

void CameraAnimator::orbit(double azi, double zen)
{
	m_camFrom = m_cam;
	m_camTo = m_cam;
	quat rot = m_camFrom.getOrbitalRotation();
	cml::quaternion_rotate_about_world_y(rot, cml::rad(azi));
	cml::quaternion_rotate_about_world_x(rot, cml::rad(zen));
	m_camTo.setOrbitalRotation(rot);
	m_zoomToPointer = false;

	startAnimation(m_fastAnimationTime);
}

void CameraAnimator::tilt(double deg)
{
	m_camFrom = m_cam;
	m_camTo = m_cam;
	m_camTo.setOrbitalTilt(cml::clamp(m_camTo.getOrbitalTilt() + deg, getCameraMinTilt(), getCameraMaxTilt()));
	m_zoomToPointer = false;

	startAnimation(m_fastAnimationTime);
}

void CameraAnimator::rotate(double deg)
{
	m_camFrom = m_cam;
	m_camTo = m_cam;
	m_camTo.setHeading(m_camTo.getHeading() + deg);
	m_zoomToPointer = false;

	startAnimation(m_fastAnimationTime);
}

void CameraAnimator::tiltAndRotate(double tiltDeg,double rotateDeg)
{
	m_camFrom = m_cam;
	m_camTo = m_cam;
	m_camTo.setOrbitalTilt(cml::clamp(m_camTo.getOrbitalTilt() + tiltDeg, getCameraMinTilt(), getCameraMaxTilt()));
	m_camTo.setHeading(m_camTo.getHeading() + rotateDeg);
	m_zoomToPointer = false;

	startAnimation(m_fastAnimationTime);
}

void CameraAnimator::goToCamera(const Camera& cam)
{
	goToCamera(cam,m_longAnimationTime);
}

void CameraAnimator::goToCamera(const Camera& cam,int timeInMilliseconds)
{
	if (timeInMilliseconds > 0) 
	{
		m_zoomToPointer = false;

		m_camFrom = m_cam;
		m_camTo = cam;

		//validate the new camera range and tilt are in the allowed ranges
		m_camTo.setOrbitalRange(cml::clamp(m_camTo.getOrbitalRange(),getCameraMinRange(),getCameraMaxRange()));
		m_camTo.setOrbitalTilt(cml::clamp(m_camTo.getOrbitalTilt(),getCameraMinTilt(),getCameraMaxTilt()));

		// Adjust perspective etc.
		m_camTo.setFovy(m_camFrom.getFovy());
		m_camTo.setAspect(m_camFrom.getAspect());
		m_camTo.setNearz(m_camFrom.getNearz());
		m_camTo.setFarz(m_camFrom.getFarz());

		std::string camString;
		camToCookieStr(m_camFrom,camString);
		TRACE_INFO("Going from camera: " << camString);
		camToCookieStr(m_camTo,camString);
		TRACE_INFO("to camera: " << camString);
		
		startAnimation(timeInMilliseconds);
	}
	else 
	{
		setCamera(cam);
	}	
}

void CameraAnimator::rotate(const quat & rotation)
{
	m_zoomToPointer = false;
	quat rot = m_cam.getOrbitalRotation();	
	rot *= rotation;
	rot.normalize();
	m_cam.setOrbitalRotation(rot);
	m_cameraHasChanged = true;
}

void CameraAnimator::drift(const quat & rotation)
{
	stop();
	m_isDrifting = true;
	m_zoomToPointer = false;
	m_rotation = rotation;

	vec3 axis;
	cml::quaternion_to_axis_angle(m_rotation,axis,m_stopDecayFactor);
	m_stopDecayFactor /= 100;
}

void CameraAnimator::zoomDrift(const double & zoomFactor)
{
	stop();
	m_isZoomDrifting = true;
	m_zoomToPointer = false;
	m_zoomFactorDrifting = zoomFactor;
}

// Calculate an appropriate range for a distance of metres on the earth
double CameraAnimator::calcAppropriateRange(const Camera& cam, int nHeight, double metres) const
{
	// Pixel height at earth surface
	double earthrad = metres / SphereMath::knEarthRadius;

	// Pixel height in arcradians
	double pixelrad = cml::rad(cam.getFovy()/nHeight);

	// Distance in world coordinates
	double dist = earthrad / pixelrad;

	// Orbital range
	double rng = dist * SphereMath::knEarthRadius;

	return cml::clamp(rng, getCameraMinRange(), getCameraMaxRange());
}

double CameraAnimator::getZoomLogScale() const
{
	//convert to log scale 
	double minLog = log10(getCameraMinRange());
	double maxLog = log10(getCameraMaxRange());
	double rangeLog = log10(m_cam.getOrbitalRange());

	//find position in log scale
	return (rangeLog-minLog)/(maxLog-minLog);
}

void CameraAnimator::goToIndex(const PYXIcosIndex & index)
{
	goToIndex(index,m_longAnimationTime);
}

void CameraAnimator::goToIndex(const PYXIcosIndex & index,int timeInMilliseconds)
{
	m_tripIterator.reset();

	CoordLatLon llgc;
	SnyderProjection::getInstance()->pyxisToNative(index, &llgc);
	CoordLatLon llgd = WGS84::getInstance()->toDatum(llgc);

	double metres = SnyderProjection::getInstance()->resolutionToPrecision(index.getResolution()) * SphereMath::knEarthRadius;
	double rng = calcAppropriateRange(m_cam, getContext().getViewportHeight(), metres);	

	Camera cam;
	camFromSettings(cam, llgd.latInDegrees(), llgd.lonInDegrees(), 0, 0, 0, rng);

	cam.setOrbitalAltitude(getContext().getMeshElevation(cam.getOrbitalTargetLatLon()));

	goToCamera(cam,timeInMilliseconds);
}

void CameraAnimator::tripAnimation( const PYXPointer<FeatureIterator> & features )
{
	m_tripIterator = features;

	startTripStep();
}

void CameraAnimator::startTripStep()
{
	if (m_tripIterator->end())
	{
		m_tripIterator.reset();
		return;
	}

	boost::intrusive_ptr<IFeature> current = m_tripIterator->getFeature();

	PYXCoord3DDouble startLocation = current->getGeometry()->getBoundingCircle().getCenter();

	double lat,lon,heading,alt,tlt,range;
	camToSettings(m_cam,lat,lon,heading,alt,tlt,range);

	CoordLatLon ll = WGS84::getInstance()->toDatum(CmlConvertor::toLatLon(startLocation));

	boost::intrusive_ptr<IFeature> next;

	m_tripIterator->next();

	if (!m_tripIterator->end())
	{
		next = m_tripIterator->getFeature();
	}

	if (next)
	{
		PYXCoord3DDouble nextLocation = m_tripIterator->getFeature()->getGeometry()->getBoundingCircle().getCenter();		
		heading = SphereMath::headingInDegrees(CmlConvertor::toLatLon(startLocation),CmlConvertor::toLatLon(nextLocation));

		//adjusting heading to match the camera heading (cw vs ccw and zero is 90);
		heading = (360-heading)+180; 
		if (heading > 360)
		{
			heading -= 360;
		}

		//tlt = 45.0;
		//alt = 0;
	}
	else
	{
		//heading = 0;
		tlt = 0;
		//alt = 0;
	}

	Camera cam;
	camFromSettings(cam,ll.latInDegrees(),ll.lonInDegrees(),heading,alt,tlt,range);

	goToCamera(cam,2000);
}
