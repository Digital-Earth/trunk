/******************************************************************************
scale_renderer.cpp

begin		: 2010-01-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "scale_renderer.h"

#include "view_open_gl_thread.h"
#include "gl_utils.h"

#include "exceptions.h"

#include "pyxis/utility/great_circle_arc.h"

ScaleRenderer::ScaleRenderer(ViewOpenGLThread & viewThread) : 
	Component(viewThread), 	
	m_center(0.0,0.0),	
	m_scaleLength(200.0)	
{		
	m_rangeTable.push_back(std::pair<double,std::string>(1,"1 m"));
	m_rangeTable.push_back(std::pair<double,std::string>(2,"2 m"));
	m_rangeTable.push_back(std::pair<double,std::string>(5,"5 m"));
	m_rangeTable.push_back(std::pair<double,std::string>(10,"10 m"));
	m_rangeTable.push_back(std::pair<double,std::string>(20,"20 m"));
	m_rangeTable.push_back(std::pair<double,std::string>(50,"50 m"));
	m_rangeTable.push_back(std::pair<double,std::string>(100,"100 m"));
	m_rangeTable.push_back(std::pair<double,std::string>(200,"200 m"));
	m_rangeTable.push_back(std::pair<double,std::string>(500,"500 m"));
	m_rangeTable.push_back(std::pair<double,std::string>(1000,"1 km"));
	m_rangeTable.push_back(std::pair<double,std::string>(2000,"2 km"));
	m_rangeTable.push_back(std::pair<double,std::string>(5000,"5 km"));
	m_rangeTable.push_back(std::pair<double,std::string>(10000,"10 km"));
	m_rangeTable.push_back(std::pair<double,std::string>(20000,"20 km"));
	m_rangeTable.push_back(std::pair<double,std::string>(50000,"50 km"));
	m_rangeTable.push_back(std::pair<double,std::string>(100000,"100 km"));
	m_rangeTable.push_back(std::pair<double,std::string>(200000,"200 km"));
	m_rangeTable.push_back(std::pair<double,std::string>(500000,"500 km"));
	m_rangeTable.push_back(std::pair<double,std::string>(1000000,"1,000 km"));
	m_rangeTable.push_back(std::pair<double,std::string>(2000000,"2,000 km"));
	m_rangeTable.push_back(std::pair<double,std::string>(5000000,"5,000 km"));
	m_rangeTable.push_back(std::pair<double,std::string>(10000000,"10,000 km"));
	m_rangeTable.push_back(std::pair<double,std::string>(20000000,"20,000 km"));
	m_rangeTable.push_back(std::pair<double,std::string>(50000000,"50,000 km"));
	m_rangeTable.push_back(std::pair<double,std::string>(100000000,"100,000 km"));
	
	m_actualPixelSize = 1;
	m_currentPixelRange = 1;

	getViewThread().getCameraAnimator().getCameraChangeNotifier().attach(this,&ScaleRenderer::onCameraChange);
}

ScaleRenderer::~ScaleRenderer(void)
{
	getViewThread().getCameraAnimator().getCameraChangeNotifier().detach(this,&ScaleRenderer::onCameraChange);
	forgetPendingRequests(m_texturePacker);
}

bool ScaleRenderer::initialize()
{
	RangeTableStrings::iterator it = m_rangeTable.begin();

	while (it != m_rangeTable.end())
	{
		PYXPointer<PackedTextureItem> item = packTextureFromBitmapDefinition(m_texturePacker,"<Text>"+it->second+"</Text>");

		if (item)
		{
			m_rangeTextures[it->second] = item;
		} 
		else 
		{
			//failed to create a string
			return false;
		}

		++it;
	}

	updateScale(getViewThread().getCamera());

	return true;
}



void ScaleRenderer::render()
{
	getViewThread().setState("render Scale");
	
	glTranslated(m_center[0],m_center[1],0);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	if (m_currentTexture)
	{
		m_currentTexture->draw(vec3(0,0,0),vec3(1,0,0),vec3(0,1,0));


		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POINT_SMOOTH);
		glLineWidth(5);
		glPointSize(5);		
		glColor4f(0.0f,0.0f,0.0f,0.5f);
		glBegin(GL_LINE_STRIP);		
		glVertex2d(-(m_currentPixelRange/m_actualPixelSize),10);
		glVertex2d(-(m_currentPixelRange/m_actualPixelSize),-10);
		glVertex2d((m_currentPixelRange/m_actualPixelSize),-10);
		glVertex2d((m_currentPixelRange/m_actualPixelSize),10);		
		glEnd();
		glBegin(GL_POINTS);		
		glVertex2d(-(m_currentPixelRange/m_actualPixelSize),10);
		glVertex2d(-(m_currentPixelRange/m_actualPixelSize),-10);
		glVertex2d((m_currentPixelRange/m_actualPixelSize),-10);
		glVertex2d((m_currentPixelRange/m_actualPixelSize),10);		
		glEnd();

		glLineWidth(3);
		glPointSize(3);
		glColor4f(0.0f,0.0f,0.0f,0.5f);
		glBegin(GL_LINE_STRIP);		
		glVertex2d(-(m_currentPixelRange/m_actualPixelSize),10);
		glVertex2d(-(m_currentPixelRange/m_actualPixelSize),-10);
		glVertex2d((m_currentPixelRange/m_actualPixelSize),-10);
		glVertex2d((m_currentPixelRange/m_actualPixelSize),10);		
		glEnd();
		glBegin(GL_POINTS);		
		glVertex2d(-(m_currentPixelRange/m_actualPixelSize),10);
		glVertex2d(-(m_currentPixelRange/m_actualPixelSize),-10);
		glVertex2d((m_currentPixelRange/m_actualPixelSize),-10);
		glVertex2d((m_currentPixelRange/m_actualPixelSize),10);		
		glEnd();

		glLineWidth(1);

		glColor3f(1.0f,1.0f,1.0f);
		glBegin(GL_LINE_STRIP);		
		glVertex2d(-(m_currentPixelRange/m_actualPixelSize),10);
		glVertex2d(-(m_currentPixelRange/m_actualPixelSize),-10);
		glVertex2d((m_currentPixelRange/m_actualPixelSize),-10);
		glVertex2d((m_currentPixelRange/m_actualPixelSize),10);		
		glEnd();
	}	
}

void ScaleRenderer::findScaleResolution()
{
	double scale_actual_size = m_actualPixelSize * m_scaleLength/2;

	RangeTableStrings::iterator it = m_rangeTable.begin();

	while (it != m_rangeTable.end())
	{
		if (it->first > scale_actual_size)
		{
			m_currentPixelRange = it->first;	
			m_currentTexture = m_rangeTextures[it->second];
			break;
		}			

		++it;
	}
}

void ScaleRenderer::recalibrateScaleFromScreenSpace()
{
	//local screen coordinates
	vec2 point1(-(m_currentPixelRange/m_actualPixelSize),10);
	vec2 point2(+(m_currentPixelRange/m_actualPixelSize),10);

	//transfer to screen coordinates (viewport coords)
	point1 = localToScreenCoordinate(point1);
	point2 = localToScreenCoordinate(point2);
	
	//create rays
	Ray ray1(getViewThread().getRay(point1));
	Ray ray2(getViewThread().getRay(point2));

	vec3 earthPoint1;
	vec3 earthPoint2;

	//try to intersects rays
	if (getViewThread().findRayIntersection(ray1,earthPoint1) && getViewThread().findRayIntersection(ray2,earthPoint2))
	{
		//we found to points on the earth surface - calculate the distance between the two points
		double distance = GreatCircleArc::calcDistance(CmlConvertor::toLatLon(earthPoint1),CmlConvertor::toLatLon(earthPoint2),SphereMath::knEarthRadius);
		
		//calibarate scale to be more precises.
		m_actualPixelSize /= m_currentPixelRange/distance;
	}
}

void ScaleRenderer::updateScale(Camera & camera)
{
	//calculate the scaling factor depend on the camera Fov and screen height.
	double pixelFactor = tan(camera.getFovy()/2)/(getViewThread().getViewportHeight()/2);
	
	double dist = camera.getOrbitalRange(); 

	m_actualPixelSize = dist * pixelFactor;	

	//find inital scale to use
	findScaleResolution();

	//calibate to be more percise and take into account the perpective applied by the camera
	recalibrateScaleFromScreenSpace();

	//find a better scale to use if the recalibrate was to big
	findScaleResolution();
}

void ScaleRenderer::onCameraChange(PYXPointer<NotifierEvent> event)
{
	CameraChangeEvent * cameraEvent = dynamic_cast<CameraChangeEvent*>(event.get());

	if (cameraEvent != NULL)
	{
		Camera & camera = cameraEvent->getCamera();

		updateScale(camera);
	}
}