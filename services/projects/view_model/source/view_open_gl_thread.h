#pragma once
#ifndef VIEW_MODEL__VIEW_OPEN_GL_THREAD_H
#define VIEW_MODEL__VIEW_OPEN_GL_THREAD_H
/******************************************************************************
view_open_gl_thread.h

begin		: 2009-11-01
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "sync_context.h"
#include "view.h"
#include "view_model_api.h"
#include "open_gl_context.h"
#include "camera.h"
#include "ray.h"
#include "surface_fillers.h"
#include "annotation.h"
#include "component.h"
#include "components\camera_controller.h"
#include "components\rhombus_renderer.h"
#include "components\safe_render_container_component.h"
#include "components\zoom_controller.h" 
#include "Component_resource.h"

//pyxlib
#include "pyxis/pipe/process.h"

#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

/*!
This class is used to store a delayed event that triggered inside the OpenGLThread.
Because the MainThread is sleeping while the OpenGLThread is rendering, we can't invoke the event. 
We need to delayed it until the MainThread is running again.

The Delayed event are stored at a list of DelayNotifiyEvent.
Once the OpenGLThread::invoke returned to the MainThread, all delayed events are triggered one by one.

to make it simple to use, use the safeNotify function that would check the currentThread and do what it should do to make it work
*/
//! Used by the OpenGLThread to notify events
struct DelayedNotifyEvent
{
	Notifier * m_notifier;
	PYXPointer<NotifierEvent> m_spEvent;
};


/*
TODO:

2. LOD - remove this or change the way STile is using is static variable - foo!!!

3. calcVisibleTiles is in global scope - fix that
*/


/*!
The OpenGLThread is a bad name. it should be ViewOpenGLContext. That because out Main thread is the OpenGL thread.
This class is used to perfrom OpenGL operations in a sync fasion.
We create a Sync::JobListContext which is a simple JobList. each time a renderer want to perform some post poressing with OpenGL operations, it schedule a Job inisde the joblist.
The main thread call JobListContext::performJobsWithTimeout(5 miliseconds) to perform post processing jobs before rendering the scene.
*/
//! The OpenGLThread that is responsible for all OpenGL activites for the ViewClass
class ViewOpenGLThread : public Sync::JobListContext
{
	friend class View;

protected:
	//! create and ViewOpenGLThread class. 
	ViewOpenGLThread(View & view);
	virtual void afterInvoke(Sync::Job & job);

public:
	//! call dispose if needed
	virtual ~ViewOpenGLThread(void);

protected:
	//! reference to the view class
	View & m_view;
	PYXPointer<IViewModel> m_viewHandle;

public:
	View & getView() { return m_view; }
	PYXPointer<IViewModel> getViewHandle() { return m_viewHandle; }	

private:
	//! called inside the OpenGL thread context when thread is started. called from View when View created
	void internalInit();
		
	//! called inside the OpenGL thread context when thread is disposed. called automaticly when ViewOpenGLThread stoped
	void internalDispose();
	

//Global OpenGL Resources 
//TODO: move it to the right renderer
private:
	// Textures
	enum
	{
		knTexCompass,
		knTexCount // must be last
	};
	unsigned int texids[knTexCount];
	
	void initGlobalGLResources();
	void uninitGlobalGLResources();


// Configuration
//TODO: move it to the right renderer
private:
	ViewConfiguration m_config;

public:
	ViewConfiguration & getConfiguration() { return m_config; };

private:
	//! copy the view state to openGLThread - usually happens before rendering
	void copyViewState();

//Render
protected:
	//! render the globe - invoked from View::display 
	void setupRender();

	//! render the globe - invoked from View::display 
	void render();

private:
	//! make sure that camera aspect is correct
	void updateCameraAspect();

	//! the actual rendering function
	void internalRender();

	void internalRenderBackground(int eyeAlt /*in meters*/);
	
	void renderCompass();	

protected:
	void releaseOpenGLResources();

protected:
	//! called by the View class when the visibleTiles is needed.
	void copyVisiblieTilesToView();

//Viewport - updated by the View::resize function
private:
	int m_nWidth;
	int m_nHeight;

protected:
	//! called inside the OpenGL thread context when the View is been reshaped
	void internalReshape(int nWidth, int nHeight);

public:
	int getViewportWidth() const
	{
		return m_nWidth;
	}

	int getViewportHeight() const
	{
		return m_nHeight;
	}

//Mouse XY - been updated before every render loop from the View class
protected:
	int m_nMouseX;
	int m_nMouseY;

