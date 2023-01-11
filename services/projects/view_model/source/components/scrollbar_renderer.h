#pragma once
#ifndef VIEW_MODEL__SCROLLBAR_RENDERER_H
#define VIEW_MODEL__SCROLLBAR_RENDERER_H
/******************************************************************************
scrollbar_renderer.h

begin		: 2009-11-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "component.h"
#include "texture_packer.h"

#include <map>
#include <set>



/*!

ScrollBarRenderer - render a scroll bar

-- Description:
     - this renderer use the Performance Mesurment class to reterive it's data.

-- OpenGL extentions: 
     - None

-- Limiations:
     - None

*/
//! ScrollBarRenderer  - render the pyxis grid for all visible tiles on View. 
class ScrollBarRenderer  : public Component
{

public:
	ScrollBarRenderer(ViewOpenGLThread & viewThread);
	static PYXPointer<ScrollBarRenderer> create(ViewOpenGLThread & viewThread) { return PYXNEW(ScrollBarRenderer,viewThread); }

	virtual ~ScrollBarRenderer(void);

	virtual bool initialize();
	virtual void render();	

protected:
	TexturePacker m_texturePacker;
	PYXPointer<PackedTextureItem> m_plusButtonTexture;
	PYXPointer<PackedTextureItem> m_minusButtonTexture;
	PYXPointer<PackedTextureItem> m_scollbarTexture;
	PYXPointer<PackedTextureItem> m_scollbarButtonTexture;

	//IUIController API
public:
	virtual void onMouseDown(PYXPointer<UIMouseEvent> event);
	virtual void onMouseMove(PYXPointer<UIMouseEvent> event);
	virtual void onMouseUp(PYXPointer<UIMouseEvent> event);

	//Internal postion
protected:
	//! the position of the scroll bar. between 0 and 1
	double m_scrollPosition;

	//! true when scrolling
	bool   m_scrolling;

	//Properties
protected:
	//! center of the scroll bar - screen cordiantes
	vec2   m_center; 

	//! size of the buttons
	vec2   m_buttonsScaling; 

	//! length of the scrool bar - in pixels
	double m_scrollAreaLength;

	//! roation of the scroll bar
	double m_rotationAngle;	

public:
	const bool & isScrolling() const { return m_scrolling; }

	const double & getScrollPosition() const { return m_scrollPosition; }
	void setScrollPosition(const double & position) { m_scrollPosition = position; }

	const vec2 & getCenter() const { return m_center; }
	void setCenter(const vec2 & center) { m_center = center; }

	const vec2 & getButtonScaling() const { return m_buttonsScaling; }
	void setButtonScaling(const vec2 & buttonScaling) { m_buttonsScaling = buttonScaling; }

	const double & getScrollAreaLength() const { return m_scrollAreaLength; }
	void setScrollAreaLength(const double & scrollAreaLength) { m_scrollAreaLength = scrollAreaLength; }

	const double & getRotationAngle() const { return m_rotationAngle; }
	void setRotationAngle(const double & rotationAngle) { m_rotationAngle = rotationAngle; }

protected:
	//convert mouse X,Y from event to local codinates
	vec2 getMouseCoordinate(const PYXPointer<UIMouseEvent> & event);

	//Notfiers
protected:
	Notifier m_srcollbarChangeNotifier;
	Notifier m_startScrollingNotifier;
	Notifier m_stopScrollingNotifier;
	Notifier m_plusButtonClickedNotifier;
	Notifier m_minusButtonClickedNotifier;

public:
	Notifier & getSrcollbarChangeNotifier() { return m_srcollbarChangeNotifier; }
	Notifier & getStartScrollingNotifier() { return m_startScrollingNotifier; }
	Notifier & getStopScrollingNotifier() { return m_stopScrollingNotifier; }
	Notifier & getPlusButtonClickedNotifier() { return m_plusButtonClickedNotifier; }
	Notifier & getMinusButtonClickedNotifier() { return m_minusButtonClickedNotifier; }	
};


class ScrollBarEvent : public NotifierEvent
{
public:
	ScrollBarEvent(PYXPointer<ScrollBarRenderer> scrollbar) : m_scrollbar(scrollbar) {};
	virtual ~ScrollBarEvent() {};

protected:
	PYXPointer<ScrollBarRenderer> m_scrollbar;	

public:
	PYXPointer<ScrollBarRenderer> getScrollbar() { return m_scrollbar; }
};

class ScrollBarChangeEvent : public ScrollBarEvent
{
public:
	ScrollBarChangeEvent(PYXPointer<ScrollBarRenderer> scrollbar,double delta) : ScrollBarEvent(scrollbar), m_delta(delta) {};
	virtual ~ScrollBarChangeEvent() {};

protected:	
	double m_delta;

public:	
	double getDelta() { return m_delta; }
};

#endif

