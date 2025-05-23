/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class DefaultXYAsyncValueGetter : XYAsyncValueGetter {
  private HandleRef swigCPtr;

  public DefaultXYAsyncValueGetter(IntPtr cPtr, bool cMemoryOwn) : base(pyxlibPINVOKE.DefaultXYAsyncValueGetterUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(DefaultXYAsyncValueGetter obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~DefaultXYAsyncValueGetter() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_DefaultXYAsyncValueGetter(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public static SWIGTYPE_p_boost__intrusive_ptrTDefaultXYAsyncValueGetter_t create(IXYCoverage coverage, SWIGTYPE_p_XYAsyncValueConsumer consumer, int width, int height) {
    SWIGTYPE_p_boost__intrusive_ptrTDefaultXYAsyncValueGetter_t ret = new SWIGTYPE_p_boost__intrusive_ptrTDefaultXYAsyncValueGetter_t(pyxlibPINVOKE.DefaultXYAsyncValueGetter_create(IXYCoverage.getCPtr(coverage), SWIGTYPE_p_XYAsyncValueConsumer.getCPtr(consumer), width, height), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public DefaultXYAsyncValueGetter(IXYCoverage coverage, SWIGTYPE_p_XYAsyncValueConsumer consumer, int width, int height) : this(pyxlibPINVOKE.new_DefaultXYAsyncValueGetter(IXYCoverage.getCPtr(coverage), SWIGTYPE_p_XYAsyncValueConsumer.getCPtr(consumer), width, height), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override void addAsyncRequests(PYXTile tile) {
    pyxlibPINVOKE.DefaultXYAsyncValueGetter_addAsyncRequests(swigCPtr, PYXTile.getCPtr(tile));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override void addAsyncRequest(PYXIcosIndex index) {
    pyxlibPINVOKE.DefaultXYAsyncValueGetter_addAsyncRequest(swigCPtr, PYXIcosIndex.getCPtr(index));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override bool join() {
    bool ret = pyxlibPINVOKE.DefaultXYAsyncValueGetter_join(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
