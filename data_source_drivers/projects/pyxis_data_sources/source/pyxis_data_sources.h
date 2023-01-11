#ifndef PYXIS_DATA_SOURCES_H
#define PYXIS_DATA_SOURCES_H

// TODO determine this properly
#define PYXIS_DATA_SOURCE_HAS_DECLSPEC

#ifdef PYXIS_DATA_SOURCE_HAS_DECLSPEC // defined in config system
	// Import/export code unless the user has specifically disabled it.
#	if !defined(PYXIS_DATA_SOURCE_STATIC_LINK)
		// Export if this is our own source, otherwise import:
#		ifdef PYXIS_DATA_SOURCES_SOURCE
#			define PXYIS_DATA_SOURCE_DECL __declspec(dllexport)
#		else
#			define PXYIS_DATA_SOURCE_DECL __declspec(dllimport)
#		endif
#	endif
#endif

#ifndef PXYIS_DATA_SOURCE_DECL
#	define PXYIS_DATA_SOURCE_DECL
#endif

// TODO make this more like boost mechanism
#if !defined(PYXIS_DATA_SOURCES_SOURCE)
#	pragma comment(lib, "pyxis_data_sources.lib")
#endif

#endif // guard
