#pragma once
#ifndef VIEW_MODEL__ANNOTATIONS_CONTROLLER_H
#define VIEW_MODEL__ANNOTATIONS_CONTROLLER_H
/******************************************************************************
annotations_controller.h

begin		: 2010-March-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "component.h"
#include "view_model_api.h"
#include "annotation.h"
#include "surface_memento.h"
#include <vector>



/*!

AnnotationsController - controller that responsible to trigger UI events for selected / picked annotations

-- Description:
     - calls pickAnnotaitons on rootComponent
     - handle mouse events on picked annotations     

-- OpenGL extensions
	 - None

-- Limitations: 
     - None
	 
*/
//! AnnotationsController - controller that responsible to trigger UI events for selected / picked annotations
class AnnotationsController : public Component
{
public:
	AnnotationsController(Component & rootComponent);

	static PYXPointer<AnnotationsController> create(Component & rootComponent)
	{
		return PYXNEW(AnnotationsController,rootComponent);
	}

	virtual ~AnnotationsController(void);
	
	virtual void render(); 
	
	//UI Events handling
public:
	virtual void onMouseMove(PYXPointer<UIMouseEvent> event);
	virtual void onMouseClick(PYXPointer<UIMouseEvent> event);
	virtual void onMouseDoubleClick(PYXPointer<UIMouseEvent> event);

protected:
	//root component to pickAnnotations from
	Component & m_rootComponent;

	//vector of picked annotions
	PickedIAnnotationVector m_pickedAnnotations;

	//current annotation selected
	PYXPointer<IAnnotation> m_currentAnnotation;
};

#endif