/******************************************************************************
bitmap.cpp

begin		: Feb 01, 2010
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "bitmap.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/file_utils.h"

#include "pyxis/utility/bitmap_server_provider.h"

// standard includes
#include <cassert>


// {C0413A48-71A2-4c7f-83E5-447EFFFC346F}
PYXCOM_DEFINE_IID(IBitmap, 
0xc0413a48, 0x71a2, 0x4c7f, 0x83, 0xe5, 0x44, 0x7e, 0xff, 0xfc, 0x34, 0x6f);





PYXPointer<PYXBitmap> PYXBitmap::createFromDefinition(const std::string & definition)
{
	std::string bitmapData = BitmapServerProvider::getBitmapServerProvider()->loadIcon(definition);

	//the string come incoded in base64 - convert it.
	bitmapData = XMLUtils::fromBase64(bitmapData);

	if (bitmapData.size() < sizeof(BitmapServerData))
	{	
		PYXTHROW(PYXException, "failed to load bitmap from path.");				
	}	

	const BitmapServerData * data = static_cast<const BitmapServerData *>(static_cast<const void *>(bitmapData.c_str()));

	if (bitmapData.size() != data->width*data->height*4 + sizeof(int[2]))
	{		
		PYXTHROW(PYXException,"bitmap size in not aligen to given width and height");
	}

	PYXPointer<PYXBitmap> bitmap = PYXNEW(PYXBitmap,data->width,data->height);

	unsigned char * rgbaData = bitmap->getData();

	const unsigned char * brgaData = &data->bitmap0;	
	
	//convert into RGBA
	for(unsigned int i=0; i < bitmap->getWidth()*bitmap->getHeight()*4; i+=4)
	{
		rgbaData[0] = brgaData[1];
		rgbaData[1] = brgaData[2];
		rgbaData[2] = brgaData[0];
		rgbaData[3] = brgaData[3];

		//move to the next pixel
		rgbaData+=4;
		rgbaData+=4;
	}

	return bitmap;
}

PYXPointer<PYXBitmap> PYXBitmap::createFromPath(const std::string & path)
{
	std::string bitmapData = BitmapServerProvider::getBitmapServerProvider()->loadBitmap(path);

	//the string come incoded in base64 - convert it.
	bitmapData = XMLUtils::fromBase64(bitmapData);

	if (bitmapData.size() < sizeof(BitmapServerData))
	{	
		PYXTHROW(PYXException, "failed to load bitmap from path.");				
	}	

	const BitmapServerData * data = static_cast<const BitmapServerData *>(static_cast<const void *>(bitmapData.c_str()));

	if (bitmapData.size() != data->width*data->height*4 + sizeof(int[2]))
	{		
		PYXTHROW(PYXException,"bitmap size in not aligen to given width and height");
	}

	PYXPointer<PYXBitmap> bitmap = PYXNEW(PYXBitmap,data->width,data->height);

	unsigned char * rgbaData = bitmap->getData();

	const unsigned char * brgaData = &data->bitmap0;	
	
	//convert into RGBA
	for(unsigned int i=0; i < bitmap->getWidth()*bitmap->getHeight()*4; i+=4)
	{
		rgbaData[0] = brgaData[1];
		rgbaData[1] = brgaData[2];
		rgbaData[2] = brgaData[0];
		rgbaData[3] = brgaData[3];

		//move to the next pixel
		rgbaData+=4;
		rgbaData+=4;
	}

	return bitmap;
}

PYXPointer<PYXBitmap> PYXBitmap::create(const unsigned int & width,const unsigned int & height)
{
	PYXPointer<PYXBitmap> bitmap = PYXNEW(PYXBitmap,width,height);

	return bitmap;
}


PYXBitmap::PYXBitmap(const unsigned int & width,const unsigned int & height) : m_width(width),m_height(height)
{
	m_data.reset(new unsigned char[m_width*m_height*4]);
}

PYXBitmap::~PYXBitmap()
{
}