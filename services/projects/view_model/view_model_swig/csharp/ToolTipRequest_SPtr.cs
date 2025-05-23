/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class ToolTipRequest_SPtr : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public ToolTipRequest_SPtr(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(ToolTipRequest_SPtr obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~ToolTipRequest_SPtr() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        view_model_swigPINVOKE.delete_ToolTipRequest_SPtr(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public ToolTipRequest_SPtr() : this(view_model_swigPINVOKE.new_ToolTipRequest_SPtr__SWIG_0(), true) {
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public ToolTipRequest_SPtr(ToolTipRequest p, bool add_ref) : this(view_model_swigPINVOKE.new_ToolTipRequest_SPtr__SWIG_1(ToolTipRequest.getCPtr(p), add_ref), true) {
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public ToolTipRequest_SPtr(ToolTipRequest p) : this(view_model_swigPINVOKE.new_ToolTipRequest_SPtr__SWIG_2(ToolTipRequest.getCPtr(p)), true) {
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public ToolTipRequest_SPtr(ToolTipRequest_SPtr rhs) : this(view_model_swigPINVOKE.new_ToolTipRequest_SPtr__SWIG_3(ToolTipRequest_SPtr.getCPtr(rhs)), true) {
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset() {
    view_model_swigPINVOKE.ToolTipRequest_SPtr_reset__SWIG_0(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset(ToolTipRequest rhs) {
    view_model_swigPINVOKE.ToolTipRequest_SPtr_reset__SWIG_1(swigCPtr, ToolTipRequest.getCPtr(rhs));
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public ToolTipRequest get() {
    IntPtr cPtr = view_model_swigPINVOKE.ToolTipRequest_SPtr_get(swigCPtr);
    ToolTipRequest ret = (cPtr == IntPtr.Zero) ? null : new ToolTipRequest(cPtr, false);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public ToolTipRequest __ref__() {
    ToolTipRequest ret = new ToolTipRequest(view_model_swigPINVOKE.ToolTipRequest_SPtr___ref__(swigCPtr), false);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public ToolTipRequest __deref__() {
    IntPtr cPtr = view_model_swigPINVOKE.ToolTipRequest_SPtr___deref__(swigCPtr);
    ToolTipRequest ret = (cPtr == IntPtr.Zero) ? null : new ToolTipRequest(cPtr, false);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void swap(ToolTipRequest_SPtr rhs) {
    view_model_swigPINVOKE.ToolTipRequest_SPtr_swap(swigCPtr, ToolTipRequest_SPtr.getCPtr(rhs));
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isNull() {
    bool ret = view_model_swigPINVOKE.ToolTipRequest_SPtr_isNull(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isNotNull() {
    bool ret = view_model_swigPINVOKE.ToolTipRequest_SPtr_isNotNull(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public ToolTipRequest_SPtr create(string message, int x, int y, int period) {
    ToolTipRequest_SPtr ret = new ToolTipRequest_SPtr(view_model_swigPINVOKE.ToolTipRequest_SPtr_create(swigCPtr, view_model_swigPINVOKE.UnmanagedString.Create(message), x, y, period), true);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public string getMessage() {
  // Generated from typemap(csout) const string &
  string ret = view_model_swigPINVOKE.UnmanagedString.Consume(view_model_swigPINVOKE.ToolTipRequest_SPtr_getMessage(swigCPtr));
  
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public int getX() {
    int ret = view_model_swigPINVOKE.ToolTipRequest_SPtr_getX(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getY() {
    int ret = view_model_swigPINVOKE.ToolTipRequest_SPtr_getY(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getPeriod() {
    int ret = view_model_swigPINVOKE.ToolTipRequest_SPtr_getPeriod(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
