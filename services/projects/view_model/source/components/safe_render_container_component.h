#pragma once
#ifndef VIEW_MODEL__SAFE_RENDER_CONTAINER_COMPONENT_H
#define VIEW_MODEL__SAFE_RENDER_CONTAINER_COMPONENT_H
/******************************************************************************
container_component.h

begin		: 2010-01-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "container_component.h"

/*!
SafeRenderContainerComponent - used for render component without affecting the OpenGL between two Components rendering

designed to be used when the components not related to each other
*/
class SafeRenderContainerComponent : public ContainerComponent
{
public:
	SafeRenderContainerComponent (ViewOpenGLThread & thread);
	static PYXPointer<SafeRenderContainerComponent> create(ViewOpenGLThread & thread) { return PYXNEW(SafeRenderContainerComponent,thread); }
	virtual ~SafeRenderContainerComponent ();

//Render API
public:	
	virtual void render(); 
};


/*!
HotKeyEvent - used by HotKeySafeRenderContainerComponent to notify on HotKey Pressed
*/
class HotKeyEvent : public NotifierEvent
{
public:
	HotKeyEvent(const char & chr,PYXPointer<Component> component) : m_char(chr),m_component(component) {};
	static PYXPointer<HotKeyEvent> create(const char & chr,PYXPointer<Component> component) { return PYXNEW(HotKeyEvent,chr,component); }
	virtual ~HotKeyEvent () {};

protected:
	char m_char;
	PYXPointer<Component> m_component;

public:
	const char & getChar() { return m_char; } 
	PYXPointer<Component> getComponent() { return m_component; }
};

/*!
HotKeySafeRenderContainerComponent - used for render component without affecting the OpenGL between two Components,
  the HotKeySafeRender allow to attach HotKey for child components.

  once a hotkey was pressed, an HotKeyEvent is triggred.
*/
class HotKeySafeRenderContainerComponent : public SafeRenderContainerComponent
{
public:
	HotKeySafeRenderContainerComponent(ViewOpenGLThread & thread);
	static PYXPointer<HotKeySafeRenderContainerComponent> create(ViewOpenGLThread & thread) { return PYXNEW(HotKeySafeRenderContainerComponent,thread); }
	virtual ~HotKeySafeRenderContainerComponent ();

//UI Events API
public:
	virtual void onKeyPressed(PYXPointer<UIKeyEvent> event);

protected:
	typedef std::map<char,PYXPointer<Component>> HotKeyMap;
	HotKeyMap m_hotKeyMap;

public:
	void addHotKey(const char & chr, PYXPointer<Component> component);
	void removeHotKey(const char & chr);

protected:
	Notifier m_hotKeyNotifier;

public:
	Notifier & getHotKeyNotifier() { return m_hotKeyNotifier; }
};

#endif
