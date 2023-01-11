#pragma once
#ifndef VIEW_MODEL__SCALE_RENDERER_H
#define VIEW_MODEL__SCALE_RENDERER_H
/******************************************************************************
scale_renderer.h

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

ScaleRenderer - render a scale on the size of a pixel on the screen

-- Description:
     - displace the scale 

-- OpenGL extentions: 
     - None

-- Limitations:
     - only display in scale meteres

*/
//! ScaleRenderer  - render a scale on the size of a pixel on the screen
class ScaleRenderer  : public Component
{

public:
	ScaleRenderer(ViewOpenGLThread & viewThread);
	static PYXPointer<ScaleRenderer> create(ViewOpenGLThread & viewThread) { return PYXNEW(ScaleRenderer,viewThread); }

	virtual ~ScaleRenderer(void);

	virtual bool initialize();
	virtual void render();	

protected:
	typedef std::vector<std::pair<double,std::string>> RangeTableStrings;
	typedef std::map<std::string,PYXPointer<PackedTextureItem>> RangeTableTextures;

	TexturePacker m_texturePacker;

	RangeTableStrings m_rangeTable;
	RangeTableTextures m_rangeTextures;
	
	//IUIController API
public:
	//None...
	
	//interal state
protected:
	double m_actualPixelSize;
	double m_currentPixelRange;
	PYXPointer<PackedTextureItem> m_currentTexture;

	//Properties
protected:
	//! center of the scroll bar - screen cordiantes
	vec2   m_center; 

	//! length of the scale bar - in pixels
	double m_scaleLength;

public:
	const vec2 & getCenter() const { return m_center; }
	void setCenter(const vec2 & center) { m_center = center; }

	const double & getScaleLength() const { return m_scaleLength; }
	void setScaleLength(const double & scaleLength) { m_scaleLength = scaleLength; }


protected:
	void updateScale(Camera & camera);

	//! find the right used scale to use from m_actualPixelSize value.
	void findScaleResolution();

	//! recalibrate the the m_actualPixelSize value by using screen space ray projection to mesh
	void recalibrateScaleFromScreenSpace();

	void onCameraChange(PYXPointer<NotifierEvent> event);
};

#endif

