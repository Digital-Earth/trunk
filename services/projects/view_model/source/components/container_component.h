#pragma once
#ifndef VIEW_MODEL__CONTAINER_COMPONENT_H
#define VIEW_MODEL__CONTAINER_COMPONENT_H
/******************************************************************************
container_component.h

begin		: 2010-01-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "component.h"

#include <vector>

/*!

ContainerComponent - base class of a component that has child components.

the basic behavoir of the Container is to render all children in their order.

and give the children a chance to handle UI events (in reverse order of renderering)

the container checks if the control is visible before calling render, and if it's enabled and visible before sending UI events

*/
class ContainerComponent : public Component
{

public:	
	ComponentsVector m_children;
	
	ContainerComponent(ViewOpenGLThread & thread);	
	static PYXPointer<ContainerComponent> create(ViewOpenGLThread & thread) { return PYXNEW(ContainerComponent,thread); }
	virtual ~ContainerComponent ();


//Render API
public:
	virtual bool initialize();
	virtual void releaseOpenGLResources();
	virtual void render(); 
	virtual void pickAnnotations(const Ray & ray,PickedIAnnotationVector & resultVector);

//UI Events API
public:
	virtual void onMouseClick(PYXPointer<UIMouseEvent> event);
	virtual void onMouseDoubleClick(PYXPointer<UIMouseEvent> event);

	virtual void onMouseMove(PYXPointer<UIMouseEvent> event);
	virtual void onMouseUp(PYXPointer<UIMouseEvent> event);
	virtual void onMouseDown(PYXPointer<UIMouseEvent> event);
	virtual void onMouseWheel(PYXPointer<UIMouseEvent> event);
	
	virtual void onKeyPressed(PYXPointer<UIKeyEvent> event);
	virtual void onKeyDown(PYXPointer<UIKeyEvent> event);
	virtual void onKeyUp(PYXPointer<UIKeyEvent> event);

public:
	void addChild(PYXPointer<Component> component);
	void removeChild(PYXPointer<Component> component);
	void clearChildren();
	const ComponentsVector & getChildren() { return m_children; }

	//! used to propogate mouse events to active components - used for local cordiante transformation;
	virtual PYXPointer<UIMouseEvent> propogateMouseEvent(PYXPointer<UIMouseEvent> event);
};

#endif
