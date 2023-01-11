/******************************************************************************
view.cpp

begin		: 2007-07-25
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#include "view.h"
#include "view_open_gl_thread.h"

// view model includes
#include "exceptions.h"
#include "fill_utils.h"
#include "gl_utils.h"
#include "pyxtree_utils.h"
#include "stars.h"
#include "stile.h"
#include "vector_utils.h"

#include "animation.h"
#include "performance_counter.h"
#include "garbage_collector.h"


// pyxlib includes
#include "pyxis/derm/child_iterator.h"
#include "pyxis/derm/exhaustive_iterator.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/wgs84.h"

#include "pyxis/data/pyx_feature.h"
#include "pyxis/data/feature_iterator_linq.h"

#include "pyxis/geometry/tile_collection.h"
#include "pyxis/geometry/cell.h"
#include "pyxis/geometry/vector_geometry.h"
#include "pyxis/geometry/vector_geometry2.h"

#include "pyxis/region/curve_region.h"
#include "pyxis/region/multi_polygon_region.h"
#include "pyxis/region/vector_point_region.h"

#include "pyxis/utility/app_services.h"
#include "pyxis/utility/sphere_math.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/sxs.h"

// boost includes
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/xtime.hpp>

// standard includes
#include <set>


#define SMARTZOOM 1
#define VECTORTEST 0

#if VECTORTEST
#include "vector_utils.h"

const int lineCount = 10;
float lineData[lineCount][10][3];
double polyoff = 0;

int featSeed = 0;
const int featCount = 1000;
const int featVertCount = 1000;
float featVertData[featCount][featVertCount*3];
#endif

namespace
{

//! Tester class
Tester<View> gTester;

// TODO change this to use cml
double defaulteye[3] = { ((11001.00+6371.007)/6371.007), 0, 0 };
double defaultcenter[3] = { 0, 0, 0 };
double defaultup[3] = { 0, 0, 1 };

//! Compass size in pixels.
const int knCompassSize = 64;

#if VECTORTEST
void initFeatVert(int feat)
{
	vec3 p(1, 0, 0);
	vec3 d(0, 0, 1);
	vec3 c(0, 1, 0);

	p = vec3(((double)rand()/RAND_MAX)*2-1, ((double)rand()/RAND_MAX)*2-1, ((double)rand()/RAND_MAX)*2-1); // cml::random_unit(p);
	d = vec3(((double)rand()/RAND_MAX)*2-1, ((double)rand()/RAND_MAX)*2-1, ((double)rand()/RAND_MAX)*2-1); // cml::random_unit(d);
	p.normalize();
	d.normalize();
	c = cml::unit_cross(p, d);
	d = cml::unit_cross(c, p);

	double step = 500; // in metres
	double turn = 0;

	//FILE* fp = fopen("c:\\out.txt", "w");
	//fprintf(fp, "%17g %17g %17g\n", p[0], p[1], p[2]);

	featVertData[feat][0] = (float)p[0];
	featVertData[feat][1] = (float)p[1];
	featVertData[feat][2] = (float)p[2];

	for (int n = 1; n != featVertCount; ++n)
	{
		// Adjust point
		p += d * (step/SphereMath::knEarthRadius);
		p.normalize();

		//fprintf(fp, "%17g %17g %17g\n", p[0], p[1], p[2]);

		// Re-orthogonalize direction
		c = cml::unit_cross(p, d);
		d = cml::unit_cross(c, p);

		// Adjust direction by turn amount
		turn = (((double)rand()/RAND_MAX)*2-1) * cml::constantsd::pi()/8;
		d = cml::rotate_vector(d, p, turn);

		// Adjust step by up to 10%
		step *= 0.8 + ((double)rand()/RAND_MAX) * 0.4;
		cml::clamp(step, 1.0, 1000.0);

		featVertData[feat][n*3+0] = (float)p[0];
		featVertData[feat][n*3+1] = (float)p[1];
		featVertData[feat][n*3+2] = (float)p[2];
	}

	//fclose(fp);
}

void initFeat()
{
	srand(featSeed);
	for (int feat = 0; feat != featCount; ++feat)
	{
		initFeatVert(feat);
	}
}
#endif

}

int View::m_nNextID = 1;

View::ViewMap View::m_viewMap;

void View::test()
{
}

View::View()
{	
	initialize();
}

View::View(ViewTheme theme)
{
	if (theme == knViewEmbedded) {
		m_config.m_bOptDrawBackground = false;
		m_config.m_bDefaultShowControls = false;
	}
	initialize();	
}

void View::initialize() {
	m_nID = m_nNextID++;
	m_bDisposed = false;
	m_nRes = -1;
	m_postProcessingTime = 2;

	// Register view
	assert(!isView(m_nID));
	m_viewMap[m_nID] = this;

	bool bEnableLOD = getAppProperty("View",
		"EnableLOD",
		false,
		"Whether to enable dual LOD");
	View::lodEnabled(bEnableLOD);

	m_endMoveNotifier.setNotifierName("viewEndMove");
	m_pointerChangedNotifier.setNotifierName("viewPointerChanged");
	m_streamingChangedNotifier.setNotifierName("viewStreamingChanged");
	m_eyeAltChangedNotifier.setNotifierName("viewEyeAltChanged");
	m_resolutionChangedNotifier.setNotifierName("viewResolutionChanged");
	m_errorNotifier.setNotifierName("viewError");
	m_POIChangeNotifier.setNotifierName("viewPOIChange");

	m_annotationClickNotifier.setNotifierName("viewAnnotationClick");
	m_annotationDoubleClickNotifier.setNotifierName("viewAnnotationDoubleClick");
	m_annotationMouseEnterNotifier.setNotifierName("viewAnnotationMouseEnter");
	m_annotationMouseLeaveNotifier.setNotifierName("viewAnnotationMouseLeave");
	m_annotationMouseMoveNotifier.setNotifierName("viewAnnotationMouseMove");

	m_openGLThread = PYXNEW(ViewOpenGLThread,*this);
	m_openGLThread->normal.invoke(&ViewOpenGLThread::internalInit);
}

View::~View()
{
	if (!m_bDisposed)
	{
		dispose();
	}

	// Unregister view
	ViewMap::iterator it = m_viewMap.find(m_nID);
	assert(it != m_viewMap.end());
	m_viewMap.erase(it);
}

void View::dispose()
{
	if (!m_bDisposed)
	{
		m_bDisposed = true;

		m_openGLThread->normal.invoke(&ViewOpenGLThread::internalDispose);
		m_openGLThread = NULL;	
	}
}

void View::closeAllResources()
{
	GarbageCollector::getInstance()->waitUntilAllObjectsDestroy();
	FillUtils::closeAllResources();
	Annotation::closeAllResources();
}

double View::calcEyeAltitude(const Camera& cam)
{
	return cam.getOribitalEyeAltitiude();	
}

static double viewportLerp[][2] =
{
	{ 0.00, 0.00 },
	{ 0.00, 0.50 },
	{ 0.00, 1.00 },
	{ 0.50, 0.00 },
	{ 0.50, 0.50 },
	{ 0.50, 1.00 },
	{ 1.00, 0.00 },
	{ 1.00, 0.50 },
	{ 1.00, 1.00 },
	{ 0.25, 0.25 },
	{ 0.25, 0.75 },
	{ 0.75, 0.25 },
	{ 0.75, 0.75 }
};


/*
PYXIcosIndex View::calcLookAtCell(const Camera& cam, int nRes)
{
	PYXIcosIndex index;

	vec3 eye;
	cam.getEye(eye.data());
	vec3 center;
	cam.getCenter(center.data());
	vec3 d(center - eye);
	d.normalize();

	double t = intersectRayWithUnitSphere(eye, d);
	if (t != std::numeric_limits<double>::infinity())
	{
		vec3 i(eye + t*d);
		CoordLatLon ll(SphereMath::xyzll(PYXCoord3DDouble(i[0], i[1], i[2])));
		SnyderProjection::getInstance()->nativeToPYXIS(ll, &index, nRes);
	}

	return index;
}

vec3 View::calcLookAt(const Camera& cam, double x, double y)
{
	assert(0 <= x && x <= 1);
	assert(0 <= y && y <= 1);

	vec3 pos;

	// TODO

	return pos;
}
*/

