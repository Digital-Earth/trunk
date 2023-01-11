// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the APPLICATION_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// APPLICATION_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifndef SWIG
	#define MODULE_APPLICATION_HAS_DECLSPEC
#endif 

#ifdef MODULE_APPLICATION_HAS_DECLSPEC
	#ifdef APPLICATION_EXPORTS
		#define APPLICATION_API __declspec(dllexport)
	#else
		#define APPLICATION_API __declspec(dllimport)
	#endif
#endif
