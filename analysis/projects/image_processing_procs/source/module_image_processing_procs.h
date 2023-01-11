#ifndef MODULE_IMAGE_PROCESSING_PROCS_H
#define MODULE_IMAGE_PROCESSING_PROCS_H
/******************************************************************************
module_image_processing_procs.h

begin		: 2007-06-26
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/


// Swig can't handle declspecs, and doesn't need to know them anyway.
// Everyone else gets a proper declspec.
#ifndef SWIG
#   define MODULE_IMAGE_PROCESSING_PROCS_HAS_DECLSPEC
#endif

#ifdef MODULE_IMAGE_PROCESSING_PROCS_HAS_DECLSPEC // defined in config system
	/*
	We need to import/export our code unless the user has specifically
	disabled it by defining MODULE_XXX_STATIC_LINK if they want all module
	libraries to be statically linked:
	*/
#	if !defined(MODULE_IMAGE_PROCESSING_PROCS_STATIC_LINK)
		// Export if this is our own source, otherwise import:
#		ifdef MODULE_IMAGE_PROCESSING_PROCS_SOURCE
#			define MODULE_IMAGE_PROCESSING_PROCS_DECL __declspec(dllexport)
#		else
#			define MODULE_IMAGE_PROCESSING_PROCS_DECL __declspec(dllimport)
#		endif  // MODULE_XXX_SOURCE
#	endif  // MODULE_XXX_STATIC_LINK
#endif  // MODULE_XXX_HAS_DECLSPEC

// If MODULE_XXX_DECL isn't defined yet, define it now:
#ifndef MODULE_IMAGE_PROCESSING_PROCS_DECL
#	define MODULE_IMAGE_PROCESSING_PROCS_DECL
#endif

// TODO make this more like boost mechanism
#if !defined(MODULE_IMAGE_PROCESSING_PROCS_SOURCE)
#	pragma comment(lib, "image_processing_procs.lib")
#endif

#endif // guard