Camera View::getCamera() const
{
	return m_openGLThread->getCamera();
}

int View::getViewportWidth() const
{
	return m_openGLThread->getViewportWidth();
}

int View::getViewportHeight() const
{
	return m_openGLThread->getViewportHeight();
}

int View::getMouseX() const
{
	return m_openGLThread->getMouseX();
}
	
int View::getMouseY() const
{
	return m_openGLThread->getMouseY();
}

double View::calcEyeAltitude() const
{
	return calcEyeAltitude(getCamera());
}


PYXIcosIndex View::findIndexToNavigateTo(const PYXPointer<PYXGeometry> & geometry)
{
	PYXIcosIndex navIndex;

	PYXPointer<PYXCell> cell = boost::dynamic_pointer_cast<PYXCell>(geometry);

	if (cell)
	{
		navIndex = cell->getIndex();
		int targetResolution = (navIndex.getResolution() + getViewDataResolution()) / 2; //average between two resolutions
		navIndex.setResolution(targetResolution);
	}
	else 
	{
		PYXBoundingCircle circle = geometry->getBoundingCircle();

		if (circle.getRadius()==0)
		{
			//zoom into that circle a little bit
			navIndex = CmlConvertor::toPYXIcosIndex(circle.getCenter(),std::min(PYXMath::knMaxAbsResolution-1,getViewDataResolution()+5));
		}
		else
		{
			//find a resolution where the radius will be 200 pixels
			navIndex = CmlConvertor::toPYXIcosIndex(circle.getCenter(),SnyderProjection::getInstance()->precisionToResolution(circle.getRadius()/200));
		}
	}

	return navIndex;
}

