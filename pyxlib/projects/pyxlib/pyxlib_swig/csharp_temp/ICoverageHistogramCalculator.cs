/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class ICoverageHistogramCalculator : PYXCOM_IUnknown {
  private HandleRef swigCPtr;

  public ICoverageHistogramCalculator(IntPtr cPtr, bool cMemoryOwn) : base(pyxlibPINVOKE.ICoverageHistogramCalculatorUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(ICoverageHistogramCalculator obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~ICoverageHistogramCalculator() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_ICoverageHistogramCalculator(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public static GUID iid {
    get {
      IntPtr cPtr = pyxlibPINVOKE.ICoverageHistogramCalculator_iid_get();
      GUID ret = (cPtr == IntPtr.Zero) ? null : new GUID(cPtr, false);
      if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public virtual PYXCellHistogram_SPtr getHistogram(int fieldIndex, IFeature_SPtr feature) {
    PYXCellHistogram_SPtr ret = new PYXCellHistogram_SPtr(pyxlibPINVOKE.ICoverageHistogramCalculator_getHistogram(swigCPtr, fieldIndex, IFeature_SPtr.getCPtr(feature)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
