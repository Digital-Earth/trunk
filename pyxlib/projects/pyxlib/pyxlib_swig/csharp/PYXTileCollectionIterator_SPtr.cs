/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class PYXTileCollectionIterator_SPtr : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public PYXTileCollectionIterator_SPtr(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(PYXTileCollectionIterator_SPtr obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~PYXTileCollectionIterator_SPtr() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_PYXTileCollectionIterator_SPtr(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public PYXTileCollectionIterator_SPtr() : this(pyxlibPINVOKE.new_PYXTileCollectionIterator_SPtr__SWIG_0(), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXTileCollectionIterator_SPtr(PYXTileCollectionIterator p, bool add_ref) : this(pyxlibPINVOKE.new_PYXTileCollectionIterator_SPtr__SWIG_1(PYXTileCollectionIterator.getCPtr(p), add_ref), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXTileCollectionIterator_SPtr(PYXTileCollectionIterator p) : this(pyxlibPINVOKE.new_PYXTileCollectionIterator_SPtr__SWIG_2(PYXTileCollectionIterator.getCPtr(p)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXTileCollectionIterator_SPtr(PYXTileCollectionIterator_SPtr rhs) : this(pyxlibPINVOKE.new_PYXTileCollectionIterator_SPtr__SWIG_3(PYXTileCollectionIterator_SPtr.getCPtr(rhs)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset() {
    pyxlibPINVOKE.PYXTileCollectionIterator_SPtr_reset__SWIG_0(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset(PYXTileCollectionIterator rhs) {
    pyxlibPINVOKE.PYXTileCollectionIterator_SPtr_reset__SWIG_1(swigCPtr, PYXTileCollectionIterator.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXTileCollectionIterator get() {
    IntPtr cPtr = pyxlibPINVOKE.PYXTileCollectionIterator_SPtr_get(swigCPtr);
    PYXTileCollectionIterator ret = (cPtr == IntPtr.Zero) ? null : new PYXTileCollectionIterator(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXTileCollectionIterator __ref__() {
    PYXTileCollectionIterator ret = new PYXTileCollectionIterator(pyxlibPINVOKE.PYXTileCollectionIterator_SPtr___ref__(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXTileCollectionIterator __deref__() {
    IntPtr cPtr = pyxlibPINVOKE.PYXTileCollectionIterator_SPtr___deref__(swigCPtr);
    PYXTileCollectionIterator ret = (cPtr == IntPtr.Zero) ? null : new PYXTileCollectionIterator(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void swap(PYXTileCollectionIterator_SPtr rhs) {
    pyxlibPINVOKE.PYXTileCollectionIterator_SPtr_swap(swigCPtr, PYXTileCollectionIterator_SPtr.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isNull() {
    bool ret = pyxlibPINVOKE.PYXTileCollectionIterator_SPtr_isNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isNotNull() {
    bool ret = pyxlibPINVOKE.PYXTileCollectionIterator_SPtr_isNotNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXTileCollectionIterator_SPtr create(PYXTileCollection tileCollection) {
    PYXTileCollectionIterator_SPtr ret = new PYXTileCollectionIterator_SPtr(pyxlibPINVOKE.PYXTileCollectionIterator_SPtr_create(swigCPtr, PYXTileCollection.getCPtr(tileCollection)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void next() {
    pyxlibPINVOKE.PYXTileCollectionIterator_SPtr_next(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool end() {
    bool ret = pyxlibPINVOKE.PYXTileCollectionIterator_SPtr_end(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXGeometry_SPtr getGeometry() {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXTileCollectionIterator_SPtr_getGeometry(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXTile_SPtr getTile() {
    PYXTile_SPtr ret = new PYXTile_SPtr(pyxlibPINVOKE.PYXTileCollectionIterator_SPtr_getTile(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
