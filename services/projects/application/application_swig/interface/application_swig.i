%module application;
%{


// So PYXObject operators are accessible (better way?)
#define SWIG_INTERNAL
#include "../../../../config/windows/force_include.h"
#include "pyxis/utility/exception.h"
#include "document.h"
#include "camera_view.h"
#include "pyxis/utility/pyxcom.h"

// We get 64-bit portability warning C4267
// It's safe for us now, and might even be safe when re-SWIG'd for Win64.
// So I sent a message to swig-devel mailing list and disabled it for now.
// http://msdn2.microsoft.com/en-gb/library/6kck0s93(VS.80).aspx
#pragma warning(disable: 4267)

%}

//%include "application.h"
// TODO sort this out
#define APPLICATION_API
#define PYXLIB_DECL

////////////////////////////////////////////////////////////////////////////////

%import "pyxlib_swig.i"

%include "document.h"
%include "camera_view.h"

////////////////////////////////////////////////////////////////////////////////////

%template(IDocument_SPtr) boost::intrusive_ptr<IDocument>;
%template(IDocument_CSPtr) boost::intrusive_ptr<const IDocument>;
%template(ICameraView_SPtr) boost::intrusive_ptr<ICameraView>;
%template(ICameraView_CSPtr) boost::intrusive_ptr<const ICameraView>;


FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IDocument)
FEATURE_QUERY_INTERFACE(IDocument, IProcess)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, ICameraView)
FEATURE_QUERY_INTERFACE(ICameraView, IProcess)