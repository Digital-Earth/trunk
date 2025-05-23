/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class UIMouseEvent_SPtr : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public UIMouseEvent_SPtr(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(UIMouseEvent_SPtr obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~UIMouseEvent_SPtr() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        view_model_swigPINVOKE.delete_UIMouseEvent_SPtr(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public UIMouseEvent_SPtr() : this(view_model_swigPINVOKE.new_UIMouseEvent_SPtr__SWIG_0(), true) {
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public UIMouseEvent_SPtr(UIMouseEvent p, bool add_ref) : this(view_model_swigPINVOKE.new_UIMouseEvent_SPtr__SWIG_1(UIMouseEvent.getCPtr(p), add_ref), true) {
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public UIMouseEvent_SPtr(UIMouseEvent p) : this(view_model_swigPINVOKE.new_UIMouseEvent_SPtr__SWIG_2(UIMouseEvent.getCPtr(p)), true) {
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public UIMouseEvent_SPtr(UIMouseEvent_SPtr rhs) : this(view_model_swigPINVOKE.new_UIMouseEvent_SPtr__SWIG_3(UIMouseEvent_SPtr.getCPtr(rhs)), true) {
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset() {
    view_model_swigPINVOKE.UIMouseEvent_SPtr_reset__SWIG_0(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset(UIMouseEvent rhs) {
    view_model_swigPINVOKE.UIMouseEvent_SPtr_reset__SWIG_1(swigCPtr, UIMouseEvent.getCPtr(rhs));
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public UIMouseEvent get() {
    IntPtr cPtr = view_model_swigPINVOKE.UIMouseEvent_SPtr_get(swigCPtr);
    UIMouseEvent ret = (cPtr == IntPtr.Zero) ? null : new UIMouseEvent(cPtr, false);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public UIMouseEvent __ref__() {
    UIMouseEvent ret = new UIMouseEvent(view_model_swigPINVOKE.UIMouseEvent_SPtr___ref__(swigCPtr), false);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public UIMouseEvent __deref__() {
    IntPtr cPtr = view_model_swigPINVOKE.UIMouseEvent_SPtr___deref__(swigCPtr);
    UIMouseEvent ret = (cPtr == IntPtr.Zero) ? null : new UIMouseEvent(cPtr, false);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void swap(UIMouseEvent_SPtr rhs) {
    view_model_swigPINVOKE.UIMouseEvent_SPtr_swap(swigCPtr, UIMouseEvent_SPtr.getCPtr(rhs));
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isNull() {
    bool ret = view_model_swigPINVOKE.UIMouseEvent_SPtr_isNull(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isNotNull() {
    bool ret = view_model_swigPINVOKE.UIMouseEvent_SPtr_isNotNull(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public UIMouseEvent_SPtr create(double x, double y, int delta, bool leftButton, bool rightButton, bool middleButton, bool altKey, bool shiftKey, bool ctrlKey) {
    UIMouseEvent_SPtr ret = new UIMouseEvent_SPtr(view_model_swigPINVOKE.UIMouseEvent_SPtr_create(swigCPtr, x, y, delta, leftButton, rightButton, middleButton, altKey, shiftKey, ctrlKey), true);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public double getMouseX() {
    double ret = view_model_swigPINVOKE.UIMouseEvent_SPtr_getMouseX(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public double getMouseY() {
    double ret = view_model_swigPINVOKE.UIMouseEvent_SPtr_getMouseY(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getWheelDelta() {
    int ret = view_model_swigPINVOKE.UIMouseEvent_SPtr_getWheelDelta(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isLeftButtonDown() {
    bool ret = view_model_swigPINVOKE.UIMouseEvent_SPtr_isLeftButtonDown(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isRightButtonDown() {
    bool ret = view_model_swigPINVOKE.UIMouseEvent_SPtr_isRightButtonDown(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isMiddleButtonDown() {
    bool ret = view_model_swigPINVOKE.UIMouseEvent_SPtr_isMiddleButtonDown(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public double mouseDistanceFrom(double X, double Y) {
    double ret = view_model_swigPINVOKE.UIMouseEvent_SPtr_mouseDistanceFrom(swigCPtr, X, Y);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isInsideRect(double Xmin, double Xmax, double Ymin, double Ymax) {
    bool ret = view_model_swigPINVOKE.UIMouseEvent_SPtr_isInsideRect(swigCPtr, Xmin, Xmax, Ymin, Ymax);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isConsumed() {
    bool ret = view_model_swigPINVOKE.UIMouseEvent_SPtr_isConsumed(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void setConsumed() {
    view_model_swigPINVOKE.UIMouseEvent_SPtr_setConsumed(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isAltKeyPressed() {
    bool ret = view_model_swigPINVOKE.UIMouseEvent_SPtr_isAltKeyPressed(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isCtrlKeyPressed() {
    bool ret = view_model_swigPINVOKE.UIMouseEvent_SPtr_isCtrlKeyPressed(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isShiftKeyPressed() {
    bool ret = view_model_swigPINVOKE.UIMouseEvent_SPtr_isShiftKeyPressed(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
