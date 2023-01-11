#pragma once
#ifndef VIEW_MODEL__TEXTURE_PACKER_H
#define VIEW_MODEL__TEXTURE_PACKER_H

/******************************************************************************
texture_packer.h

begin		: 2009-11-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "open_gl_texture.h"

#include "pyxis\utility\object.h"

#include <boost/thread/mutex.hpp>

#include <string>
#include <map>
#include <list>

//! Helper class for specifiy a place
struct PackingPlace {
	int x;
	int y;
};

/*!
the PackedTextureItem is the result class for asking the TexturePacker to pack a texture.

Keep a reference pointer (PYXPointer<>) to this as long as you need this icon/texture area.

When the PackedTextureItem reference count would be zero - it would auto maticly release it self from the texture.
*/
//! PackedTextureItem represenet a part of a texture
class PackedTextureItem : public PYXObject
{
	friend class PackedTexture;
	friend class TexturePacker;

protected:
	//! the acctual x-place inside the texture
	int m_x;
	//! the acctual y-place inside the texture
	int m_y;
	//! the acctual Item width
	int m_width;
	//! the acctual Item height
	int m_height;

	//! the name of PackedTextureItem
	const std::string m_name;
	//! the packer that create this PackedTextureItem
	mutable PYXPointer<TexturePacker> m_packer;

	//! the texture (PackedTexture) that is item is been packed inside
	PYXPointer<PackedTexture> m_texture;

	//! ctor - only created by the TexturePacker
	PackedTextureItem(PYXPointer<TexturePacker> packer,const std::string & name,PYXPointer<PackedTexture> texture,int x,int y,int width,int height);

	//! remove itself from the texture and the texture packer map
	virtual ~PackedTextureItem();

protected:
	//! we need to override the release operation to make sure the PackedTextureItem would not destory the reference counting
	virtual long release() const;

public:
	//! get the actual texture
	const PYXPointer<PackedTexture> & getTexture();
	//! get name
	const std::string & getName();
	//! get texture packer
	const PYXPointer<TexturePacker> & getTexturePacker();
	
	//! get the actual left texture coord between [0..1]
	float getLeftTextureCoord() const;
	//! get the actual right texture coord between [0..1]
	float getRightTextureCoord() const;
	//! get the actual top texture coord between [0..1]
	float getTopTextureCoord() const;
	//! get the actual bottom texture coord between [0..1]
	float getBottomTextureCoord() const;

	const int & getWidth() const;
	const int & getHeight() const;

protected:
	//! get the actual packed left coord in the packedStatus array of PackedTexture
	int getPackedX() const;
	//! get the actual packed top coord in the packedStatus array of PackedTexture
	int getPackedY() const;
	//! get the actual packed height in units of the packedStatus array of PackedTexture
	int getPackedHeight() const;
	//! get the actual packed width in units of the packedStatus array of PackedTexture
	int getPackedWidth() const;

public:
	
	//! draw a rectangle using the current PackedTextureItem as the texture
	void draw(const vec3 & center,const vec3 left,const vec3 up);
};

//TODO: if texture become invalid - we need to recreate all PackedItems. and we can't do it. so we need to figure this one out
//TODO: right now, the textures are not been freed once allocated - need to fix this also
class PackedTexture : public OpenGLTexture
{
	friend class PackedTextureItem;
	friend class TexturePacker;

//static decleartions
public:
	static const int knTextureSize = 256;
	static const int knGridSize = 32;
	static const int knGridLength = knTextureSize/knGridSize;

	
	typedef bool PackingStatus[knGridLength][knGridLength];

//ctor and dtor
public:
	PackedTexture();
	virtual ~PackedTexture();

//members
protected:	
	//! true when the texture region is been used by a PackedTextureItem
	PackingStatus m_packingStatus;
	
protected:
	//! update a region on the PackingStatus
	void updatePackingStatus(const bool & value, const int & x, const int & y, const int & width, const int & height);
	
	//! find a free place for packing
	bool findPackingPlace(const int & width, const int & height, PackingPlace & place);

	//! query if a region is free
	bool isFree(const int & x, const int & y, const int & width, const int & height);
};

class TexturePacker : public PYXObject
{
	friend class PackedTextureItem;

public:
	TexturePacker(void);
	virtual ~TexturePacker(void);

protected:
	typedef std::list<PYXPointer<PackedTexture>> PackedTextureList;
	typedef std::map<std::string,PackedTextureItem*> PackedTextureItemsMap;
	
	//! list of all textures used for packing
	PackedTextureList m_textures;

	//! map of all packed textures items
	PackedTextureItemsMap m_packedItems;

	//! mutex for thread safey
	static boost::recursive_mutex s_mutex;

public:
	//! return true if the name has a PackedTextureItem inside
	bool hasTextureItem(const std::string & name);

	//! get a PackedTextureItem. throws if there is not packed texture item
	PYXPointer<PackedTextureItem> getTextureItem(const std::string & name);
	
	//! packed a texture item with a specific name
	PYXPointer<PackedTextureItem> packTextureItem(const std::string & name, const int & width, const int & height, const OpenGLTexture::DataFormat & format, const unsigned char * data);

protected:
	//! called by PackedTextureItem dtor - auto clean up
	void removePackedTextureItem(const PackedTextureItem* item);

public:
	//! erase and invalidate all pakced items. note - this packed items that exsits can't be used anymore
	void clearOpenGLResources();
};

#endif