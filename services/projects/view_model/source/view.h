#pragma once
#ifndef VIEW_MODEL__VIEW_H
#define VIEW_MODEL__VIEW_H
/******************************************************************************
view.h

begin		: 2007-07-25
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"
#include "view_model_api.h"

// view model includes
#include "camera.h"
#include "stile.h"

#include "open_gl_context.h"


// pyxlib includes
#include "pyxis/derm/index.h"
#include "pyxis/geometry/multi_cell.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/profile.h"

// boost includes
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

// standard includes
#include <cassert>
#include <list>
#include <map>
#include <vector>

/*! 
A ViewEvent is raised for the current View when one of the following occurs: 
- The pointer index changes.
- The streaming progress value changes while visualizing a pipeline.
- The eye altitude changes.
- A fill or cache error occurs while retrieving data for tiles.
*/
class VIEW_MODEL_API ViewEvent : public NotifierEvent
{	
public:

	//! Create a new instance.
	static PYXPointer< ViewEvent > create(std::string strValue)
	{
		return PYXNEW(ViewEvent, strValue);
	}

	//! Getter for the new value that the observer is to be notified about.
	std::string getValue()
	{
		return m_strValue;
	}

protected:

	//! Construct a new event.
	explicit ViewEvent(std::string strValue) : 
		m_strValue(strValue)
	{
	}

private:

	//! Stores the new value that the observer is to be notified about.
	std::string m_strValue;
};


//! struct for saving all view configurations
struct ViewConfiguration
{
	bool m_bOptDrawBackground;
	bool m_bDefaultShowControls;

	bool m_bOptShowAtmosphere;
	bool m_bOptShowDiagnostic;
	bool m_bOptShowStatus;
	bool m_bOptShowWorldAxes;
	bool m_bOptLockGrid;

	ViewConfiguration() :
		m_bOptDrawBackground(true),
		m_bDefaultShowControls(true),
		m_bOptShowAtmosphere(true),
		m_bOptShowDiagnostic(false),
		m_bOptShowStatus(true),
		m_bOptShowWorldAxes(false),
		m_bOptLockGrid(false)
	{
	}
};

enum ViewTheme
{
	knViewDefault,
	knViewEmbedded
};


/*!
A view expects an OpenGL context when it is created.
*/
//! Model for a view.
class VIEW_MODEL_API View
{
	friend class ViewOpenGLThread;

public:

	static void test();

	static bool isView(int nID)
	{
		return m_viewMap.find(nID) != m_viewMap.end();
	}

	static View* getView(int nID)
	{
		View* p = 0;
		ViewMap::iterator it = m_viewMap.find(nID);
		if (it != m_viewMap.end())
		{
			p = it->second;
		}
		return p;
	}

	static int getViewCount()
	{
		return static_cast<int>(m_viewMap.size());
	}

	static void closeAllResources();	

public:

	//! Calculates eye altitude (distance from eye to center of reference sphere) in world coordinates.
	static double View::calcEyeAltitude(const Camera& cam);


	//static PYXIcosIndex calcLookAtCell(const Camera& cam, int nRootRes);

	//! X and Y are on the view plane from 0 (bottom left) to 1 (top right). TODO ensure using "clipspace" coords
	//static vec3 calcLookAt(const Camera& cam, double x, double y);

private:

	typedef std::map<int, View* > ViewMap;

private:

	static int m_nNextID;
	static ViewMap m_viewMap;

	PYXPointer<ViewOpenGLThread> m_openGLThread;

	//! Utility method to fill a buffer with a string to notify with a view event.
	inline void setBufferText(char* buf, unsigned int size, double lat, double lng);

	void updatePointerString();

	
public:

	View();

	View(ViewTheme theme);

	~View();

	void dispose();

private:
	void initialize();

public:

	int getID() const
	{
		return m_nID;
	}

	Camera getCamera() const;

	int getViewportWidth() const;
	
	int getViewportHeight() const;	

	double calcEyeAltitude() const;
	
	boost::intrusive_ptr<IProcess> getViewPointProcess() const
	{
		return m_spViewPointProcess;
	}

	void setViewPointProcess(boost::intrusive_ptr<IProcess> spViewPointProcess);

	static const int knDataResOffset = 9;

	//! Return the resolution of the tiles being rendered
	int getViewTileResolution()
	{
		return m_nRes;
	}

