/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class PYXInnerTileIntersectionIterator : PYXAbstractIterator {
  private HandleRef swigCPtr;

  public PYXInnerTileIntersectionIterator(IntPtr cPtr, bool cMemoryOwn) : base(pyxlibPINVOKE.PYXInnerTileIntersectionIteratorUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(PYXInnerTileIntersectionIterator obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~PYXInnerTileIntersectionIterator() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_PYXInnerTileIntersectionIterator(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public virtual PYXInnerTile getTile() {
    PYXInnerTile ret = new PYXInnerTile(pyxlibPINVOKE.PYXInnerTileIntersectionIterator_getTile(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual PYXInnerTileIntersection getIntersection() {
    PYXInnerTileIntersection ret = (PYXInnerTileIntersection)pyxlibPINVOKE.PYXInnerTileIntersectionIterator_getIntersection(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static bool intersects(PYXInnerTileIntersectionIterator a, PYXInnerTileIntersectionIterator b) {
    bool ret = pyxlibPINVOKE.PYXInnerTileIntersectionIterator_intersects(PYXInnerTileIntersectionIterator.getCPtr(a), PYXInnerTileIntersectionIterator.getCPtr(b));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual void nextTileWithIntersection() {
    pyxlibPINVOKE.PYXInnerTileIntersectionIterator_nextTileWithIntersection(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

}
