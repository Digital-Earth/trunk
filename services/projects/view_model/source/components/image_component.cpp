/******************************************************************************
image_component.cpp

begin		: 2013-08-08
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "image_component.h"

#include "view_open_gl_thread.h"
#include "gl_utils.h"

#include "exceptions.h"

#include "pyxis/utility/great_circle_arc.h"

ImageComponent::ImageComponent(ViewOpenGLThread & viewThread,const std::string & imagePath) : 
	Component(viewThread),
	m_imagePath(imagePath)
{
}

ImageComponent::~ImageComponent(void)
{
	forgetPendingRequests(m_texturePacker);
}

bool ImageComponent::initialize()
{
	m_texture = packTextureFromBitmapDefinition(m_texturePacker,"<src>"+m_imagePath+"</src>");

	return m_texture;
}

void ImageComponent::render()
{
	getViewThread().setState("render image " + m_imagePath);
	
	//center the image
	glTranslated(-m_texture->getWidth()/2,+m_texture->getHeight()/2,0);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	m_texture->draw(vec3(0,0,0),vec3(1,0,0),vec3(0,1,0));
}
