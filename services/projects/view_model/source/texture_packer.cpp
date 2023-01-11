/******************************************************************************
texture_packer.cpp

begin		: 2009-11-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "texture_packer.h"
#include "exceptions.h"

/////////////////////////////////////////
//
// PackedTextureItem
//
/////////////////////////////////////////

PackedTextureItem::PackedTextureItem(PYXPointer<TexturePacker> packer,const std::string & name,PYXPointer<PackedTexture> texture,int x,int y,int width,int height)
: m_packer(packer),m_name(name),m_texture(texture),m_x(x),m_y(y),m_width(width),m_height(height)
{
	m_texture->updatePackingStatus(true,getPackedX(),getPackedY(),getPackedWidth(),getPackedHeight());
}

PackedTextureItem::~PackedTextureItem()
{
}

long PackedTextureItem::release() const
{
	long refCount = PYXObject::release();

	if (refCount==0)
	{
		//this is a critical section	
		boost::recursive_mutex::scoped_lock lock(TexturePacker::s_mutex);

		//no one did getPackedTextureItem - so we can remove it from the TexturePacker.
		//which means that now, no one could get reference to this object anymore.
		if (PYXObject::getRefCount() == 0)
		{
			m_texture->updatePackingStatus(false,getPackedX(),getPackedY(),getPackedWidth(),getPackedHeight());
			m_packer->removePackedTextureItem(this);
		}
				
		return 0;
	}
	
	return refCount;
}

const PYXPointer<PackedTexture> & PackedTextureItem::getTexture()
{
	return m_texture;
}

const std::string & PackedTextureItem::getName()
{
	return m_name;
}

const PYXPointer<TexturePacker> & PackedTextureItem::getTexturePacker()
{
	return m_packer;
}

const int & PackedTextureItem::getWidth() const
{
	return m_width;

}
const int & PackedTextureItem::getHeight() const
{
	return m_height;
}

float PackedTextureItem::getLeftTextureCoord() const
{
	return ((float)m_x+0.5f)/m_texture->getWidth();
}

float PackedTextureItem::getRightTextureCoord() const
{
	return ((float)m_x+m_width-0.5f)/m_texture->getWidth();
}

float PackedTextureItem::getTopTextureCoord() const
{
	return ((float)m_y+m_height-0.5f)/m_texture->getHeight();
}

float PackedTextureItem::getBottomTextureCoord() const
{
	return ((float)m_y+0.5f)/m_texture->getHeight();
}

int PackedTextureItem::getPackedX() const
{
	return m_x/PackedTexture::knGridSize;
}

int PackedTextureItem::getPackedY() const
{
	return m_y/PackedTexture::knGridSize;
}

int PackedTextureItem::getPackedHeight() const
{
	return (m_height-1)/PackedTexture::knGridSize+1;
}

int PackedTextureItem::getPackedWidth() const
{
	return (m_width-1)/PackedTexture::knGridSize+1;
}

void PackedTextureItem::draw(const vec3 & center,const vec3 left,const vec3 up)
{
	double halfWidth = getWidth()/2;
	double halfHeight = getHeight()/2;
	vec3 coord;

	m_texture->startUsing();

	glBegin(GL_QUADS);	
	glTexCoord2f(getRightTextureCoord(),getTopTextureCoord());
	coord = center+left*halfWidth-up*halfHeight;
	glVertex3dv(coord.data());	
		
	glTexCoord2f(getRightTextureCoord(),getBottomTextureCoord());
	coord = center+left*halfWidth+up*halfHeight;
	glVertex3dv(coord.data());		

	glTexCoord2f(getLeftTextureCoord(),getBottomTextureCoord());
	coord = center-left*halfWidth+up*halfHeight;
	glVertex3dv(coord.data());
	
	glTexCoord2f(getLeftTextureCoord(),getTopTextureCoord());
	coord = center-left*halfWidth-up*halfHeight;
	glVertex3dv(coord.data());
	
	glEnd();

	m_texture->stopUsing();
}

/////////////////////////////////////////
//
// PackedTexture
//
/////////////////////////////////////////

PackedTexture::PackedTexture() : OpenGLTexture(knTexture2D,knTextureRGBA)
{
	//allocate texture
	setSize(knTextureSize,knTextureSize,knDataRGBA,knTextelUnsignedByte);
	setMinFilter(OpenGLTexture::knTextureMinLinear);

	//zero the packingStatus
	memset(m_packingStatus,false,sizeof(m_packingStatus));	
}

PackedTexture::~PackedTexture()
{
}

//! called by PackedTextureItem ctor and dtor
void PackedTexture::updatePackingStatus(const bool & value, const int & x, const int & y, const int & width, const int & height)
{
	for(int i_x=x; i_x<x+width; i_x++)
	{
		for(int i_y=y; i_y<y+height; i_y++)
		{
			m_packingStatus[i_x][i_y] = value;
		}
	}
}

bool PackedTexture::findPackingPlace(const int & width, const int & height, PackingPlace & place)
{
	int packedWidth = (width-1)/knGridSize+1;
	int packedHeight = (height-1)/knGridSize+1;

	for(place.x=0;place.x<knGridLength-packedWidth+1;place.x++)
	{
		for(place.y=0;place.y<knGridLength-packedHeight+1;place.y++)
		{
			if (isFree(place.x,place.y,packedWidth,packedHeight))
				return true;
		}
	}
	return false;
}

bool PackedTexture::isFree(const int & x, const int & y, const int & width, const int & height)
{
	for(int i_x=x; i_x<x+width; i_x++)
	{
		for(int i_y=y; i_y<y+height; i_y++)
		{
			if (m_packingStatus[i_x][i_y])
			{
				return false;
			}
		}
	}
	return true;
}


/////////////////////////////////////////
//
// TexturePacker
//
/////////////////////////////////////////

boost::recursive_mutex TexturePacker::s_mutex;

TexturePacker::TexturePacker(void)
{
}

TexturePacker::~TexturePacker(void)
{
}

bool TexturePacker::hasTextureItem(const std::string & name)
{
	//this is a critical section
	boost::recursive_mutex::scoped_lock lock(s_mutex);

	return m_packedItems.find(name) != m_packedItems.end();
}

PYXPointer<PackedTextureItem> TexturePacker::getTextureItem(const std::string & name)
{
	//this is a critical section
	boost::recursive_mutex::scoped_lock lock(s_mutex);

	PackedTextureItemsMap::iterator item = m_packedItems.find(name);

	if (item == m_packedItems.end())
	{
		return PYXPointer<PackedTextureItem>();
	}

	try
	{
		return PYXPointer<PackedTextureItem>(item->second);
	}
	catch (...)
	{
		TRACE_ERROR("failed to fetch packedTextureItem from map");
		return PYXPointer<PackedTextureItem>();
	}
}

PYXPointer<PackedTextureItem> TexturePacker::packTextureItem(const std::string & name,const int & width,const int & height,const OpenGLTexture::DataFormat & format,const unsigned char * data)
{
	//this is a critical section
	boost::recursive_mutex::scoped_lock lock(s_mutex);

	//check if we have that name already
	if (hasTextureItem(name))
	{
		PYXTHROW(PYXException,"Can't packed two textures with the same name");
	}

	PackedTextureList::iterator itTexture = m_textures.begin();

	PackingPlace place;
	PYXPointer<PackedTexture> spTexture = NULL;

	//find a texture with a free place to pack
	while (itTexture != m_textures.end())
	{
		spTexture = *itTexture;
		if (spTexture->findPackingPlace(width,height,place))
		{
			break;
		}
		itTexture++;
	}

	//if we didn't found any texture - create another one
	if (itTexture == m_textures.end())
	{
		//create new texture and place it in front of all textures
		spTexture = PYXNEW(PackedTexture);
		m_textures.push_front(spTexture);

		//find a place in a the new texture
		spTexture->findPackingPlace(width,height,place);
	}
	
	spTexture->setDataRegion(place.x*PackedTexture::knGridSize,place.y*PackedTexture::knGridSize,width,height,format,data);
	spTexture->stopUsing();
	PYXPointer<PackedTextureItem> spItem = PYXNEW(PackedTextureItem,this,name,spTexture,place.x*PackedTexture::knGridSize,place.y*PackedTexture::knGridSize,width,height);

	m_packedItems[name] = spItem.get();

	return spItem;
}

void TexturePacker::removePackedTextureItem(const PackedTextureItem* item)
{
	//this is a critical section
	boost::recursive_mutex::scoped_lock lock(s_mutex);

	m_packedItems.erase(item->m_name);
}

void TexturePacker::clearOpenGLResources()
{
	//this is a critical section
	boost::recursive_mutex::scoped_lock lock(s_mutex);

	PackedTextureList::iterator itTexture = m_textures.begin();

	//find a texture with a free place to pack
	while (itTexture != m_textures.end())
	{
		//very dangures operation, the texture would not be validated any more. this mean that the state of this TexturePakcer is unsafe anymore
		(*itTexture)->forceRelease();
		itTexture++;
	}

	//erase all textures and packed textures items
	m_textures.clear();
	m_packedItems.clear();
}