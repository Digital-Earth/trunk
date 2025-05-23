/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class XYCoverageValueGetterDefault : XYCoverageValueGetter {
  private HandleRef swigCPtr;

  public XYCoverageValueGetterDefault(IntPtr cPtr, bool cMemoryOwn) : base(pyxlibPINVOKE.XYCoverageValueGetterDefaultUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(XYCoverageValueGetterDefault obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~XYCoverageValueGetterDefault() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_XYCoverageValueGetterDefault(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public void setXYCoverageValue(IXYCoverage pXYCoverage) {
    pyxlibPINVOKE.XYCoverageValueGetterDefault_setXYCoverageValue(swigCPtr, IXYCoverage.getCPtr(pXYCoverage));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override bool getCoverageValue(PYXCoord2DDouble native, PYXValue pValue) {
    bool ret = pyxlibPINVOKE.XYCoverageValueGetterDefault_getCoverageValue(swigCPtr, PYXCoord2DDouble.getCPtr(native), PYXValue.getCPtr(pValue));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public XYCoverageValueGetterDefault() : this(pyxlibPINVOKE.new_XYCoverageValueGetterDefault(), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

}
