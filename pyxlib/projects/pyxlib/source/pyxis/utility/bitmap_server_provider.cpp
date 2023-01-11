/******************************************************************************
bitmap_server_provider.cpp

begin		: 2009-11-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "bitmap_server_provider.h"
#include "exceptions.h"
#include "xml_utils.h"

PYXPointer<BitmapServerProvider> BitmapServerProvider::m_spProvider;

PYXPointer<BitmapServerProvider> BitmapServerProvider::getBitmapServerProvider()
{
	return m_spProvider;
}

void BitmapServerProvider::setBitmapServerProvider( PYXPointer<BitmapServerProvider> spProvider)
{
	m_spProvider = spProvider;
}



std::string BitmapServerProvider::loadIcon(std::string iconStyle)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setBitmapServerProvider()");
	return "";
}

std::string BitmapServerProvider::loadResource(std::string resourceName)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setBitmapServerProvider()");
	return "";
}

std::string BitmapServerProvider::loadBitmap(std::string path)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setBitmapServerProvider()");
	return "";
}

std::string BitmapServerProvider::forceRGB(std::string path)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setBitmapServerProvider()");
}


