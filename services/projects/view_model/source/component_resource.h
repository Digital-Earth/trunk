#pragma once
#ifndef VIEW_MODEL__COMPONENT_RESOURCE_H
#define VIEW_MODEL__COMPONENT_RESOURCE_H
/******************************************************************************
component_resource.h

begin		: 2010-05-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"
#include "view_model_api.h"
#include "sync_context.h"

#include "rhombus_bitmap.h"
#include "open_gl_texture.h"
#include "texture_packer.h"

#include "pyxis/utility/object.h"

#include <map>

/*!
ComponentResources - helper class for loading textures and created packed textures.

Components that need to display text or bitmaps, use the BitmapServer to generate textures for them.
Sometimes, the creation of the bitmap can take long time.

Therefore, the ComponentResources has an ansync-API to request textures,
and check if the thier texture is ready already

*/
class ComponentResources
{
	friend class ViewOpenGLThread;
	friend class Component;

protected:
	typedef std::map<std::string,PYXPointer<PackedTextureItem>> LoadedTexturesMap;
	typedef std::map<TexturePacker*,LoadedTexturesMap> LoadedTextureForTexturePackerMap;	
	LoadedTextureForTexturePackerMap m_loadedTexturesForTexturePacker;
	
	Sync::WorkerThreadContext m_loaderThread;
	ViewOpenGLThread * m_openGLThread;

public:
	ComponentResources() {};
	virtual ~ComponentResources() {};

protected:
	void setOpenGLThread(ViewOpenGLThread * thread);

//must be called inside openGL context
protected:
	//! load a RhombusRGBA from application resource. blocking function
	PYXPointer<RhombusRGBA> loadRhombusBitmapFromResource(const std::string & resourceName);

	//! load a texture from application resource. blocking function
	PYXPointer<OpenGLTexture> loadTextureFromResource(const OpenGLTexture::TextureFormat & format,const std::string & resourceName);

	//! load a texture from application resource. blocking function
	bool loadTextureFromResource(OpenGLTexture & texture,const std::string & resourceName,unsigned int mipLevel = 0);

	//! load a texture region from application resource. blocking function
	bool loadTextureRegionFromResource(OpenGLTexture & texture,const std::string & resourceName,int x,int y,int width,int height);

	//! load a texture from application resource and pack it inside a given texture packer. blocking function
	PYXPointer<PackedTextureItem> packTextureFromResource(TexturePacker & packer,const std::string & resourceName);

	//! load a texture from IconStyle and pack it inside a given texture packer. blocking function
	PYXPointer<PackedTextureItem> packTextureFromBitmapDefinition(TexturePacker & packer,const std::string & bitmapDefinition);

//Non blocking API - must be called inside openGL context
public:
	//! retrun the requested bitmap if found inside the texturePacker, else request a texture from BitmapServerProvider
	PYXPointer<PackedTextureItem> getTextureFromBitmapDefinition(TexturePacker & packer,const std::string & bitmapDefinition);
	
	//! make sure that bitmap is ready inside the texturePacker. if there is not bitmap. it would be requested from BitmapServerProvider
	void prepareBitampDefinition(TexturePacker & packer,const std::string & bitmapDefinition);	

	//! forget all pending request for this texture packer
	void forgetPendingRequests(TexturePacker & packer);

private:
	//! called inside the loaderThread to perfrom nonblocking loading
	void requestBitmpaDefinition(TexturePacker * packer,std::string bitmapDefinition);

	//! called inside the openGLThread after we got result from bitmapServerProvider
	void packBitmapDefinition(TexturePacker * packer,std::string bitmapDefinition,std::string bitmapData);
};

#endif