	bool m_needToFindPointerLocation;

	vec3 m_pointerLocation;

	int  m_pointerResolution;

	PYXPointer<Surface::Patch> m_pointerPatch;
	
	//! find pointer location on surface by intersecting the STile mesh
	void findPointerLocation();

	//! update the pointer location 
	void updatePointerLocation();

	void cameraChanged(PYXPointer<NotifierEvent> event);

public:
	int getMouseX() {return m_nMouseX; };
	int getMouseY() {return m_nMouseY; };

	//! creates a selection Ray from mouse position
	Ray getMouseRay() { return Ray(getCamera(),m_nMouseX,m_nMouseY,m_nWidth,m_nHeight); };

	//! creates a selection Ray from screen position
	Ray getRay(vec2 screenPoint) { return Ray(getCamera(),(int)(screenPoint[0]),(int)(screenPoint[1]),m_nWidth,m_nHeight); };
	
	//! find intersection point of a ray with the earth mesh
	bool findRayIntersection(const Ray & ray,vec3 & intersection);

	//! gets pointer location of the earth surface (on the earth mesh)
	const vec3 & getPointerLocation();

	vec3 getPointerLocationOnUnitShpere() const;
	
	//! gets pointer index of the earth
	PYXIcosIndex getPointerIndex(int nRes);

	PYXPointer<Surface::Patch> getPointerPatch() { return m_pointerPatch; }

public:
	PYXPointer<PYXGeometry> getPOI()
	{
		return m_POI;
	}

	void setPOI(const PYXPointer<PYXGeometry> & geometry);

	//calculate the watershed area from a given root location
	PYXPointer<PYXGeometry> calculateWatershed(const PYXIcosIndex & location);

	//calculate the flow lines from a given root location
	PYXPointer<PYXGeometry> calculateWatershedFlow(const PYXIcosIndex & location);

private:
	PYXPointer<PYXGeometry> m_POI;

//Resolution - updated inside the render loop and the changes are invoked back to the View class
private:
	//! The view's current resolution (at the root level).
	int m_nRes;

public:
	//! Return the resolution of the tiles being rendered
	int getViewTileResolution()
	{
		return m_nRes;
	}

	//! Return the resolution of the data being rendered
	int getViewDataResolution()
	{
		return m_nRes + View::knDataResOffset;
	}


	int getResolution() { return m_nRes; }

//Camera - been updated before every render loop from the View class
private:
	mat4 m_cameraProjection;

	CameraAnimator m_cameraAnimator; 
	PYXPointer<CameraController> m_cameraControler;
	PYXPointer<HotKeySafeRenderContainerComponent> m_rootComponent;
	PYXPointer<ZoomDriftScrollBarController> m_zoomDriftController;
	PYXPointer<ZoomRangeScrollBarController> m_zoomRangeController;

	PYXPointer<RhombusRenderer> m_rhombusRenderer;

	void onHotKeyPressed(PYXPointer<NotifierEvent> event);

public:
	CameraAnimator & getCameraAnimator() { return m_cameraAnimator; }

	const Camera & getCamera() const;
	Camera & getCamera();

	//! apply OpenGL matrices from current camera - should be called from renderers only
	void applyCamera();

	//! apply OpenGL matrices from current camera - should be called from renderers only
	void applyCameraWorld(bool bInfinite = false);

	PYXCoord2DDouble projectToScreenSpace(PYXCoord3DDouble xyz);

private:
	//! set the OpenGL GL_PROJECTION matrix for perspective camera. used by applayCamera and applyCameraWorld.
	void applyCameraPerspective(bool bInfinite = false);

public:
	AppProperty<bool> useShaders;
	AppProperty<bool> useNonPowerTwoTextures;
	AppProperty<int> cameraGotoTime;

//visualization pipeline
private:
	// The visualization pipeline.
	boost::intrusive_ptr<IProcess> m_spViewPointProcess;

public:
	boost::intrusive_ptr<IProcess> getViewPointProcess();

protected:
	//! called by the View::setViewPointProcess function to update the OpenGLThread ViewPointProcess
	void setViewPointProcess(boost::intrusive_ptr<IProcess> spViewPointProcess);	

	//! handler function for pipeline changes
	// TODO: Is this thread safe??
	void handleDataChange( PYXPointer<NotifierEvent> spEvent);

	boost::intrusive_ptr<IProcess> getElevationOutputProcess();

public:
	typedef std::vector<PYXPointer<STile>> VisibleTilesVector;

// The procref that are being displayed.
private:
	ProcRef m_procref;	

