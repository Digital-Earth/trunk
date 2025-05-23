/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class IRegion : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public IRegion(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(IRegion obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~IRegion() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_IRegion(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public virtual IRegion_SPtr clone() {
    IRegion_SPtr ret = new IRegion_SPtr(pyxlibPINVOKE.IRegion_clone(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual int getVerticesCount() {
    int ret = pyxlibPINVOKE.IRegion_getVerticesCount(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual SWIGTYPE_p_boost__intrusive_ptrTIRegionVisitor_t getVisitor() {
    SWIGTYPE_p_boost__intrusive_ptrTIRegionVisitor_t ret = new SWIGTYPE_p_boost__intrusive_ptrTIRegionVisitor_t(pyxlibPINVOKE.IRegion_getVisitor(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual void serializeVisitor(SWIGTYPE_p_PYXWireBuffer buffer, SWIGTYPE_p_boost__intrusive_ptrTIRegionVisitor_t visitor) {
    pyxlibPINVOKE.IRegion_serializeVisitor(swigCPtr, SWIGTYPE_p_PYXWireBuffer.getCPtr(buffer), SWIGTYPE_p_boost__intrusive_ptrTIRegionVisitor_t.getCPtr(visitor));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public virtual SWIGTYPE_p_boost__intrusive_ptrTIRegionVisitor_t deserializeVisitor(SWIGTYPE_p_PYXWireBuffer buffer) {
    SWIGTYPE_p_boost__intrusive_ptrTIRegionVisitor_t ret = new SWIGTYPE_p_boost__intrusive_ptrTIRegionVisitor_t(pyxlibPINVOKE.IRegion_deserializeVisitor__SWIG_0(swigCPtr, SWIGTYPE_p_PYXWireBuffer.getCPtr(buffer)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual SWIGTYPE_p_boost__intrusive_ptrTIRegionVisitor_t deserializeVisitor(SWIGTYPE_p_PYXWireBuffer buffer, PYXIcosIndex index) {
    SWIGTYPE_p_boost__intrusive_ptrTIRegionVisitor_t ret = new SWIGTYPE_p_boost__intrusive_ptrTIRegionVisitor_t(pyxlibPINVOKE.IRegion_deserializeVisitor__SWIG_1(swigCPtr, SWIGTYPE_p_PYXWireBuffer.getCPtr(buffer), PYXIcosIndex.getCPtr(index)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
