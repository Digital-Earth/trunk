/******************************************************************************
exceptions.cpp

begin      : 29/04/2008 4:56:42 PM
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_GDAL_SOURCE

#include "exceptions.h"

// {68F0FC89-2D83-439c-BD4E-72A8A9CCDCED}
PYXCOM_DEFINE_CLSID(GDALSRSInitError, 
0x68f0fc89, 0x2d83, 0x439c, 0xbd, 0x4e, 0x72, 0xa8, 0xa9, 0xcc, 0xdc, 0xed);
PYXCOM_CLASS_INTERFACES(GDALSRSInitError, IProcessInitError::iid, PYXCOM_IUnknown::iid);

// {62878998-D2B8-4F98-BA48-7ECAD2B523F0}
PYXCOM_DEFINE_CLSID(MissingGeometryInitError, 
0x62878998, 0xd2b8, 0x4f98, 0xba, 0x48, 0x7e, 0xca, 0xd2, 0xb5, 0x23, 0xf0);
PYXCOM_CLASS_INTERFACES(MissingGeometryInitError, IProcessInitError::iid, PYXCOM_IUnknown::iid);

// {ACA50CE2-E822-49D2-AFE1-1AE5BA7966E9}
PYXCOM_DEFINE_CLSID(MissingWorldFileInitError, 
0xaca50ce2, 0xe822, 0x49d2, 0xaf, 0xe1, 0x1a, 0xe5, 0xba, 0x79, 0x66, 0xe9);
PYXCOM_CLASS_INTERFACES(MissingWorldFileInitError, IProcessInitError::iid, PYXCOM_IUnknown::iid);

