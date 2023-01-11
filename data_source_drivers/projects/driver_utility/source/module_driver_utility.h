#ifndef MODULE_DRIVER_UTILITY_H
#define MODULE_DRIVER_UTILITY_H
/******************************************************************************
module_driver_utility.h

begin		: 2008-06-03
copyright	: (C) 2008 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// Swig can't handle declspecs, and doesn't need to know them anyway.
// Everyone else gets a proper declspec.
#ifndef SWIG
#   define MODULE_DRIVER_UTILITY_HAS_DECLSPEC
#endif

//MODULE_GDAL_DECL
#ifdef MODULE_DRIVER_UTILITY_HAS_DECLSPEC // defined in config system
	/*
	We need to import/export our code unless the user has specifically
	disabled it by defining MODULE_XXX_STATIC_LINK if they want all module
	libraries to be statically linked:
	*/
#	if !defined(MODULE_DRIVERY_UTILITY_STATIC_LINK)
		// Export if this is our own source, otherwise import:
#		ifdef MODULE_DRIVER_UTILITY_SOURCE
#			define MODULE_DRIVER_UTILITY_DECL __declspec(dllexport)
#		else
#			define MODULE_DRIVER_UTILITY_DECL __declspec(dllimport)
#		endif  // MODULE_XXX_SOURCE
#	endif  // MODULE_XXX_STATIC_LINK
#endif  // MODULE_XXX_HAS_DECLSPEC

// If MODULE_XXX_DECL isn't defined yet, define it now:
#ifndef MODULE_DRIVER_UTILITY_DECL
#	define MODULE_DRIVER_UTILITY_DECL
#endif

// TODO make this more like boost mechanism
#if !defined(MODULE_DRIVER_UTILITY_SOURCE)
#	pragma comment(lib, "driver_utility.lib")
#endif

#endif // guard
