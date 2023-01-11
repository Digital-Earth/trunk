/******************************************************************************
screen_displacer_component.cpp

begin		: 2010-01-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "screen_displacer_component.h"
#include "view_open_gl_thread.h"


ScreenDisplacerComponent::ScreenDisplacerComponent(ViewOpenGLThread & thread) : 
	ContainerComponent(thread), 
	m_center(0,0), 
	m_scaling(1,1), 
	m_rotationAngle(0), 
	m_anchor(knBottomLeft),
	m_anchorPoint(0,0)
{
	getViewThread().getResizeNotifier().attach(this,&ScreenDisplacerComponent::onResize);
}

ScreenDisplacerComponent::~ScreenDisplacerComponent()
{
	getViewThread().getResizeNotifier().detach(this,&ScreenDisplacerComponent::onResize);
}

bool ScreenDisplacerComponent::initialize()
{
	updateAnchorPoint();
	return ContainerComponent::initialize();
}

void ScreenDisplacerComponent::render()
{
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, viewport[2], 0.0, viewport[3]);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	

	//apply displacement
	glTranslated(m_anchorPoint[0]+m_center[0],m_anchorPoint[1]+m_center[1],0);
	glRotated(m_rotationAngle,0.0,0.0,0.1);
	glScaled(m_scaling[0],m_scaling[1],1.0);

	//render children
	ContainerComponent::render();	
}

vec2 ScreenDisplacerComponent::getMouseCoordinate(const PYXPointer<UIMouseEvent> & event) const
{
	vec2 mouse = vec2(event->getMouseX(),event->getMouseY())-m_anchorPoint-m_center;
	
	//rotate in -rotationAngle
	mouse = cml::rotate_vector_2D(mouse,cml::rad(-m_rotationAngle));

	//unscale
	mouse[0] /= m_scaling[0];
	mouse[1] /= m_scaling[1];

	return mouse;
}


void ScreenDisplacerComponent::onMouseClick(PYXPointer<UIMouseEvent> event)
{
	vec2 localMouse = getMouseCoordinate(event);

	//generate local event
	PYXPointer<UIMouseEvent> localEvent = UIMouseEvent::create(localMouse[0],localMouse[1],event->getWheelDelta(),event->isLeftButtonDown(),event->isRightButtonDown(),event->isMiddleButtonDown(),event->isAltKeyPressed(),event->isShiftKeyPressed(),event->isCtrlKeyPressed());

	ContainerComponent::onMouseClick(localEvent);
	
	if (localEvent->isConsumed())
	{
		event->setConsumed();
	}
}

void ScreenDisplacerComponent::onMouseDoubleClick(PYXPointer<UIMouseEvent> event)
{
	vec2 localMouse = getMouseCoordinate(event);

	//generate local event
	PYXPointer<UIMouseEvent> localEvent = UIMouseEvent::create(localMouse[0],localMouse[1],event->getWheelDelta(),event->isLeftButtonDown(),event->isRightButtonDown(),event->isMiddleButtonDown(),event->isAltKeyPressed(),event->isShiftKeyPressed(),event->isCtrlKeyPressed());

	ContainerComponent::onMouseDoubleClick(localEvent);
	
	if (localEvent->isConsumed())
	{
		event->setConsumed();
	}
}

void ScreenDisplacerComponent::onMouseMove(PYXPointer<UIMouseEvent> event)
{
	vec2 localMouse = getMouseCoordinate(event);

	//generate local event
	PYXPointer<UIMouseEvent> localEvent = UIMouseEvent::create(localMouse[0],localMouse[1],event->getWheelDelta(),event->isLeftButtonDown(),event->isRightButtonDown(),event->isMiddleButtonDown(),event->isAltKeyPressed(),event->isShiftKeyPressed(),event->isCtrlKeyPressed());

	ContainerComponent::onMouseMove(localEvent);
	
	if (localEvent->isConsumed())
	{
		event->setConsumed();
	}
}

void ScreenDisplacerComponent::onMouseUp(PYXPointer<UIMouseEvent> event)
{
	vec2 localMouse = getMouseCoordinate(event);

	//generate local event
	PYXPointer<UIMouseEvent> localEvent = UIMouseEvent::create(localMouse[0],localMouse[1],event->getWheelDelta(),event->isLeftButtonDown(),event->isRightButtonDown(),event->isMiddleButtonDown(),event->isAltKeyPressed(),event->isShiftKeyPressed(),event->isCtrlKeyPressed());

	ContainerComponent::onMouseUp(localEvent);
	
	if (localEvent->isConsumed())
	{
		event->setConsumed();
	}
}

void ScreenDisplacerComponent::onMouseDown(PYXPointer<UIMouseEvent> event)
{
	vec2 localMouse = getMouseCoordinate(event);

	//generate local event
	PYXPointer<UIMouseEvent> localEvent = UIMouseEvent::create(localMouse[0],localMouse[1],event->getWheelDelta(),event->isLeftButtonDown(),event->isRightButtonDown(),event->isMiddleButtonDown(),event->isAltKeyPressed(),event->isShiftKeyPressed(),event->isCtrlKeyPressed());

	ContainerComponent::onMouseDown(localEvent);
	
	if (localEvent->isConsumed())
	{
		event->setConsumed();
	}
}

void ScreenDisplacerComponent::onMouseWheel(PYXPointer<UIMouseEvent> event)
{
	vec2 localMouse = getMouseCoordinate(event);

	//generate local event
	PYXPointer<UIMouseEvent> localEvent = UIMouseEvent::create(localMouse[0],localMouse[1],event->getWheelDelta(),event->isLeftButtonDown(),event->isRightButtonDown(),event->isMiddleButtonDown(),event->isAltKeyPressed(),event->isShiftKeyPressed(),event->isCtrlKeyPressed());

	ContainerComponent::onMouseWheel(localEvent);
	
	if (localEvent->isConsumed())
	{
		event->setConsumed();
	}
}

PYXPointer<UIMouseEvent> ScreenDisplacerComponent::propogateMouseEvent(PYXPointer<UIMouseEvent> event)
{
	//do native event propogation...
	event = ContainerComponent::propogateMouseEvent(event);

	//apply local cordinates
	vec2 localMouse = getMouseCoordinate(event);

	//generate local event
	PYXPointer<UIMouseEvent> localEvent = UIMouseEvent::create(localMouse[0],localMouse[1],event->getWheelDelta(),event->isLeftButtonDown(),event->isRightButtonDown(),event->isMiddleButtonDown(),event->isAltKeyPressed(),event->isShiftKeyPressed(),event->isCtrlKeyPressed());

	return localEvent;
}

void ScreenDisplacerComponent::onResize(PYXPointer<NotifierEvent> event)
{
	updateAnchorPoint();
}

void ScreenDisplacerComponent::updateAnchorPoint()
{
	switch(m_anchor)
	{
	case knTopLeft:
		m_anchorPoint = vec2(0,getViewThread().getViewportHeight());
		break;
	case knTopCenter:
		m_anchorPoint = vec2(getViewThread().getViewportWidth()/2,getViewThread().getViewportHeight());
		break;
	case knTopRight:
		m_anchorPoint = vec2(getViewThread().getViewportWidth(),getViewThread().getViewportHeight());
		break;
	case knLeftCenter:
		m_anchorPoint = vec2(0,getViewThread().getViewportHeight()/2);
		break;
	case knCenter:
		m_anchorPoint = vec2(getViewThread().getViewportWidth()/2,getViewThread().getViewportHeight()/2);
		break;
	case knRightCenter:
		m_anchorPoint = vec2(getViewThread().getViewportWidth(),getViewThread().getViewportHeight()/2);
		break;
	case knBottomLeft:
		m_anchorPoint = vec2(0,0);
		break;
	case knBottomCenter:
		m_anchorPoint = vec2(getViewThread().getViewportWidth()/2,0);
		break;
	case knBottomRight:
		m_anchorPoint = vec2(getViewThread().getViewportWidth(),0);
		break;
	}
}


vec2 ScreenDisplacerComponent::localToScreenCoordinate(const vec2 & coord)
{
	vec2 result = Component::localToScreenCoordinate(coord);

	//scale
	result[0] *= m_scaling[0];
	result[1] *= m_scaling[1];

	//rotate in rotationAngle
	result = cml::rotate_vector_2D(result,cml::rad(m_rotationAngle));

	result = result+m_anchorPoint+m_center;

	return result;
}

vec2 ScreenDisplacerComponent::screenToLocalCoodinate(const vec2 & coord)
{
	vec2 result = Component::screenToLocalCoodinate(coord);

	result = result-m_anchorPoint-m_center;

	//rotate in -rotationAngle
	result = cml::rotate_vector_2D(result,cml::rad(-m_rotationAngle));

	//unscale
	result[0] /= m_scaling[0];
	result[1] /= m_scaling[1];

	return result;
}
