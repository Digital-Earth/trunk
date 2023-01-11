/******************************************************************************
module_driver_utility.cpp

begin		: 2008-06-03
copyright	: (C) 2008 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_DRIVER_UTILITY_SOURCE
#include "module_driver_utility.h"
#include "coord_converter_impl.h"

PYXCOM_BEGIN_CLASS_OBJECT_TABLE	
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(CoordConverterImpl),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(CoordConverterImplFactory)
PYXCOM_END_CLASS_OBJECT_TABLE