void View::goToCamera(const Camera& cam)
{
	m_openGLThread->getCameraAnimator().goToCamera(cam);
}

void View::goToCamera(const Camera& cam,int timeInMiliseconds)
{
	m_openGLThread->getCameraAnimator().goToCamera(cam,timeInMiliseconds);
}

void View::goToIndex(const PYXIcosIndex& index)
{
	m_openGLThread->getCameraAnimator().goToIndex(index);
}

void View::goToLatlon(const CoordLatLon& latlon)
{
	goToIndex(CmlConvertor::toPYXIcosIndex(latlon,getViewDataResolution()));
}

void View::goToFeature(const boost::intrusive_ptr<IFeature> & feature)
{
	goToGeometry(feature->getGeometry());
}

void View::goToGeometry(const PYXPointer<PYXGeometry> & geometry)
{	
	PYXIcosIndex navIndex = findIndexToNavigateTo(geometry);
	
	if (!navIndex.isNull())
	{
		goToIndex(navIndex);
	}
}

void View::goToIndex(const PYXIcosIndex& index,int timeInMiliseconds)
{
	m_openGLThread->getCameraAnimator().goToIndex(index,timeInMiliseconds);
}

void View::goToLatlon(const CoordLatLon& latlon,int timeInMiliseconds)
{
	goToIndex(CmlConvertor::toPYXIcosIndex(latlon,getViewDataResolution()),timeInMiliseconds);
}

void View::goToFeature(const boost::intrusive_ptr<IFeature> & feature,int timeInMiliseconds)
{
	goToGeometry(feature->getGeometry(),timeInMiliseconds);
}

void View::goToGeometry(const PYXPointer<PYXGeometry> & geometry,int timeInMiliseconds)
{	
	PYXIcosIndex navIndex = findIndexToNavigateTo(geometry);
	
	if (!navIndex.isNull())
	{
		goToIndex(navIndex,timeInMiliseconds);
	}
}

PYXCoord3DDouble View::getPointerLocation()
{
	if (m_bDisposed)
	{
		//just ignore the request
		return PYXCoord3DDouble();
	}

	return CmlConvertor::toPYXCoord3D(m_openGLThread->getPointerLocation());	
}

