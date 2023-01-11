// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once

// Local Header Files
#pragma warning(disable : 4192)	// ignore excluded symbol warnings
#import "riched20.dll" raw_interfaces_only, raw_native_types, no_namespace, named_guids, exclude("UINT_PTR")  rename("FindText", "FindTextRichEd")

#include <malloc.h>
#include <memory.h>
#include <tchar.h>
