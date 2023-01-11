/******************************************************************************
excel.h

begin      : 07/11/2007 11:59:38 AM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#ifndef EXCEL__EXCEL_H
#define EXCEL__EXCEL_H

// TODO determine this properly
#define EXCEL_HAS_DECLSPEC

#ifdef EXCEL_HAS_DECLSPEC // defined in config system
	// Import/export code unless the user has specifically disabled it.
#	if !defined(EXCEL_STATIC_LINK)
		// Export if this is our own source, otherwise import:
#		ifdef EXCEL_SOURCE
#			define EXCEL_DECL __declspec(dllexport)
#		else
#			define EXCEL_DECL __declspec(dllimport)
#		endif
#	endif
#endif

#ifndef EXCEL_DECL
#	define EXCEL_DECL
#endif

// TODO make this more like boost mechanism
#if !defined(EXCEL_SOURCE)
#	pragma comment(lib, "excel.lib")
#endif

#endif
