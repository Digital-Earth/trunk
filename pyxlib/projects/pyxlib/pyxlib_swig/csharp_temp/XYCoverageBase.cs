/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class XYCoverageBase : IXYCoverage {
  private HandleRef swigCPtr;

  public XYCoverageBase(IntPtr cPtr, bool cMemoryOwn) : base(pyxlibPINVOKE.XYCoverageBaseUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(XYCoverageBase obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~XYCoverageBase() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_XYCoverageBase(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public override PYXTableDefinition_CSPtr getCoverageDefinition() {
    PYXTableDefinition_CSPtr ret = new PYXTableDefinition_CSPtr(pyxlibPINVOKE.XYCoverageBase_getCoverageDefinition__SWIG_0(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override XYCoverageValueGetter getCoverageValueGetter() {
    IntPtr cPtr = pyxlibPINVOKE.XYCoverageBase_getCoverageValueGetter(swigCPtr);
    XYCoverageValueGetter ret = (cPtr == IntPtr.Zero) ? null : new XYCoverageValueGetter(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override SWIGTYPE_p_boost__intrusive_ptrTXYAsyncValueGetter_t getAsyncCoverageValueGetter(SWIGTYPE_p_XYAsyncValueConsumer consumer, int matrixWidth, int matrixHeight) {
    SWIGTYPE_p_boost__intrusive_ptrTXYAsyncValueGetter_t ret = new SWIGTYPE_p_boost__intrusive_ptrTXYAsyncValueGetter_t(pyxlibPINVOKE.XYCoverageBase_getAsyncCoverageValueGetter(swigCPtr, SWIGTYPE_p_XYAsyncValueConsumer.getCPtr(consumer), matrixWidth, matrixHeight), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual bool getCoverageValue(PYXCoord2DDouble native, PYXValue pValue, int nResolution, int nFieldIndex) {
    bool ret = pyxlibPINVOKE.XYCoverageBase_getCoverageValue__SWIG_0(swigCPtr, PYXCoord2DDouble.getCPtr(native), PYXValue.getCPtr(pValue), nResolution, nFieldIndex);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual bool getCoverageValue(PYXCoord2DDouble native, PYXValue pValue, int nResolution) {
    bool ret = pyxlibPINVOKE.XYCoverageBase_getCoverageValue__SWIG_1(swigCPtr, PYXCoord2DDouble.getCPtr(native), PYXValue.getCPtr(pValue), nResolution);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual void getMatrixOfValues(PYXCoord2DDouble nativeCentre, PYXValue pValues, int sizeX, int sizeY, int nResolution, int nFieldIndex) {
    pyxlibPINVOKE.XYCoverageBase_getMatrixOfValues__SWIG_0(swigCPtr, PYXCoord2DDouble.getCPtr(nativeCentre), PYXValue.getCPtr(pValues), sizeX, sizeY, nResolution, nFieldIndex);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public virtual void getMatrixOfValues(PYXCoord2DDouble nativeCentre, PYXValue pValues, int sizeX, int sizeY, int nResolution) {
    pyxlibPINVOKE.XYCoverageBase_getMatrixOfValues__SWIG_1(swigCPtr, PYXCoord2DDouble.getCPtr(nativeCentre), PYXValue.getCPtr(pValues), sizeX, sizeY, nResolution);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override bool hasSpatialReferenceSystem() {
    bool ret = pyxlibPINVOKE.XYCoverageBase_hasSpatialReferenceSystem(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override void setSpatialReferenceSystem(SWIGTYPE_p_boost__intrusive_ptrTISRS_t spSRS) {
    pyxlibPINVOKE.XYCoverageBase_setSpatialReferenceSystem(swigCPtr, SWIGTYPE_p_boost__intrusive_ptrTISRS_t.getCPtr(spSRS));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override ICoordConverter getCoordConverter() {
    IntPtr cPtr = pyxlibPINVOKE.XYCoverageBase_getCoordConverter(swigCPtr);
    ICoordConverter ret = (cPtr == IntPtr.Zero) ? null : new ICoordConverter(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override double getSpatialPrecision() {
    double ret = pyxlibPINVOKE.XYCoverageBase_getSpatialPrecision(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override PYXRect2DDouble getBounds() {
    PYXRect2DDouble ret = new PYXRect2DDouble(pyxlibPINVOKE.XYCoverageBase_getBounds(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override PYXCoord2DDouble getStepSize() {
    PYXCoord2DDouble ret = new PYXCoord2DDouble(pyxlibPINVOKE.XYCoverageBase_getStepSize(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override PYXCoord2DDouble nativeToRasterSubPixel(PYXCoord2DDouble native) {
    PYXCoord2DDouble ret = new PYXCoord2DDouble(pyxlibPINVOKE.XYCoverageBase_nativeToRasterSubPixel(swigCPtr, PYXCoord2DDouble.getCPtr(native)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override void tileLoadHint(PYXTile tile) {
    pyxlibPINVOKE.XYCoverageBase_tileLoadHint(swigCPtr, PYXTile.getCPtr(tile));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override void tileLoadDoneHint(PYXTile tile) {
    pyxlibPINVOKE.XYCoverageBase_tileLoadDoneHint(swigCPtr, PYXTile.getCPtr(tile));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

}
