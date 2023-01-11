#ifndef LIBRARY_CONFIG_H
#define LIBRARY_CONFIG_H
/******************************************************************************
library_config.h

begin		: 2007-02-07
copyright	: (C) 2007 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// TODO make this properly defined
#ifdef LIBRARY_SOURCE
#	define LIBRARY_DECL __declspec(dllexport)
#else
#	define LIBRARY_DECL __declspec(dllimport)
#	pragma comment(lib, "library.lib")
#endif

#endif // guard