	//! Return the resolution of the data being rendered
	int getViewDataResolution()
	{
		return m_nRes + knDataResOffset;
	}

public: // opengl
	void display();

	void reshape(int nWidth, int nHeight);

	void releaseOpenGLResources();

private:
	PYXIcosIndex findIndexToNavigateTo(const PYXPointer<PYXGeometry> & geometry);

public:

	void goToCamera(const Camera& cam);
	void goToCamera(const Camera& cam,int timeInMiliseconds);

	void goToIndex(const PYXIcosIndex& index);
	void goToIndex(const PYXIcosIndex& index,int timeInMiliseconds);

	void goToGeometry(const PYXPointer<PYXGeometry> & geometry);
	void goToGeometry(const PYXPointer<PYXGeometry> & geometry,int timeInMiliseconds);

	void goToFeature(const boost::intrusive_ptr<IFeature> & feature);
	void goToFeature(const boost::intrusive_ptr<IFeature> & feature,int timeInMiliseconds);

	void goToLatlon(const CoordLatLon& latlon);
	void goToLatlon(const CoordLatLon& latlon,int timeInMiliseconds);

	void startTrip(const PYXPointer<FeatureIterator> & features);

	int getMouseX() const;

	int getMouseY() const;

	PYXCoord3DDouble getPointerLocation();

	PYXIcosIndex getPointerIndex(int nRes);

	PYXIcosIndex getIndexFromScreen(int x,int y);
	PYXCoord3DDouble projectFromScreenSpace(int x,int y);
	PYXCoord2DDouble projectToScreenSpace(PYXCoord3DDouble xyz);

	PYXPointer<PYXGeometry> getPOI();

	void setPOI(const PYXPointer<PYXGeometry> poiGeometry);

	PYXPointer<PYXGeometry> calculateWatershed(const PYXIcosIndex & location);

	PYXPointer<PYXGeometry> calculateWatershedFlow(const PYXIcosIndex & location);

	//! Get List of all Visible PYXIcosIndex
	void getVisibleTiles(std::vector<PYXIcosIndex>& vecCells);

	//! Adds the visible tiles to a multi cell.
	void getVisibleTiles(PYXMultiCell& cellCollection);

	//! Adds the visible tiles to a tile collection.
	void getVisibleTiles(PYXTileCollection& tileCollection);

	//! return the screen geometry - tile collection or a polygon
	PYXPointer<PYXGeometry> getScreenGeometry(int borderOffsetInPixels = 0);

public: // options

	bool getOptShowAtmosphere() const
	{
		return m_config.m_bOptShowAtmosphere;
	}

	void setOptShowAtmosphere(bool b)
	{
		m_config.m_bOptShowAtmosphere = b;
	}

	bool getOptShowDiagnostic() const
	{
		return m_config.m_bOptShowDiagnostic;
	}

	void setOptShowDiagnostic(bool b)
	{
		m_config.m_bOptShowDiagnostic = b;
	}

	bool getOptShowStatus() const
	{
		return m_config.m_bOptShowStatus;
	}

	void setOptShowStatus(bool b)
	{
		m_config.m_bOptShowStatus = b;
	}

	bool getOptShowWorldAxes() const
	{
		return m_config.m_bOptShowWorldAxes;
	}

	void setOptShowWorldAxes(bool b)
	{
		m_config.m_bOptShowWorldAxes = b;
	}

	bool getOptLockGrid() const
	{
		return m_config.m_bOptLockGrid;
	}

	void setOptLockGrid(bool b)
	{
		m_config.m_bOptLockGrid = b;
	}

	//! Whether LOD is enabled. Call with true or false to set.
	static bool lodEnabled(int bSet = -1)
	{	
		static bool b = false;
		if (bSet == 0)
		{
			b = false;
		}
		else if (bSet == 1)
		{
			b = true;
		}
		return b;
	}

public: // notifiers

	Notifier& getEndMoveNotifier()
	{
		return m_endMoveNotifier;
	}

	Notifier& getPointerNotifier()
	{
		return m_pointerChangedNotifier;
	}

	Notifier& getStreamingNotifier()
	{
		return m_streamingChangedNotifier;
	}

	Notifier& getEyeAltNotifier()
	{
		return m_eyeAltChangedNotifier;
	}

