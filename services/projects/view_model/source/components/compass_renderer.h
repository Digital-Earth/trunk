#pragma once
#ifndef VIEW_MODEL__COMPASS_RENDERER_H
#define VIEW_MODEL__COMPASS_RENDERER_H
/******************************************************************************
compass_renderer.h

begin		: 2009-11-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "component.h"
#include "texture_packer.h"
#include "camera_animator.h"

#include <map>
#include <vector>


/*!

CompassRenderer - render a compass that is rotated from camera heading

-- Description:
     - render a compass that is rotated from camera heading

-- OpenGL extentions: 
     - None

-- Limitations:
     - 

*/
//! CompassRenderer  - render a compass that is rotated from camera heading
class CompassRenderer  : public Component
{

public:
	CompassRenderer(ViewOpenGLThread & viewThread);
	static PYXPointer<CompassRenderer> create(ViewOpenGLThread & viewThread) { return PYXNEW(CompassRenderer,viewThread); }

	virtual ~CompassRenderer(void);

	virtual bool initialize();
	virtual void render();	

protected:
	TexturePacker m_texturePacker;
	PYXPointer<PackedTextureItem> m_compassTexture;
	
	//IUIController API
public:
	//None...
	void onMouseDown(PYXPointer<UIMouseEvent> event);
	void onMouseMove(PYXPointer<UIMouseEvent> event);
	void onMouseUp(PYXPointer<UIMouseEvent> event);
	void onMouseDoubleClick(PYXPointer<UIMouseEvent> event);

protected:
	bool checkMouseOnCompass(PYXPointer<UIMouseEvent> & event);

	//interal state
protected:
	double m_rotation;

	//! remember the rotation where the user click down.
	double m_start_rotation;
	
	//Properties
protected:
	//! length of the scale bar - in pixels
	double m_scale;

public:
	const double & getScale() const { return m_scale; }
	void setScale(const double & scale) { m_scale = scale; }
};

#endif

