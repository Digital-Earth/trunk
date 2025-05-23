/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class AnnotationMouseEvent : UIMouseEvent {
  private HandleRef swigCPtr;

  public AnnotationMouseEvent(IntPtr cPtr, bool cMemoryOwn) : base(view_model_swigPINVOKE.AnnotationMouseEventUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(AnnotationMouseEvent obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~AnnotationMouseEvent() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        view_model_swigPINVOKE.delete_AnnotationMouseEvent(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public AnnotationMouseEvent(UIMouseEvent_SPtr mouseEvent, IAnnotation_SPtr annotation, IViewModel_SPtr view) : this(view_model_swigPINVOKE.new_AnnotationMouseEvent(UIMouseEvent_SPtr.getCPtr(mouseEvent), IAnnotation_SPtr.getCPtr(annotation), IViewModel_SPtr.getCPtr(view)), true) {
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public static AnnotationMouseEvent_SPtr create(UIMouseEvent_SPtr mouseEvent, IAnnotation_SPtr annotation, IViewModel_SPtr view) {
    AnnotationMouseEvent_SPtr ret = new AnnotationMouseEvent_SPtr(view_model_swigPINVOKE.AnnotationMouseEvent_create(UIMouseEvent_SPtr.getCPtr(mouseEvent), IAnnotation_SPtr.getCPtr(annotation), IViewModel_SPtr.getCPtr(view)), true);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public IAnnotation_SPtr getAnnotation() {
    IAnnotation_SPtr ret = new IAnnotation_SPtr(view_model_swigPINVOKE.AnnotationMouseEvent_getAnnotation(swigCPtr), true);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public IViewModel_SPtr getViewModel() {
    IViewModel_SPtr ret = new IViewModel_SPtr(view_model_swigPINVOKE.AnnotationMouseEvent_getViewModel(swigCPtr), true);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static AnnotationMouseEvent dynamic_cast(NotifierEvent pSource) {
    IntPtr cPtr = view_model_swigPINVOKE.AnnotationMouseEvent_dynamic_cast(NotifierEvent.getCPtr(pSource));
    AnnotationMouseEvent ret = (cPtr == IntPtr.Zero) ? null : new AnnotationMouseEvent(cPtr, false);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
