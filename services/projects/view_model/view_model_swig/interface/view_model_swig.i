
%module (directors="1") view_model_swig;

%include "std_vector.i"

%{

// So PYXObject operators are accessible (better way?)
#define SWIG_INTERNAL
#include "../../../../config/windows/force_include.h"

#include "pyxis/utility/exception.h"

#include "camera.h"
#include "go_to_pipeline_command.h"
#include "view.h"
#include "ui_events.h"
#include "view_model_api.h"
// #include "rhombus.h"
// #include "rhombus_utils.h"

// We get 64-bit portability warning C4267
// It's safe for us now, and might even be safe when re-SWIG'd for Win64.
// So I sent a message to swig-devel mailing list and disabled it for now.
// http://msdn2.microsoft.com/en-gb/library/6kck0s93(VS.80).aspx
#pragma warning(disable: 4267)

%}

%ignore PYXRhombusFiller;

// TODO sort this out
#undef LIBRARY_DECL
#define VIEW_MODEL_API
#define PYXLIB_DECL

%import "pyxlib_swig.i"

%template(GoToPipelineCommand_SPtr) PYXPointer<GoToPipelineCommand>;
%template(GoToPipelineCommand_CSPtr) PYXPointer<const GoToPipelineCommand>;



%extend Camera
{
public:
	PYXCoord3DDouble getCenter() const { return CmlConvertor::toPYXCoord3D($self->getCenter()); }
	PYXCoord3DDouble getEye() const { return CmlConvertor::toPYXCoord3D($self->getEye()); }
	PYXCoord3DDouble getUp() const { return CmlConvertor::toPYXCoord3D($self->getUp()); }
	PYXCoord3DDouble getLook() const { return CmlConvertor::toPYXCoord3D($self->getLook()); }

	void setCenter(PYXCoord3DDouble center) { $self->setCenter(center); }
}
%ignore CameraFrustum;
%ignore projectPointToUnitSphere;
%apply double * OUTPUT { double & lat, double & lon, double & hdg, double & alt, double & tlt, double & rng };
%include "camera.h"

%include "go_to_pipeline_command.h"
%include "view.h"
%include "ui_events.h"
%include "view_model_api.h"


//%apply int * OUTPUT { int * coordU, int * coordV};
//%apply double * OUTPUT { double * coordU, double * coordV};
//%include "rhombus.h"
//%include "rhombus_utils.h"

%template(UIEvent_SPtr) PYXPointer<UIEvent>;
%template(UIKeyEvent_SPtr) PYXPointer<UIKeyEvent>;
%template(UIMouseEvent_SPtr) PYXPointer<UIMouseEvent>;
%template(AnnotationMouseEvent_SPtr) PYXPointer<AnnotationMouseEvent>;
%template(IAnnotation_SPtr) PYXPointer<IAnnotation>;
%template(ViewEvent_SPtr) PYXPointer<ViewEvent>;
%template(ViewHandle_SPtr) PYXPointer<ViewHandle>;

namespace std
{
	//%template(IAnnotation_Vector) vector<PYXPointer<IAnnotation>>;
	//%template(PickedIAnnotation_Vector) vector<std::pair<double,PYXPointer<IAnnotation>>>;
	//%template(PYXRhombus_Vector) vector<PYXRhombus>;
}

%template(IViewModel_SPtr) PYXPointer<IViewModel>;
%template(ToolTipRequest_SPtr) PYXPointer<ToolTipRequest>;


%template(VectorOfFloat) std::vector<float>;

%inline %{
std::string camToCookieStr(const Camera& cam)
{
	std::string str;
	camToCookieStr(cam, str);
	return str;
}
%}

FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, GoToPipelineCommand, Command)
FEATURE_DYNAMIC_CAST(ViewEvent, NotifierEvent)
FEATURE_DYNAMIC_CAST(AnnotationMouseEvent, NotifierEvent)