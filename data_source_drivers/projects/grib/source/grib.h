#ifndef GRIB_H
#define GRIB_H

// TODO determine this properly
#define GRIB_HAS_DECLSPEC

#ifdef GRIB_HAS_DECLSPEC // defined in config system
	// Import/export code unless the user has specifically disabled it.
#	if !defined(GRIB_STATIC_LINK)
		// Export if this is our own source, otherwise import:
#		ifdef GRIB_SOURCE
#			define GRIB_DECL __declspec(dllexport)
#		else
#			define GRIB_DECL __declspec(dllimport)
#		endif
#	endif
#endif

#ifndef GRIB_DECL
#	define GRIB_DECL
#endif

// TODO make this more like boost mechanism
#if !defined(GRIB_SOURCE)
#	pragma comment(lib, "grib.lib")
#endif

#endif
