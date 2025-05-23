/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class IXYCoverage : IFeatureCollection {
  private HandleRef swigCPtr;

  public IXYCoverage(IntPtr cPtr, bool cMemoryOwn) : base(pyxlibPINVOKE.IXYCoverageUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(IXYCoverage obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~IXYCoverage() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_IXYCoverage(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public static GUID iid {
    get {
      IntPtr cPtr = pyxlibPINVOKE.IXYCoverage_iid_get();
      GUID ret = (cPtr == IntPtr.Zero) ? null : new GUID(cPtr, false);
      if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public virtual PYXTableDefinition_CSPtr getCoverageDefinition() {
    PYXTableDefinition_CSPtr ret = new PYXTableDefinition_CSPtr(pyxlibPINVOKE.IXYCoverage_getCoverageDefinition__SWIG_0(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual XYCoverageValueGetter getCoverageValueGetter() {
    IntPtr cPtr = pyxlibPINVOKE.IXYCoverage_getCoverageValueGetter(swigCPtr);
    XYCoverageValueGetter ret = (cPtr == IntPtr.Zero) ? null : new XYCoverageValueGetter(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual SWIGTYPE_p_boost__intrusive_ptrTXYAsyncValueGetter_t getAsyncCoverageValueGetter(SWIGTYPE_p_XYAsyncValueConsumer consumer, int matrixWidth, int matrixHeight) {
    SWIGTYPE_p_boost__intrusive_ptrTXYAsyncValueGetter_t ret = new SWIGTYPE_p_boost__intrusive_ptrTXYAsyncValueGetter_t(pyxlibPINVOKE.IXYCoverage_getAsyncCoverageValueGetter(swigCPtr, SWIGTYPE_p_XYAsyncValueConsumer.getCPtr(consumer), matrixWidth, matrixHeight), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual bool getCoverageValue(PYXCoord2DDouble native, PYXValue pValue) {
    bool ret = pyxlibPINVOKE.IXYCoverage_getCoverageValue(swigCPtr, PYXCoord2DDouble.getCPtr(native), PYXValue.getCPtr(pValue));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual void getMatrixOfValues(PYXCoord2DDouble nativeCentre, PYXValue pValues, int sizeX, int sizeY) {
    pyxlibPINVOKE.IXYCoverage_getMatrixOfValues(swigCPtr, PYXCoord2DDouble.getCPtr(nativeCentre), PYXValue.getCPtr(pValues), sizeX, sizeY);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public virtual bool hasSpatialReferenceSystem() {
    bool ret = pyxlibPINVOKE.IXYCoverage_hasSpatialReferenceSystem(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual void setSpatialReferenceSystem(SWIGTYPE_p_boost__intrusive_ptrTISRS_t spSRS) {
    pyxlibPINVOKE.IXYCoverage_setSpatialReferenceSystem(swigCPtr, SWIGTYPE_p_boost__intrusive_ptrTISRS_t.getCPtr(spSRS));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public virtual ICoordConverter getCoordConverter() {
    IntPtr cPtr = pyxlibPINVOKE.IXYCoverage_getCoordConverter(swigCPtr);
    ICoordConverter ret = (cPtr == IntPtr.Zero) ? null : new ICoordConverter(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual double getSpatialPrecision() {
    double ret = pyxlibPINVOKE.IXYCoverage_getSpatialPrecision(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual PYXRect2DDouble getBounds() {
    PYXRect2DDouble ret = new PYXRect2DDouble(pyxlibPINVOKE.IXYCoverage_getBounds(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual PYXCoord2DDouble getStepSize() {
    PYXCoord2DDouble ret = new PYXCoord2DDouble(pyxlibPINVOKE.IXYCoverage_getStepSize(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual PYXCoord2DDouble nativeToRasterSubPixel(PYXCoord2DDouble native) {
    PYXCoord2DDouble ret = new PYXCoord2DDouble(pyxlibPINVOKE.IXYCoverage_nativeToRasterSubPixel(swigCPtr, PYXCoord2DDouble.getCPtr(native)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual void tileLoadHint(PYXTile tile) {
    pyxlibPINVOKE.IXYCoverage_tileLoadHint(swigCPtr, PYXTile.getCPtr(tile));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public virtual void tileLoadDoneHint(PYXTile tile) {
    pyxlibPINVOKE.IXYCoverage_tileLoadDoneHint(swigCPtr, PYXTile.getCPtr(tile));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

}
