#pragma once
#ifndef VIEW_MODEL__ICON_CPU_RENDERER_H
#define VIEW_MODEL__ICON_CPU_RENDERER_H
/******************************************************************************
icon_cpu_renderer.h

begin		: 2009-11-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "icon_renderer.h"

/*!

IconCPURenderer - render icons on the screen

-- Description:
     - This renderer is strongly depend on View visible tiles implemetation and on STile structure and IconTile class.

-- OpenGL extentions: None

-- Limitations: 
     - the FPS would be less then 30 for more then 200 icons on the screen
	 - only use demo icons at the momement
	 - doesn't support depth test - so far away icons could be printed in front of closer icons

*/
//! IconCPURenderer - render icons on the screen
class IconCPURenderer : public IconRenderer
{
public:
	IconCPURenderer(ViewOpenGLThread & viewThread);
	static PYXPointer<IconCPURenderer> create(ViewOpenGLThread & viewThread) { return PYXNEW(IconCPURenderer,viewThread); }

	virtual ~IconCPURenderer(void);

	virtual bool initialize();
	virtual void render(); 

	virtual void renderOpenGLData(IconsOpenGLData & openGLData);

protected:
	vec3 m_up;
	vec3 m_left;
};

#endif