#ifndef PROTOTYPE_PROCESS_CONFIG_H
#define PROTOTYPE_PROCESS_CONFIG_H
/******************************************************************************
prototype_process_config.h

begin		: 2006-12-04
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// TODO determine this properly
#define PROTOTYPE_PROCESS_HAS_DECLSPEC

#ifdef PROTOTYPE_PROCESS_HAS_DECLSPEC // defined in config system
	/*
	We need to import/export our code unless the user has specifically
	disabled it by defining PYXLIB_STATIC_LINK if they want all pyxlib
	libraries to be statically linked:
	*/
#	if !defined(PROTOTYPE_PROCESS_STATIC_LINK)
		// Export if this is our own source, otherwise import:
#		ifdef PROTOTYPE_PROCESS_SOURCE
#			define PROTOTYPE_PROCESS_DECL __declspec(dllexport)
#		else
#			define PROTOTYPE_PROCESS_DECL __declspec(dllimport)
#		endif  // PYXLIB_SOURCE
#	endif  // STATIC_LINK
#endif  // PYXLIB_HAS_DECLSPEC

// If PYXLIB_DECL isn't defined yet, define it now:
#ifndef PROTOTYPE_PROCESS_DECL
#	define PROTOTYPE_PROCESS_DECL
#endif

// TODO make this more like boost mechanism
#if !defined(PROTOTYPE_PROCESS_SOURCE)
#	pragma comment(lib, "prototype_process.lib")
#endif

#endif // guard
