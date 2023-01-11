/******************************************************************************
view_open_gl_thread.cpp

begin		: 2009-11-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"

#include "psapi.h"

#include "view_model.h"
#include "view.h"
#include "view_open_gl_thread.h"

//include renderers
#include "components\screen_displacer_component.h"
#include "components\camera_controller.h"
#include "components\icon_cpu_renderer.h"
#include "components\icon_gpu_renderer.h"
#include "components\icon_annotation_controller.h"
#include "components\icosahedron_renderer.h"
#include "components\performance_renderer.h"
#include "components\scrollbar_renderer.h"
#include "components\scale_renderer.h"
#include "components\compass_renderer.h"
#include "components\annotations_controller.h"
#include "components\rhombus_renderer.h"
#include "components\image_component.h"

#include "pyxis\derm\iterator_linq.h"

// view model includes
#include "exceptions.h"
#include "fill_utils.h"
#include "gl_utils.h"
#include "pyxtree_utils.h"
#include "stars.h"
#include "vector_utils.h"


// pyxlib includes
#include "pyxis/derm/child_iterator.h"
#include "pyxis/derm/exhaustive_iterator.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/wgs84.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/pipe/pipe_utils.h"
#include "pyxis/procs/viewpoint.h"
#include "pyxis/procs/named_geometry_proc.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/sphere_math.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/sxs.h"

#include "pyxis/data/feature_group.h"
#include "pyxis/data/writeable_feature.h"
#include "pyxis/region/circle_region.h"
#include "pyxis/region/multi_curve_region.h"
#include "pyxis/geometry/vector_geometry.h"
#include "pyxis/geometry/vector_geometry2.h"

#include <boost/algorithm/string/predicate.hpp>
#include <fstream>

ViewOpenGLThread::ViewOpenGLThread(View & view) 
	:	m_view(view),
		useShaders("View", "UseShaders", true, "Use OpenGL shaders if available on graphics card (default=true)"),
		useNonPowerTwoTextures("View", "NonPowerTwoTextures", true, "Use OpenGL non power two texture if available on graphics card (default=true)"),
		cameraGotoTime("View","CameraGotoTime", 10, "Time in seconds to perform the goto operation (default=10 seconds)")
{
	m_viewHandle = ViewHandle::create(view.getID());

	copyViewState();
	
	m_pointerLocation.zero();
	m_nMouseX = 0;
	m_nMouseY = 0;
	m_needToFindPointerLocation = true;
	m_nRes = 10;

	m_radiusClick = false;

	m_componentResources.setOpenGLThread(this);

	// Default camera
	Camera cam;
	camFromSettings(cam, 0, 0, 0, 0, 0, SphereMath::knEarthRadius*3);
	m_cameraAnimator.setCamera(cam);

	m_cameraAnimator.getCameraChangeNotifier().attach(this,&ViewOpenGLThread::cameraChanged);

	// attach contexts...
	m_cameraAnimator.setContext(*this);
	m_cameraAnimator.setUsingTerrainModel(true);
	m_cameraAnimator.setLongAnimationTime(cameraGotoTime*1000);

	m_cameraControler = CameraController::create(*this);

	m_rootComponent = HotKeySafeRenderContainerComponent::create(*this);
	m_rootComponent->getHotKeyNotifier().attach(this,&ViewOpenGLThread::onHotKeyPressed);

	m_zoomDriftController = ZoomDriftScrollBarController::create(*this);
	m_zoomRangeController = ZoomRangeScrollBarController::create(*this);

	PYXPointer<AnnotationsController> annotationController = AnnotationsController::create(*m_rootComponent);

	m_rootComponent->addChild(m_cameraControler);
	m_rootComponent->addChild(annotationController);
	//m_rootComponent->getChildren().push_back(m_zoomDriftController);
	//m_rootComponent->getChildren().push_back(m_zoomRangeController);

	m_viewPointFiller = ViewPointFiller::create(this);

	TRACE_INFO("ViewOpenGLThread was created");
}

ViewOpenGLThread::~ViewOpenGLThread(void)
{	
}

void ViewOpenGLThread::afterInvoke(Sync::Job & job)
{
	notifyDelayedEvents();
}

///////////////////////////////////////////////////////////////////////////////////////
//
// internalInit
//
//////////////////////////////////////////////////////////////////////////////////////

void ViewOpenGLThread::internalInit()
{
	auto debugTools = getAppProperty("View","DebugTools",false,"Enable debug tools for the view engine");


	//TODO: change this into non static
	initGlobalGLResources();

	// OpenGL context
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_DITHER); // apparently this is on by default, we don't need it


	// init renderers
	// Creation order == Rendering Order
	
	PYXPointer<Component> renderer;

	renderer = IcosahedronRenderer::create(*this);
	if (renderer->initialize())
	{
		renderer->setVisible(false);
		m_rootComponent->addChild(renderer);
		if (debugTools)
		{
			m_rootComponent->addHotKey('i',renderer);
			m_rootComponent->addHotKey('I',renderer);
		}
	}

	/*
	renderer = TilesCPURenderer::create(*this);
	if (renderer->initialize())
	{
		m_rootComponent->addChild(renderer);
		m_rootComponent->addHotKey('e',renderer);
		m_rootComponent->addHotKey('E',renderer);
		m_rootComponent->addHotKey('v',renderer);
		m_rootComponent->addHotKey('V',renderer);
	}
	*/

	m_rhombusRenderer = RhombusRenderer::create(*this);
	if (m_rhombusRenderer->initialize())
	{
		m_rootComponent->addChild(m_rhombusRenderer );
		if (debugTools)
		{
			m_rootComponent->addHotKey('r',m_rhombusRenderer );
			m_rootComponent->addHotKey('R',m_rhombusRenderer );
			m_rootComponent->addHotKey('l',m_rhombusRenderer );
			m_rootComponent->addHotKey('L',m_rhombusRenderer );
		}
		m_rootComponent->addHotKey('0',m_rhombusRenderer );
		m_rootComponent->addHotKey('1',m_rhombusRenderer );
		m_rootComponent->addHotKey('2',m_rhombusRenderer );		
	}

	/*
	renderer = PyxisGridRenderer::create(*this);
	if (renderer->initialize())
	{
		m_rootComponent->addChild(renderer);
		m_rootComponent->addHotKey('0',renderer);
		m_rootComponent->addHotKey('2',renderer);
		m_rootComponent->addHotKey('4',renderer);
	}
	*/
	
	PYXPointer<IconRenderer> iconRenderer;

	if (useShaders)
	{

		// try to create GPU icon renderer
		renderer = iconRenderer = IconGPURenderer::create(*this);
		if (renderer->initialize())
		{
			TRACE_INFO("Using IconGPURenderer");
		}
		else
		{
			TRACE_INFO("Failed to create IconGPURenderer, Using IconCPURenderer");
			renderer = iconRenderer = IconCPURenderer::create(*this);
			renderer->initialize();
		}
	} 
	else
	{
		TRACE_INFO("View doesn't allow to use Shaders, creating IconCPURenderer");
		
		renderer = iconRenderer = IconCPURenderer::create(*this);
		renderer->initialize();
	}

	PYXPointer<IconAnnotationsController> iconAnnotationController = IconAnnotationsController::create(*this);
	iconAnnotationController->setIconRenderer(iconRenderer);
	iconAnnotationController->initialize();
	m_rootComponent->addChild(iconAnnotationController);

	m_rootComponent->addChild(renderer);

	renderer = PerformanceRenderer::create(*this);
	if (renderer->initialize())
	{
		renderer->setVisible(false);
		m_rootComponent->addChild(renderer);
		if (debugTools)
		{
			m_rootComponent->addHotKey('p',renderer);
			m_rootComponent->addHotKey('P',renderer);
		}		
	}


	PYXPointer<ScreenDisplacerComponent> controlsDisplacer = ScreenDisplacerComponent::create(*this);

	PYXPointer<ScreenDisplacerComponent> displacer = ScreenDisplacerComponent::create(*this);
	renderer = ScrollBarRenderer::create(*this);

	if (displacer->initialize() && renderer->initialize())
	{
		displacer->addChild(renderer);
		displacer->setAnchor(ScreenDisplacerComponent::knBottomRight);
		displacer->setCenter(vec2(-50,150));

		ScrollBarRenderer & scrollbar = *((ScrollBarRenderer*)renderer.get());
		scrollbar.setButtonScaling(vec2(0.5,0.5));
		//scrollbar.setCenter(vec2(500,500));
		scrollbar.setVisible(false);

		m_zoomDriftController->setScrollbar(&scrollbar);

		controlsDisplacer->addChild(displacer);
	}

	displacer = ScreenDisplacerComponent::create(*this);
	renderer = ScrollBarRenderer::create(*this);

	if (displacer->initialize() && renderer->initialize())
	{
		displacer->addChild(renderer);
		displacer->setAnchor(ScreenDisplacerComponent::knTopRight);
		displacer->setCenter(vec2(-50,-250));

		ScrollBarRenderer & scrollbar = *((ScrollBarRenderer*)renderer.get());
		scrollbar.setButtonScaling(vec2(0.5,0.5));
		scrollbar.setScrollAreaLength(200);

		m_zoomRangeController->setScrollbar(&scrollbar);

		controlsDisplacer->addChild(displacer);
	}

	displacer = ScreenDisplacerComponent::create(*this);
	renderer = ScaleRenderer::create(*this);

	if (displacer->initialize() && renderer->initialize())
	{
		displacer->addChild(renderer);
		displacer->setAnchor(ScreenDisplacerComponent::knBottomCenter);
		displacer->setCenter(vec2(0,20));
		ScaleRenderer & scale = *((ScaleRenderer*)renderer.get());
		scale.setScaleLength(200);
		
		controlsDisplacer->addChild(displacer);
	}	
	
	displacer = ScreenDisplacerComponent::create(*this);
	renderer = CompassRenderer::create(*this);

	if (displacer->initialize() && renderer->initialize())
	{
		displacer->addChild(renderer);
		displacer->setAnchor(ScreenDisplacerComponent::knTopRight);
		displacer->setCenter(vec2(-50,-60));
		
		controlsDisplacer->addChild(displacer);
	}

	auto logoOverlay = getAppProperty("View","LogoOverlay",std::string(""),"point to an image to be used as logo overlay on the globe");

	if (logoOverlay != "")
	{
		displacer = ScreenDisplacerComponent::create(*this);
		auto imageRenderer = ImageComponent::create(*this,logoOverlay);

		if (imageRenderer->initialize())
		{
			auto imageSize = imageRenderer->getSize();
			displacer->setCenter(vec2(imageSize[0],-imageSize[1]));
			displacer->setAnchor(ScreenDisplacerComponent::knTopLeft);
			displacer->addChild(imageRenderer);

			if (displacer->initialize())
			{
				controlsDisplacer->addChild(displacer);
			}
		}
	}
	
	if (controlsDisplacer->initialize())
	{
		if (m_config.m_bDefaultShowControls == false)
		{
			controlsDisplacer->setVisible(false);
		}

		m_rootComponent->addChild(controlsDisplacer);
		m_rootComponent->addHotKey('c',controlsDisplacer);
		m_rootComponent->addHotKey('C',controlsDisplacer);
	}	
}

