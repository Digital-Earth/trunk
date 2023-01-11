/******************************************************************************
icon_cpu_renderer.cpp

begin		: 2009-11-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "icon_cpu_renderer.h"
#include "view_open_gl_thread.h"

IconCPURenderer::IconCPURenderer(ViewOpenGLThread & viewThread) : IconRenderer(viewThread)
{	
}

IconCPURenderer::~IconCPURenderer(void)
{
}

bool IconCPURenderer::initialize() 
{
	return true;
}

void IconCPURenderer::render()
{	
	getViewThread().setState("render Icons");
	const Camera & cam = getViewThread().getCamera();	

	//calculate up and left vectors (1-unit length)
	vec3 eye = cam.getEye();
	m_up  = cam.getUp();
	m_left = cml::cross(cam.getLook(),m_up);

	//calcualte the scaling factor depend on the camera Fov and screen height.
	double pixel_factor = tan(cam.getFovy()/2)/getViewThread().getViewportHeight();
	
	// TODO: need to move icons to a property of the Camera - something like cam.getNormalizeRange()
	double dist = cam.getOrbitalRange() / SphereMath::knEarthRadius; 
	
	double pixel_size = dist * pixel_factor;

	m_up *= pixel_size;
	m_left *= pixel_size;	

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	IconRenderer::render();

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void IconCPURenderer::renderOpenGLData(IconsOpenGLData & openGLData)
{
	IconsOpenGLData::iterator it = openGLData.begin();

	openGLData.rotateIcons(m_left,m_up,getIconScaling());

	while(it != openGLData.end())
	{
		it->first->startUsing();
				
		glVertexPointer(3,GL_FLOAT,sizeof(IconOpenGLVertexData),&(it->second.getFirstItem().vertices[0].vertex[0]));		
		glTexCoordPointer(2,GL_FLOAT,sizeof(IconOpenGLVertexData),&(it->second.getFirstItem().vertices[0].textureCoord[0]));
		glColorPointer(4,GL_UNSIGNED_BYTE,sizeof(IconOpenGLVertexData),&(it->second.getFirstItem().vertices[0].color[0]));

		glDrawArrays(GL_QUADS, 0, 4*it->second.size());

		it->first->stopUsing();

		++it;
	}
}