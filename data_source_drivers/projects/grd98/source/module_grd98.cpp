/******************************************************************************
module_grd98.cpp

begin		: 2007-03-06
copyright	: (C) 2007 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_GRD98_SOURCE
#include "module_grd98.h"

#include "grd98_process.h"
#include "grd98_pipe_builder.h"

PYXCOM_BEGIN_CLASS_OBJECT_TABLE
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(GRD98Process),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(GRD98PipeBuilder),
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