PYXIcosIndex View::getPointerIndex(int nRes)
{
	if (m_bDisposed)
	{
		//just ignore the request
		return PYXIcosIndex();
	}

	return m_openGLThread->getPointerIndex(nRes);
}

PYXIcosIndex View::getIndexFromScreen(int x,int y)
{
	auto ray = m_openGLThread->getRay(vec2(x,m_openGLThread->m_nHeight-y));
	vec3 intersection;
	if (m_openGLThread->findRayIntersection(ray,intersection))
	{
		intersection.normalize();
		return CmlConvertor::toPYXIcosIndex(intersection,m_openGLThread->getViewDataResolution());
	}
	return PYXIcosIndex();
}

PYXCoord3DDouble View::projectFromScreenSpace(int x,int y) 
{
	auto ray = m_openGLThread->getRay(vec2(x,m_openGLThread->m_nHeight-y));
	vec3 intersection;
	if (m_openGLThread->findRayIntersection(ray,intersection))
	{
		return CmlConvertor::toPYXCoord3D(intersection);
	}
	return PYXCoord3DDouble();
}

PYXCoord2DDouble View::projectToScreenSpace(PYXCoord3DDouble xyz)
{
	return m_openGLThread->projectToScreenSpace(xyz);
}

void View::updatePointerString()
{
	vec3 pointer = getPointerLocation();

	if (pointer.length() == 0 )
	{
		setPointerIndex("");
	}
	else
	{
		pointer.normalize();

		PYXCoord3DDouble coord;
		coord.setX(pointer[0]);
		coord.setY(pointer[1]);
		coord.setZ(pointer[2]);
		CoordLatLon llgc = SphereMath::xyzll(coord);
		CoordLatLon llgd = WGS84::getInstance()->toDatum(llgc);
		char buf[128];
		setBufferText(buf, sizeof(buf),llgd.lat(), llgd.lon());
		setPointerIndex(buf);
	}
}


void View::getVisibleTiles(std::vector<PYXIcosIndex>& vecCells)
{
	//update visible tiles vector
	m_openGLThread->normal.invoke(&ViewOpenGLThread::copyVisiblieTilesToView);

	vecCells.clear();
	for (std::vector<PYXIcosIndex>::iterator it = m_vecTiles.begin(); it != m_vecTiles.end(); ++it)
	{
		vecCells.push_back((*it));
	}
}

/*!
Place the cells that cover the screen into a single geometry.
It is up to the caller to clear the geometry before the screen cells are added.

\param	mc		The multi cell to add the visible tiles to.
*/
void View::getVisibleTiles(PYXMultiCell& mc)
{
	//update visible tiles vector
	m_openGLThread->normal.invoke(&ViewOpenGLThread::copyVisiblieTilesToView);

	for (std::vector<PYXIcosIndex>::const_iterator it = m_vecTiles.begin(); it != m_vecTiles.end(); ++it)
	{
		const PYXIcosIndex& index = (*it);
		mc.addIndex(index);
	}
}

/*!
Place the cells that cover the screen into a single geometry.
It is up to the caller to clear the geometry before the screen cells are added.

\param	tc		The tile collection to add the visible tiles to.
*/
void View::getVisibleTiles(PYXTileCollection& tc)
{
	//update visible tiles vector
	m_openGLThread->normal.invoke(&ViewOpenGLThread::copyVisiblieTilesToView);
	int deepestResolution = 0;
	
	for (std::vector<PYXIcosIndex>::const_iterator it = m_vecTiles.begin(); it != m_vecTiles.end(); ++it)
	{
		const PYXIcosIndex& index = (*it);
        if (index.getResolution() > deepestResolution)
		{
			deepestResolution = index.getResolution();
		}		
	}
	for (std::vector<PYXIcosIndex>::const_iterator it = m_vecTiles.begin(); it != m_vecTiles.end(); ++it)
	{
		const PYXIcosIndex& index = (*it);
        tc.addTile(index, deepestResolution );
	}
}

