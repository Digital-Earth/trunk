#pragma once
#ifndef VIEW_MODEL__CAMERA_CONTROLLER_H
#define VIEW_MODEL__CAMERA_CONTROLLER_H
/******************************************************************************
camera_controller.h

begin		: 2010-01-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "component.h"
#include "camera_animator.h"

class ViewOpenGLThread;

/*!

CameraController - used for controlling the globe with mouse and keyboard gestures.

*/
//!CameraController - used for controlling the globe with mouse and keyboard gestures.
class CameraController : public Component
{
public:	
	CameraController(ViewOpenGLThread & thread) : Component(thread), m_numberOfFramesWithNoMouseMovement(0) {};
	static PYXPointer<CameraController> create(ViewOpenGLThread & thread) { return PYXNEW(CameraController,thread); }
	virtual ~CameraController() {};

protected:
	double m_lastX;
	double m_lastY;
	bool m_mouseMoving;
	int m_numberOfFramesWithNoMouseMovement;

	vec3 m_moveFrom;
	vec3 m_moveTo;
	quat m_rotation;


	//referene sphere with elevaiton
	double m_shpere_radius;

	void createReferenceSphere();
	vec3 findPointerLocationOnReferenceSphere();

	//IUIController API
public:
	virtual void render();

	virtual void onMouseMove(PYXPointer<UIMouseEvent> event);
	virtual void onMouseUp(PYXPointer<UIMouseEvent> event);
	virtual void onMouseDown(PYXPointer<UIMouseEvent> event);
	virtual void onMouseWheel(PYXPointer<UIMouseEvent> event);
	
	virtual void onKeyPressed(PYXPointer<UIKeyEvent> event);
};

#endif
