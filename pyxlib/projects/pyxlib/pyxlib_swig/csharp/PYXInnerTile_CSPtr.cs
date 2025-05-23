/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class PYXInnerTile_CSPtr : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public PYXInnerTile_CSPtr(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(PYXInnerTile_CSPtr obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~PYXInnerTile_CSPtr() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_PYXInnerTile_CSPtr(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public PYXInnerTile_CSPtr() : this(pyxlibPINVOKE.new_PYXInnerTile_CSPtr__SWIG_0(), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXInnerTile_CSPtr(PYXInnerTile p, bool add_ref) : this(pyxlibPINVOKE.new_PYXInnerTile_CSPtr__SWIG_1(PYXInnerTile.getCPtr(p), add_ref), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXInnerTile_CSPtr(PYXInnerTile p) : this(pyxlibPINVOKE.new_PYXInnerTile_CSPtr__SWIG_2(PYXInnerTile.getCPtr(p)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXInnerTile_CSPtr(PYXInnerTile_CSPtr rhs) : this(pyxlibPINVOKE.new_PYXInnerTile_CSPtr__SWIG_3(PYXInnerTile_CSPtr.getCPtr(rhs)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset() {
    pyxlibPINVOKE.PYXInnerTile_CSPtr_reset__SWIG_0(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset(PYXInnerTile rhs) {
    pyxlibPINVOKE.PYXInnerTile_CSPtr_reset__SWIG_1(swigCPtr, PYXInnerTile.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXInnerTile get() {
    IntPtr cPtr = pyxlibPINVOKE.PYXInnerTile_CSPtr_get(swigCPtr);
    PYXInnerTile ret = (cPtr == IntPtr.Zero) ? null : new PYXInnerTile(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXInnerTile __ref__() {
    PYXInnerTile ret = new PYXInnerTile(pyxlibPINVOKE.PYXInnerTile_CSPtr___ref__(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXInnerTile __deref__() {
    IntPtr cPtr = pyxlibPINVOKE.PYXInnerTile_CSPtr___deref__(swigCPtr);
    PYXInnerTile ret = (cPtr == IntPtr.Zero) ? null : new PYXInnerTile(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void swap(PYXInnerTile_CSPtr rhs) {
    pyxlibPINVOKE.PYXInnerTile_CSPtr_swap(swigCPtr, PYXInnerTile_CSPtr.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isNull() {
    bool ret = pyxlibPINVOKE.PYXInnerTile_CSPtr_isNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isNotNull() {
    bool ret = pyxlibPINVOKE.PYXInnerTile_CSPtr_isNotNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXGeometry_SPtr clone() {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXInnerTile_CSPtr_clone(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isEmpty() {
    bool ret = pyxlibPINVOKE.PYXInnerTile_CSPtr_isEmpty(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getCellResolution() {
    int ret = pyxlibPINVOKE.PYXInnerTile_CSPtr_getCellResolution(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXCell getBoundingCell() {
    PYXCell ret = new PYXCell(pyxlibPINVOKE.PYXInnerTile_CSPtr_getBoundingCell(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void getBoundingRects(ICoordConverter coordConvertor, PYXRect2DDouble pRect1, PYXRect2DDouble pRect2) {
    pyxlibPINVOKE.PYXInnerTile_CSPtr_getBoundingRects(swigCPtr, ICoordConverter.getCPtr(coordConvertor), PYXRect2DDouble.getCPtr(pRect1), PYXRect2DDouble.getCPtr(pRect2));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXGeometry_SPtr intersection(PYXGeometry geometry, bool bCommutative) {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXInnerTile_CSPtr_intersection__SWIG_0(swigCPtr, PYXGeometry.getCPtr(geometry), bCommutative), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXGeometry_SPtr intersection(PYXGeometry geometry) {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXInnerTile_CSPtr_intersection__SWIG_1(swigCPtr, PYXGeometry.getCPtr(geometry)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXGeometry_SPtr intersection(PYXInnerTile tile) {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXInnerTile_CSPtr_intersection__SWIG_2(swigCPtr, PYXInnerTile.getCPtr(tile)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXGeometry_SPtr intersection(PYXCell cell) {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXInnerTile_CSPtr_intersection__SWIG_3(swigCPtr, PYXCell.getCPtr(cell)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool intersects(PYXGeometry geometry, bool bCommutative) {
    bool ret = pyxlibPINVOKE.PYXInnerTile_CSPtr_intersects__SWIG_0(swigCPtr, PYXGeometry.getCPtr(geometry), bCommutative);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool intersects(PYXGeometry geometry) {
    bool ret = pyxlibPINVOKE.PYXInnerTile_CSPtr_intersects__SWIG_1(swigCPtr, PYXGeometry.getCPtr(geometry));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool intersects(PYXInnerTile tile) {
    bool ret = pyxlibPINVOKE.PYXInnerTile_CSPtr_intersects__SWIG_2(swigCPtr, PYXInnerTile.getCPtr(tile));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool intersects(PYXCell cell) {
    bool ret = pyxlibPINVOKE.PYXInnerTile_CSPtr_intersects__SWIG_3(swigCPtr, PYXCell.getCPtr(cell));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXIterator_SPtr getIterator() {
    PYXIterator_SPtr ret = new PYXIterator_SPtr(pyxlibPINVOKE.PYXInnerTile_CSPtr_getIterator(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void calcPerimeter(Vector_Index pVecIndex) {
    pyxlibPINVOKE.PYXInnerTile_CSPtr_calcPerimeter(swigCPtr, Vector_Index.getCPtr(pVecIndex));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXBoundingCircle getBoundingCircle() {
    PYXBoundingCircle ret = new PYXBoundingCircle(pyxlibPINVOKE.PYXInnerTile_CSPtr_getBoundingCircle(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void copyTo(PYXTileCollection pTileCollection) {
    pyxlibPINVOKE.PYXInnerTile_CSPtr_copyTo__SWIG_0(swigCPtr, PYXTileCollection.getCPtr(pTileCollection));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void copyTo(PYXTileCollection pTileCollection, int nTargetResolution) {
    pyxlibPINVOKE.PYXInnerTile_CSPtr_copyTo__SWIG_1(swigCPtr, PYXTileCollection.getCPtr(pTileCollection), nTargetResolution);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool hasIndex(PYXIcosIndex pyxIndex) {
    bool ret = pyxlibPINVOKE.PYXInnerTile_CSPtr_hasIndex(swigCPtr, PYXIcosIndex.getCPtr(pyxIndex));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXIcosIndex getRootIndex() {
    PYXIcosIndex ret = new PYXIcosIndex(pyxlibPINVOKE.PYXInnerTile_CSPtr_getRootIndex(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getDepth() {
    int ret = pyxlibPINVOKE.PYXInnerTile_CSPtr_getDepth(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getCellCount() {
    int ret = pyxlibPINVOKE.PYXInnerTile_CSPtr_getCellCount(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXTile asTile() {
    PYXTile ret = new PYXTile(pyxlibPINVOKE.PYXInnerTile_CSPtr_asTile(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isCollection() {
    bool ret = pyxlibPINVOKE.PYXInnerTile_CSPtr_isCollection(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool contains(PYXGeometry geometry) {
    bool ret = pyxlibPINVOKE.PYXInnerTile_CSPtr_contains(swigCPtr, PYXGeometry.getCPtr(geometry));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXGeometry_SPtr disjunction(PYXGeometry geometry) {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXInnerTile_CSPtr_disjunction(swigCPtr, PYXGeometry.getCPtr(geometry)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXInnerTileIntersectionIterator_SPtr getInnerTileIterator(PYXInnerTile tile) {
    PYXInnerTileIntersectionIterator_SPtr ret = new PYXInnerTileIntersectionIterator_SPtr(pyxlibPINVOKE.PYXInnerTile_CSPtr_getInnerTileIterator(swigCPtr, PYXInnerTile.getCPtr(tile)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
