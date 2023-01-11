/******************************************************************************
view_model_api.cpp

begin		: 2010-Feb-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "view_model_api.h"
#include "view.h"
#include "view_open_gl_thread.h"

#include "pyxis/pipe/process.h"
#include "pyxis/data/feature_collection.h"
#include "pyxis/pipe/pipe_manager.h"

#include <boost/thread/mutex.hpp>

#include <random>


///////////////////////////////////////////////////////////////////////////////
// ViewHandle
///////////////////////////////////////////////////////////////////////////////

PYXPointer<ViewHandle> ViewHandle::create(int id)
{
	View * view = View::getView(id);

	if (view != NULL)
	{
		return PYXNEW(ViewHandle,view);
	}
	return NULL;
}


ViewHandle::ViewHandle(View * view) : m_view(view)
{
}

ViewHandle::~ViewHandle()
{
}

int ViewHandle::getID() const
{
	return m_view->getID();
}

boost::intrusive_ptr<IProcess> ViewHandle::getViewPointProcess()
{
	return m_view->getViewPointProcess();
}

int ViewHandle::getViewportWidth() const
{
	return m_view->getViewportWidth();
}

int ViewHandle::getViewportHeight() const
{
	return m_view->getViewportHeight();
}

int ViewHandle::getMouseX() const
{
	return m_view->getMouseX();
}

int ViewHandle::getMouseY() const
{
	return m_view->getMouseY();
}

PYXCoord3DDouble ViewHandle::getPointerLocation()
{
	return m_view->getPointerLocation();
}

/*
IAnnotationVector ViewHandle::pickAnnotations()
{
	return IAnnotationVector();
}

IAnnotationVector ViewHandle::pickAnnotations(const int mouseX,const int moustY)
{
	return IAnnotationVector();
}

IAnnotationVector ViewHandle::pickAnnotationsFromProcRef(const ProcRef & procRef)
{
	return IAnnotationVector();
}

IAnnotationVector ViewHandle::pickAnnotationsFromProcRef(const ProcRef & procRef,const int mouseX,const int moustY)
{
	return IAnnotationVector();
}
*/

void ViewHandle::showToolTip(const std::string & message, const int & period)
{
	m_view->addToolTipRequest(ToolTipRequest::create(message,static_cast<int>(getMouseX()),static_cast<int>(getMouseY()),period));
}

void ViewHandle::showToolTip(const std::string & message,const int & x, const int & y, const int & period)
{
	m_view->addToolTipRequest(ToolTipRequest::create(message,x,y,period));
}


///////////////////////////////////////////////////////////////////////////////
// AnnotationMouseEvent
///////////////////////////////////////////////////////////////////////////////

AnnotationMouseEvent::AnnotationMouseEvent(PYXPointer<UIMouseEvent> mouseEvent,PYXPointer<IAnnotation> annotation, PYXPointer<IViewModel> view)
: UIMouseEvent(mouseEvent->getMouseX(),mouseEvent->getMouseY(),mouseEvent->getWheelDelta(),
			   mouseEvent->isLeftButtonDown(),mouseEvent->isRightButtonDown(),mouseEvent->isMiddleButtonDown(),
			   mouseEvent->isAltKeyPressed(),mouseEvent->isShiftKeyPressed(),mouseEvent->isCtrlKeyPressed()),
  m_annotation(annotation),
  m_view(view)
{
}