	Notifier& getResolutionNotifier()
	{
		return m_resolutionChangedNotifier;
	}

	Notifier& getErrorNotifier()
	{
		return m_errorNotifier;
	}

	Notifier& getAnnotationClickNotifier()
	{
		return m_annotationClickNotifier;
	}

	Notifier& getAnnotationDoubleClickNotifier()
	{
		return m_annotationDoubleClickNotifier;
	}
	
	Notifier& getAnnotationMouseMoveNotifier()
	{
		return m_annotationMouseMoveNotifier;
	}

	Notifier& getAnnotationMouseEnterNotifier()
	{
		return m_annotationMouseEnterNotifier;
	}

	Notifier& getAnnotationMouseLeaveNotifier()
	{
		return m_annotationMouseLeaveNotifier;
	}

	Notifier& getPOIChangeNotifier()
	{
		return m_POIChangeNotifier;
	}

private: //delayed notifications for OpenGLThread

	//! thread safe notification. can be used in Main thread and in OpenGLThread 
	void safeNotify(Notifier & notifier,PYXPointer<NotifierEvent>  spEvent);

public:

	//! Gets the error string (may be empty) and resets it to empty.
	std::string getErrorString()
	{
		std::string strError = m_strError;
		m_strError.clear();
		return strError;
	}

	//! Setter for the pointer location.
	void setPointerIndex(const std::string& strPointer);

	//! Setter for the streaming progress.
	void setStreamingProgress(const std::string & category,int progress);	

	//! Setter for the eye altitude.
	void setEyeAlt(const std::string& strEyeAlt);

	//! setter for the tile resolution.
	void setResolution(int nRes);

	//! Notifies all clients of the current View values.
	void forceNotification();
	
public:
	//! getter for the streaming progress (0-100)
	int getStreamingProgress();

	//! getter for the streaming progress (0-100) for a specific category. -1 if not category found
	int getStreamingProgress(const std::string & category);

	//! getter for the streaming progress (0-100) for a specific process. -1 if processes is not been rendered
	int getStreamingProgress(const ProcRef & procRef);

public:
	void startPerformanceRecording(std::string path);
	void stopPerfromanceRecording();


private:
	//! amount of milliseconds for rendering loop
	std::map<std::string,double> m_lastFrameTimings;

	PYXHighQualityTimer m_frameTimer;
	double m_lastTick;

	void setFrameTimeMeasurement(const std::string & category);

public:
	//! get render time for a specific section: "setup", "mesh" , "
	double getRenderTime(const std::string & category);

private:

	bool m_bDisposed;

	int m_nID;

	//! The view's current resolution (at the root level).
	int m_nRes;

	//! amount of milliseconds to perform post processing
	int m_postProcessingTime;

	//! amount of milliseconds for rendering loop
	int m_totalRenderTime;

	// cells that are being displayed.
	std::vector <PYXIcosIndex> m_vecTiles;

	// The visualization pipeline.
	boost::intrusive_ptr<IProcess> m_spViewPointProcess;

	ViewConfiguration m_config;

	// TEMP working on display strings
	std::string m_strPointer;
	
	//! The current streaming progress value (0 - 100)
	std::string m_strStreamingProgress;

	std::map<std::string,int> m_streamingProgressMap;

	//! The current eye altitude.
	std::string m_strEyeAlt;

	//! Errors from the threads.
	std::string m_strError;

	// Notifiers
	Notifier m_endMoveNotifier;

	Notifier m_pointerChangedNotifier;

	Notifier m_streamingChangedNotifier;

	Notifier m_eyeAltChangedNotifier;

	Notifier m_resolutionChangedNotifier;

	//! To notify of fill or cache errors.
	Notifier m_errorNotifier;

	// UI Notifiers

	Notifier m_annotationClickNotifier;

	Notifier m_annotationDoubleClickNotifier;

	Notifier m_annotationMouseEnterNotifier;

	Notifier m_annotationMouseLeaveNotifier;

	Notifier m_annotationMouseMoveNotifier;

	Notifier m_POIChangeNotifier;

//Toolbar notification
protected:
	PYXPointer<ToolTipRequest> m_toolTipRequest;

public:
	PYXPointer<ToolTipRequest> getToolTipRequest();
	void addToolTipRequest(PYXPointer<ToolTipRequest> request);

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
};

#endif
