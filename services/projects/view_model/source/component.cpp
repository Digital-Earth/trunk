/******************************************************************************
component.cpp

begin		: 2009-11-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "component.h"
#include "pyxis/utility/bitmap_server_provider.h"
#include "view_open_gl_thread.h"


PYXPointer<RhombusRGBA> Component::loadRhombusBitmapFromResource(const std::string & resourceName)
{
	return m_thread.getComponentResources().loadRhombusBitmapFromResource(resourceName);
}

PYXPointer<OpenGLTexture> Component::loadTextureFromResource(const OpenGLTexture::TextureFormat & format,const std::string & resourceName)
{
	return m_thread.getComponentResources().loadTextureFromResource(format,resourceName);	
}

bool Component::loadTextureFromResource(OpenGLTexture & texture,const std::string & resourceName, unsigned int mipLevel)
{	
	return m_thread.getComponentResources().loadTextureFromResource(texture,resourceName,mipLevel);	
}

bool Component::loadTextureRegionFromResource(OpenGLTexture & texture,const std::string & resourceName, int x,int y,int width,int height)
{	
	return m_thread.getComponentResources().loadTextureRegionFromResource(texture,resourceName,x,y,width,height);
}

PYXPointer<PackedTextureItem> Component::packTextureFromResource(TexturePacker & packer,const std::string & resourceName)
{
	return m_thread.getComponentResources().packTextureFromResource(packer,resourceName);
}


PYXPointer<PackedTextureItem> Component::packTextureFromBitmapDefinition(TexturePacker & packer,const std::string & bitmapDefinition)
{
	return m_thread.getComponentResources().packTextureFromBitmapDefinition(packer,bitmapDefinition);
}


PYXPointer<PackedTextureItem> Component::getTextureFromBitmapDefinition(TexturePacker & packer,const std::string & bitmapDefinition)
{
	return m_thread.getComponentResources().getTextureFromBitmapDefinition(packer,bitmapDefinition);
}
	
//! make sure that bitmap is ready inside the texturePacker. if there is not bitmap. it would be requested from BitmapServerProvider
void Component::prepareBitampDefinition(TexturePacker & packer,const std::string & bitmapDefinition)
{
	m_thread.getComponentResources().prepareBitampDefinition(packer,bitmapDefinition);
}

void Component::forgetPendingRequests(TexturePacker & packer)
{
	m_thread.getComponentResources().forgetPendingRequests(packer);
}

bool Component::isActive()
{
	return this == getViewThread().getActiveComponent();
}

void Component::setActive()
{
	if (!isActive())
	{
		getViewThread().pushActiveComponent(this);
	}
}

void Component::releaseActive()
{
	if (isActive())
	{
		getViewThread().popActiveComponent();
	}
}


vec2 Component::localToScreenCoordinate(const vec2 & coord)
{
	if (getParent() != 0)
	{
		return getParent()->localToScreenCoordinate(coord);
	}
	else
	{
		return coord;
	}
}

vec2 Component::screenToLocalCoodinate(const vec2 & coord)
{
	if (getParent() != 0)
	{
		return getParent()->screenToLocalCoodinate(coord);
	}
	else
	{
		return coord;
	}
}