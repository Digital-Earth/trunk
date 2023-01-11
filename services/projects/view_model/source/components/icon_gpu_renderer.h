#pragma once
#ifndef VIEW_MODEL__ICON_GPU_RENDERER_H
#define VIEW_MODEL__ICON_GPU_RENDERER_H
/******************************************************************************
icon_cpu_renderer.h

begin		: 2009-11-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "icon_renderer.h"
#include "open_gl_shader.h"
#include "open_gl_program.h"

#include <boost/scoped_ptr.hpp>

/*!

IconGPURenderer - render icons on the screen using a vertex shader

-- Description:
     - This renderer is strongly depend on View visible tiles implemetation and on STile structure and IconTile class.
     - Uses camera range property and Fovy to detemine icons scaling
     - Uses IconCPURenderer (CPU) to initialize icons bitmap - hack

-- OpenGL extentions:
	 - Vertex Shaders

-- Limitations: 
     - must have shaders support
     - the FPS would be less then 30 for more than 10,000 Icons on screen
	 - only use demo icons at the momement
	 - doesn't support depth test - so Icons far away could be printeded infornt of closer icons
	 
*/
//! IconGPURenderer - render icons on the screen using a vertex shader
class IconGPURenderer : public IconRenderer
{
public:
	IconGPURenderer(ViewOpenGLThread & viewThread);
	static PYXPointer<IconGPURenderer> create(ViewOpenGLThread & viewThread) { return PYXNEW(IconGPURenderer,viewThread); }

	virtual ~IconGPURenderer(void);

	virtual bool initialize();
	virtual void render(); 

	virtual void renderOpenGLData(IconsOpenGLData & openGLData);

private:	
	OpenGLShader m_shader;
	OpenGLProgram m_program;

	boost::scoped_ptr<OpenGLUniformVariable>   m_upVar;
	boost::scoped_ptr<OpenGLUniformVariable>   m_leftVar;
	boost::scoped_ptr<OpenGLUniformVariable>   m_scaleVar;
	boost::scoped_ptr<OpenGLAttributeVariable> m_offsetAttribute;
	boost::scoped_ptr<OpenGLAttributeVariable> m_iconScaleAttribute;
};


#endif