//! return the screen geometry - tile collection or a ploygon
//borderOffset can be used to generate are with borderOffset of pixels on the edges
PYXPointer<PYXGeometry> View::getScreenGeometry(int borderOffsetInPixels)
{
	std::vector<PYXCoord3DDouble> verticies;
	auto width = getViewportWidth() - borderOffsetInPixels*2;
	auto height = getViewportHeight() - borderOffsetInPixels*2;

	for(int i=0;i<40;i++)
	{
		int x = borderOffsetInPixels;
		int y = borderOffsetInPixels;
		int offset = i%10;
		switch(i/10)
		{
		case 0:
			x += width*offset/10;
			break;

		case 1:
			x += width;
			y += height*offset/10;
			break;

		case 2:
			y += height;
			x += width - width*offset/10;
			break;

		case 3:
			y += height - height*offset/10;
			break;
		}
		
		auto ray = m_openGLThread->getRay(vec2(x,y));
		vec3 intersection;
		if (m_openGLThread->findRayIntersection(ray,intersection))
		{
			intersection.normalize();
			verticies.push_back(CmlConvertor::toPYXCoord3D(intersection));
		}
		else 
		{
			//we weren't able to find intersection - fall back to visible tiles
			PYXPointer<PYXTileCollection> tiles = PYXTileCollection::create();
			getVisibleTiles(*tiles);
			
			//assume the geometry resolution is the current view data resolution
			tiles->setCellResolution(getViewDataResolution());

			return tiles;
		}
	}
	
	std::vector<PYXPointer<PYXCurveRegion>> curves;
	
	curves.push_back(PYXCurveRegion::create(verticies,true));
	return PYXVectorGeometry2::create(PYXMultiPolygonRegion::create(curves),getViewTileResolution()+5);
}

//TODO: Comment this.
void View::setBufferText(char* buf, unsigned int size, double lat, double lng)
{
	std::string str = getPointerIndex(getViewDataResolution()).toString();
	char* pIndexStr = const_cast<char*>(str.c_str());
	assert(pIndexStr != 0);
	sprintf_s(buf, size, "Lat %.4f Lon %.4f PYXIS %s", cml::deg(lat), cml::deg(lng), pIndexStr, '\0'); 
}


void View::setViewPointProcess(boost::intrusive_ptr<IProcess> spNewViewPointProcess)
{	
	m_spViewPointProcess = spNewViewPointProcess;

	//update the OpenGLThread vis pipe
	m_openGLThread->normal.invoke(&ViewOpenGLThread::setViewPointProcess, m_spViewPointProcess);
	
	//reset stream progress until next frame rendering would update to the right amount
	for (auto & item : m_streamingProgressMap) 
	{
		item.second = 0;
	}
	m_strStreamingProgress = "0";
	
	m_strError.clear();
}

void View::display()
{
	// if this happens this is bad.
	if (m_bDisposed)
	{
		//just ignore the request
		return;
	}

	static bool traceFrame = false;
	try
	{
		PerformanceCounter::startMeasurement();

		if (traceFrame)
		{
			TRACE_INFO("Problematic Frame timing");
			PerformanceCounter::traceTimeFrame(-2);
			traceFrame=false;
		}

		m_frameTimer.start();
		m_lastTick = 0;
		
		//update all animations object
		(AnimationUpdater::getInstance()).update();
		
		m_openGLThread->ASAP.invoke(&ViewOpenGLThread::setupRender);
		
		PerformanceCounter::getTimePerformanceCounter("finishSetup",1.0,1.0,0)->makeMeasurement();
		
		//do jobs for m_postProcessingTime millisecond
		if (m_openGLThread->performJobsWithTimeout(m_postProcessingTime))
		{
			//true - means there are no more jobs needed
			m_postProcessingTime = cml::clamp(m_postProcessingTime-1,2,10);		
		}
		else 
		{
			//false - we have more jobs to do
			m_postProcessingTime = cml::clamp(m_postProcessingTime+1,2,10);
		}

		setFrameTimeMeasurement("setup-processing");

		PerformanceCounter::getValuePerformanceCounter("postprocessing count")->setMeasurement(m_openGLThread->getWaitingJobsCount());

		PerformanceCounter::getTimePerformanceCounter("finishProcessing",1.0,0,0)->makeMeasurement();

		m_openGLThread->ASAP.invoke(&ViewOpenGLThread::render);


		PerformanceCounter::getTimePerformanceCounter("finishRendering")->makeMeasurement();

		setFrameTimeMeasurement("render-end");

		m_frameTimer.stop();
		
		m_totalRenderTime = (int)(1000* m_frameTimer.getTime());

		if (m_totalRenderTime > 1000)
		{
			traceFrame = true;
		}
	}
	catch (PYXException& ex)
	{
		TRACE_ERROR("Exception happen while displaying viewpoint: " << ex.getFullErrorString());
		TRACE_INFO("ViewState on excpetion:" << m_openGLThread->getState() );
	}
}


