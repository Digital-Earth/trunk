/******************************************************************************
library_config.cpp

begin		: 2007-02-07
copyright	: (C) 2007 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define LIBRARY_SOURCE
#include "library_config.h"

// pyxlib includes
#include "pyxis/utility/pyxcom.h"

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
