#pragma once
#ifndef VIEW_MODEL__COMPONENT_H
#define VIEW_MODEL__COMPONENT_H
/******************************************************************************
component.h

begin		: 2009-11-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"
#include "view_model_api.h"
#include "sync_context.h"
#include "ui_events.h"

#include "ray.h"
#include "open_gl_texture.h"
#include "texture_packer.h"

#include "pyxis/utility/object.h"

#include <vector>

typedef double PickRange;

class RhombusRGBA; //forard declartions

typedef std::pair<PickRange,PYXPointer<IAnnotation>> PickedIAnnotation;

inline bool operator <(const PickedIAnnotation & a,const PickedIAnnotation & b)
{
	return a.first < b.first;
}

typedef std::vector<PickedIAnnotation> PickedIAnnotationVector;

//forward decleration of ViewOpenGLThread class
class ViewOpenGLThread;

/*!
Component - Abstract class for components implementation

the interface of a Component is built made of the following:

1. initialize - the Component should try to initialize all its resources (like shaders, 
     OepnGL extensions, etc). and return true on success.
     this is how the view class determines which renderer to use. and if this component can be used

2. render - render on the current View - must be called by the ViewOpenGLThread

3. handle UI events - the ViewOpenGLThread would forward UI events from .Net side. see list of virtual functions start with on

4. releaseOpenGLResources() - called by ViewOpenGLThread where there is a need to release resources

5. getContext() - get the context of the Component (ViewOpenGLThread class)

6. getParent() - get the parent Component (if have any)

7. each component has a name. a reference to a component can be achieved by using getComponentByName()

8. Visible/Enabled - defines if the Component should be displayed / receive events

9. Active - define if the current component is active. while active, the component would be the first to receive the UI events.

10. schedulePostProcessing - invoke a callback that would happen later on the main thread - good for OpenGL background operations
*/
//!Renderer - Abstract class for renderer implementation
class Component : public PYXObject
{
	friend class ContainerComponent;

//Ctor and Dtor
protected:
	Component(ViewOpenGLThread & viewThread) : m_thread(viewThread), m_visible(true), m_enabled(true), m_parent(NULL) {};

public:
	virtual ~Component() {};


private:
	Component(const Component & r ) : m_thread(r.m_thread), m_visible(true), m_enabled(true), m_parent(NULL) {};
	const Component & operator=(const Component &) {};

//OpenGL thread access
protected:
	ViewOpenGLThread & m_thread;
public:
	inline ViewOpenGLThread & getViewThread() { return m_thread; };

protected:
	ContainerComponent * m_parent;

	void setParent(ContainerComponent * parent) { m_parent = parent; }
public:
	inline ContainerComponent * getParent() { return m_parent; }

protected:
	template<class T> void schedulePostProcessing(void (T::*func)()) { getViewThread().normal.beginInvokeOn((*(T*)this),func); };
	template<class T,typename A1> void schedulePostProcessing(void (T::*func)(A1 a1),A1 a1) { getViewThread().normal.beginInvokeOn((*(T*)this),func,a1); };
	template<class T,typename A1,typename A2> void schedulePostProcessing(void (T::*func)(A1 a1,A2 a2),A1 a1,A2 a2) { getViewThread().normal.beginInvokeOn((*(T*)this),func,a1,a2); };
	template<class T,typename A1,typename A2,typename A3> void schedulePostProcessing(void (T::*func)(A1 a1,A2 a2,A3 a3),A1 a1,A2 a2,A3 a3) { getViewThread().normal.beginInvokeOn((*(T*)this),func,a1,a2,a3); };

//API functions
public:
	//! initialize the renderer 
	virtual bool initialize() { return true; };

	//! release open GL resources. called when the view is no longer needed and we can release all openGL resources
	virtual void releaseOpenGLResources() {};

	//! Render a model into a view
	virtual void render() {}; 

	//! Perfrom a Ray Picking
	virtual void pickAnnotations(const Ray & ray,PickedIAnnotationVector & resultVector) {};

//UI event handling
public:
	virtual void onMouseClick(PYXPointer<UIMouseEvent> event) {};
	virtual void onMouseDoubleClick(PYXPointer<UIMouseEvent> event) {};

	virtual void onMouseMove(PYXPointer<UIMouseEvent> event) {};
	virtual void onMouseUp(PYXPointer<UIMouseEvent> event) {};
	virtual void onMouseDown(PYXPointer<UIMouseEvent> event) {};
	virtual void onMouseWheel(PYXPointer<UIMouseEvent> event) {};
	
	virtual void onKeyPressed(PYXPointer<UIKeyEvent> event) {};
	virtual void onKeyDown(PYXPointer<UIKeyEvent> event) {};
	virtual void onKeyUp(PYXPointer<UIKeyEvent> event) {};

//! Visible & Enable
protected:
	bool m_visible;
	bool m_enabled;

public:
	const bool & isVisible() const { return m_visible; }
	void setVisible(const bool & visible) { m_visible = visible; }

	void show() { m_visible = true; }
	void hide() { m_visible = false; }


	const bool & isEnabled() const { return m_enabled; }
	void setEnabled(const bool & enabled) { m_enabled = enabled; }

	//! enable the component to receive UI events
	void enable() { m_enabled = true; }

	//! disable the component from receiving UI events
	void disable() { m_enabled = false; }

	//! check if the current component is active in it's context
	bool isActive();

	//! make the current component active
	void setActive();

	//! make the current component not active
	void releaseActive();

//Screen cooridate translation
protected:

	virtual vec2 localToScreenCoordinate(const vec2 & coord);

	virtual vec2 screenToLocalCoodinate(const vec2 & coord);


//Utilites and Hepler functions
protected:
//must be called inside openGL context

	//! load a RhombusRGBA from application resource. blocking function
	PYXPointer<RhombusRGBA> loadRhombusBitmapFromResource(const std::string & resourceName);

	//! load a texture from application resource. blocking function
	PYXPointer<OpenGLTexture> loadTextureFromResource(const OpenGLTexture::TextureFormat & format,const std::string & resourceName);

	//! load a texture from application resource. blocking function
	bool loadTextureFromResource(OpenGLTexture & texture,const std::string & resourceName, unsigned int mipLevel = 0);

	//! load a texture region from application resource. blocking function
	bool loadTextureRegionFromResource(OpenGLTexture & texture,const std::string & resourceName,int x,int y,int width,int height);

	//! load a texture from application resource and pack it inside a given texture packer. blocking function
	PYXPointer<PackedTextureItem> packTextureFromResource(TexturePacker & packer,const std::string & resourceName);

	//! load a texture from IconStyle and pack it inside a given texture packer. blocking function
	PYXPointer<PackedTextureItem> packTextureFromBitmapDefinition(TexturePacker & packer,const std::string & bitmapDefinition);

//Non blocking API - must be called inside openGL context

	//! retrun the requested bitmap if found inside the texturePacker, else request a texture from BitmapServerProvider
	PYXPointer<PackedTextureItem> getTextureFromBitmapDefinition(TexturePacker & packer,const std::string & bitmapDefinition);
	
	//! make sure that bitmap is ready inside the texturePacker. if there is not bitmap. it would be requested from BitmapServerProvider
	void prepareBitampDefinition(TexturePacker & packer,const std::string & bitmapDefinition);	

	//! forget all pending request for this texture packer
	void forgetPendingRequests(TexturePacker & packer);
};

typedef std::vector<PYXPointer<Component>> ComponentsVector;

#endif