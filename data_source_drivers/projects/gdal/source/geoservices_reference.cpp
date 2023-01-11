/******************************************************************************
geoservices_reference.cpp

begin      : 2016-02-09
copyright  : (c) 2016 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_GDAL_SOURCE

// local includes
#include "geoservices_reference.h"

// {BC89116A-FF90-4015-A551-0443F2162435}
PYXCOM_DEFINE_IID(IGeoServicesReference, 
0xbc89116a, 0xff90, 0x4015, 0xa5, 0x51, 0x04, 0x43, 0xf2, 0x16, 0x24, 0x35);

// {3F4FCB36-58EA-43BE-86CB-248EFD70D2AE}
PYXCOM_DEFINE_IID(IGeoServicesFeatureServerReference, 
0x3f4fcb36, 0x58ea, 0x43be, 0x86, 0xcb, 0x24, 0x8e, 0xfd, 0x70, 0xd2, 0xae);
