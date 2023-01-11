/******************************************************************************
scrollbar_renderer.cpp

begin		: 2010-01-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "scrollbar_renderer.h"

#include "view_open_gl_thread.h"
#include "gl_utils.h"

#include "exceptions.h"



ScrollBarRenderer::ScrollBarRenderer(ViewOpenGLThread & viewThread) : 
	Component(viewThread), 
	m_scrollPosition(0.5), 
	m_scrolling(false),
	m_center(0.0,0.0),
	m_buttonsScaling(1.0,1.0),
	m_scrollAreaLength(100.0),
	m_rotationAngle(0.0)
{		
}

ScrollBarRenderer::~ScrollBarRenderer(void)
{
}

bool ScrollBarRenderer::initialize()
{
	m_plusButtonTexture = packTextureFromResource(m_texturePacker,"Textures.Scrollbar.plus_button.png");
	m_minusButtonTexture = packTextureFromResource(m_texturePacker,"Textures.Scrollbar.minus_button.png");
	m_scollbarTexture = packTextureFromResource(m_texturePacker,"Textures.Scrollbar.scroll_bar.png");
	m_scollbarButtonTexture = packTextureFromResource(m_texturePacker,"Textures.Scrollbar.scroll_button.png");

	return m_plusButtonTexture && m_minusButtonTexture && m_scollbarTexture && m_scollbarButtonTexture;
}



void ScrollBarRenderer::render()
{
	getViewThread().setState("render Scrollbar");
		
	//move to right position
	glTranslated(m_center[0],m_center[1],0);
	glRotated(m_rotationAngle,0.0,0.0,0.1);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	double buttonSize = m_plusButtonTexture->getHeight()*m_buttonsScaling[1];

	m_scollbarTexture->draw(vec3(0.0,0.0,0.0),vec3(1.0,0.0,0.0),vec3(0.0,(m_scrollAreaLength+buttonSize*1.5)/m_scollbarTexture->getHeight(),0.0));

	m_plusButtonTexture->draw(vec3(0.0,(m_scrollAreaLength+buttonSize*1.5)/2,0.0),vec3(m_buttonsScaling[0],0.0,0.0),vec3(0.0,m_buttonsScaling[1],0.0));
	m_minusButtonTexture->draw(vec3(0.0,-((m_scrollAreaLength+buttonSize*1.5))/2,0.0),vec3(m_buttonsScaling[0],0.0,0.0),vec3(0.0,m_buttonsScaling[1],0.0));
	
	m_scollbarButtonTexture->draw(vec3(0.0,-(m_scrollPosition-0.5)*m_scrollAreaLength,0.0),vec3(m_buttonsScaling[0],0.0,0.0),vec3(0.0,m_buttonsScaling[0],0.0));		
}

vec2 ScrollBarRenderer::getMouseCoordinate(const PYXPointer<UIMouseEvent> & event)
{
	vec2 mouse = vec2(event->getMouseX(),event->getMouseY())-m_center;
	
	//rotate in -rotationAngle
	return cml::rotate_vector_2D(mouse,cml::rad(-m_rotationAngle));
}

void ScrollBarRenderer::onMouseDown(PYXPointer<UIMouseEvent> event)
{
	vec2 mouse = getMouseCoordinate(event);
	double scale = std::max(m_buttonsScaling[0],m_buttonsScaling[1]);
			
	//Scrool Button
	if ( (mouse-vec2(0.0,-(m_scrollPosition-0.5)*m_scrollAreaLength)).length() < m_scollbarButtonTexture->getHeight()/2 * scale )
	{
		event->setConsumed();
		setActive();
		
		if (event->isLeftButtonDown())
		{
			m_scrolling = true;

			double newPos = cml::clamp(-mouse[1]/m_scrollAreaLength+0.5,0.0,1.0);
			double delta = newPos-m_scrollPosition;
			m_scrollPosition = newPos;			

			m_startScrollingNotifier.notify(PYXNEW(ScrollBarEvent,this));
			m_srcollbarChangeNotifier.notify(PYXNEW(ScrollBarChangeEvent,this,delta));			
		}		
		return;
	}

	//Plus Button
	if ( (mouse-vec2(0.0,(m_scrollAreaLength+m_plusButtonTexture->getHeight()*m_buttonsScaling[1]*1.5)/2)).length() < m_plusButtonTexture->getHeight()/2 * scale )
	{		
		event->setConsumed();
		setActive();
		m_scrolling = false;
		if (event->isLeftButtonDown())
		{
			m_plusButtonClickedNotifier.notify(PYXNEW(ScrollBarEvent,this));
		}
		return;
	}

	//Minus Button
	if ( (mouse-vec2(0.0,-(m_scrollAreaLength+m_minusButtonTexture->getHeight()*m_buttonsScaling[1]*1.5)/2)).length() < m_minusButtonTexture->getHeight()/2 * scale )
	{		
		event->setConsumed();
		setActive();
		m_scrolling = false;
		if (event->isLeftButtonDown())
		{
			m_minusButtonClickedNotifier.notify(PYXNEW(ScrollBarEvent,this));
		}
		return;
	}	

	//Scroll Area
	if ( abs(mouse[0]) < m_scollbarButtonTexture->getWidth()/2 * scale && abs(mouse[1])<(m_scrollAreaLength+m_plusButtonTexture->getHeight()*m_buttonsScaling[1]*1.5)/2 )
	{
		event->setConsumed();
		setActive();
		
		if (event->isLeftButtonDown())
		{
			m_scrolling = true;

			double newPos = cml::clamp(-mouse[1]/m_scrollAreaLength+0.5,0.0,1.0);
			double delta = newPos-m_scrollPosition;
			m_scrollPosition = newPos;			

			m_startScrollingNotifier.notify(PYXNEW(ScrollBarEvent,this));
			m_srcollbarChangeNotifier.notify(PYXNEW(ScrollBarChangeEvent,this,delta));			
		}		
		return;
	}
}

void ScrollBarRenderer::onMouseMove(PYXPointer<UIMouseEvent> event)
{
	if (isActive())
	{	
		event->setConsumed();

		if (m_scrolling)
		{
			vec2 mouse = getMouseCoordinate(event);

			double newPos = cml::clamp(-mouse[1]/m_scrollAreaLength+0.5,0.0,1.0);
			double delta = newPos-m_scrollPosition;
			m_scrollPosition = newPos;			

			m_srcollbarChangeNotifier.notify(PYXNEW(ScrollBarChangeEvent,this,delta));			
		}
	}
}

void ScrollBarRenderer::onMouseUp(PYXPointer<UIMouseEvent> event)
{
	if (isActive())
	{
		event->setConsumed();
		releaseActive();

		if (m_scrolling)
		{
			m_scrolling = false;
			m_stopScrollingNotifier.notify(PYXNEW(ScrollBarEvent,this));			
		}
	}
}