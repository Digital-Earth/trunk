#pragma once
#ifndef PYXIS__UTILITY__BITMAP_SERVER_PROVIDER_H
#define PYXIS__UTILITY__BITMAP_SERVER_PROVIDER_H
/******************************************************************************
bitmap_server_provider.h

begin		: 2009-11-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "object.h"

#include <string>

//! Helper class of converting std::string returned fomr BitmapServerProvider into bitmap data
struct BitmapServerData
{
	unsigned int width;
	unsigned int height;
	unsigned char bitmap0;
};

//! BitmapServerProvider is an singeltone class that allow loading icons
class PYXLIB_DECL BitmapServerProvider : public PYXObject
{
public:

	/*!
	Loads an Icon from a string the describe the Icon
    
	the iconStyle is a string in XML format.

    the returend string is in the following format: (int32 width)(int32 height)(bitmap data as 32bit_BRGA). or null if icon loading failed
	*/
	//! loads an Icon from and iconStyle
	virtual std::string loadIcon(std::string iconStyle);

	/*!
	Loads an bimap from a resource name
    
    the returend string is in the following format: (int32 width)(int32 height)(bitmap data as 32bit_BRGA). or null if icon loading failed
	*/
	//! loads an bitmap from and resource name
	virtual std::string loadResource(std::string resourceName);


	/*!
	Loads an bimap from a path of a local file name
    
    the returend string is in the following format: (int32 width)(int32 height)(bitmap data as 32bit_BRGA). or null if icon loading failed
	*/
	//! loads an bitmap from and resource name
	virtual std::string loadBitmap(std::string path);

	/*!
	Force the given file to be RGB
    
	this function might rewrite the given file if needed.
	if the file can not be rewriten -the return value would be the new path for a new image with RGB.
	*/
	//! loads an bitmap from and resource name
	virtual std::string forceRGB(std::string path);

	// SWIG doesn't know about addRef and release, since they are defined in 
	// the opaque PYXObject.  Add them here so they get director'ed.
	virtual long release() const
	{
		return PYXObject::release();
	}

	virtual long addRef() const
	{
		return PYXObject::addRef();
	}

	//! Virtual destructor.
	virtual ~BitmapServerProvider()
	{}

private:

	static PYXPointer<BitmapServerProvider> m_spProvider;

public:

	static PYXPointer<BitmapServerProvider> getBitmapServerProvider();
	static void setBitmapServerProvider( PYXPointer<BitmapServerProvider> spProvider);
};


#endif
