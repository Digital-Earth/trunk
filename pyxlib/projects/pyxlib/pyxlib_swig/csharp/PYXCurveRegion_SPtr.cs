/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class PYXCurveRegion_SPtr : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public PYXCurveRegion_SPtr(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(PYXCurveRegion_SPtr obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~PYXCurveRegion_SPtr() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_PYXCurveRegion_SPtr(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public PYXCurveRegion_SPtr() : this(pyxlibPINVOKE.new_PYXCurveRegion_SPtr__SWIG_0(), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXCurveRegion_SPtr(PYXCurveRegion p, bool add_ref) : this(pyxlibPINVOKE.new_PYXCurveRegion_SPtr__SWIG_1(PYXCurveRegion.getCPtr(p), add_ref), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXCurveRegion_SPtr(PYXCurveRegion p) : this(pyxlibPINVOKE.new_PYXCurveRegion_SPtr__SWIG_2(PYXCurveRegion.getCPtr(p)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXCurveRegion_SPtr(PYXCurveRegion_SPtr rhs) : this(pyxlibPINVOKE.new_PYXCurveRegion_SPtr__SWIG_3(PYXCurveRegion_SPtr.getCPtr(rhs)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset() {
    pyxlibPINVOKE.PYXCurveRegion_SPtr_reset__SWIG_0(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset(PYXCurveRegion rhs) {
    pyxlibPINVOKE.PYXCurveRegion_SPtr_reset__SWIG_1(swigCPtr, PYXCurveRegion.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXCurveRegion get() {
    IntPtr cPtr = pyxlibPINVOKE.PYXCurveRegion_SPtr_get(swigCPtr);
    PYXCurveRegion ret = (cPtr == IntPtr.Zero) ? null : new PYXCurveRegion(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXCurveRegion __ref__() {
    PYXCurveRegion ret = new PYXCurveRegion(pyxlibPINVOKE.PYXCurveRegion_SPtr___ref__(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXCurveRegion __deref__() {
    IntPtr cPtr = pyxlibPINVOKE.PYXCurveRegion_SPtr___deref__(swigCPtr);
    PYXCurveRegion ret = (cPtr == IntPtr.Zero) ? null : new PYXCurveRegion(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void swap(PYXCurveRegion_SPtr rhs) {
    pyxlibPINVOKE.PYXCurveRegion_SPtr_swap(swigCPtr, PYXCurveRegion_SPtr.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isNull() {
    bool ret = pyxlibPINVOKE.PYXCurveRegion_SPtr_isNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isNotNull() {
    bool ret = pyxlibPINVOKE.PYXCurveRegion_SPtr_isNotNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public IRegion_SPtr clone() {
    IRegion_SPtr ret = new IRegion_SPtr(pyxlibPINVOKE.PYXCurveRegion_SPtr_clone(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getVerticesCount() {
    int ret = pyxlibPINVOKE.PYXCurveRegion_SPtr_getVerticesCount(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public SWIGTYPE_p_boost__intrusive_ptrTIRegionVisitor_t getVisitor() {
    SWIGTYPE_p_boost__intrusive_ptrTIRegionVisitor_t ret = new SWIGTYPE_p_boost__intrusive_ptrTIRegionVisitor_t(pyxlibPINVOKE.PYXCurveRegion_SPtr_getVisitor(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void serializeVisitor(SWIGTYPE_p_PYXWireBuffer buffer, SWIGTYPE_p_boost__intrusive_ptrTIRegionVisitor_t visitor) {
    pyxlibPINVOKE.PYXCurveRegion_SPtr_serializeVisitor(swigCPtr, SWIGTYPE_p_PYXWireBuffer.getCPtr(buffer), SWIGTYPE_p_boost__intrusive_ptrTIRegionVisitor_t.getCPtr(visitor));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public SWIGTYPE_p_boost__intrusive_ptrTIRegionVisitor_t deserializeVisitor(SWIGTYPE_p_PYXWireBuffer buffer) {
    SWIGTYPE_p_boost__intrusive_ptrTIRegionVisitor_t ret = new SWIGTYPE_p_boost__intrusive_ptrTIRegionVisitor_t(pyxlibPINVOKE.PYXCurveRegion_SPtr_deserializeVisitor(swigCPtr, SWIGTYPE_p_PYXWireBuffer.getCPtr(buffer)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public double getDistanceToBorder(PYXCoord3DDouble location, double errorThreshold) {
    double ret = pyxlibPINVOKE.PYXCurveRegion_SPtr_getDistanceToBorder__SWIG_0(swigCPtr, PYXCoord3DDouble.getCPtr(location), errorThreshold);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public double getDistanceToBorder(PYXCoord3DDouble location) {
    double ret = pyxlibPINVOKE.PYXCurveRegion_SPtr_getDistanceToBorder__SWIG_1(swigCPtr, PYXCoord3DDouble.getCPtr(location));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isPointContained(PYXCoord3DDouble location, double errorThreshold) {
    bool ret = pyxlibPINVOKE.PYXCurveRegion_SPtr_isPointContained__SWIG_0(swigCPtr, PYXCoord3DDouble.getCPtr(location), errorThreshold);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isPointContained(PYXCoord3DDouble location) {
    bool ret = pyxlibPINVOKE.PYXCurveRegion_SPtr_isPointContained__SWIG_1(swigCPtr, PYXCoord3DDouble.getCPtr(location));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXRegion.CellIntersectionState intersects(PYXIcosIndex index, bool asTile) {
    PYXRegion.CellIntersectionState ret = (PYXRegion.CellIntersectionState)pyxlibPINVOKE.PYXCurveRegion_SPtr_intersects__SWIG_0(swigCPtr, PYXIcosIndex.getCPtr(index), asTile);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXRegion.CellIntersectionState intersects(PYXIcosIndex index) {
    PYXRegion.CellIntersectionState ret = (PYXRegion.CellIntersectionState)pyxlibPINVOKE.PYXCurveRegion_SPtr_intersects__SWIG_1(swigCPtr, PYXIcosIndex.getCPtr(index));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXRegion.CellIntersectionState intersects(PYXBoundingCircle circle, double errorThreshold) {
    PYXRegion.CellIntersectionState ret = (PYXRegion.CellIntersectionState)pyxlibPINVOKE.PYXCurveRegion_SPtr_intersects__SWIG_2(swigCPtr, PYXBoundingCircle.getCPtr(circle), errorThreshold);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXRegion.CellIntersectionState intersects(PYXBoundingCircle circle) {
    PYXRegion.CellIntersectionState ret = (PYXRegion.CellIntersectionState)pyxlibPINVOKE.PYXCurveRegion_SPtr_intersects__SWIG_3(swigCPtr, PYXBoundingCircle.getCPtr(circle));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXBoundingCircle getBoundingCircle() {
    PYXBoundingCircle ret = new PYXBoundingCircle(pyxlibPINVOKE.PYXCurveRegion_SPtr_getBoundingCircle(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXCurveRegion_SPtr create() {
    PYXCurveRegion_SPtr ret = new PYXCurveRegion_SPtr(pyxlibPINVOKE.PYXCurveRegion_SPtr_create__SWIG_0(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXCurveRegion_SPtr create(PYXCurveRegion curve) {
    PYXCurveRegion_SPtr ret = new PYXCurveRegion_SPtr(pyxlibPINVOKE.PYXCurveRegion_SPtr_create__SWIG_1(swigCPtr, PYXCurveRegion.getCPtr(curve)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXCurveRegion_SPtr create(PYXCoord3DDouble pointA, PYXCoord3DDouble pointB) {
    PYXCurveRegion_SPtr ret = new PYXCurveRegion_SPtr(pyxlibPINVOKE.PYXCurveRegion_SPtr_create__SWIG_2(swigCPtr, PYXCoord3DDouble.getCPtr(pointA), PYXCoord3DDouble.getCPtr(pointB)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXCurveRegion_SPtr create(SWIGTYPE_p_std__vectorTPYXCoord3DTdouble_t_t points, bool closeCurve) {
    PYXCurveRegion_SPtr ret = new PYXCurveRegion_SPtr(pyxlibPINVOKE.PYXCurveRegion_SPtr_create__SWIG_3(swigCPtr, SWIGTYPE_p_std__vectorTPYXCoord3DTdouble_t_t.getCPtr(points), closeCurve), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXCurveRegion_SPtr create(SWIGTYPE_p_std__vectorTPYXCoord3DTdouble_t_t points) {
    PYXCurveRegion_SPtr ret = new PYXCurveRegion_SPtr(pyxlibPINVOKE.PYXCurveRegion_SPtr_create__SWIG_4(swigCPtr, SWIGTYPE_p_std__vectorTPYXCoord3DTdouble_t_t.getCPtr(points)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void closeCurve() {
    pyxlibPINVOKE.PYXCurveRegion_SPtr_closeCurve(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isClosed() {
    bool ret = pyxlibPINVOKE.PYXCurveRegion_SPtr_isClosed(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXCoord3DDouble getVertex(uint index) {
    PYXCoord3DDouble ret = new PYXCoord3DDouble(pyxlibPINVOKE.PYXCurveRegion_SPtr_getVertex(swigCPtr, index), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void test() {
    pyxlibPINVOKE.PYXCurveRegion_SPtr_test(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

}
