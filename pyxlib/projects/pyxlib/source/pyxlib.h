#ifndef PYXLIB_H
#define PYXLIB_H

// Swig can't handle declspecs, and doesn't need to know them anyway.
// Everyone else gets a proper declspec.
#ifndef SWIG
#   define PYXLIB_HAS_DECLSPEC
#endif

#ifdef PYXLIB_HAS_DECLSPEC // defined in config system
	/*
	We need to import/export our code unless the user has specifically
	disabled it by defining PYXLIB_STATIC_LINK if they want all pyxlib
	libraries to be statically linked:
	*/
#	if !defined(PYXLIB_STATIC_LINK)
		// Export if this is our own source, otherwise import:
#		ifdef PYXLIB_SOURCE
#			define PYXLIB_DECL __declspec(dllexport)
#		else
#			define PYXLIB_DECL __declspec(dllimport)
#		endif  // PYXLIB_SOURCE
#	endif  // STATIC_LINK
#endif  // PYXLIB_HAS_DECLSPEC

// If PYXLIB_DECL isn't defined yet, define it now:
#ifndef PYXLIB_DECL
#	define PYXLIB_DECL
#endif

// TODO make this more like boost mechanism
#if !defined(PYXLIB_SOURCE)
#	pragma comment(lib, "pyxlib.lib")
#endif

// Define INSTANCE_COUNTING if you want to track the number of instances of
// every class derived from PYXObject or PYXCOM.
//#define INSTANCE_COUNTING

#endif // guard
