/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class PYXVectorGeometry2_CSPtr : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public PYXVectorGeometry2_CSPtr(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(PYXVectorGeometry2_CSPtr obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~PYXVectorGeometry2_CSPtr() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_PYXVectorGeometry2_CSPtr(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public PYXVectorGeometry2_CSPtr() : this(pyxlibPINVOKE.new_PYXVectorGeometry2_CSPtr__SWIG_0(), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXVectorGeometry2_CSPtr(PYXVectorGeometry2 p, bool add_ref) : this(pyxlibPINVOKE.new_PYXVectorGeometry2_CSPtr__SWIG_1(PYXVectorGeometry2.getCPtr(p), add_ref), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXVectorGeometry2_CSPtr(PYXVectorGeometry2 p) : this(pyxlibPINVOKE.new_PYXVectorGeometry2_CSPtr__SWIG_2(PYXVectorGeometry2.getCPtr(p)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXVectorGeometry2_CSPtr(PYXVectorGeometry2_CSPtr rhs) : this(pyxlibPINVOKE.new_PYXVectorGeometry2_CSPtr__SWIG_3(PYXVectorGeometry2_CSPtr.getCPtr(rhs)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset() {
    pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_reset__SWIG_0(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset(PYXVectorGeometry2 rhs) {
    pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_reset__SWIG_1(swigCPtr, PYXVectorGeometry2.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXVectorGeometry2 get() {
    IntPtr cPtr = pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_get(swigCPtr);
    PYXVectorGeometry2 ret = (cPtr == IntPtr.Zero) ? null : new PYXVectorGeometry2(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXVectorGeometry2 __ref__() {
    PYXVectorGeometry2 ret = new PYXVectorGeometry2(pyxlibPINVOKE.PYXVectorGeometry2_CSPtr___ref__(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXVectorGeometry2 __deref__() {
    IntPtr cPtr = pyxlibPINVOKE.PYXVectorGeometry2_CSPtr___deref__(swigCPtr);
    PYXVectorGeometry2 ret = (cPtr == IntPtr.Zero) ? null : new PYXVectorGeometry2(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void swap(PYXVectorGeometry2_CSPtr rhs) {
    pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_swap(swigCPtr, PYXVectorGeometry2_CSPtr.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isNull() {
    bool ret = pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_isNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isNotNull() {
    bool ret = pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_isNotNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void serialize(SWIGTYPE_p_std__basic_ostreamTchar_t arg0) {
    pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_serialize__SWIG_0(swigCPtr, SWIGTYPE_p_std__basic_ostreamTchar_t.getCPtr(arg0));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void serialize(SWIGTYPE_p_PYXWireBuffer arg0) {
    pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_serialize__SWIG_1(swigCPtr, SWIGTYPE_p_PYXWireBuffer.getCPtr(arg0));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public IRegion_SPtr getRegion() {
    IRegion_SPtr ret = new IRegion_SPtr(pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_getRegion(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXGeometry_SPtr clone() {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_clone(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isEmpty() {
    bool ret = pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_isEmpty(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getCellResolution() {
    int ret = pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_getCellResolution(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXGeometry_SPtr intersection(PYXGeometry geometry, bool bCommutative) {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_intersection__SWIG_0(swigCPtr, PYXGeometry.getCPtr(geometry), bCommutative), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXGeometry_SPtr intersection(PYXGeometry geometry) {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_intersection__SWIG_1(swigCPtr, PYXGeometry.getCPtr(geometry)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXGeometry_SPtr intersection(PYXTileCollection collection) {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_intersection__SWIG_2(swigCPtr, PYXTileCollection.getCPtr(collection)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXGeometry_SPtr intersection(PYXTile tile) {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_intersection__SWIG_3(swigCPtr, PYXTile.getCPtr(tile)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXGeometry_SPtr intersection(PYXCell cell) {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_intersection__SWIG_4(swigCPtr, PYXCell.getCPtr(cell)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool contains(PYXGeometry geometry) {
    bool ret = pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_contains(swigCPtr, PYXGeometry.getCPtr(geometry));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool intersects(PYXGeometry geometry, bool bCommutative) {
    bool ret = pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_intersects__SWIG_0(swigCPtr, PYXGeometry.getCPtr(geometry), bCommutative);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool intersects(PYXGeometry geometry) {
    bool ret = pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_intersects__SWIG_1(swigCPtr, PYXGeometry.getCPtr(geometry));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXIterator_SPtr getIterator() {
    PYXIterator_SPtr ret = new PYXIterator_SPtr(pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_getIterator(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXInnerTileIntersectionIterator_SPtr getInnerTileIterator(PYXInnerTile tile) {
    PYXInnerTileIntersectionIterator_SPtr ret = new PYXInnerTileIntersectionIterator_SPtr(pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_getInnerTileIterator(swigCPtr, PYXInnerTile.getCPtr(tile)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void calcPerimeter(Vector_Index pVecIndex) {
    pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_calcPerimeter(swigCPtr, Vector_Index.getCPtr(pVecIndex));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXBoundingCircle getBoundingCircle() {
    PYXBoundingCircle ret = new PYXBoundingCircle(pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_getBoundingCircle(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void copyTo(PYXTileCollection pTileCollection) {
    pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_copyTo__SWIG_0(swigCPtr, PYXTileCollection.getCPtr(pTileCollection));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void copyTo(PYXTileCollection pTileCollection, int nTargetResolution) {
    pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_copyTo__SWIG_1(swigCPtr, PYXTileCollection.getCPtr(pTileCollection), nTargetResolution);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void getBoundingRects(ICoordConverter coordConvertor, PYXRect2DDouble pRect1, PYXRect2DDouble pRect2) {
    pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_getBoundingRects(swigCPtr, ICoordConverter.getCPtr(coordConvertor), PYXRect2DDouble.getCPtr(pRect1), PYXRect2DDouble.getCPtr(pRect2));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isCollection() {
    bool ret = pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_isCollection(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXGeometry_SPtr disjunction(PYXGeometry geometry) {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXVectorGeometry2_CSPtr_disjunction(swigCPtr, PYXGeometry.getCPtr(geometry)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