void View::reshape(int nWidth, int nHeight)
{
	m_openGLThread->ASAP.invoke(&ViewOpenGLThread::internalReshape,nWidth,nHeight);	
}

void View::releaseOpenGLResources()
{
	m_vecTiles.clear();
	m_openGLThread->ASAP.invoke(&ViewOpenGLThread::releaseOpenGLResources);
}

/*!
Sets and notifies all observers of change in pointer location.

\param strPointer	The new pointer location.
*/
void View::setPointerIndex(const std::string& strPointer)
{
	if((m_strPointer != strPointer) && strPointer != "")
	{
		safeNotify(m_pointerChangedNotifier,ViewEvent::create(strPointer));
	}

	m_strPointer = strPointer;
}

/*!
Sets and notifies all observers of change in streaming progress.

\param strProgress	The new streaming progress.
*/
void View::setStreamingProgress(const std::string& category,int progress)
{
	m_streamingProgressMap[category] = progress;

	int minProgress = 100;
	for(auto & item : m_streamingProgressMap) 
	{
		if (minProgress > item.second) 
		{
			minProgress = item.second;
		}
	}

	auto strProgress = StringUtils::toString(minProgress);

	if(m_strStreamingProgress != strProgress)
	{
		safeNotify(m_streamingChangedNotifier,ViewEvent::create(strProgress));
	}

	m_strStreamingProgress = strProgress;
}

//! getter for the streaming progress (0-100)
int View::getStreamingProgress()
{
	return StringUtils::fromString<int>(m_strStreamingProgress);
}

//! getter for the streaming progress (0-100) for a specific category. -1 if not category found
int View::getStreamingProgress(const std::string & category)
{
	if (m_streamingProgressMap.find(category) != m_streamingProgressMap.end()) 
	{
		return m_streamingProgressMap[category];
	} 
	return -1;
}

//! getter for the streaming progress (0-100) for a specific process. -1 if processes is not been rendered
int View::getStreamingProgress(const ProcRef & procRef)
{
	if (m_openGLThread)
	{
		return m_openGLThread->m_rhombusRenderer->getProcessLoadingProgress(procRef);
	}
	return -1;
}


/*!
Sets and notifies all observers of change in eye altitude.

\param strEyeAlt	The new eye altitude.
*/
void View::setEyeAlt(const std::string& strEyeAlt)
{
	if(m_strEyeAlt != strEyeAlt)
	{
		safeNotify(m_eyeAltChangedNotifier,ViewEvent::create(strEyeAlt));
	}

	m_strEyeAlt = strEyeAlt;
}

/*!
Sets and notifies all observers of change in data resolution.

\param nRes	The new resolution.
*/
void View::setResolution(int nRes)
{
	if(m_nRes != nRes)
	{
		m_nRes = nRes;
		safeNotify(m_resolutionChangedNotifier,ViewEvent::create("Resolution " + intToString(m_nRes + knDataResOffset, 0)));		
	}

	
}

PYXPointer<PYXGeometry> View::getPOI()
{
	if (m_openGLThread)
	{
		return m_openGLThread->getPOI();
	}
	return 0;
}

void View::setPOI(const PYXPointer<PYXGeometry> poiGeometry)
{
	if (m_openGLThread)
	{
		return m_openGLThread->setPOI(poiGeometry);
	}
}

