/******************************************************************************
zoom_renderer.cpp

begin		: 2010-01-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "zoom_controller.h"

#include "view_open_gl_thread.h"
#include "gl_utils.h"

#include "exceptions.h"


ZoomDriftScrollBarController::ZoomDriftScrollBarController(ViewOpenGLThread & viewThread) : Component(viewThread)
{
}

ZoomDriftScrollBarController::ZoomDriftScrollBarController(PYXPointer<ScrollBarRenderer> scrollbar) : Component(scrollbar->getViewThread())
{
	setScrollbar(scrollbar);
}

ZoomDriftScrollBarController::~ZoomDriftScrollBarController()
{
	//detach if needed
	setScrollbar(NULL);
}

PYXPointer<ScrollBarRenderer> ZoomDriftScrollBarController::getScrollbar()
{
	return m_scrollbar;
}

void ZoomDriftScrollBarController::setScrollbar(PYXPointer<ScrollBarRenderer> scrollbar)
{
	if (m_scrollbar)
	{		
		m_scrollbar->getSrcollbarChangeNotifier().detach(this,&ZoomDriftScrollBarController::onScrollChange);
		m_scrollbar->getStartScrollingNotifier().detach(this,&ZoomDriftScrollBarController::onStartScroling);
		m_scrollbar->getStopScrollingNotifier().detach(this,&ZoomDriftScrollBarController::onStopScroling);
		m_scrollbar->getPlusButtonClickedNotifier().detach(this,&ZoomDriftScrollBarController::onPlusButtonClicked);
		m_scrollbar->getMinusButtonClickedNotifier().detach(this,&ZoomDriftScrollBarController::onMinusButtonClicked);
	}
	m_scrollbar = scrollbar;

	if (m_scrollbar)
	{		
		m_scrollbar->getSrcollbarChangeNotifier().attach(this,&ZoomDriftScrollBarController::onScrollChange);
		m_scrollbar->getStartScrollingNotifier().attach(this,&ZoomDriftScrollBarController::onStartScroling);
		m_scrollbar->getStopScrollingNotifier().attach(this,&ZoomDriftScrollBarController::onStopScroling);
		m_scrollbar->getPlusButtonClickedNotifier().attach(this,&ZoomDriftScrollBarController::onPlusButtonClicked);
		m_scrollbar->getMinusButtonClickedNotifier().attach(this,&ZoomDriftScrollBarController::onMinusButtonClicked);
	}
}

void ZoomDriftScrollBarController::onScrollChange(PYXPointer<NotifierEvent> event)
{
	ScrollBarChangeEvent * scrollChangeEvent = dynamic_cast<ScrollBarChangeEvent*>(event.get());

	if (scrollChangeEvent != NULL)
	{
		ScrollBarRenderer & scrollbar = *(scrollChangeEvent->getScrollbar());
		scrollbar.getViewThread().getCameraAnimator().zoomDrift(1+(scrollbar.getScrollPosition()-0.5)*0.1);
	}
}

void ZoomDriftScrollBarController::onStartScroling(PYXPointer<NotifierEvent> event)
{
}

void ZoomDriftScrollBarController::onStopScroling(PYXPointer<NotifierEvent> event)
{
	ScrollBarEvent * scrollEvent = dynamic_cast<ScrollBarEvent*>(event.get());

	if (scrollEvent != NULL)
	{
		ScrollBarRenderer & scrollbar = *(scrollEvent->getScrollbar());
	
		scrollbar.getViewThread().getCameraAnimator().stop();			
		scrollbar.setScrollPosition(0.5);
	}
}

void ZoomDriftScrollBarController::onPlusButtonClicked(PYXPointer<NotifierEvent> event)
{
	ScrollBarEvent * scrollEvent = dynamic_cast<ScrollBarEvent*>(event.get());

	if (scrollEvent != NULL)
	{
		ScrollBarRenderer & scrollbar = *(scrollEvent->getScrollbar());
	
		scrollbar.getViewThread().getCameraAnimator().zoomInSome();
	}
}

void ZoomDriftScrollBarController::onMinusButtonClicked(PYXPointer<NotifierEvent> event)
{
	ScrollBarEvent * scrollEvent = dynamic_cast<ScrollBarEvent*>(event.get());

	if (scrollEvent != NULL)
	{
		ScrollBarRenderer & scrollbar = *(scrollEvent->getScrollbar());
	
		scrollbar.getViewThread().getCameraAnimator().zoomOutSome();
	}
}





ZoomRangeScrollBarController::ZoomRangeScrollBarController(ViewOpenGLThread & viewThread) : Component(viewThread)
{
}

ZoomRangeScrollBarController::ZoomRangeScrollBarController(PYXPointer<ScrollBarRenderer> scrollbar) : Component(scrollbar->getViewThread())
{
	setScrollbar(scrollbar);
}

ZoomRangeScrollBarController::~ZoomRangeScrollBarController()
{
	//detach if needed
	setScrollbar(NULL);
}

PYXPointer<ScrollBarRenderer> ZoomRangeScrollBarController::getScrollbar()
{
	return m_scrollbar;
}

void ZoomRangeScrollBarController::setScrollbar(PYXPointer<ScrollBarRenderer> scrollbar)
{
	if (m_scrollbar)
	{		
		m_scrollbar->getSrcollbarChangeNotifier().detach(this,&ZoomRangeScrollBarController::onScrollChange);
		m_scrollbar->getPlusButtonClickedNotifier().detach(this,&ZoomRangeScrollBarController::onPlusButtonClicked);
		m_scrollbar->getMinusButtonClickedNotifier().detach(this,&ZoomRangeScrollBarController::onMinusButtonClicked);
		m_scrollbar->getViewThread().getCameraAnimator().getCameraChangeNotifier().detach(this,&ZoomRangeScrollBarController::onCameraChange);
	}
	m_scrollbar = scrollbar;

	if (m_scrollbar)
	{		
		m_scrollbar->getSrcollbarChangeNotifier().attach(this,&ZoomRangeScrollBarController::onScrollChange);
		m_scrollbar->getPlusButtonClickedNotifier().attach(this,&ZoomRangeScrollBarController::onPlusButtonClicked);
		m_scrollbar->getMinusButtonClickedNotifier().attach(this,&ZoomRangeScrollBarController::onMinusButtonClicked);
		m_scrollbar->getViewThread().getCameraAnimator().getCameraChangeNotifier().attach(this,&ZoomRangeScrollBarController::onCameraChange);
	}
}

void ZoomRangeScrollBarController::onScrollChange(PYXPointer<NotifierEvent> event)
{
	ScrollBarChangeEvent * scrollChangeEvent = dynamic_cast<ScrollBarChangeEvent*>(event.get());

	if (scrollChangeEvent != NULL)
	{
		ScrollBarRenderer & scrollbar = *(scrollChangeEvent->getScrollbar());
		scrollbar.getViewThread().getCameraAnimator().zoomToLogScale(scrollbar.getScrollPosition());
	}
}

void ZoomRangeScrollBarController::onStartScroling(PYXPointer<NotifierEvent> event)
{
}

void ZoomRangeScrollBarController::onStopScroling(PYXPointer<NotifierEvent> event)
{	
}

void ZoomRangeScrollBarController::onPlusButtonClicked(PYXPointer<NotifierEvent> event)
{
	ScrollBarEvent * scrollEvent = dynamic_cast<ScrollBarEvent*>(event.get());

	if (scrollEvent != NULL)
	{
		ScrollBarRenderer & scrollbar = *(scrollEvent->getScrollbar());
	
		scrollbar.getViewThread().getCameraAnimator().zoomInSome();
	}
}

void ZoomRangeScrollBarController::onMinusButtonClicked(PYXPointer<NotifierEvent> event)
{
	ScrollBarEvent * scrollEvent = dynamic_cast<ScrollBarEvent*>(event.get());

	if (scrollEvent != NULL)
	{
		ScrollBarRenderer & scrollbar = *(scrollEvent->getScrollbar());
	
		scrollbar.getViewThread().getCameraAnimator().zoomOutSome();
	}
}

void ZoomRangeScrollBarController::onCameraChange(PYXPointer<NotifierEvent> event)
{
	CameraChangeEvent * cameraEvent = dynamic_cast<CameraChangeEvent*>(event.get());

	if (cameraEvent != NULL && ! m_scrollbar->isScrolling() )
	{
		m_scrollbar->setScrollPosition(cameraEvent->getAnimator().getZoomLogScale());
	}
}