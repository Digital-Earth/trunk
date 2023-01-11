
%module library_swig;
%{

// So PYXObject operators are accessible (better way?)
#define SWIG_INTERNAL
#include "../../../../config/windows/force_include.h"

#include "library_process_resolver.h"

// We get 64-bit portability warning C4267
// It's safe for us now, and might even be safe when re-SWIG'd for Win64.
// So I sent a message to swig-devel mailing list and disabled it for now.
// http://msdn2.microsoft.com/en-gb/library/6kck0s93(VS.80).aspx
#pragma warning(disable: 4267)

%}

// TODO sort this out
#undef LIBRARY_DECL
#define LIBRARY_DECL
#define PYXLIB_DECL

// Standard type mappings
%include "std_vector.i"

//%import "../../../third_party/boost/boost/intrusive_ptr.hpp"
%import "../../../third_party/boost/boost/smart_ptr/intrusive_ptr.hpp"
%include "pyxis/utility/pointer.h"

%template(LibraryProcessResolver_SPtr) PYXPointer<LibraryProcessResolver>;

%import "pyxlib_swig.i"

%module(directors="1") library_process_resolver
%{
	#include "library_process_resolver.h"
%}

%feature("director") LibraryProcessResolver;
%include "library_process_resolver.h"