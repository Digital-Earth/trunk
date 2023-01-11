#pragma once
#ifndef VIEW_MODEL__IMAGE_COMPONENT_H
#define VIEW_MODEL__IMAGE_COMPONENT_H
/******************************************************************************
image_component.h

begin		: 2013-08-08
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "component.h"
#include "texture_packer.h"
#include "camera_animator.h"

#include <map>
#include <vector>


/*!

ImageComponent - render a texture (from an image on the screen)

-- Description:
     - display the image

-- OpenGL extentions: 
     - None

-- Limitations:
     - None

*/
//! ImageComponent - render a texture (from an image on the screen)
class ImageComponent  : public Component
{

public:
	ImageComponent(ViewOpenGLThread & viewThread,const std::string & imagePath);
	static PYXPointer<ImageComponent> create(ViewOpenGLThread & viewThread,const std::string & imagePath) { return PYXNEW(ImageComponent,viewThread,imagePath); }

	virtual ~ImageComponent(void);

	virtual bool initialize();
	virtual void render();	

protected:
	PYXPointer<PackedTextureItem> m_texture;
	TexturePacker m_texturePacker;
	std::string m_imagePath;
	
public:
	vec2 getSize() const { return vec2(m_texture->getWidth(),m_texture->getHeight()); }	
};

#endif