PYXPointer<PYXGeometry> View::calculateWatershed(const PYXIcosIndex & location)
{
	if (m_openGLThread)
	{
		return m_openGLThread->calculateWatershed(location);
	}
	return 0;
}

PYXPointer<PYXGeometry> View::calculateWatershedFlow(const PYXIcosIndex & location)
{
	if (m_openGLThread)
	{
		return m_openGLThread->calculateWatershedFlow(location);
	}
	return 0;
}

void View::onMouseClick(const int & x,const int & y,const int & delta,
				 const bool & leftButton,const bool & rightButton,const bool & middleButton,
				 const bool & altKey,const bool & shiftKey,const bool & ctrlKey)
{
	if (m_openGLThread)
	{
		m_openGLThread->onMouseClick(x,y,delta,leftButton,rightButton,middleButton,altKey,shiftKey,ctrlKey);
	}
}

void View::onMouseDoubleClick(const int & x,const int & y,const int & delta,
				 const bool & leftButton,const bool & rightButton,const bool & middleButton,
				 const bool & altKey,const bool & shiftKey,const bool & ctrlKey)
{
	if (m_openGLThread)
	{
		m_openGLThread->onMouseDoubleClick(x,y,delta,leftButton,rightButton,middleButton,altKey,shiftKey,ctrlKey);
	}
}

void View::onMouseMove(const int & x,const int & y,const int & delta,
				 const bool & leftButton,const bool & rightButton,const bool & middleButton,
				 const bool & altKey,const bool & shiftKey,const bool & ctrlKey)
{
	if (m_openGLThread)
	{
		m_openGLThread->onMouseMove(x,y,delta,leftButton,rightButton,middleButton,altKey,shiftKey,ctrlKey);
	}
}

void View::onMouseUp(const int & x,const int & y,const int & delta,
				 const bool & leftButton,const bool & rightButton,const bool & middleButton,
				 const bool & altKey,const bool &shiftKey,const bool & ctrlKey)
{
	if (m_openGLThread)
	{
		m_openGLThread->onMouseUp(x,y,delta,leftButton,rightButton,middleButton,altKey,shiftKey,ctrlKey);
	}
}

void View::onMouseDown(const int & x,const int & y,const int & delta,
				 const bool & leftButton,const bool & rightButton,const bool & middleButton,
				 const bool & altKey,const bool & shiftKey,const bool & ctrlKey)
{
	if (m_openGLThread)
	{
		m_openGLThread->onMouseDown(x,y,delta,leftButton,rightButton,middleButton,altKey,shiftKey,ctrlKey);
	}
}

void View::onMouseWheel(const int & x,const int & y,const int & delta,
				 const bool & leftButton,const bool & rightButton,const bool & middleButton,
				 const bool & altKey,const bool & shiftKey,const bool & ctrlKey)
{
	if (m_openGLThread)
	{
		m_openGLThread->onMouseWheel(x,y,delta,leftButton,rightButton,middleButton,altKey,shiftKey,ctrlKey);
	}
}

void View::onKeyPressed(const char & keyChar,const int & keyCode,const bool & altKey,const bool & shiftKey,const bool & ctrlKey)
{
	if (m_openGLThread)
	{
		m_openGLThread->onKeyPressed(keyChar,keyCode,altKey,shiftKey,ctrlKey);
	}
}

void View::onKeyUp(const char & keyChar,const int & keyCode,const bool & altKey,const bool & shiftKey,const bool & ctrlKey)
{
	if (m_openGLThread)
	{
		m_openGLThread->onKeyUp(keyChar,keyCode,altKey,shiftKey,ctrlKey);
	}
}

void View::onKeyDown(const char & keyChar,const int & keyCode,const bool & altKey,const bool & shiftKey,const bool & ctrlKey)
{
	if (m_openGLThread)
	{
		m_openGLThread->onKeyDown(keyChar,keyCode,altKey,shiftKey,ctrlKey);
	}
}


