/******************************************************************************
icon_gpu_renderer.cpp

begin		: 2009-11-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "icon_gpu_renderer.h"
#include "view_open_gl_thread.h"

#include <boost/scoped_array.hpp>

IconGPURenderer::IconGPURenderer(ViewOpenGLThread & viewThread) : 
	IconRenderer(viewThread), 
	m_shader(OpenGLShader::knVertexShader)
{
}

IconGPURenderer::~IconGPURenderer(void)
{
}

bool IconGPURenderer::initialize() 
{	

	//TODO: should we allow shaders to read their code from a file?

	m_shader.setCode(
"uniform vec3 left;\n"
"uniform vec3 up;\n"
"uniform float scale;\n"
"attribute vec2 offset;\n"
"attribute vec2 iconScale;\n"
"\n"
"void main()\n"
"{\n"
"   gl_TexCoord[0] = gl_MultiTexCoord0;\n"
"   gl_FrontColor = gl_Color;\n"
"   vec4 vertex = gl_Vertex;\n"
"   vertex.xyz=vertex.xyz+offset[0]*left*scale*iconScale[0];\n"
"   vertex.xyz=vertex.xyz+offset[1]*up*scale*iconScale[1];\n"
"   gl_Position = gl_ModelViewProjectionMatrix*vertex;\n"
"}\n"
);

	if (! m_shader.compile())
	{
		TRACE_INFO( "failed to compile m_shader: " + m_shader.getCompileErrors());
		return false;
	}

	if (! m_shader.isValid())
	{
		return false;
	}

	m_program.attach(m_shader);

	if (! m_program.link())
	{
		TRACE_INFO( "failed to compile shader: " + m_program.getLinkErrors());
		return false;
	}
	
	if (! m_program.isValid())
	{
		return false;
	}

	m_upVar.reset(new OpenGLUniformVariable(m_program.getUniformVariable("up")));
	m_leftVar.reset(new OpenGLUniformVariable(m_program.getUniformVariable("left")));
	m_scaleVar.reset(new OpenGLUniformVariable(m_program.getUniformVariable("scale")));
	m_offsetAttribute.reset(new OpenGLAttributeVariable(m_program.getAttributeVariable("offset")));
	m_iconScaleAttribute.reset(new OpenGLAttributeVariable(m_program.getAttributeVariable("iconScale")));

	return true;
}


void IconGPURenderer::render()
{	
	getViewThread().setState("render Icons");
	const Camera & cam = getViewThread().getCamera();	

	//calculate up and left vectors (1-unit length)
	vec3 eye = cam.getEye();
	vec3 up  = cam.getUp();
	vec3 left = cml::cross(cam.getLook(),up);

	//calculate the scaling factor depend on the camera Fov and screen height.
	double pixel_factor = tan(cam.getFovy()/2)/getViewThread().getViewportHeight();
	
	// TODO: need to move icons to a property of the Camera - something like cam.getNormalizeRange()
	double dist = cam.getOrbitalRange() / SphereMath::knEarthRadius; 
	
	double pixel_size = dist * pixel_factor;

	up *= pixel_size;
	left *= pixel_size;	


	//rendering with shader program

	m_program.startUsing();

	m_upVar->set((float)up[0],(float)up[1],(float)up[2]);
	m_leftVar->set((float)left[0],(float)left[1],(float)left[2]);
	m_scaleVar->set(1.0f);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	IconRenderer::render();

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);


	m_program.stopUsing();
}

void IconGPURenderer::renderOpenGLData(IconsOpenGLData & openGLData)
{
	IconsOpenGLData::iterator it = openGLData.begin();

	while(it != openGLData.end())
	{
		//NOTE: openGLData.getIconScaling() - animated icon scaling. this->getIconScaling() - global icon scaling
		m_scaleVar->set(this->getIconScaling());					

		it->first->startUsing();
				
		glVertexPointer(3,GL_FLOAT,sizeof(IconOpenGLVertexData),&(it->second.getFirstItem().vertices[0].vertex[0]));		
		glTexCoordPointer(2,GL_FLOAT,sizeof(IconOpenGLVertexData),&(it->second.getFirstItem().vertices[0].textureCoord[0]));		
		glColorPointer(4,GL_UNSIGNED_BYTE,sizeof(IconOpenGLVertexData),&(it->second.getFirstItem().vertices[0].color[0]));

		m_offsetAttribute->startUsing(2,GL_FLOAT,false,sizeof(IconOpenGLVertexData),&(it->second.getFirstItem().vertices[0].pixelOffset[0]));
		m_iconScaleAttribute->startUsing(2,GL_FLOAT,false,sizeof(IconOpenGLVertexData),&(it->second.getFirstItem().vertices[0].scale[0]));

		glDrawArrays(GL_QUADS, 0, 4*it->second.size());

		m_offsetAttribute->stopUsing();
		m_iconScaleAttribute->stopUsing();
		it->first->stopUsing();

		++it;
	}
}