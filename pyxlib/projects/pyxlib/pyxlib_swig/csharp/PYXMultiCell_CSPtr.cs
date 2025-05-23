/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class PYXMultiCell_CSPtr : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public PYXMultiCell_CSPtr(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(PYXMultiCell_CSPtr obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~PYXMultiCell_CSPtr() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_PYXMultiCell_CSPtr(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public PYXMultiCell_CSPtr() : this(pyxlibPINVOKE.new_PYXMultiCell_CSPtr__SWIG_0(), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXMultiCell_CSPtr(PYXMultiCell p, bool add_ref) : this(pyxlibPINVOKE.new_PYXMultiCell_CSPtr__SWIG_1(PYXMultiCell.getCPtr(p), add_ref), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXMultiCell_CSPtr(PYXMultiCell p) : this(pyxlibPINVOKE.new_PYXMultiCell_CSPtr__SWIG_2(PYXMultiCell.getCPtr(p)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXMultiCell_CSPtr(PYXMultiCell_CSPtr rhs) : this(pyxlibPINVOKE.new_PYXMultiCell_CSPtr__SWIG_3(PYXMultiCell_CSPtr.getCPtr(rhs)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset() {
    pyxlibPINVOKE.PYXMultiCell_CSPtr_reset__SWIG_0(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset(PYXMultiCell rhs) {
    pyxlibPINVOKE.PYXMultiCell_CSPtr_reset__SWIG_1(swigCPtr, PYXMultiCell.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXMultiCell get() {
    IntPtr cPtr = pyxlibPINVOKE.PYXMultiCell_CSPtr_get(swigCPtr);
    PYXMultiCell ret = (cPtr == IntPtr.Zero) ? null : new PYXMultiCell(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXMultiCell __ref__() {
    PYXMultiCell ret = new PYXMultiCell(pyxlibPINVOKE.PYXMultiCell_CSPtr___ref__(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXMultiCell __deref__() {
    IntPtr cPtr = pyxlibPINVOKE.PYXMultiCell_CSPtr___deref__(swigCPtr);
    PYXMultiCell ret = (cPtr == IntPtr.Zero) ? null : new PYXMultiCell(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void swap(PYXMultiCell_CSPtr rhs) {
    pyxlibPINVOKE.PYXMultiCell_CSPtr_swap(swigCPtr, PYXMultiCell_CSPtr.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isNull() {
    bool ret = pyxlibPINVOKE.PYXMultiCell_CSPtr_isNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isNotNull() {
    bool ret = pyxlibPINVOKE.PYXMultiCell_CSPtr_isNotNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXGeometry_SPtr clone() {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXMultiCell_CSPtr_clone(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isEmpty() {
    bool ret = pyxlibPINVOKE.PYXMultiCell_CSPtr_isEmpty(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getCellResolution() {
    int ret = pyxlibPINVOKE.PYXMultiCell_CSPtr_getCellResolution(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXGeometry_SPtr intersection(PYXGeometry geometry, bool bCommutative) {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXMultiCell_CSPtr_intersection__SWIG_0(swigCPtr, PYXGeometry.getCPtr(geometry), bCommutative), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXGeometry_SPtr intersection(PYXGeometry geometry) {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXMultiCell_CSPtr_intersection__SWIG_1(swigCPtr, PYXGeometry.getCPtr(geometry)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXGeometry_SPtr intersection(PYXCell cell) {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXMultiCell_CSPtr_intersection__SWIG_2(swigCPtr, PYXCell.getCPtr(cell)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXGeometry_SPtr intersection(PYXMultiCell multiCell) {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXMultiCell_CSPtr_intersection__SWIG_3(swigCPtr, PYXMultiCell.getCPtr(multiCell)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool intersects(PYXGeometry geometry, bool bCommutative) {
    bool ret = pyxlibPINVOKE.PYXMultiCell_CSPtr_intersects__SWIG_0(swigCPtr, PYXGeometry.getCPtr(geometry), bCommutative);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool intersects(PYXGeometry geometry) {
    bool ret = pyxlibPINVOKE.PYXMultiCell_CSPtr_intersects__SWIG_1(swigCPtr, PYXGeometry.getCPtr(geometry));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool intersects(PYXCell cell) {
    bool ret = pyxlibPINVOKE.PYXMultiCell_CSPtr_intersects__SWIG_2(swigCPtr, PYXCell.getCPtr(cell));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool intersects(PYXMultiCell multiCell) {
    bool ret = pyxlibPINVOKE.PYXMultiCell_CSPtr_intersects__SWIG_3(swigCPtr, PYXMultiCell.getCPtr(multiCell));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXIterator_SPtr getIterator() {
    PYXIterator_SPtr ret = new PYXIterator_SPtr(pyxlibPINVOKE.PYXMultiCell_CSPtr_getIterator(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void calcPerimeter(Vector_Index pVecIndex) {
    pyxlibPINVOKE.PYXMultiCell_CSPtr_calcPerimeter(swigCPtr, Vector_Index.getCPtr(pVecIndex));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void copyTo(PYXTileCollection pTileCollection) {
    pyxlibPINVOKE.PYXMultiCell_CSPtr_copyTo__SWIG_0(swigCPtr, PYXTileCollection.getCPtr(pTileCollection));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void copyTo(PYXTileCollection pTileCollection, int nTargetResolution) {
    pyxlibPINVOKE.PYXMultiCell_CSPtr_copyTo__SWIG_1(swigCPtr, PYXTileCollection.getCPtr(pTileCollection), nTargetResolution);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXBoundingCircle getBoundingCircle() {
    PYXBoundingCircle ret = new PYXBoundingCircle(pyxlibPINVOKE.PYXMultiCell_CSPtr_getBoundingCircle(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isCollection() {
    bool ret = pyxlibPINVOKE.PYXMultiCell_CSPtr_isCollection(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool contains(PYXGeometry geometry) {
    bool ret = pyxlibPINVOKE.PYXMultiCell_CSPtr_contains(swigCPtr, PYXGeometry.getCPtr(geometry));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXGeometry_SPtr disjunction(PYXGeometry geometry) {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXMultiCell_CSPtr_disjunction(swigCPtr, PYXGeometry.getCPtr(geometry)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXInnerTileIntersectionIterator_SPtr getInnerTileIterator(PYXInnerTile tile) {
    PYXInnerTileIntersectionIterator_SPtr ret = new PYXInnerTileIntersectionIterator_SPtr(pyxlibPINVOKE.PYXMultiCell_CSPtr_getInnerTileIterator(swigCPtr, PYXInnerTile.getCPtr(tile)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void getBoundingRects(ICoordConverter coordConvertor, PYXRect2DDouble pRect1, PYXRect2DDouble pRect2) {
    pyxlibPINVOKE.PYXMultiCell_CSPtr_getBoundingRects(swigCPtr, ICoordConverter.getCPtr(coordConvertor), PYXRect2DDouble.getCPtr(pRect1), PYXRect2DDouble.getCPtr(pRect2));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

}
