/******************************************************************************
component_resource.cpp

begin		: 2010-05-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "component.h"
#include "component_resource.h"
#include "pyxis/utility/bitmap_server_provider.h"
#include "view_open_gl_thread.h"

void ComponentResources::setOpenGLThread(ViewOpenGLThread * thread)
{
	m_openGLThread = thread;
}

PYXPointer<RhombusRGBA> ComponentResources::loadRhombusBitmapFromResource(const std::string & resourceName)
{
	std::string bitmapData = BitmapServerProvider::getBitmapServerProvider()->loadResource(resourceName);

	if (bitmapData.size() < sizeof(BitmapServerData))
	{
		TRACE_ERROR("got invalid bitmapdata for resource name : " << resourceName);
		return NULL;
	}

	//the string come incoded in base64 - convert it.
	bitmapData = XMLUtils::fromBase64(bitmapData);

	const BitmapServerData * data = static_cast<const BitmapServerData *>(static_cast<const void *>(bitmapData.c_str()));

	if (data->width != 82) 
	{
		TRACE_ERROR("image width must be 82 pixels to generate RhombusRGBA from reosurce " << resourceName);
		return NULL;
	}
	if (data->height != 82) 
	{
		TRACE_ERROR("image width must be 82 pixels to generate RhombusRGBA from reosurce " << resourceName);
		return NULL;
	}

	if (bitmapData.size() != data->width*data->height*4 + sizeof(int[2]))
	{
		PYXTHROW(PYXException,"icon bitmap size in not aligen to given width and height");
	}

	PYXPointer<RhombusRGBA> rgba = RhombusRGBA::create(RhombusRGBA::knEvenResolution);
	RhombusRGBA::RGBABuffer & buffer = rgba->getBuffer();

	int alphaSum = 0;

	const unsigned char * pixel = (&data->bitmap0);
	for(int y=0;y<RhombusRGBA::height;y++)
	{
		for(int x=0;x<RhombusRGBA::width;x++)
		{
			buffer.rgba[y][x][2] = *pixel;
			pixel++;
			buffer.rgba[y][x][1] = *pixel;
			pixel++;
			buffer.rgba[y][x][0] = *pixel;
			pixel++;
			buffer.rgba[y][x][3] = *pixel;
			alphaSum += *pixel;
			pixel++;
		}
	}

	rgba->setAllOpaque(alphaSum == RhombusRGBA::height*RhombusRGBA::width*255);
	rgba->setAllTransparent(alphaSum == 0);
	return rgba;
}


PYXPointer<OpenGLTexture> ComponentResources::loadTextureFromResource(const OpenGLTexture::TextureFormat & format,const std::string & resourceName)
{
	std::string bitmapData = BitmapServerProvider::getBitmapServerProvider()->loadResource(resourceName);

	if (bitmapData.size() < sizeof(BitmapServerData))
	{
		TRACE_ERROR("got invalid bitmapdata for resource name : " << resourceName);
		return NULL;
	}

	//the string come incoded in base64 - convert it.
	bitmapData = XMLUtils::fromBase64(bitmapData);

	const BitmapServerData * data = static_cast<const BitmapServerData *>(static_cast<const void *>(bitmapData.c_str()));

	if (bitmapData.size() != data->width*data->height*4 + sizeof(int[2]))
	{
		PYXTHROW(PYXException,"icon bitmap size in not aligen to given width and height");
	}

	PYXPointer<OpenGLTexture> texture = OpenGLTexture::create(OpenGLTexture::knTexture2D,format);
	texture->setData(data->width,data->height,OpenGLTexture::knDataBRGA,&(data->bitmap0));

	return texture;
}

bool ComponentResources::loadTextureFromResource(OpenGLTexture & texture,const std::string & resourceName,unsigned int mipLevel)
{
	std::string bitmapData = BitmapServerProvider::getBitmapServerProvider()->loadResource(resourceName);

	if (bitmapData.size() < sizeof(BitmapServerData))
	{
		TRACE_ERROR("got invalid bitmapdata for resource name : " << resourceName);
		return false;
	}

	//the string come incoded in base64 - convert it.
	bitmapData = XMLUtils::fromBase64(bitmapData);

	const BitmapServerData * data = static_cast<const BitmapServerData *>(static_cast<const void *>(bitmapData.c_str()));

	if (bitmapData.size() != data->width*data->height*4 + sizeof(int[2]))
	{
		PYXTHROW(PYXException,"icon bitmap size in not aligen to given width and height");
	}
	
	if (mipLevel==0)
	{
		texture.setData(data->width,data->height,OpenGLTexture::knDataBRGA,&(data->bitmap0));
	}
	else
	{
		texture.setMipLevelData(mipLevel,data->width,data->height,OpenGLTexture::knDataBRGA,&(data->bitmap0));
	}
	
	
	return true;
}

bool ComponentResources::loadTextureRegionFromResource(OpenGLTexture & texture,const std::string & resourceName,int x,int y,int width,int height)
{
	std::string bitmapData = BitmapServerProvider::getBitmapServerProvider()->loadResource(resourceName);

	if (bitmapData.size() < sizeof(BitmapServerData))
	{
		TRACE_ERROR("got invalid bitmapdata for resource name : " << resourceName);
		return false;
	}

	//the string come incoded in base64 - convert it.
	bitmapData = XMLUtils::fromBase64(bitmapData);

	const BitmapServerData * data = static_cast<const BitmapServerData *>(static_cast<const void *>(bitmapData.c_str()));

	if (bitmapData.size() != data->width*data->height*4 + sizeof(int[2]))
	{
		PYXTHROW(PYXException,"icon bitmap size in not aligen to given width and height");
	}

	texture.setDataRegion(x,y,width,height,OpenGLTexture::knDataBRGA,&(data->bitmap0));

	return true;
}



PYXPointer<PackedTextureItem> ComponentResources::packTextureFromResource(TexturePacker & packer,const std::string & resourceName)
{
	std::string bitmapData = BitmapServerProvider::getBitmapServerProvider()->loadResource(resourceName);

	if (bitmapData.size() < sizeof(BitmapServerData))
	{
		TRACE_ERROR("got invalid bitmap data for resource name : " << resourceName);
		return NULL;
	}

	//the string come encoded in base64 - convert it.
	bitmapData = XMLUtils::fromBase64(bitmapData);

	const BitmapServerData * data = static_cast<const BitmapServerData *>(static_cast<const void *>(bitmapData.c_str()));

	if (bitmapData.size() != data->width*data->height*4 + sizeof(int[2]))
	{
		PYXTHROW(PYXException,"icon bitmap size in not align to given width and height");
	}

	PYXPointer<PackedTextureItem> newItem = packer.packTextureItem(resourceName,data->width,data->height,OpenGLTexture::knDataBRGA,&(data->bitmap0));
	
	return newItem;
}


PYXPointer<PackedTextureItem> ComponentResources::packTextureFromBitmapDefinition(TexturePacker & packer,const std::string & iconStyle)
{
	std::string bitmapData = BitmapServerProvider::getBitmapServerProvider()->loadIcon(iconStyle);

	if (bitmapData.size() < sizeof(BitmapServerData))
	{
		TRACE_ERROR("got invalid icon style : " << iconStyle);
		return NULL;
	}

	//the string come incoded in base64 - convert it.
	bitmapData = XMLUtils::fromBase64(bitmapData);

	const BitmapServerData * data = static_cast<const BitmapServerData *>(static_cast<const void *>(bitmapData.c_str()));

	if (bitmapData.size() != data->width*data->height*4 + sizeof(int[2]))
	{
		PYXTHROW(PYXException,"icon bitmap size in not aligen to given width and height");
	}

	PYXPointer<PackedTextureItem> newItem = packer.packTextureItem(iconStyle,data->width,data->height,OpenGLTexture::knDataBRGA,&(data->bitmap0));
	
	return newItem;
}



//! retrun the requested bitmap if found inside the texturePacker, else request a texture from BitmapServerProvider
PYXPointer<PackedTextureItem> ComponentResources::getTextureFromBitmapDefinition(TexturePacker & packer,const std::string & bitmapDefinition)
{
	PYXPointer<PackedTextureItem> item;
	try
	{
		item = packer.getTextureItem(bitmapDefinition);
	}
	catch(...)
	{
		TRACE_ERROR("Failed to getTextureItem : " << bitmapDefinition);
	}

	//check if we have an item	
	if (item)
	{
		//if we still hold an reference for this packedTextureItem - remove our reference.
		if (m_loadedTexturesForTexturePacker[&packer].find(bitmapDefinition) != m_loadedTexturesForTexturePacker[&packer].end())
		{
			m_loadedTexturesForTexturePacker[&packer].erase(bitmapDefinition);
		}
	
		return item;
	}
	else
	{
		prepareBitampDefinition(packer,bitmapDefinition);
	}
	
	return PYXPointer<PackedTextureItem>();
}

//! make sure that bitmap is ready inside the texturePacker. if there is not bitmap. it would be requested from BitmapServerProvider
void ComponentResources::prepareBitampDefinition(TexturePacker & packer,const std::string & bitmapDefinition)
{
	//request the item
	if (packer.hasTextureItem(bitmapDefinition))
	{
		return;
	}

	//check if we requested this packed texture item
	if (m_loadedTexturesForTexturePacker[&packer].find(bitmapDefinition) == m_loadedTexturesForTexturePacker[&packer].end())
	{
		//create a null pointer in the map - mark that we have request this item
		m_loadedTexturesForTexturePacker[&packer][bitmapDefinition] = PYXPointer<PackedTextureItem>();

		//invoke a request to the loader thread to load this bitmapDefinition
		m_loaderThread.normal.beginInvokeOn(*this,&ComponentResources::requestBitmpaDefinition,&packer,bitmapDefinition);
	}
}

void ComponentResources::requestBitmpaDefinition(TexturePacker * packer,std::string bitmapDefinition)
{
	std::string bitmapData = BitmapServerProvider::getBitmapServerProvider()->loadIcon(bitmapDefinition);

	if (bitmapData.size() < sizeof(BitmapServerData))
	{
		TRACE_ERROR("got invalid bitmap Defintion : " << bitmapDefinition);		
	} 
	else
	{
		m_openGLThread->normal.beginInvokeOn(*this,&ComponentResources::packBitmapDefinition,packer,bitmapDefinition,bitmapData);
	}
}

void ComponentResources::packBitmapDefinition(TexturePacker * packer,std::string bitmapDefinition,std::string bitmapData)
{
	//the string come incoded in base64 - convert it.
	std::string bitmapDataUnpacked = XMLUtils::fromBase64(bitmapData);

	const BitmapServerData * data = static_cast<const BitmapServerData *>(static_cast<const void *>(bitmapDataUnpacked.c_str()));

	if (bitmapDataUnpacked.size() != data->width*data->height*4 + sizeof(int[2]))
	{
		PYXTHROW(PYXException,"bitmap size in not aligen to given width and height");
	}

	//temporarily store this bitmap definition.	
	m_loadedTexturesForTexturePacker[packer][bitmapDefinition] = packer->packTextureItem(bitmapDefinition,data->width,data->height,OpenGLTexture::knDataBRGA,&(data->bitmap0));	
}

void ComponentResources::forgetPendingRequests(TexturePacker & packer)
{
	m_loadedTexturesForTexturePacker.erase(&packer);
}
