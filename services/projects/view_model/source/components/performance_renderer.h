#pragma once
#ifndef VIEW_MODEL__PERFORMANCE_RENDERER_H
#define VIEW_MODEL__PERFORMANCE_RENDERER_H
/******************************************************************************
performance_renderer.h

begin		: 2009-11-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "component.h"
#include "performance_counter.h"
#include "texture_packer.h"

#include <map>
#include <set>

/*!

PerformanceRenderer - render a performance graph 

-- Description:
     - this renderer use the Performance Mesurment class to reterive it's data.

-- OpenGL extentions: 
     - None

-- Limiations:
     - None

*/
//! PerformanceRenderer - render the pyxis grid for all visible tiles on View. 
class PerformanceRenderer : public Component
{

public:
	PerformanceRenderer(ViewOpenGLThread & viewThread);
	static PYXPointer<PerformanceRenderer> create(ViewOpenGLThread & viewThread) { return PYXNEW(PerformanceRenderer,viewThread); }

	virtual ~PerformanceRenderer(void);

	virtual bool initialize();
	virtual void render();

protected:
	TexturePacker m_texturePacker;
	PYXPointer<PackedTextureItem> m_currentText;
	std::string m_text;
};


#endif