/*! Called by client to force a notification when switching between Views.
*/
void View::forceNotification()
{
	safeNotify(m_pointerChangedNotifier,ViewEvent::create(m_strPointer));
	safeNotify(m_streamingChangedNotifier,ViewEvent::create(m_strStreamingProgress));
	safeNotify(m_eyeAltChangedNotifier,ViewEvent::create(m_strEyeAlt));
	safeNotify(m_resolutionChangedNotifier,ViewEvent::create("Resolution " + intToString(m_nRes + knDataResOffset, 0)));
}


void View::safeNotify(Notifier & notifier,PYXPointer<NotifierEvent> spEvent)
{
	//we check to see if we have m_openGLThread before we check if we are inside it.
	if (m_openGLThread && m_openGLThread->inWorkerThread())
	{
		m_openGLThread->addDelayedEvent(notifier,spEvent);
	}
	else 
	{
		notifier.notify(spEvent);
	}
}

void View::setFrameTimeMeasurement(const std::string & category)
{
	auto tick = m_frameTimer.tick();
	m_lastFrameTimings[category] = (tick-m_lastTick)*1000;
	m_lastTick = tick;
}

double View::getRenderTime(const std::string & category)
{
	return m_lastFrameTimings[category];
}


void View::startPerformanceRecording(std::string path)
{
	PerformanceCounter::startRecord(path);
}

void View::stopPerfromanceRecording()
{
	PerformanceCounter::stopRecord();
}


PYXPointer<ToolTipRequest> View::getToolTipRequest()
{
	PYXPointer<ToolTipRequest> request = m_toolTipRequest;
	m_toolTipRequest = NULL;
	return request;
}

void View::addToolTipRequest(PYXPointer<ToolTipRequest> request)
{
	m_toolTipRequest = request;
}

//TODO: move this into pyxlib
class GeometryVerticesIterator : public FeatureIterator
{
public:
	static PYXPointer<GeometryVerticesIterator> create(boost::intrusive_ptr<IFeature> feature)
	{
		return PYXNEW(GeometryVerticesIterator,feature);
	}

	GeometryVerticesIterator(boost::intrusive_ptr<IFeature> feature)
	{
		PYXPointer<PYXGeometry> geom = feature->getGeometry();
		PYXPointer<PYXVectorGeometry2> vectorGeom = boost::dynamic_pointer_cast<PYXVectorGeometry2>(geom);
		PYXPointer<PYXTableDefinition> tableDef = PYXTableDefinition::create();

		if (vectorGeom)
		{
			PYXPointer<IRegion> region = vectorGeom->getRegion();

			PYXPointer<PYXCurveRegion> curve = boost::dynamic_pointer_cast<PYXCurveRegion>(region);

			if (curve)
			{
				for(int i=0;i<curve->getVerticesCount();++i)
				{
					boost::intrusive_ptr<PYXFeature> vertexFeature = new PYXFeature(
						PYXVectorGeometry2::create(PYXVectorPointRegion::create(curve->getVertex(i)),geom->getCellResolution()), //geom
						feature->getID()+":vertex"+ StringUtils::toString(i),"", //id
						false,													//writable
						tableDef);												//tableDef
					m_features.push_back(vertexFeature );
				}
				m_feature = m_features.begin();
				return;
			}
		}		
		m_features.push_back(feature);
		m_feature = m_features.begin();
	}

public:
	virtual bool end() const 
	{
		return m_feature == m_features.end();
	}

	virtual void next()
	{
		++m_feature;
	}

	virtual boost::intrusive_ptr<IFeature> getFeature() const
	{
		return *m_feature;
	}

private:
	std::list<boost::intrusive_ptr<IFeature>> m_features;
	std::list<boost::intrusive_ptr<IFeature>>::const_iterator m_feature;
};


void View::startTrip( const PYXPointer<FeatureIterator> & features )
{
	m_openGLThread->getCameraAnimator().tripAnimation(
		PYXFeatureIteratorLinq(features)
		.orderForTrip(CmlConvertor::toPYXCoord3D(getCamera().getOrbitalTargetLatLon()))
		.selectMany(boost::bind(&GeometryVerticesIterator::create,_1))
		);
}
