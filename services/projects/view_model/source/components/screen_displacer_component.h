#pragma once
#ifndef VIEW_MODEL__SCREEN_DISPLACER_COMPONENT_H
#define VIEW_MODEL__SCREEN_DISPLACER_COMPONENT_H
/******************************************************************************
screen_displacer_component.h

begin		: 2010-01-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "container_component.h"

/*!
ScreenDisplacerComponent - used to place 2D Components on the screen.

The ScreenDisplacerComponent is applying a 2D orthognal camera with viewport in size of the screen (in pixels) and apply a transformation (translate, rotate and scale).
after applying the transformation it render all its children components.

Another feature of the ScreenDisplacerComponent is translating into Local Cordinates UI Mouse Events.

placing on the screen is done as follows:
1. translate to AnchorPoint - (TopLeft,BottomCenter etc)
2. translate to center (relative to anchor point)
3. rotate in rotationAngle
4. scale (can be different in x and y)

"Local" coordinates are "pre=translated" units.  IE: They will be transformed according to the matrix described above
when they are mapped into "screen" coordinates.

"Screen" coordinates are measured in pixels.

Note that all coordinates are in OpenGL format. this mean that (0,0) is the BottomLeft of the screen.
*/
class ScreenDisplacerComponent : public ContainerComponent
{
public:
	ScreenDisplacerComponent (ViewOpenGLThread & thread);
	static PYXPointer<ScreenDisplacerComponent> create(ViewOpenGLThread & thread) { return PYXNEW(ScreenDisplacerComponent,thread); }
	virtual ~ScreenDisplacerComponent ();

//Render API
public:
	virtual bool initialize();
	virtual void render(); 

//Screen cooridate translation
protected:
	virtual vec2 localToScreenCoordinate(const vec2 & coord);
	virtual vec2 screenToLocalCoodinate(const vec2 & coord);

public:
	enum Anchor
	{
		knTopLeft			= 0,
		knTopCenter			= 1,
		knTopRight			= 2,
		knLeftCenter		= 3,
		knCenter			= 4,
		knRightCenter		= 5,
		knBottomLeft		= 6,
		knBottomCenter		= 7,
		knBottomRight		= 8
	};

protected:
	Anchor m_anchor;
	vec2   m_anchorPoint;
	vec2   m_center;
	vec2   m_scaling;
	double m_rotationAngle;

	void updateAnchorPoint();
	
public:
	const Anchor & getAnchor() const { return m_anchor; }
	void setAnchor(const Anchor & anchor) { m_anchor = anchor; updateAnchorPoint(); }

	const vec2 & getCenter() const { return m_center; }
	void setCenter(const vec2 & center) { m_center = center; }

	const vec2 & getScaling() const { return m_scaling; }
	void setScaling(const vec2 & scaling) { m_scaling = scaling; }

	const double & getRotationAngle() const { return m_rotationAngle; }
	void setRotationAngle(const double & rotationAngle) { m_rotationAngle = rotationAngle; }

	vec2 getMouseCoordinate(const PYXPointer<UIMouseEvent> & event) const;

//UI Events API
public:
	virtual void onMouseClick(PYXPointer<UIMouseEvent> event);
	virtual void onMouseDoubleClick(PYXPointer<UIMouseEvent> event);

	virtual void onMouseMove(PYXPointer<UIMouseEvent> event);
	virtual void onMouseUp(PYXPointer<UIMouseEvent> event);
	virtual void onMouseDown(PYXPointer<UIMouseEvent> event);
	virtual void onMouseWheel(PYXPointer<UIMouseEvent> event);

	virtual PYXPointer<UIMouseEvent> propogateMouseEvent(PYXPointer<UIMouseEvent> event);

//resize notification
public:
	void onResize(PYXPointer<NotifierEvent> event);
};

#endif
