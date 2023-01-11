/******************************************************************************
grib.cpp

begin      : 26/03/2007 6:06:39 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define GRIB_SOURCE
#include "grib.h"

// local includes
#include "grib_pipe_builder.h"
#include "grib_process.h"


// pyxlib includes
#include "pyxis/utility/mem_utils.h"

PYXCOM_BEGIN_CLASS_OBJECT_TABLE
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(GRIBProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(GRIBPipeBuilder),
PYXCOM_END_CLASS_OBJECT_TABLE

namespace
{

//! Static object to manage module.
struct ModuleManager
{

	ModuleManager()
	{
	}

	~ModuleManager()
	{
	}

} moduleManager;

}
