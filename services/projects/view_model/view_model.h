// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the VIEW_MODEL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// VIEW_MODEL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef VIEW_MODEL_EXPORTS
#define VIEW_MODEL_API __declspec(dllexport)
#else
#define VIEW_MODEL_API __declspec(dllimport)
#endif

#pragma once

// Size limit for the STile cache.  This is also used to control the 
// texture cache size.
const int knMaxSTileCacheSize = 250;
