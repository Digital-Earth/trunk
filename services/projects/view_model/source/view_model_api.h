#pragma once
#ifndef VIEW_MODEL__VIEW_MODEL_API_H
#define VIEW_MODEL__VIEW_MODEL_API_H
/******************************************************************************
view_model_api.h

begin		: 2010-Feb-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"
#include "cml_utils.h"
#include "ui_events.h"

#include "pyxis/utility/object.h"
#include "pyxis/utility/notifier.h"
#include "pyxis/derm/index.h"
#include "pyxis/pipe/process.h"
#include "pyxis/data/feature.h"


#include <map>
#include <vector>
#include <set>

class IViewModel;

/*!
IAnnotation - and interface class to represent a specific IFeature inside a IProcess.

the ViewModel would generate an IAnnotation for every visible feature on the Globe.
*/
class VIEW_MODEL_API IAnnotation : public PYXObject
{
public:
	//! Returns the procref that was used to request the Annotation
	virtual const ProcRef & getProcRef() const = 0;

	virtual const boost::intrusive_ptr<IProcess> & getProcess() const = 0;

	//! Returns the featureID of the Annotation
	virtual const std::string & getFeatureID() const = 0;

	//! Returns the ViewModel that includes this Annotation.
	virtual PYXPointer<IViewModel> getViewModel() = 0;

	//! Returns the IFeature associated with the Annotation
	virtual boost::intrusive_ptr<IFeature> getFeature() = 0;
};

//TODO: failed to create this type cross SWIG
//typedef std::vector<PYXPointer<IAnnotation>> IAnnotationVector;

/*!
IViewModel - and interface class to represent a View Control.

the IViewModel is an limited interface exposed to the user scripting environment
*/
class VIEW_MODEL_API IViewModel : public PYXObject
{
public:
	//! Returns the ViewModel ID
	virtual int getID() const = 0;

	//! Returns the current ViewPointProcess that is used for visualization
	virtual boost::intrusive_ptr<IProcess> getViewPointProcess() = 0;

	//! Returns the current Viewport Width 
	virtual int getViewportWidth() const = 0;

	//! Returns the current Viewport Height
	virtual int getViewportHeight() const = 0;

	//! Returns the current X cord of the mouse position
	virtual int getMouseX() const = 0;

	//! Returns the current Y cord of the mouse position
	virtual int getMouseY() const = 0;

	//! Returns the current 3D space on the earth surface under the mouse position
	virtual PYXCoord3DDouble getPointerLocation() = 0;

	//TODO: failed to create this type cross SWIG
	/*
	virtual IAnnotationVector pickAnnotations() = 0;
	virtual IAnnotationVector pickAnnotations(const int mouseX,const int moustY) = 0;

	virtual IAnnotationVector pickAnnotationsFromProcRef(const ProcRef & procRef) = 0;
	virtual IAnnotationVector pickAnnotationsFromProcRef(const ProcRef & procRef,const int mouseX,const int moustY) = 0;
	*/

	//! Show a tool tip on the Viewport on mouse location with a given message and for a period (in milliseconds of time.
	virtual void showToolTip(const std::string & message, const int & period) = 0;	

	//! Show a tool tip on the Viewport with a given message and for a period (in milliseconds of time.
	virtual void showToolTip(const std::string & message,const int & x, const int & y, const int & period) = 0;	
};

//!Forward declaration of the View class
class View;

//! Implementation of IViewModel with the View class
class VIEW_MODEL_API ViewHandle : public IViewModel
{
public:
	static PYXPointer<ViewHandle> create(int id);

protected:
	ViewHandle(View * view);
	virtual ~ViewHandle();

public:
	virtual int getID() const;

	virtual boost::intrusive_ptr<IProcess> getViewPointProcess();

	virtual int getViewportWidth() const;

	virtual int getViewportHeight() const;

	virtual int getMouseX() const;

	virtual int getMouseY() const;

	virtual PYXCoord3DDouble getPointerLocation();

	/*
	virtual IAnnotationVector pickAnnotations();
	virtual IAnnotationVector pickAnnotations(const int mouseX,const int moustY);

	virtual IAnnotationVector pickAnnotationsFromProcRef(const ProcRef & procRef);
	virtual IAnnotationVector pickAnnotationsFromProcRef(const ProcRef & procRef,const int mouseX,const int moustY);
	*/

	virtual void showToolTip(const std::string & message, const int & period);	
	virtual void showToolTip(const std::string & message,const int & x, const int & y, const int & period);	

protected:
	View * m_view;
};

/*!

AnnotationMouseEvent - event data when an Annotation was click/hovered/ etc...

*/
class VIEW_MODEL_API AnnotationMouseEvent : public UIMouseEvent
{
public:
	AnnotationMouseEvent(PYXPointer<UIMouseEvent> mouseEvent,PYXPointer<IAnnotation> annotation, PYXPointer<IViewModel> view);
	static PYXPointer<AnnotationMouseEvent> create(PYXPointer<UIMouseEvent> mouseEvent,PYXPointer<IAnnotation> annotation, PYXPointer<IViewModel> view)
	{
		return PYXNEW(AnnotationMouseEvent,mouseEvent,annotation,view);
	}
	virtual ~AnnotationMouseEvent() {}
		

protected:
	PYXPointer<IAnnotation> m_annotation;
	PYXPointer<IViewModel> m_view;

public:
	PYXPointer<IAnnotation> getAnnotation() { return m_annotation; }
	PYXPointer<IViewModel> getViewModel() { return m_view; }
};


/*!
ToolTipRequest - helper class to pass ToolTip show request from View to GlobeControl
*/

class VIEW_MODEL_API ToolTipRequest : public PYXObject
{
public:
	ToolTipRequest(const std::string & message,const int & x, const int & y, const int & period) : m_message(message),m_x(x),m_y(y),m_period(period)
	{		
	}

	static PYXPointer<ToolTipRequest> create(const std::string & message,const int & x, const int & y, const int & period)
	{
		return PYXNEW(ToolTipRequest,message,x,y,period);
	}
	
	virtual ~ToolTipRequest()
	{
	}

protected:
	std::string m_message;
	int m_x;
	int m_y;
	int m_period;

public:
	const std::string & getMessage() const { return m_message; }
	const int & getX() const { return m_x; }
	const int & getY() const { return m_y; }
	const int & getPeriod() const { return m_period; }
};

#endif