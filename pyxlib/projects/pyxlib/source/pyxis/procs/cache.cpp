/******************************************************************************
cache.cpp

begin      : 3/14/2007 3:45:46 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxlib.h"
#include "cache.h"

// {E6605A22-377E-41c0-A1FC-F0FD9F2DCDAC}
PYXCOM_DEFINE_IID(ICache, 
0xe6605a22, 0x377e, 0x41c0, 0xa1, 0xfc, 0xf0, 0xfd, 0x9f, 0x2d, 0xcd, 0xac);

Notifier CacheManager::m_cacheCreatedNotifier("New cache created notifier");