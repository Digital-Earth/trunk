#pragma once
#ifndef VIEW_MODEL__ZOOM_CONROLLER_H
#define VIEW_MODEL__ZOOM_CONROLLER_H
/******************************************************************************
zoom_renderer.h

begin		: 2009-11-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "component.h"
#include "scrollbar_renderer.h"

class ZoomDriftScrollBarController : public Component
{
public:
	ZoomDriftScrollBarController(ViewOpenGLThread & viewThread);
	ZoomDriftScrollBarController(PYXPointer<ScrollBarRenderer> scrollbar);

	static PYXPointer<ZoomDriftScrollBarController> create(ViewOpenGLThread & thread) { return PYXNEW(ZoomDriftScrollBarController,thread); }
	static PYXPointer<ZoomDriftScrollBarController> create(PYXPointer<ScrollBarRenderer> scrollbar) { return PYXNEW(ZoomDriftScrollBarController,scrollbar); }

	virtual ~ZoomDriftScrollBarController();

protected:
	PYXPointer<ScrollBarRenderer> m_scrollbar;	

public:
	PYXPointer<ScrollBarRenderer> getScrollbar();
	void setScrollbar(PYXPointer<ScrollBarRenderer> scrollbar);

	void onScrollChange(PYXPointer<NotifierEvent> event);
	void onStartScroling(PYXPointer<NotifierEvent> event);
	void onStopScroling(PYXPointer<NotifierEvent> event);
	void onPlusButtonClicked(PYXPointer<NotifierEvent> event);
	void onMinusButtonClicked(PYXPointer<NotifierEvent> event);
};


class ZoomRangeScrollBarController : public Component
{
public:
	ZoomRangeScrollBarController(ViewOpenGLThread & viewThread);
	ZoomRangeScrollBarController(PYXPointer<ScrollBarRenderer> scrollbar);

	static PYXPointer<ZoomRangeScrollBarController> create(ViewOpenGLThread & thread) { return PYXNEW(ZoomRangeScrollBarController,thread); }
	static PYXPointer<ZoomRangeScrollBarController> create(PYXPointer<ScrollBarRenderer> scrollbar) { return PYXNEW(ZoomRangeScrollBarController,scrollbar); }

	virtual ~ZoomRangeScrollBarController();

protected:
	PYXPointer<ScrollBarRenderer> m_scrollbar;	

public:
	PYXPointer<ScrollBarRenderer> getScrollbar();
	void setScrollbar(PYXPointer<ScrollBarRenderer> scrollbar);

	void onScrollChange(PYXPointer<NotifierEvent> event);
	void onStartScroling(PYXPointer<NotifierEvent> event);
	void onStopScroling(PYXPointer<NotifierEvent> event);
	void onPlusButtonClicked(PYXPointer<NotifierEvent> event);
	void onMinusButtonClicked(PYXPointer<NotifierEvent> event);

	void onCameraChange(PYXPointer<NotifierEvent> event);
};

#endif

