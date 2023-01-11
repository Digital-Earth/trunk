/******************************************************************************
compass_renderer.cpp

begin		: 2010-01-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "compass_renderer.h"

#include "view_open_gl_thread.h"
#include "gl_utils.h"

#include "exceptions.h"



CompassRenderer::CompassRenderer(ViewOpenGLThread & viewThread) : 
	Component(viewThread),
	m_scale(0.8),
	m_rotation(0.0)
{				
}

CompassRenderer::~CompassRenderer(void)
{
}

bool CompassRenderer::initialize()
{
	m_compassTexture = packTextureFromResource(m_texturePacker,"Textures.Compass.compass.png");
	
	return m_compassTexture;
}



void CompassRenderer::render()
{
	getViewThread().setState("render Compass");
	m_rotation = getViewThread().getCamera().getHeading();
	
	//perform rotation and scaleing
	glRotated(m_rotation , 0, 0, 1);
	glScaled(m_scale,m_scale,m_scale);

	//because our texture is not centered, we need to fix the center of it by 5 pixels.
	glTranslated(0,5,0);

	//enable texture options
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	
	//draw texture
	m_compassTexture->draw(vec3(0,0,0),vec3(1,0,0),vec3(0,1,0));
}

bool CompassRenderer::checkMouseOnCompass(PYXPointer<UIMouseEvent> & event)
{
	//check the compass ring
	//the numbers 35 and 45 are the extrnal and internal radius of the compass.png resource.
	if (event->mouseDistanceFrom(0,0) > 35*m_scale && event->mouseDistanceFrom(0,0) < 45*m_scale)
	{
		return true;
	}

	//check the north button:
	//the center of the North button is at (45,0) at raduis 12, and rotated by rotation (taken from compass.png resource).
	if (event->mouseDistanceFrom(45*m_scale*cos(cml::rad(m_rotation+90)),45*m_scale*sin(cml::rad(m_rotation+90))) < 12)
	{
		return true;
	}

	return false;
}

void CompassRenderer::onMouseDoubleClick(PYXPointer<UIMouseEvent> event)
{
	//check if click was ontop inside compass
	if (checkMouseOnCompass(event))
	{
		Camera camera = getViewThread().getCamera();
		camera.setHeading(0);

		getViewThread().getCameraAnimator().goToCamera(camera);

		event->setConsumed();
						
		return;
	}	
}


void CompassRenderer::onMouseDown(PYXPointer<UIMouseEvent> event)
{	
	//inside compass
	if (checkMouseOnCompass(event))
	{
		m_start_rotation = atan2(-event->getMouseX(),event->getMouseY())-cml::rad(m_rotation);
		event->setConsumed();
		setActive();
				
		return;
	}	
}

void CompassRenderer::onMouseMove(PYXPointer<UIMouseEvent> event)
{
	if (isActive())
	{	
		event->setConsumed();

		m_rotation = atan2(-event->getMouseX(),event->getMouseY());

		Camera camera = getViewThread().getCamera();
		camera.setHeading(cml::deg(m_rotation-m_start_rotation));
		getViewThread().getCameraAnimator().setCamera(camera);
	}
}

void CompassRenderer::onMouseUp(PYXPointer<UIMouseEvent> event)
{
	if (isActive())
	{
		event->setConsumed();
		releaseActive();
	}
}