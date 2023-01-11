#ifndef MODULE_LUA_PROCS_H
#define MODULE_LUA_PROCS_H
/******************************************************************************
module_lua_procs.h

begin		: 2007-03-23
copyright	: (C) 2007 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// Swig can't handle declspecs, and doesn't need to know them anyway.
// Everyone else gets a proper declspec.
#ifndef SWIG
#   define MODULE_LUA_PROCS_HAS_DECLSPEC
#endif

#ifdef MODULE_LUA_PROCS_HAS_DECLSPEC // defined in config system
	/*
	We need to import/export our code unless the user has specifically
	disabled it by defining MODULE_XXX_STATIC_LINK if they want all module
	libraries to be statically linked:
	*/
#	if !defined(MODULE_LUA_PROCS_STATIC_LINK)
		// Export if this is our own source, otherwise import:
#		ifdef MODULE_LUA_PROCS_SOURCE
#			define MODULE_LUA_PROCS_DECL __declspec(dllexport)
#		else
#			define MODULE_LUA_PROCS_DECL __declspec(dllimport)
#		endif  // MODULE_XXX_SOURCE
#	endif  // MODULE_XXX_STATIC_LINK
#endif  // MODULE_XXX_HAS_DECLSPEC

// If MODULE_XXX_DECL isn't defined yet, define it now:
#ifndef MODULE_LUA_PROCS_DECL
#	define MODULE_LUA_PROCS_DECL
#endif

// TODO make this more like boost mechanism
#if !defined(MODULE_LUA_PROCS_SOURCE)
#	pragma comment(lib, "lua_procs.lib")
#endif

#endif // guard
