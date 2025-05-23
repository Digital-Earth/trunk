/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class PYXRegion : IRegion {
  private HandleRef swigCPtr;

  public PYXRegion(IntPtr cPtr, bool cMemoryOwn) : base(pyxlibPINVOKE.PYXRegionUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(PYXRegion obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~PYXRegion() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_PYXRegion(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public virtual PYXRegion.CellIntersectionState intersects(PYXIcosIndex index, bool asTile) {
    PYXRegion.CellIntersectionState ret = (PYXRegion.CellIntersectionState)pyxlibPINVOKE.PYXRegion_intersects__SWIG_0(swigCPtr, PYXIcosIndex.getCPtr(index), asTile);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual PYXRegion.CellIntersectionState intersects(PYXIcosIndex index) {
    PYXRegion.CellIntersectionState ret = (PYXRegion.CellIntersectionState)pyxlibPINVOKE.PYXRegion_intersects__SWIG_1(swigCPtr, PYXIcosIndex.getCPtr(index));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public enum CellIntersectionState {
    knNone,
    knPartial,
    knComplete
  }

}