///////////////////////////////////////////////////////////////////////////////////////
//
// internalDispose
//
//////////////////////////////////////////////////////////////////////////////////////

void ViewOpenGLThread::internalDispose()
{
	if (m_spViewPointProcess != 0)
	{
		m_spViewPointProcess->getDataChanged().detach(this, &ViewOpenGLThread::handleDataChange);
		m_spViewPointProcess = 0;
	}

	//destory all fillers. wait until they finish.
	m_viewPointFiller.reset();

	//and now it's safe to destory all components
	m_rootComponent = NULL;
	m_zoomDriftController = NULL;
	m_zoomRangeController = NULL;
	m_cameraControler = NULL;
	m_rhombusRenderer = NULL;

	//TODO: change this into non static
	uninitGlobalGLResources();	
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Global OpenGL Resources
//
//////////////////////////////////////////////////////////////////////////////////////


void ViewOpenGLThread::initGlobalGLResources()
{
	//TODO: this is not safe - because the GLUtils is createing font textures that is context dependtent and store them in a static class - bad!!!
	GLUtils::init();
}


void ViewOpenGLThread::uninitGlobalGLResources()
{
	//TODO: this is not safe - because the GLUtils is createing font textures that is context dependtent and store them in a static class - bad!!!
	GLUtils::uninit();
}


void ViewOpenGLThread::releaseOpenGLResources()
{
	for(unsigned int i=0;i<m_rootComponent->getChildren().size();i++)
	{
		m_rootComponent->getChildren()[i]->releaseOpenGLResources();
	}	
}

///////////////////////////////////////////////////////////////////////////////////////
//
// calcAppropriateResolution
//
//////////////////////////////////////////////////////////////////////////////////////


int ViewOpenGLThread::calcAppropriateResolution(const Camera& cam, int nHeight)
{
	// This code assumes an orbital camera with altitude 0.
	int nRes = 2;

	double dist = cam.getOrbitalRange()/SphereMath::knEarthRadius;

	// Pixel height in arcradians
	double pixelrad = cml::rad(cam.getFovy()/nHeight);

	// Pixel height at earth surface
	double earthrad = pixelrad * dist;

	// We want one pixel to correspond to one cell
	nRes = SnyderProjection::getInstance()->precisionToResolution(earthrad) - 9;

	if (nRes < 2)
	{
		nRes = 2;
	}
	else if (28 < nRes)
	{
		nRes = 28;
	}

	return nRes;
}

void ViewOpenGLThread::updateViewResolution()
{
	vec3 eyePos = getCamera().getEye();
	int nCalcRes = calcAppropriateResolution(getCamera(), m_nHeight);

	if (m_nRes != nCalcRes)	
	{
		m_nRes = nCalcRes;
		m_view.setResolution(nCalcRes);
		PerformanceCounter::getValuePerformanceCounter("Resolution")->setMeasurement(m_nRes);
	}
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Render
//
//////////////////////////////////////////////////////////////////////////////////////

void ViewOpenGLThread::setupRender()
{	
	copyViewState();

	PerformanceCounter::getTimePerformanceCounter("start Update camera",0.0f,1.0f,0.0f)->makeMeasurement();

	setState("update Camera");
	bool surfaceNeedUpdate = m_cameraAnimator.updateCamera();

	//update the camera aspect ratio to match current viewPort size.
	updateCameraAspect();

	mat4 modelView;
	mat4 projection;
	m_cameraAnimator.getCamera().getModelViewMatrix(modelView);
	m_cameraAnimator.getCamera().getProjectionMatrix(projection);
	m_cameraProjection = projection * modelView;

	PerformanceCounter::getTimePerformanceCounter("start Update visible tiles",0.0f,1.0f,1.0f)->makeMeasurement();

	setFrameTimeMeasurement("setup-camera");

	//make sure no one change our process
	if (ProcRef(m_spViewPointProcess) != m_procref)
	{
		setViewPointProcess(m_spViewPointProcess);
		m_procref=ProcRef(m_spViewPointProcess);
		surfaceNeedUpdate=true;
	}

	setState("update Surface");

	if (! m_config.m_bOptLockGrid)
	{
		auto surfaceRefined = refineSurface();

		setFrameTimeMeasurement("setup-terrain-refine");

		if (surfaceRefined || surfaceNeedUpdate)
		{
			PerformanceCounter::getTimePerformanceCounter("start modify surface",0.0f,1.0f,1.0f)->makeMeasurement();
			modifySurfaceFromCamera();
		}
	}
	
	setFrameTimeMeasurement("setup-terrain-update");

	PerformanceCounter::getTimePerformanceCounter("start find pointer location",0.0f,0.0f,1.0f)->makeMeasurement();

	std::map<std::string,long> status = MemoryManager::getInstance()->getMemoryStatus();

	//PerformanceCounter::getValuePerformanceCounter("Mem usage",1.0f,1.0f,0.0f)->setMeasurement((int)MemoryManager::getInstance()->getMemoryUsagePercent());
	
	const int MB = 1024*1024;
	for(std::map<std::string,long>::iterator it = status.begin();it != status.end(); ++it)
	{
		if (it->second > 10*MB)
		{
			if (boost::starts_with(it->first,"class") || boost::starts_with(it->first,"struct"))
			{
				PerformanceCounter::getValuePerformanceCounter(it->first,1.0f,0.5f,1.0f)->setMeasurement((int)(it->second/MB));
			}
		}
	}

	PerformanceCounter::getValuePerformanceCounter("Mem:free",0.0f,0.8f,0.0f)->setMeasurement((int)(status["free"]/MB));
	PerformanceCounter::getValuePerformanceCounter("Mem:sqlite",0.6f,0.6f,0.0f)->setMeasurement((int)(status["sqlite"]/MB));
	PerformanceCounter::getValuePerformanceCounter("Mem:managed",1.0f,1.0f,0.0f)->setMeasurement((int)(status["managed"]/MB));
	PerformanceCounter::getValuePerformanceCounter("Mem:other",0.6f,0.6f,0.0f)->setMeasurement((int)(status["other"]/MB));

	if (m_needToFindPointerLocation)
	{
		updatePointerLocation();
	}

	updateViewResolution();
}


void ViewOpenGLThread::updateCameraAspect()
{
	double aspect = static_cast<double>(m_nWidth) / m_nHeight;
	m_cameraAnimator.setAspect(aspect);	
}

bool ViewOpenGLThread::refineSurface()
{
	bool wasChange = false;
	PerformanceCounter::getValuePerformanceCounter("Need Divided",1.0f,1.0f,1.0f)->setMeasurement(getSurface()->getPatchesNeedDividing().size());

	const unsigned int maxSurfaceToDividePerFrame = 5;

	//divide on patch
	if (getSurface()->getPatchesNeedDividing().size() > 0)
	{
		for(int i=0;i<maxSurfaceToDividePerFrame;i++)
		{
			if (! (i<static_cast<int>(getSurface()->getPatchesNeedDividing().size())))
			{
				//no more patches to divide
				break;
			}
			PYXPointer<Surface::Patch> patchToDivide = getSurface()->getPatchesNeedDividing()[i];
			if (patchToDivide != NULL)
			{
				PYXPointer<Surface::Patch::VertexBuffer> patchVertices = getElevations()->get(patchToDivide)->getData();

				//divide patch
				patchToDivide->divide();
				patchToDivide->removeVisiblityBlock();
				wasChange = true;

				//start elevation processing
				for(int i=0;i<9;i++)
				{
					PYXPointer<Surface::Patch> subPatch = patchToDivide->getSubPatch(i);
					getElevations()->get(subPatch)->validate();
				}
			}
		}
	}

	//Unify one patch
	if (getSurface()->getPatchesNeedUnifing().size() > 0)
	{
		int unifiedCount = std::min(getSurface()->getPatchesNeedUnifing().size(),maxSurfaceToDividePerFrame);

		for(int i=0;i<unifiedCount;i++)
		{
			getSurface()->getPatchesNeedUnifing()[i]->unify();
			wasChange = true;
		}
	}

	return wasChange;
}

void ViewOpenGLThread::modifySurfaceFromCamera()
{
	//refresh the surface
	//we must do it after unifying and dividing to include the new rhombus into the frame
	getSurface()->findVisiblePatchs(getCamera(),getViewportHeight());
}

//! render the globe - invoked from View::display 
void ViewOpenGLThread::render()
{
	PerformanceCounter::getTimePerformanceCounter("startRendering",0.0f,1.0f,0.0f)->makeMeasurement();

	internalRender();
}

//! called by the View class when the visibleTiles is needed.
void ViewOpenGLThread::copyViewState()
{
	//copy the config
	m_config = m_view.m_config;
}


//! the actual rendering function
void ViewOpenGLThread::internalRender()
{	
	//make sure we have the right viewport
	glViewport(0, 0, m_nWidth, m_nHeight);
	
	// Eye altitude in metres
	vec3 eyePos = getCamera().getEye();
	double eyeAlt = (eyePos.length() - 1) * SphereMath::knEarthRadius;

	if (m_config.m_bOptDrawBackground)
	{
		internalRenderBackground((int)eyeAlt);
	}
	else 
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	if (m_config.m_bOptShowWorldAxes)
	{
		applyCameraWorld();

		glLineWidth(2);
		glBegin(GL_LINES);
		glColor3d(0.25, 0, 0); glVertex3d(0, 0, 0); glVertex3d(-10, 0, 0);
		glColor3d(1, 0, 0);    glVertex3d(0, 0, 0); glVertex3d(10, 0, 0);

		glColor3d(0, 0.25, 0); glVertex3d(0, 0, 0); glVertex3d(0, -10, 0);
		glColor3d(0, 1, 0);    glVertex3d(0, 0, 0); glVertex3d(0, 10, 0);

		glColor3d(0, 0, 0.25); glVertex3d(0, 0, 0); glVertex3d(0, 0, -10);
		glColor3d(0, 0, 1);    glVertex3d(0, 0, 0); glVertex3d(0, 0, 10);
		glEnd();
		glLineWidth(1);
	}

	m_rootComponent->render();
	
	setState("Done Rendering");

	boost::intrusive_ptr<IViewPoint> spViewPoint = 
		boost::dynamic_pointer_cast<IViewPoint>(m_spViewPointProcess);

	if (m_config.m_bOptShowDiagnostic)
	{
		/*
		GLUtils::drawStringAt(16, m_nHeight - 16*3, std::string("res: ") + 
			StringUtils::toString(m_nRes) + "/" + StringUtils::toString(m_nRes+5) + "/" + 
			StringUtils::toString(m_nRes+11));
		GLUtils::drawStringAt(16, m_nHeight - 16*4, std::string("tiles: ") + 
			StringUtils::toString(m_visibleTiles.size()) + " cache: " + 
			StringUtils::toString(STile::getCount(m_procref)));
		*/

		// TODO all of this is currently incorrect since camera representation change
		//GLUtils::drawStringAt(16, 120, std::string("looking at ") + calcLookAtCell(m_cam, gridres + 7).toString());
		//GLUtils::drawStringAt(16, 80, m_cam.getPerspectiveString());
		//GLUtils::drawStringAt(16, 60, m_cam.getEyeString());
		//GLUtils::drawStringAt(16, 40, m_cam.getCenterString());
		//GLUtils::drawStringAt(16, 20, m_cam.getUpString());

		// Camera settings
		double lat, lon, alt, rng, tlt, hdg = 0;
		camToSettings(getCamera(), lat, lon, hdg, alt, tlt, rng);
		GLUtils::drawStringAt(16, 200, std::string("lat: ") + StringUtils::toString(lat));
		GLUtils::drawStringAt(16, 180, std::string("lon: ") + StringUtils::toString(lon));
		GLUtils::drawStringAt(16, 160, std::string("hdg: ") + StringUtils::toString(hdg));
		GLUtils::drawStringAt(16, 140, std::string("alt: ") + StringUtils::toString(alt));
		GLUtils::drawStringAt(16, 120, std::string("tlt: ") + StringUtils::toString(tlt));
		GLUtils::drawStringAt(16, 100, std::string("rng: ") + StringUtils::toString(rng));


		PYXIcosIndex index;
		char buf[64];
		/*
		index = getPointerIndex(m_nRes + 0);
		sprintf_s(buf, sizeof(buf), "Grid 0   %s", index.toString().c_str());
		GLUtils::drawStringAt(0, 64, buf);
		index = getPointerIndex(m_nRes + 2);
		sprintf_s(buf, sizeof(buf), "Grid 2   %s", index.toString().c_str());
		GLUtils::drawStringAt(0, 48, buf);
		index = getPointerIndex(m_nRes + 4);
		sprintf_s(buf, sizeof(buf), "Grid 4   %s", index.toString().c_str());
		GLUtils::drawStringAt(0, 32, buf);
		index = getPointerIndex(m_nRes + View::knDataResOffset);
		sprintf_s(buf, sizeof(buf), "Grid 11  %s", index.toString().c_str());
		GLUtils::drawStringAt(0, 16, buf);
		*/

		index = getPointerIndex(m_pointerResolution);
		sprintf_s(buf, sizeof(buf), "Current Vertex %s", index.toString().c_str());
		GLUtils::drawStringAt(0, 32, buf);

		index = getPointerIndex(m_pointerResolution + 8);
		sprintf_s(buf, sizeof(buf), "Current Cell   %s", index.toString().c_str());
		GLUtils::drawStringAt(0, 16, buf);

		PYXPointer<Surface::Patch> patch = getPointerPatch();
		if (patch)
		{
			sprintf_s(buf, sizeof(buf), "Current Patch %s", patch->getKey().toString().c_str());
			GLUtils::drawStringAt(0, 48, buf);
		}
		/*
		PYXPointer<Surface::Patch> patch = getPointerPatch();		
		if (patch)
		{
			PYXPointer<Surface::Patch> patchParent = patch->getParent();
			if (patchParent)
			{
				sprintf_s(buf, sizeof(buf), "Patch Prio %s %s (Parent %s %s)", 
					StringUtils::toString(patch->getPriority()).c_str(),
					StringUtils::toString(patch->isVisible()).c_str(),
					StringUtils::toString(patchParent->getPriority()).c_str(),
					StringUtils::toString(patchParent->isVisible()).c_str());
			}
			else
			{
				sprintf_s(buf, sizeof(buf), "Patch Prio %s %s", 
					StringUtils::toString(patch->getPriority()).c_str(),
					StringUtils::toString(patch->isVisible()).c_str());
			}
			GLUtils::drawStringAt(0, 48, buf);
		}
		*/


		sprintf_s(buf, sizeof(buf), "%d %d %d %d", m_nMouseX, m_nMouseY, m_nWidth, m_nHeight);
		GLUtils::drawStringAt(0, 240, buf);

		sprintf_s(buf, sizeof(buf), "Render time: %d (Processing %d)", m_view.m_totalRenderTime,m_view.m_postProcessingTime);
		GLUtils::drawStringAt(0, 256, buf);

		if (!m_view.m_strError.empty())
		{
			GLUtils::drawStringAt(16, 260, m_view.m_strError);
		}		
	}

	if (m_config.m_bOptShowStatus)
	{
		// Eye alt status
		{
			char buf[32];
			if (eyeAlt < 1000)
			{
				sprintf_s(buf, sizeof(buf), "Eye Alt %d m", static_cast<int>(eyeAlt + 0.5));
			}
			else
			{
				sprintf_s(buf, sizeof(buf), "Eye Alt %.2f km", eyeAlt/1000);
			}
			//GLUtils::drawStringAt(m_nWidth - (16*static_cast<int>(strlen(buf))), 0, buf);			
			std::string strEyeAlt(buf, strlen(buf));
			m_view.setEyeAlt(strEyeAlt);
		}
	}

	//reset viewpoint
	glViewport(0, 0, m_nWidth, m_nHeight);
}

void ViewOpenGLThread::internalRenderBackground(int eyeAlt)
{
	{
		// TODO this atmosphere calculation is reasonable but probably could be improved.
		double r = 0;
		double g = 0;
		double b = 0;
		if (m_config.m_bOptShowAtmosphere)
		{
			r = 135/255.0;
			g = 206/255.0;
			b = 235/255.0;
			if (eyeAlt < 100000)
			{
				// blue
			}
			else if (eyeAlt < 150000)
			{
				// lerp
				double k = (150000 - eyeAlt) / 50000;
				r *= k;
				g *= k;
				b *= k;
			}
			else
			{
				// black
				r = g = b = 0;
			}
		}
		glClearColor(
			static_cast<GLclampf>(r), 
			static_cast<GLclampf>(g), 
			static_cast<GLclampf>(b), 
			1);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if ((!m_config.m_bOptShowAtmosphere || 125000 < eyeAlt) )
	{
		applyCameraWorld(true);
#if 0
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glVertexPointer(3, GL_DOUBLE, 6 * sizeof(double), &starsmag4[0]);
		glColorPointer(3, GL_DOUBLE, 6 * sizeof(double), &starsmag4[3]);
		glDrawArrays(GL_POINTS, 0, 518);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
#else
		glScaled(100, 100, 100);
#if 0
		// Use single color
		glColor3d(1, 1, 1);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 4 + 3 * sizeof(GLfloat), ((char*)stardata)+4);
		glDrawArrays(GL_POINTS, 0, starcount);
		glDisableClientState(GL_VERTEX_ARRAY);
#else
		// Use color in arrays
	
		glInterleavedArrays(GL_C4UB_V3F, 0, stardata);
		glDrawArrays(GL_POINTS, 0, starcount);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

#endif
#endif
	}

#if 0
	// PROTOTYPE rendering atmospheric glow
	// Works pretty well except when tilted and zooming in
	// May need to solve for ellipse (not circle)
	// Should render glow with some transparency
	if (getOptShowAtmosphere() && 125000 < eyeAlt)
	{
		applyCamera();
		glTranslated(0, 0, -1); // add more altitude
		glRotated(m_cam.getOrbitalTilt(), 1, 0, 0); // undo tilt
		double z = 1 / m_cam.getEye().length(); // solve for tangent z
		double si = sqrt(1 - z*z); // inner scale factor
		double so = si * (1 + 200000/SphereMath::knEarthRadius); // outer scale factor (at 200km)
		glBegin(GL_QUAD_STRIP);
		for (int nDeg = 0; nDeg != 361; ++nDeg)
		{
			double fRad = cml::rad(static_cast<double>(nDeg));
			glColor3d(0/255.0, 178/255.0, 238/255.0); // TODO choose better color
			glVertex3d(si * cos(fRad), si * sin(fRad), z);
			glColor3d(0, 0, 0);
			glVertex3d(so * cos(fRad), so * sin(fRad), z);
		}
		glEnd();
	}
#endif
}


//! called by the View class when the visibleTiles is needed.
void ViewOpenGLThread::copyVisiblieTilesToView()
{
	Surface::PatchVector & visible = getSurface()->getVisiblePatches();
	Surface::PatchVector::iterator it = visible .begin();

	std::set<PYXIcosIndex> visibleCells;
	while (it != visible.end())
	{
		visibleCells.insert((*it)->getRhombus().getIndex(0));
		visibleCells.insert((*it)->getRhombus().getIndex(1));
		visibleCells.insert((*it)->getRhombus().getIndex(2));
		visibleCells.insert((*it)->getRhombus().getIndex(3));
		++it;
	}

	m_view.m_vecTiles.clear();
	for(std::set<PYXIcosIndex>::iterator it = visibleCells.begin(); it != visibleCells.end(); it++)
	{
		m_view.m_vecTiles.push_back(*it);
	}
}



void ViewOpenGLThread::internalReshape(int nWidth, int nHeight)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;
   
	//noify with a fake event...
	m_resizeNotifier.notify(UIEvent::create(false,false,false));
}

///////////////////////////////////////////////////////////////////////////////////////
//
// HotKey
//
//////////////////////////////////////////////////////////////////////////////////////

#ifdef INSTANCE_COUNTING

void takeSnapshot()
{
	static InstanceCounter::InstanceCountMap s_snapshot1;
	static InstanceCounter::InstanceCountMap s_snapshot2;
	static bool hadSnapshot = false;

	if (hadSnapshot)
	{
		s_snapshot2 = InstanceCounter::takeSnapShot();
		InstanceCounter::traceObjectCountChange(s_snapshot1,s_snapshot2);
		s_snapshot1 = s_snapshot2;
	}
	else 
	{
		s_snapshot1 = InstanceCounter::takeSnapShot();
		hadSnapshot = true;
	}
}

#endif

void ViewOpenGLThread::onHotKeyPressed(PYXPointer<NotifierEvent> event)
{
	HotKeyEvent * hotKeyEvent = dynamic_cast<HotKeyEvent*>(event.get());

	if (hotKeyEvent != NULL)
	{
		switch (hotKeyEvent->getChar())
		{
		case 'e':
		case 'E':
			{
				/*
				TilesCPURenderer * renderer = dynamic_cast<TilesCPURenderer*>(hotKeyEvent->getComponent().get());
				if (renderer != NULL)
				{
					renderer->setShowingRaster( ! renderer->isShowingRaster());
				}
				*/
			}
			break;			
		case 'v':
		case 'V':
			{
				/*
				TilesCPURenderer * renderer = dynamic_cast<TilesCPURenderer*>(hotKeyEvent->getComponent().get());
				if (renderer != NULL)
				{
					renderer->setShowingVectors( ! renderer->isShowingVectors());
				}
				*/
			}
			break;
		case 'r':
		case 'R':
			{
				hotKeyEvent->getComponent()->setVisible(! hotKeyEvent->getComponent()->isVisible());				
			}
			break;
		case 'l':
		case 'L':
			{
				RhombusRenderer * renderer = dynamic_cast<RhombusRenderer*>(hotKeyEvent->getComponent().get());
				renderer->setRenderLinesOnly(!renderer->getRenderLinesOnly());
			}
			break;
		case '0':
			{
				RhombusRenderer * renderer = dynamic_cast<RhombusRenderer*>(hotKeyEvent->getComponent().get());
				renderer->setGridAlpha(0);
			}
			break;
		case '1':
			{
				RhombusRenderer * renderer = dynamic_cast<RhombusRenderer*>(hotKeyEvent->getComponent().get());
				renderer->setGridAlpha(50);
			}
			break;
		case '2':
			{
				RhombusRenderer * renderer = dynamic_cast<RhombusRenderer*>(hotKeyEvent->getComponent().get());
				renderer->setGridAlpha(100);
			}
			break;

		default:
			hotKeyEvent->getComponent()->setVisible( ! hotKeyEvent->getComponent()->isVisible() );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////
//
// ViewPointProcess
//
//////////////////////////////////////////////////////////////////////////////////////

boost::intrusive_ptr<IProcess> ViewOpenGLThread::getViewPointProcess()
{
	return m_spViewPointProcess;
}

boost::intrusive_ptr<IProcess> ViewOpenGLThread::getElevationOutputProcess()
{
	boost::intrusive_ptr<IProcess> channelSelector = 
		PipeUtils::createProcess("{16B34600-CB76-4613-B3B4-7D1B3C4FB499}")
		.addInput(0,m_spViewPointProcess)
		.setAttribute("Channel_Selected","1")
		.setProcName("elevation output")
		.setProcDescription("elevation output")
		.getProcess(true);

	return channelSelector;
}

void ViewOpenGLThread::handleDataChange( PYXPointer<NotifierEvent> spEvent)
{
	PYXPointer<ProcessDataChangedEvent> processDataChangedEvent =
		boost::dynamic_pointer_cast<ProcessDataChangedEvent>(spEvent);
}

void ViewOpenGLThread::setViewPointProcess(boost::intrusive_ptr<IProcess> spViewPointProcess)
{
	MEASURE_FUNC_CALL("setViewPointProcess");
	
	if (!spViewPointProcess)
	{
		PYXTHROW(PYXVisualizationException, "Can't set ViewPointProcess to null!");
	}

	if(m_spViewPointProcess)
	{
		m_spViewPointProcess->getDataChanged().detach(this, &ViewOpenGLThread::handleDataChange);
	}

	m_spViewPointProcess = spViewPointProcess;
	boost::intrusive_ptr<IViewPoint> spViewPoint = 
		boost::dynamic_pointer_cast<IViewPoint>(m_spViewPointProcess);

	if(m_spViewPointProcess->getInitState() != IProcess::knInitialized)
	{
		m_spViewPointProcess->initProc(true);
	}
	m_procref=ProcRef(m_spViewPointProcess);

	m_spViewPointProcess->getDataChanged().attach(this, &ViewOpenGLThread::handleDataChange);			

	//m_VisibleStore->setViewPointProcess(spViewPoint);

	m_viewPointFiller->validateViewpoint(m_spViewPointProcess);	
	m_viewPortProcessChangeNotifier.notify(ProcessEvent::create(spViewPointProcess));
}


///////////////////////////////////////////////////////////////////////////////////////
//
// Apply Camera 
//
//////////////////////////////////////////////////////////////////////////////////////

const Camera & ViewOpenGLThread::getCamera() const
{
	return m_cameraAnimator.getCamera();
}

Camera & ViewOpenGLThread::getCamera() 
{
	return m_cameraAnimator.getCamera();;
}



void ViewOpenGLThread::applyCameraPerspective(bool bInfinite)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	double nearZ = getCamera().getNearz();
	double farZ = bInfinite? std::max(getCamera().getFarz()*100,100.0) : getCamera().getFarz();
	gluPerspective(getCamera().getFovy(), getCamera().getAspect(), nearZ , farZ);
}

//! apply OpenGL matrixes from current camera - shoudl be called from renderers only
void ViewOpenGLThread::applyCamera()
{
	applyCameraPerspective();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	/*
	if (getCamera().getMode() == 0)
	{
		double eye[3];
		double center[3];
		double up[3];

		getCamera().getEye(eye);
		getCamera().getCenter(center);
		getCamera().getUp(up);

		gluLookAt(
			eye[0], eye[1], eye[2],
			center[0], center[1], center[2],
			up[0], up[1], up[2]);
	}
	else 
	*/
	if (getCamera().getMode() == 1)
	{
		vec3 axis;
		double angle;
		cml::quaternion_to_axis_angle(getCamera().getOrbitalRotation(), axis, angle, DBL_EPSILON);

		glTranslated(0, 0, -getCamera().getOrbitalRange()/SphereMath::knEarthRadius);            // range
		glRotated(getCamera().getOrbitalTilt(), -1, 0, 0);                     // tilt
		
		// NOTE don't do the final rotation as we'll use the matrix stack to manage local coordinate transforms
		//glTranslated(0, 0, /*-1*/ - getCamera().getOrbitalAltitude()/SphereMath::knEarthRadius); // altitude

		// NOTE don't do the final rotation as we'll use the matrix stack to manage local coordinate transforms
		//glRotatef(cml::deg(angle), axis[0], axis[1], axis[2]);         // orientation
	}
}

//! apply OpenGL matrixes from current camera - shoudl be called from renderers only
void ViewOpenGLThread::applyCameraWorld(bool bInfinite)
{
    applyCameraPerspective(bInfinite);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	/*
	if (getCamera().getMode() == 0)
	{
		double eye[3];
		double center[3];
		double up[3];

		getCamera().getEye(eye);
		getCamera().getCenter(center);
		getCamera().getUp(up);

		gluLookAt(
			eye[0], eye[1], eye[2],
			center[0], center[1], center[2],
			up[0], up[1], up[2]);
	}
	else 
	*/
	if (getCamera().getMode() == 1)
	{
		vec3 axis;
		double angle;
		cml::quaternion_to_axis_angle(getCamera().getOrbitalRotation(), axis, angle, DBL_EPSILON);

		if (!bInfinite)
		{
			glTranslated(0, 0, -getCamera().getOrbitalRange()/SphereMath::knEarthRadius);        // range
		}
		glRotated(getCamera().getOrbitalTilt(), -1, 0, 0);                     // tilt
		if (!bInfinite)
		{
			glTranslated(0, 0, -1 - getCamera().getOrbitalAltitude()/SphereMath::knEarthRadius); // altitude
		}
		glRotated(cml::deg(angle), axis[0], axis[1], axis[2]);           // orientation
	}
}

PYXCoord2DDouble ViewOpenGLThread::projectToScreenSpace(PYXCoord3DDouble xyz)
{
	auto screenVec = m_cameraProjection * vec4(xyz.x(),xyz.y(),xyz.z(),1.0);
	return PYXCoord2DDouble((1 + screenVec[0] / screenVec[3]) / 2  *getViewportWidth(),(1 - screenVec[1] / screenVec[3]) / 2  *getViewportHeight());
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Mouse Selection
//
//////////////////////////////////////////////////////////////////////////////////////

void ViewOpenGLThread::cameraChanged(PYXPointer<NotifierEvent> event)
{
	m_needToFindPointerLocation = true;
}

bool ViewOpenGLThread::findRayIntersection(const Ray & ray,vec3 & intersection)
{
	PYXPointer<Surface::Patch> intersectedPatch;
	double minTime = 0;

	Surface::PatchVector & visible = getSurface()->getVisiblePatches();
	Surface::PatchVector::iterator it = visible.begin();

	while(it != visible.end())
	{
		//fast look up first...
		if (ray.intersectsWithSphere((*it)->getBBox().getCenter(),(*it)->getBBox().getRadius()))
		{
			PYXPointer<Surface::Patch::VertexBuffer> mesh = (*it)->vertices;
			if (mesh)
			{
				double time = 0;
				if (mesh->intersects(ray,time))
				{
					if (!intersectedPatch || time < minTime)
					{
						minTime = time;
						intersectedPatch = *it;
					}
				}
			}
		}
		++it;
	}

	if (intersectedPatch)
	{
		intersection = ray.getPointFromTime(minTime);
		return true;
	}

	return false;
}

void ViewOpenGLThread::findPointerLocation()
{
	Ray ray = getMouseRay();

	PYXPointer<Surface::Patch> intersectedPatch;
	double minTime = 0;

	Surface::PatchVector & visible = getSurface()->getVisiblePatches();
	Surface::PatchVector::iterator it = visible.begin();

	while(it != visible.end())
	{
		//fast look up first...
		if (ray.intersectsWithSphere((*it)->getBBox().getCenter(),(*it)->getBBox().getRadius()))
		{
			PYXPointer<Surface::Patch::VertexBuffer> mesh = (*it)->vertices;
			if (mesh)
			{
				double time = 0;
				if (mesh->intersects(ray,time))
				{
					if (!intersectedPatch || time < minTime)
					{
						minTime = time;
						intersectedPatch = *it;
					}
				}
			}
		}
		++it;
	}

	if (intersectedPatch)
	{
		m_pointerLocation = ray.getPointFromTime(minTime);
		m_pointerResolution = intersectedPatch->getRhombus().getIndex(0).getResolution();
		m_pointerPatch = intersectedPatch;

		double elevation = getMeshElevation(CmlConvertor::toLatLon(m_pointerLocation));
		PerformanceCounter::getValuePerformanceCounter("Mesh Elevation",0.0f,1.0f,0.0f)->setMeasurement((int)(elevation/100+500));
	}
	else
	{
		m_pointerLocation.zero();
		m_pointerPatch.reset();
	}

	m_needToFindPointerLocation = false;
}

vec3 ViewOpenGLThread::getPointerLocationOnUnitShpere() const
{	
	double ndx, ndy;
	GLUtils::screenToNDC(m_nMouseX, m_nHeight-m_nMouseY, m_nWidth, m_nHeight, ndx, ndy);
	return projectPointToUnitSphere(getCamera(), ndx, ndy);
}

void ViewOpenGLThread::updatePointerLocation() 
{	
	setState("find pointer location");
	//find the pointer location
	findPointerLocation();

	setState("update pointer string");
	m_view.updatePointerString();	
}

const vec3 & ViewOpenGLThread::getPointerLocation() 
{	
	if (m_needToFindPointerLocation)
	{
		normal.invoke(&ViewOpenGLThread::updatePointerLocation);
	}
	return m_pointerLocation;
}

PYXIcosIndex ViewOpenGLThread::getPointerIndex(int nRes)
{
	PYXIcosIndex index;
	vec3 v = getPointerLocation();

	if (v[0] || v[1] || v[2])
	{
		// TODO this code seems to be yielding inaccurate results!

		v.normalize();

#if 0
		// Trying more stuff
		PYXCoord3DDouble coord(v[0], v[1], v[2]);
		CoordLatLon ll = SphereMath::xyzll(coord);
		SnyderProjection::getInstance()->nativeToPYXIS(ll, &index, nRes);
		
		CoordLatLon llgc(SphereMath::xyzll(PYXCoord3DDouble(v[0], v[1], v[2])));
		SnyderProjection::getInstance()->nativeToPYXIS(llgc, &index, nRes);
#endif

		SnyderProjection::getInstance()->xyzToPYXIS(PYXCoord3DDouble(v[0], v[1], v[2]), &index, nRes);
	}

	return index;
}


///////////////////////////////////////////////////////////////////////////////////////
//
// getMeshElevation
//
//////////////////////////////////////////////////////////////////////////////////////

double ViewOpenGLThread::getMeshElevation(const CoordLatLon & latLon)
{
	double elevation = 0;

	Ray ray(vec3(0,0,0),CmlConvertor::toVec3(latLon));

	Surface::PatchVector::iterator it = getSurface()->begin();

	while (it != getSurface()->end())
	{
		if (findMeshElevation(*it,ray,elevation))
		{
			return elevation;
		}
		++it;
	}

	//if we can't find elevation - return zero elevation;
	return 0;
}

double ViewOpenGLThread::getMeshElevation(const PYXPointer<Surface::Patch> & patch,const vec2 & uv)
{
	vec2 currentUV(uv);
	double elevation = 0;

	PYXPointer<Surface::Patch> it = patch;

	while(it->isVisible())
	{
		if (it->hasVisiblityBlock() || !it->isDivided())
		{
			break;
		}

		int zoomIndex = Surface::UVOffset::uvToIndex(currentUV);
		PYXPointer<Surface::Patch> child = it->getSubPatch(zoomIndex);
		
		if (!child->isVisible())
		{
			//oops, we gone to far, go back
			break;
		}
		it = child;
		currentUV = Surface::UVOffset::zoomIn(zoomIndex,currentUV);
	}

	PYXPointer<Surface::Patch::VertexBuffer> mesh = it->getVertices();
	if (mesh)
	{
		elevation = mesh->getElevation(currentUV);
		return (elevation-1)*SphereMath::knEarthRadius;
	}

	//if we can't find elevation - return zero elevation;
	return 0;
}

bool ViewOpenGLThread::findMeshElevation(const PYXPointer<Surface::Patch> & patch,const Ray & ray,double & elevation)
{
	if (ray.intersectsWithSphere(patch->getBBox().getCenter(),patch->getBBox().getRadius()))
	{
		//go deeper...
		if (patch->isDivided() && ! patch->hasVisiblityBlock())
		{
			for(int i=0;i<9;i++)
			{
				if (findMeshElevation(patch->getSubPatch(i),ray,elevation))
				{
					return true;
				}
			}
			return false;
		}
		else //check intersection on this patch
		{
			PYXPointer<Surface::Patch::VertexBuffer> mesh = patch->getVertices();

			if (mesh) 
			{
				double time = 0;
				if (mesh->intersects(ray,time))
				{
					//convert the answer into meters;
					elevation = (time-1)*SphereMath::knEarthRadius;
					return true;
				}
			}
		}
	}
	return false;
}

void ViewOpenGLThread::setPOI(const PYXPointer<PYXGeometry> & geometry)
{
	if (m_extraFeaturesPipeline)
	{
		m_spViewPointProcess->getParameter(1)->removeValue(m_spViewPointProcess->getParameter(1)->findValue(m_extraFeaturesPipeline));
		m_extraFeaturesPipeline.reset();
	}

	
	if (!geometry)
	{
		if (m_POI)
		{
			//we need to notify that there is no POI 
			setViewPointProcess(m_spViewPointProcess);
			m_POI = geometry;
			m_view.safeNotify(m_view.getPOIChangeNotifier(),ViewEvent::create("new POI"));
		}
		return;
	}

	boost::intrusive_ptr<IProcess> process = getViewPointProcess();

	boost::intrusive_ptr<IViewPoint> viewPoint = process->QueryInterface<IViewPoint>();

	boost::intrusive_ptr<IProcess> fcProc = PYXCOMCreateInstance<IProcess>(strToGuid("{DEF42C63-F377-4065-8C58-7215FBA45222}"));
	assert(fcProc);

	boost::intrusive_ptr<IProcess> fcAreaStyleProc = PYXCOMCreateInstance<IProcess>(strToGuid("{A7B8FC7A-6041-4a15-BD97-7F4A976D91AD}"));
	assert(fcProc);

	boost::intrusive_ptr<IProcess> geometryProc = PYXCOMCreateInstance<IProcess>(strToGuid("{2309AE25-FE1A-482f-9642-D2ADD1520629}"));
	assert(geometryProc);

	boost::intrusive_ptr<IWritableFeature> geometryFeature = geometryProc->QueryInterface<IWritableFeature>();

	geometryFeature->setGeometry(geometry);

	fcProc->getParameter(0)->addValue(geometryProc);

	fcAreaStyleProc->getParameter(0)->addValue(fcProc);
	std::map<std::string,std::string> attributes;
	attributes["area_colour"] = "uint8_t[3] 250 250 100";
	fcAreaStyleProc->setAttributes(attributes);

	m_extraFeaturesPipeline = fcAreaStyleProc;
	if (m_extraFeaturesPipeline->initProc(true) != IProcess::knInitialized)
	{
		TRACE_INFO("Failed to initalize POI pipeline");

		//we need to notify that there is no POI 
		setViewPointProcess(m_spViewPointProcess);
		m_POI.reset();
		m_view.safeNotify(m_view.getPOIChangeNotifier(),ViewEvent::create("new POI"));
		return;
	}

	m_spViewPointProcess->getParameter(1)->addValue(m_extraFeaturesPipeline);

	setViewPointProcess(m_spViewPointProcess);

	m_POI = geometry;
	m_view.safeNotify(m_view.getPOIChangeNotifier(),ViewEvent::create("new POI"));
}

///////////////////////////////////////////////////////////////////////////////////////
//
// UI event sink
//
//////////////////////////////////////////////////////////////////////////////////////

void ViewOpenGLThread::pushActiveComponent(PYXPointer<Component> component)
{
	m_activeComponents.push_back(component);
}

void ViewOpenGLThread::popActiveComponent()
{
	if (m_activeComponents.size()>0)
	{
		m_activeComponents.pop_back();
	}
}

PYXPointer<Component> ViewOpenGLThread::getActiveComponent()
{
	if (m_activeComponents.size()>0)
	{
		return m_activeComponents.back();
	}
	return NULL;
}

void ViewOpenGLThread::onMouseClick(const int & x,const int & y,const int & delta,
				 const bool & leftButton,const bool & rightButton,const bool & middleButton,
				 const bool & altKey,const bool & shiftKey,const bool & ctrlKey)
{	
	//convert to OpenGL coords
	m_nMouseX = x;
	m_nMouseY = m_nHeight - y;

	PYXPointer<UIMouseEvent> event = UIMouseEvent::create(m_nMouseX,m_nMouseY,delta,leftButton,rightButton,middleButton,altKey,shiftKey,ctrlKey);

	PYXPointer<Component> active = getActiveComponent();
	PYXPointer<Component> lastActive = NULL;

	//Try active component vector first	
	while (active && active != lastActive)	
	{
		PYXPointer<UIMouseEvent> localEvent = event;
		//let the active controller to handle the event.	
		if (active->getParent())
		{
			localEvent = active->getParent()->propogateMouseEvent(localEvent);
		}

		active->onMouseClick(localEvent);
		
		if (localEvent->isConsumed())
		{
			return;
		}
		
		//check if the active controller remove itself. if that happen, give the active conrtoller a second change
		lastActive = active;
		active = getActiveComponent();
	}

	m_rootComponent->onMouseClick(event);

	if (shiftKey)
	{

		PYXCoord3DDouble coord = CmlConvertor::toPYXCoord3D(getPointerLocation());
		coord.normalize();

		if (m_radiusClick)
		{
			setPOI(PYXVectorGeometry::create(PYXCircleRegion::create(m_lastClickPos,SphereMath::distanceBetween(coord,m_lastClickPos)),getViewDataResolution()-4));
		}
		else {
			m_lastClickPos = coord;
		}

		m_radiusClick = !m_radiusClick;
	}
}

void ViewOpenGLThread::onMouseDoubleClick(const int & x,const int & y,const int & delta,
				 const bool & leftButton,const bool & rightButton,const bool & middleButton,
				 const bool & altKey,const bool & shiftKey,const bool & ctrlKey)
{
	//convert to OpenGL coords
	m_nMouseX = x;
	m_nMouseY = m_nHeight - y;

	PYXPointer<UIMouseEvent> event = UIMouseEvent::create(m_nMouseX,m_nMouseY,delta,leftButton,rightButton,middleButton,altKey,shiftKey,ctrlKey);

	PYXPointer<Component> active = getActiveComponent();
	PYXPointer<Component> lastActive = NULL;

	//Try active component vector first	
	while (active && active != lastActive)	
	{
		PYXPointer<UIMouseEvent> localEvent = event;
		//let the active controller to handle the event.	
		if (active->getParent())
		{
			localEvent = active->getParent()->propogateMouseEvent(localEvent);
		}

		active->onMouseDoubleClick(localEvent);
		
		if (localEvent->isConsumed())
		{
			return;
		}
		
		//check if the active controller remove itself. if that happen, give the active conrtoller a second change
		lastActive = active;
		active = getActiveComponent();
	}

	m_rootComponent->onMouseDoubleClick(event);
}

void ViewOpenGLThread::onMouseMove(const int & x,const int & y,const int & delta,
				 const bool & leftButton,const bool & rightButton,const bool & middleButton,
				 const bool & altKey,const bool & shiftKey,const bool & ctrlKey)
{
	//convert to OpenGL coords
	m_nMouseX = x;
	m_nMouseY = m_nHeight - y;

	m_needToFindPointerLocation = true;

	PYXPointer<UIMouseEvent> event = UIMouseEvent::create(m_nMouseX,m_nMouseY,delta,leftButton,rightButton,middleButton,altKey,shiftKey,ctrlKey);

	PYXPointer<Component> active = getActiveComponent();
	PYXPointer<Component> lastActive = NULL;

	//Try active component vector first	
	while (active && active != lastActive)	
	{
		PYXPointer<UIMouseEvent> localEvent = event;
		//let the active controller to handle the event.	
		if (active->getParent())
		{
			localEvent = active->getParent()->propogateMouseEvent(localEvent);
		}

		active->onMouseMove(localEvent);
		
		if (localEvent->isConsumed())
		{
			return;
		}

		//check if the active controller remove itself. if that happen, give the active conrtoller a second change
		lastActive = active;
		active = getActiveComponent();
	}

	m_rootComponent->onMouseMove(event);
}

void ViewOpenGLThread::onMouseUp(const int & x,const int & y,const int & delta,
				 const bool & leftButton,const bool & rightButton,const bool & middleButton,
				 const bool & altKey,const bool & shiftKey,const bool & ctrlKey)
{
	//convert to OpenGL coords
	m_nMouseX = x;
	m_nMouseY = m_nHeight - y;

	PYXPointer<UIMouseEvent> event = UIMouseEvent::create(m_nMouseX,m_nMouseY,delta,leftButton,rightButton,middleButton,altKey,shiftKey,ctrlKey);

	PYXPointer<Component> active = getActiveComponent();
	PYXPointer<Component> lastActive = NULL;

	//Try active component vector first	
	while (active && active != lastActive)	
	{
		PYXPointer<UIMouseEvent> localEvent = event;
		//let the active controller to handle the event.	
		if (active->getParent())
		{
			localEvent = active->getParent()->propogateMouseEvent(localEvent);
		}

		active->onMouseUp(localEvent);
		
		if (localEvent->isConsumed())
		{
			return;
		}		

		//check if the active controller remove itself. if that happen, give the active conrtoller a second change
		lastActive = active;
		active = getActiveComponent();
	}

	m_rootComponent->onMouseUp(event);
}

void ViewOpenGLThread::onMouseDown(const int & x,const int & y,const int & delta,
				 const bool & leftButton,const bool & rightButton,const bool & middleButton,
				 const bool & altKey,const bool & shiftKey,const bool & ctrlKey)
{
	//convert to OpenGL coords
	m_nMouseX = x;
	m_nMouseY = m_nHeight - y;	

	PYXPointer<UIMouseEvent> event = UIMouseEvent::create(m_nMouseX,m_nMouseY,delta,leftButton,rightButton,middleButton,altKey,shiftKey,ctrlKey);

	PYXPointer<Component> active = getActiveComponent();
	PYXPointer<Component> lastActive = NULL;

	//Try active component vector first	
	while (active && active != lastActive)	
	{
		PYXPointer<UIMouseEvent> localEvent = event;
		//let the active controller to handle the event.	
		if (active->getParent())
		{
			localEvent = active->getParent()->propogateMouseEvent(localEvent);
		}

		active->onMouseDown(localEvent);
		
		if (localEvent->isConsumed())
		{
			return;
		}		

		//check if the active controller remove itself. if that happen, give the active conrtoller a second change
		lastActive = active;
		active = getActiveComponent();
	}

	m_rootComponent->onMouseDown(event);
}

void ViewOpenGLThread::onMouseWheel(const int & x,const int & y,const int & delta,
				 const bool & leftButton,const bool & rightButton,const bool & middleButton,
				 const bool & altKey,const bool & shiftKey,const bool & ctrlKey)
{
	//convert to OpenGL coords
	m_nMouseX = x;
	m_nMouseY = m_nHeight - y;

	PYXPointer<UIMouseEvent> event = UIMouseEvent::create(m_nMouseX,m_nMouseY,delta,leftButton,rightButton,middleButton,altKey,shiftKey,ctrlKey);

	PYXPointer<Component> active = getActiveComponent();
	PYXPointer<Component> lastActive = NULL;

	//Try active component vector first	
	while (active && active != lastActive)	
	{
		PYXPointer<UIMouseEvent> localEvent = event;
		//let the active controller to handle the event.	
		if (active->getParent())
		{
			localEvent = active->getParent()->propogateMouseEvent(localEvent);
		}

		active->onMouseWheel(localEvent);
		
		if (localEvent->isConsumed())
		{
			return;
		}		

		//check if the active controller remove itself. if that happen, give the active conrtoller a second change
		lastActive = active;
		active = getActiveComponent();
	}

	m_rootComponent->onMouseWheel(event);
}

void perfromSnyderTests()
{
	Performance::HighQualityTimer timer;

	const SnyderProjection* snyder = SnyderProjection::getInstance();
	PYXIcosIndex index[5];
	PYXCoordPolar polar[5];
	char polarFace[5];
	index[0] = PYXIcosIndex("1-2010304050601030202");
	index[1] = PYXIcosIndex("1-2010506010200300201");
	index[2] = PYXIcosIndex("1-2000303020201010201");
	index[3] = PYXIcosIndex("1-2020202020204030300");
	index[4] = PYXIcosIndex("1-2050205000502030020");
	

	PYXCoord2DDouble native;

	timer.start();
	int count=0;
	for(int i=0;i<1000000;i++)
	{
		for(int j=0;j<5;j++)
		{
			snyder->pyxisToNative(index[j],&native);
			count++;
		}
	}
	timer.stop();

	TRACE_INFO("Snyder pyxis to Native " << timer.getTime() << " sec for " << count << " operations");

	timer.start();
	count=0;
	for(int i=0;i<1000000;i++)
	{
		for(int j=0;j<5;j++)
		{
			PYXIcosMath::indexToPolar(index[j],&polar[j], &polarFace[j]);			
			count++;
		}
	}
	timer.stop();

	TRACE_INFO("index to polar " << timer.getTime() << " sec for " << count << " operations");

	timer.start();
	count=0;
	for(int i=0;i<1000000;i++)
	{
		for(int j=0;j<5;j++)
		{
			PYXIcosMath::polarToIndex(polar[j], 20, polarFace[j], &index[j]);
			count++;
		}
	}
	timer.stop();

	TRACE_INFO("polar to index " << timer.getTime() << " sec for " << count << " operations");

	count = 0;
	timer.start();
	for(double x=-179;x<180;x+=0.1)
	{
		for(double y=-80;y<80;y+=0.1)
		{
			native.setX(x);
			native.setY(y);
			snyder->nativeToPYXIS(native, &index[0], 20);
			count++;
			if (count == 5000000)
			{
				break;
			}
		}
		if (count == 5000000)
		{
			break;
		}
	}

	timer.stop();

	TRACE_INFO("Snyder native to pyxis " << timer.getTime() << " sec for " << count << " operations");
}

boost::intrusive_ptr<IProcess> buildWatershedProcess(const boost::intrusive_ptr<IProcess> & elevationProcess, const PYXIcosIndex & location, GUID watershedProccessGuid) 
{
	auto watershedProcess = PYXCOMCreateInstance<IProcess>(watershedProccessGuid);
	auto geometry = PYXCOMCreateInstance<IWritableFeature,NamedGeometryProc>();

	geometry->setGeometry(PYXCell::create(location));

	watershedProcess->getParameter(0)->addValue(elevationProcess);
	watershedProcess->getParameter(1)->addValue(geometry->QueryInterface<IProcess>());

	if (watershedProcess->initProc() != IProcess::knInitialized)
	{
		return nullptr;
	}
	return watershedProcess;
}

PYXPointer<PYXGeometry> ViewOpenGLThread::calculateWatershed(const PYXIcosIndex & location)
{
	if (!m_viewPointFiller->hasElevation()) 
	{
		return nullptr;
	}
	
	auto watershedProcess = buildWatershedProcess(getElevationOutputProcess(),location,strToGuid("{4D367363-2421-4DF2-AD97-529A4EDDE0A8}"));
	if (!watershedProcess)
	{
		return nullptr;
	}

	auto result = watershedProcess->getOutput()->QueryInterface<IFeature>()->getGeometry();
	return result;	
}

PYXPointer<PYXGeometry> ViewOpenGLThread::calculateWatershedFlow(const PYXIcosIndex & location)
{
	if (!m_viewPointFiller->hasElevation()) 
	{
		return nullptr;
	}
	
	auto watershedProcess = buildWatershedProcess(getElevationOutputProcess(),location,strToGuid("{4BC01EC7-191B-4983-BB5C-3709B4F7AFC9}"));
	if (!watershedProcess)
	{
		return nullptr;
	}

	auto result = watershedProcess->getOutput()->QueryInterface<IFeature>()->getGeometry();
	return result;	
}


class MemoryMapper
{
private:
	struct Module
	{
		std::string name;
		void* from;
		void* to;
	};

	std::vector<Module> m_modules;
	std::map<DWORD,std::string> m_knownVTables;

public:
	MemoryMapper() {};

	void mapModules() {
		HANDLE hProcess = GetCurrentProcess();
		HMODULE hMods[1024];
		DWORD cbNeeded;
		
		if( EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
		{
			for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ )
			{
				std::string name;
				TCHAR szModName[MAX_PATH];
				MODULEINFO moduleInfo;

				// Get the full path to the module's file.

				if ( GetModuleFileNameEx( hProcess, hMods[i], szModName,
										  sizeof(szModName) / sizeof(TCHAR)))
				{
					auto wname = std::wstring(szModName);
					name = std::string(wname.begin(),wname.end());
				}

				if ( GetModuleInformation( hProcess, hMods[i], &moduleInfo,sizeof(MODULEINFO)))
				{
					Module m;
					m.name = name;
					m.from = moduleInfo.lpBaseOfDll;
					m.to = ((char*)moduleInfo.lpBaseOfDll+moduleInfo.SizeOfImage);
					m_modules.push_back(m);
				}
			}
		}
		CloseHandle( hProcess );
	}

	bool isValidAddress(void* address)
	{
		for(auto & module : m_modules)
		{
			if (module.from <= address && address < module.to)
			{
				return true;
			}
		}
		return false;
	}

	std::string getName(void * object)
	{
		DWORD vtableAddr = *((DWORD*)object);

		auto it = m_knownVTables.find(vtableAddr);
		if (it != m_knownVTables.end())
		{
			return it->second;
		}

		if (isValidAddress((void*)vtableAddr))
		{
			try
			{
				m_knownVTables[vtableAddr] = typeid(*(PYXObject*)object).name();
			}
			catch(...)
			{
				m_knownVTables[vtableAddr] = "unknown";
			}
		}
		else 
		{
			return "unknown";
		}

		return m_knownVTables[vtableAddr];
	}
};


void ViewOpenGLThread::onKeyPressed(const char & keyChar,const int & keyCode,const bool & altKey,const bool & shiftKey,const bool & ctrlKey)
{
	PYXPointer<UIKeyEvent> event = UIKeyEvent::create(keyChar,keyCode,altKey,shiftKey,ctrlKey);

#ifdef INSTANCE_COUNTING
	if (keyChar == 'Q' || keyChar == 'q')
	{
		takeSnapshot();
	}
#endif

#ifdef SNYDER_PERFORMNACE_TESTING
	if (keyChar == 'S' || keyChar == 's')
	{
		perfromSnyderTests();
	}
#endif

	if (keyChar == 'M' || keyChar == 'm')
	{
		std::map<std::string,long> status = MemoryManager::getInstance()->getMemoryStatus();

		for(std::map<std::string,long>::iterator it = status.begin();it != status.end(); ++it)
		{
			if (it->second/1024/1024 > 1)
			{
				TRACE_INFO(it->first << " = " << (it->second/1024/1024) << " MB"); 
			}
		}
	}

	if (keyChar == 'H' || keyChar == 'h')
	{
		MemoryMapper mapper;
		mapper.mapModules();

		HANDLE heap = GetProcessHeap();
		PROCESS_HEAP_ENTRY entry;
		entry.lpData = 0;

		long count = 0;
		std::map<long,long> blockSizeHistogram;
		std::map<long,int> blockSizeOverhead;
		std::map<std::string,long> usagePerClass;

		DWORD largestFreeBlock = 0;
		DWORD commited = 0;
		DWORD uncommited = 0;
		DWORD free = 0;
		DWORD used = 0;
		DWORD largeblocks = 0;

		while(HeapWalk(heap,&entry))
		{
			if (entry.wFlags & PROCESS_HEAP_ENTRY_BUSY)
			{
				count++;
				blockSizeHistogram[entry.cbData]++;
				blockSizeOverhead[entry.cbData] = entry.cbOverhead;
				
				used += entry.cbData + entry.cbOverhead;

				switch(entry.cbData)
				{
				case 24:
				case 28:
				case 44:
				case 48:
				case 104:
				case 112:
				case 172:
				case 456:
				case 816:
				case 2416:
				case 3864:
				case 8192:
				case 86400:
					{
						std::string name = mapper.getName(entry.lpData);
						usagePerClass[name] += entry.cbData + entry.cbOverhead;
					}
					break;
				}
			}
			else if (entry.wFlags & PROCESS_HEAP_UNCOMMITTED_RANGE)
			{
				if (largestFreeBlock < entry.cbData) {
					largestFreeBlock = entry.cbData;
				}
				free += entry.cbData;

				if (entry.cbData > 2*1024*1024)
				{
					largeblocks ++;
				}
			}
			else if (entry.wFlags & PROCESS_HEAP_REGION)
			{
				commited += entry.Region.dwCommittedSize;
				uncommited += entry.Region.dwUnCommittedSize;
			}
			else 
			{
				free += entry.cbData;
				if (largestFreeBlock < entry.cbData) {
					largestFreeBlock = entry.cbData;
				}

				if (entry.cbData > 2*1024*1024)
				{
					largeblocks ++;
				}
			}
		}

		TRACE_INFO("largest free heap block " << largestFreeBlock);
		TRACE_INFO("total commited " << commited);
		TRACE_INFO("total uncommited " << uncommited);
		TRACE_INFO("total heap size " << (commited+uncommited));

		TRACE_INFO("total used " << used);
		TRACE_INFO("total free " << free);
		TRACE_INFO("total heap size " << (used + free));
		TRACE_INFO("large blocks " << largeblocks);

		long totalSmallMallocs = 0;
		long totalMallocs = 0;
		for(auto & bin : blockSizeHistogram)
		{
			long blockSize = bin.first + blockSizeOverhead[bin.first];
			totalMallocs += blockSize * bin.second;
			if (blockSize * bin.second > 1024*1024) {
				TRACE_INFO("Block data size " << bin.first << " [ " << blockSizeOverhead[bin.first] << " ] allocated " << bin.second << " times");
			} else {
				totalSmallMallocs += blockSize * bin.second;
			}
		}

		TRACE_INFO("all other allocations allocated " << totalSmallMallocs << " bytes");
		TRACE_INFO("all allocations allocated " << totalMallocs << " bytes");

		for(auto & cls : usagePerClass)
		{
			TRACE_INFO(cls.first << " consumed " << cls.second << " bytes");
		}
	}

	PYXPointer<Component> active = getActiveComponent();
	PYXPointer<Component> lastActive = NULL;

	//Try active component vector first	
	while (active && active != lastActive)	
	{
		//let the active controller to handle the event.	
		active->onKeyPressed(event);
		
		if (event->isConsumed())
		{
			return;
		}

		//check if the active controller remove itself. if that happen, give the active conrtoller a second change
		lastActive = active;
		active = getActiveComponent();
	}

	m_rootComponent->onKeyPressed(event);
}

void ViewOpenGLThread::onKeyUp(const char & keyChar,const int & keyCode,const bool & altKey,const bool & shiftKey,const bool & ctrlKey)
{
	PYXPointer<UIKeyEvent> event = UIKeyEvent::create(keyChar,keyCode,altKey,shiftKey,ctrlKey);

	PYXPointer<Component> active = getActiveComponent();
	PYXPointer<Component> lastActive = NULL;

	//Try active component vector first	
	while (active && active != lastActive)	
	{
		//let the active controller to handle the event.	
		active->onKeyUp(event);
		
		if (event->isConsumed())
		{
			return;
		}

		//check if the active controller remove itself. if that happen, give the active conrtoller a second change
		lastActive = active;
		active = getActiveComponent();
	}

	m_rootComponent->onKeyUp(event);
}

void ViewOpenGLThread::onKeyDown(const char & keyChar,const int & keyCode,const bool & altKey,const bool & shiftKey,const bool & ctrlKey)
{
	PYXPointer<UIKeyEvent> event = UIKeyEvent::create(keyChar,keyCode,altKey,shiftKey,ctrlKey);

	PYXPointer<Component> active = getActiveComponent();
	PYXPointer<Component> lastActive = NULL;

	//Try active component vector first	
	while (active && active != lastActive)	
	{
		//let the active controller to handle the event.	
		active->onKeyDown(event);
		
		if (event->isConsumed())
		{
			return;
		}

		//check if the active controller remove itself. if that happen, give the active conrtoller a second change
		lastActive = active;
		active = getActiveComponent();
	}

	m_rootComponent->onKeyDown(event);
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Delayed notifications
//
//////////////////////////////////////////////////////////////////////////////////////

//! used by safeNotify to store delayed events
void ViewOpenGLThread::addDelayedEvent(Notifier & notifier,PYXPointer<NotifierEvent> spEvent)
{
	DelayedNotifyEvent delayedEvent;
	delayedEvent.m_notifier = &notifier;
	delayedEvent.m_spEvent = spEvent;
	m_delayedEvents.push_back(delayedEvent);
}

//! used by invoke to trigger all delyed events when return to the calling thread
void ViewOpenGLThread::notifyDelayedEvents()
{
	if (inWorkerThread())
	{
		PYXTHROW(PYXException,"Cant notify delayed notifications inside OpenGLThread");
	}
	
	std::list<DelayedNotifyEvent>::iterator item = m_delayedEvents.begin();

	while (item != m_delayedEvents.end())
	{
		item->m_notifier->notify(item->m_spEvent);
		item++;
	}

	m_delayedEvents.clear();	
}

//! thread safe notification. can be used in Main thread and in OpenGLThread 
void ViewOpenGLThread::safeNotify(Notifier & notifier,PYXPointer<NotifierEvent> spEvent)
{
	if (inWorkerThread())
	{
		addDelayedEvent(notifier,spEvent);
	} else 
	{
		notifier.notify(spEvent);
	}
}