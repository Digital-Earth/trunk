/******************************************************************************
derm_utils.cpp

begin		: 2007-12-05
copyright	: (C) 2007 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "derm_utils.h"

#include "pyxis/utility/pyxcom.h"
#include "pyxis/derm/wgs84_coord_converter.h"

PYXCOM_BEGIN_CLASS_OBJECT_TABLE
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(WGS84CoordConverter),
PYXCOM_END_CLASS_OBJECT_TABLE_EXPORT_AS(pyxis__derm__derm_utils__GCO)

void DermUtils::initialize()
{
	PYXCOMRegister(pyxis__derm__derm_utils__GCO);
}

void DermUtils::uninitialize()
{
}
