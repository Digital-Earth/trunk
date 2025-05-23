/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class IViewModel_SPtr : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public IViewModel_SPtr(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(IViewModel_SPtr obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~IViewModel_SPtr() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        view_model_swigPINVOKE.delete_IViewModel_SPtr(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public IViewModel_SPtr() : this(view_model_swigPINVOKE.new_IViewModel_SPtr__SWIG_0(), true) {
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public IViewModel_SPtr(IViewModel p, bool add_ref) : this(view_model_swigPINVOKE.new_IViewModel_SPtr__SWIG_1(IViewModel.getCPtr(p), add_ref), true) {
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public IViewModel_SPtr(IViewModel p) : this(view_model_swigPINVOKE.new_IViewModel_SPtr__SWIG_2(IViewModel.getCPtr(p)), true) {
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public IViewModel_SPtr(IViewModel_SPtr rhs) : this(view_model_swigPINVOKE.new_IViewModel_SPtr__SWIG_3(IViewModel_SPtr.getCPtr(rhs)), true) {
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset() {
    view_model_swigPINVOKE.IViewModel_SPtr_reset__SWIG_0(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset(IViewModel rhs) {
    view_model_swigPINVOKE.IViewModel_SPtr_reset__SWIG_1(swigCPtr, IViewModel.getCPtr(rhs));
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public IViewModel get() {
    IntPtr cPtr = view_model_swigPINVOKE.IViewModel_SPtr_get(swigCPtr);
    IViewModel ret = (cPtr == IntPtr.Zero) ? null : new IViewModel(cPtr, false);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public IViewModel __ref__() {
    IViewModel ret = new IViewModel(view_model_swigPINVOKE.IViewModel_SPtr___ref__(swigCPtr), false);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public IViewModel __deref__() {
    IntPtr cPtr = view_model_swigPINVOKE.IViewModel_SPtr___deref__(swigCPtr);
    IViewModel ret = (cPtr == IntPtr.Zero) ? null : new IViewModel(cPtr, false);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void swap(IViewModel_SPtr rhs) {
    view_model_swigPINVOKE.IViewModel_SPtr_swap(swigCPtr, IViewModel_SPtr.getCPtr(rhs));
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isNull() {
    bool ret = view_model_swigPINVOKE.IViewModel_SPtr_isNull(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isNotNull() {
    bool ret = view_model_swigPINVOKE.IViewModel_SPtr_isNotNull(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getID() {
    int ret = view_model_swigPINVOKE.IViewModel_SPtr_getID(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public IProcess_SPtr getViewPointProcess() {
    IProcess_SPtr ret = new IProcess_SPtr(view_model_swigPINVOKE.IViewModel_SPtr_getViewPointProcess(swigCPtr), true);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getViewportWidth() {
    int ret = view_model_swigPINVOKE.IViewModel_SPtr_getViewportWidth(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getViewportHeight() {
    int ret = view_model_swigPINVOKE.IViewModel_SPtr_getViewportHeight(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getMouseX() {
    int ret = view_model_swigPINVOKE.IViewModel_SPtr_getMouseX(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getMouseY() {
    int ret = view_model_swigPINVOKE.IViewModel_SPtr_getMouseY(swigCPtr);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXCoord3DDouble getPointerLocation() {
    PYXCoord3DDouble ret = new PYXCoord3DDouble(view_model_swigPINVOKE.IViewModel_SPtr_getPointerLocation(swigCPtr), true);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void showToolTip(string message, int period) {
    view_model_swigPINVOKE.IViewModel_SPtr_showToolTip__SWIG_0(swigCPtr, view_model_swigPINVOKE.UnmanagedString.Create(message), period);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

  public void showToolTip(string message, int x, int y, int period) {
    view_model_swigPINVOKE.IViewModel_SPtr_showToolTip__SWIG_1(swigCPtr, view_model_swigPINVOKE.UnmanagedString.Create(message), x, y, period);
    if (view_model_swigPINVOKE.SWIGPendingException.Pending) throw view_model_swigPINVOKE.SWIGPendingException.Retrieve();
  }

}
