#ifndef FORCE_INCLUDE_H
#define FORCE_INCLUDE_H
/******************************************************************************
force_include.h

begin		: 2003-12-08
copyright	: (C) 2003 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
****************************************************************************/

// memory leak detection
#ifndef NDEBUG
#ifdef _WINDOWS 
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#endif

/*
This is the first file included when compiling on the Windows platform. It is
intended for global pragmas and defines not required by other platforms. The
file is included through the /FI compiler option.
*/

// turns off Windows macro definitions for min and max
// which interfere with the standard C++ library
#define NOMINMAX

#ifdef _MSC_VER
#	ifndef _CRT_SECURE_NO_DEPRECATE
#		define _CRT_SECURE_NO_DEPRECATE
#	endif
#	ifndef _CRT_NONSTDC_NO_DEPRECATE
#		define _CRT_NONSTDC_NO_DEPRECATE
#	endif
#	ifndef _SCL_SECURE_NO_DEPRECATE
#		define _SCL_SECURE_NO_DEPRECATE
#	endif

	/*
	The MSVC 8.0 compiler exhibits a bug related to template specialization
	and use of 'dllexport', whereby error C2375 is generated and the project
	cannot build.  See here for details:

	http://connect.microsoft.com/VisualStudio/feedback/ViewFeedback.aspx?FeedbackID=99611

	It appears that this has been fixed in an as-yet-unreleased version.
	Remove this definition when the project can build without it.
	*/
#	define MSVC_BUG_99611
#endif

// dllexport and template issues, TODO is there a better way to resolve?
// http://groups.google.ca/group/microsoft.public.vc.language/browse_frm/thread/c1ad0f87f1ff721b/2d35884bbec66f97?lnk=st&q=warning+c4251&rnum=3&hl=en#2d35884bbec66f97
#pragma warning(disable:4251)

// The following code automatically creates manifests to reference the c runtime.
// Source: http://stackoverflow.com/questions/110249/building-and-deploying-dll-on-windows-sxs-manifests-and-all-that-jazz

/*----------------------------------------------------------------------------*/

#if _MSC_VER >= 1400

/*----------------------------------------------------------------------------*/

#pragma message ( "Setting up manifest..." )

/*----------------------------------------------------------------------------*/

/*
#ifndef _CRT_ASSEMBLY_VERSION
#include <crtassem.h>
#endif 

#ifdef WIN64
    #pragma message ( "processorArchitecture=amd64" )
    #define MF_PROCESSORARCHITECTURE "amd64"
#else
    #pragma message ( "processorArchitecture=x86" )
    #define MF_PROCESSORARCHITECTURE "x86"
#endif 

#pragma message ( "Microsoft.Windows.Common-Controls=6.0.0.0") 
#pragma comment ( linker,"/manifestdependency:\"type='win32' " \
                          "name='Microsoft.Windows.Common-Controls' " \
                          "version='6.0.0.0' " \
                          "processorArchitecture='" MF_PROCESSORARCHITECTURE "' " \
                          "publicKeyToken='6595b64144ccf1df'\"" )

#ifdef _DEBUG
    #pragma message ( __LIBRARIES_ASSEMBLY_NAME_PREFIX ".DebugCRT=" _CRT_ASSEMBLY_VERSION ) 
    #pragma comment(linker,"/manifestdependency:\"type='win32' "            \
                "name='" __LIBRARIES_ASSEMBLY_NAME_PREFIX ".DebugCRT' "         \
                "version='" _CRT_ASSEMBLY_VERSION "' "                          \
                "processorArchitecture='" MF_PROCESSORARCHITECTURE "' "         \
                "publicKeyToken='" _VC_ASSEMBLY_PUBLICKEYTOKEN "'\"")
#else
    #pragma message ( __LIBRARIES_ASSEMBLY_NAME_PREFIX ".CRT=" _CRT_ASSEMBLY_VERSION ) 
    #pragma comment(linker,"/manifestdependency:\"type='win32' "            \
                "name='" __LIBRARIES_ASSEMBLY_NAME_PREFIX ".CRT' "              \
                "version='" _CRT_ASSEMBLY_VERSION "' "                          \
                "processorArchitecture='" MF_PROCESSORARCHITECTURE "' "         \
                "publicKeyToken='" _VC_ASSEMBLY_PUBLICKEYTOKEN "'\"")
#endif

#ifdef _MFC_ASSEMBLY_VERSION
    #ifdef _DEBUG
        #pragma message ( __LIBRARIES_ASSEMBLY_NAME_PREFIX ".MFC=" _CRT_ASSEMBLY_VERSION ) 
        #pragma comment(linker,"/manifestdependency:\"type='win32' "            \
                        "name='" __LIBRARIES_ASSEMBLY_NAME_PREFIX ".MFC' "              \
                        "version='" _MFC_ASSEMBLY_VERSION "' "                          \
                        "processorArchitecture='" MF_PROCESSORARCHITECTURE "' "         \
                        "publicKeyToken='" _VC_ASSEMBLY_PUBLICKEYTOKEN "'\"")
    #else
        #pragma message ( __LIBRARIES_ASSEMBLY_NAME_PREFIX ".MFC=" _CRT_ASSEMBLY_VERSION ) 
        #pragma comment(linker,"/manifestdependency:\"type='win32' "            \
                        "name='" __LIBRARIES_ASSEMBLY_NAME_PREFIX ".MFC' "              \
                        "version='" _MFC_ASSEMBLY_VERSION "' "                          \
                        "processorArchitecture='" MF_PROCESSORARCHITECTURE "' "         \
                        "publicKeyToken='" _VC_ASSEMBLY_PUBLICKEYTOKEN "'\"")
    #endif
#endif /* _MFC_ASSEMBLY_VERSION */

#endif /* _MSC_VER */


#endif // guard
