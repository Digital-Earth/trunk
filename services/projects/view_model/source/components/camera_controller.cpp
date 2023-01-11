/******************************************************************************
camera_controller.cpp

begin		: 2010-01-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "camera_controller.h"
#include "STile.h"
#include "view_open_gl_thread.h"
#include "gl_utils.h"

#include "pyxis/utility/sphere_math.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/wgs84.h"

void CameraController::render()
{
	m_numberOfFramesWithNoMouseMovement++;
}

void CameraController::onMouseMove(PYXPointer<UIMouseEvent> event)
{	
	double dx = event->getMouseX()-m_lastX;
	double dy = event->getMouseY()-m_lastY;

	m_mouseMoving = (dx != 0 || dy != 0);
	m_numberOfFramesWithNoMouseMovement = 0;
	
	m_lastX = event->getMouseX();
	m_lastY = event->getMouseY();
	
	if (event->isLeftButtonDown())
	{			
		m_moveTo = findPointerLocationOnReferenceSphere();

		cml::quaternion_rotation_vec_to_vec(m_rotation, m_moveFrom, m_moveTo, true);

		getViewThread().getCameraAnimator().stopAnimation();
		getViewThread().getCameraAnimator().rotate(m_rotation);		
	
		m_moveTo = m_moveFrom;		
	} 
	else if (event->isMiddleButtonDown())
	{	
		double tiltDelta = -(double)10*dy / getViewThread().getViewportHeight() * 180;
        

        double rotateDelta = (double)10*dx / getViewThread().getViewportWidth() * 180;
        // Account for upper/lower half of screen and apply rotate delta
        if (event->getMouseY() < getViewThread().getViewportHeight() / 2)
        {
            rotateDelta = -rotateDelta;
        }

		getViewThread().getCameraAnimator().tiltAndRotate(tiltDelta,rotateDelta);
	} 
	/*
	else if (event->isRightButtonDown())
	{
		double zoomSpeed = 1.0 + (((double)m_lastY) / getViewThread().getViewportHeight() - 0.5) * 0.05;

		getViewThread().getCameraAnimator().zoomDrift(zoomSpeed);
	} 
	*/
}

void CameraController::onMouseUp(PYXPointer<UIMouseEvent> event)
{	
	if (m_mouseMoving && event->isLeftButtonDown() && m_numberOfFramesWithNoMouseMovement < 2)
	{
		getViewThread().getCameraAnimator().drift(m_rotation);	
	}

	if (event->isRightButtonDown())
	{
		getViewThread().getCameraAnimator().stopAnimation();
	}

	getViewThread().getCameraAnimator().setLockCameraAltitude(false);

	event->setConsumed();
}

void CameraController::onMouseDown(PYXPointer<UIMouseEvent> event)
{	
	createReferenceSphere();

	m_moveTo = findPointerLocationOnReferenceSphere();

	getViewThread().getCameraAnimator().stopAnimation();

	m_moveFrom = m_moveTo;

	m_mouseMoving = false;
	m_lastX = event->getMouseX();
	m_lastY = event->getMouseY();

	if (event->isLeftButtonDown())
	{
		getViewThread().getCameraAnimator().setLockCameraAltitude(true);
	}

	event->setConsumed();
}

void CameraController::onMouseWheel(PYXPointer<UIMouseEvent> event)
{	
	if (event->getWheelDelta() < 0)
    {
        getViewThread().getCameraAnimator().zoomOutSomePointer();
    }
    else if (0 < event->getWheelDelta())
    {
        getViewThread().getCameraAnimator().zoomInSomePointer();
    }
}
	
void CameraController::onKeyPressed(PYXPointer<UIKeyEvent> event)
{
	bool eventConsumed = true;


	double f = getViewThread().getCamera().getOribitalEyeAltitiude() / 6371007.0;
	f *= 2;
	
	if(event->isAltKeyPressed())
	{
		f*=2;
	}

	switch (event->getKeyCode())
    {
        // Pan
		case knKeys_Up:
            getViewThread().getCameraAnimator().orbit(0, f);
            break;
        case knKeys_Down:
            getViewThread().getCameraAnimator().orbit(0, -f);
            break;
        case knKeys_Left:
            getViewThread().getCameraAnimator().orbit(f, 0);
            break;
        case knKeys_Right:
            getViewThread().getCameraAnimator().orbit(-f, 0);
            break;

        // tilt
        case knKeys_PageUp:
            getViewThread().getCameraAnimator().tilt(-5);
            break;
        case knKeys_PageDown:
            getViewThread().getCameraAnimator().tilt(5);
            break;

        // rotate
        case knKeys_Home:
            getViewThread().getCameraAnimator().rotate(-5);
            break;
        case knKeys_End:
            getViewThread().getCameraAnimator().rotate(5);
            break;

		default:
			switch (event->getKeyChar())
			{
                // zoom
                case '+':
                case '=':
                    getViewThread().getCameraAnimator().zoomInSome();                    
                    break;
                case '-':
                case '_':
                    getViewThread().getCameraAnimator().zoomOutSome();
                    break;
                case '{':
					getViewThread().getCameraAnimator().zoomOutSomePointer();										
                    break;
                case '}':
					getViewThread().getCameraAnimator().zoomInSomePointer();					
                    break;
				case '|':
				case '\\':
					{
						vec3 screenCenter;
						vec3 screenCenter1;

						if (getViewThread().findRayIntersection(
								getViewThread().getRay(vec2(getViewThread().getViewportWidth()/2,
															getViewThread().getViewportHeight()/2)),screenCenter) &&
							getViewThread().findRayIntersection(
								getViewThread().getRay(vec2(getViewThread().getViewportWidth()/2+1,
															getViewThread().getViewportHeight()/2)),screenCenter1))
						{
							quat rotation;

							if (event->getKeyChar() == '|')
							{
								screenCenter1 = (screenCenter + screenCenter1)/2;
							}

							cml::quaternion_rotation_vec_to_vec(rotation, screenCenter, screenCenter1, true);

							getViewThread().getCameraAnimator().drift(rotation);
						}
					}
					break;

				default:
					eventConsumed = false;
			}
    }

	if (eventConsumed)
	{
		event->setConsumed();
	}
}

void CameraController::createReferenceSphere()
{
	m_shpere_radius = getViewThread().getPointerLocation().length();
}

vec3 CameraController::findPointerLocationOnReferenceSphere()
{
	Ray ray(getViewThread().getMouseRay());
		
	vec3 result = cml::zero_3D();

	ray.intersectsWithSphere(cml::zero_3D(),m_shpere_radius,result);

	return result;
}