	PYXCoord3DDouble m_lastClickPos;
	bool m_radiusClick;
	boost::intrusive_ptr<IProcess> m_extraFeaturesPipeline;
	
	PYXPointer<ViewPointFiller> m_viewPointFiller;

public:
	const PYXPointer<Surface> & getSurface() { return m_viewPointFiller->getSurface(); }
	const PYXPointer<SurfaceMemento<VersionedMemento<Surface::Patch::VertexBuffer>>> & getElevations() { return m_viewPointFiller->getElevations(); };

public:
	bool refineSurface();
	void modifySurfaceFromCamera();

	double getMeshElevation(const CoordLatLon & latLon);
	double getMeshElevation(const PYXPointer<Surface::Patch> & patch,const vec2 & uv);

	bool findMeshElevation(const PYXPointer<Surface::Patch> & patch,const Ray & ray,double & elevation);

public:
	//! Calculates an appropriate resolution for given camera.
	int calcAppropriateResolution(const Camera& cam, int nHeight);

protected:
	//! update the current resolution from current camera
	void updateViewResolution();

private:
	void clearFillTiles();


//Exception handling and logging
private:
	std::string m_state;

public:
	const std::string & getState() const { return m_state; }
	void setState(const std::string & state) { m_state = state; }

//timing
public:
	void setFrameTimeMeasurement(const std::string & category)
	{
		m_view.setFrameTimeMeasurement(category);
	}

//UI events sink
public:
	void onMouseClick(const int & x,const int & y,const int & delta,
				 const bool & leftButton,const bool & rightButton,const bool & middleButton,
				 const bool & altKey,const bool & shiftKey,const bool & ctrlKey);

	void onMouseDoubleClick(const int & x,const int & y,const int & delta,
				 const bool & leftButton,const bool & rightButton,const bool & middleButton,
				 const bool & altKey,const bool & shiftKey,const bool & ctrlKey);

	void onMouseMove(const int & x,const int & y,const int & delta,
				 const bool & leftButton,const bool & rightButton,const bool & middleButton,
				 const bool & altKey,const bool & shiftKey,const bool & ctrlKey);

	void onMouseUp(const int & x,const int & y,const int & delta,
				 const bool & leftButton,const bool & rightButton,const bool & middleButton,
				 const bool & altKey,const bool & shiftKey,const bool & ctrlKey);

	void onMouseDown(const int & x,const int & y,const int & delta,
				 const bool & leftButton,const bool & rightButton,const bool & middleButton,
				 const bool & altKey,const bool & shiftKey,const bool & ctrlKey);

	void onMouseWheel(const int & x,const int & y,const int & delta,
				 const bool & leftButton,const bool & rightButton,const bool & middleButton,
				 const bool & altKey,const bool & shiftKey,const bool & ctrlKey);


	void onKeyPressed(const char & keyChar,const int & keyCode,const bool & altKey,const bool & shiftKey,const bool & crtlKey);

	void onKeyUp(const char & keyChar,const int & keyCode,const bool & altKey,const bool & shiftKey,const bool & crtlKey);

	void onKeyDown(const char & keyChar,const int & keyCode,const bool & altKey,const bool & shiftKey,const bool & crtlKey);

protected:
	Notifier m_resizeNotifier;
	Notifier m_viewPortProcessChangeNotifier;

public:
	Notifier & getResizeNotifier() { return m_resizeNotifier; }
	Notifier & getViewPortProcessChangeNotifier() { return m_viewPortProcessChangeNotifier; }

//Active API
protected:
	ComponentsVector m_activeComponents;

//Active API
public:
	virtual void pushActiveComponent(PYXPointer<Component> component);
	virtual void popActiveComponent();

	virtual PYXPointer<Component> getActiveComponent();	


//Componentes resources managemnt
protected:
	ComponentResources m_componentResources;

public:
	ComponentResources & getComponentResources() { return m_componentResources; }

//delayed notifiations for OpenGLThread
private: 
	std::list<DelayedNotifyEvent> m_delayedEvents;

	//! used by safeNotify to store delayed events
	void addDelayedEvent(Notifier & notifier,PYXPointer<NotifierEvent>  spEvent);

	//! used by invoke to trigger all delyed events when return to the calling thread
	void notifyDelayedEvents();

	//! thread safe notification. can be used in Main thread and in OpenGLThread 
	void safeNotify(Notifier & notifier,PYXPointer<NotifierEvent>  spEvent);
};

#endif