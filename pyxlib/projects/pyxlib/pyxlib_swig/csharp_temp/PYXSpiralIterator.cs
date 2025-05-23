/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class PYXSpiralIterator : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public PYXSpiralIterator(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(PYXSpiralIterator obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~PYXSpiralIterator() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_PYXSpiralIterator(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public static int knMaxSpiralingDepth {
    get {
      int ret = pyxlibPINVOKE.PYXSpiralIterator_knMaxSpiralingDepth_get();
      if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public static SWIGTYPE_p_boost__intrusive_ptrTPYXSpiralIterator_t create(PYXIcosIndex rootIndex, int nDepth, int nExtraSpirals) {
    SWIGTYPE_p_boost__intrusive_ptrTPYXSpiralIterator_t ret = new SWIGTYPE_p_boost__intrusive_ptrTPYXSpiralIterator_t(pyxlibPINVOKE.PYXSpiralIterator_create__SWIG_0(PYXIcosIndex.getCPtr(rootIndex), nDepth, nExtraSpirals), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static SWIGTYPE_p_boost__intrusive_ptrTPYXSpiralIterator_t create(PYXIcosIndex rootIndex, int nDepth) {
    SWIGTYPE_p_boost__intrusive_ptrTPYXSpiralIterator_t ret = new SWIGTYPE_p_boost__intrusive_ptrTPYXSpiralIterator_t(pyxlibPINVOKE.PYXSpiralIterator_create__SWIG_1(PYXIcosIndex.getCPtr(rootIndex), nDepth), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static void test() {
    pyxlibPINVOKE.PYXSpiralIterator_test();
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXSpiralIterator(PYXIcosIndex rootIndex, int nDepth, int nExtraSpirals) : this(pyxlibPINVOKE.new_PYXSpiralIterator__SWIG_0(PYXIcosIndex.getCPtr(rootIndex), nDepth, nExtraSpirals), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXSpiralIterator(PYXIcosIndex rootIndex, int nDepth) : this(pyxlibPINVOKE.new_PYXSpiralIterator__SWIG_1(PYXIcosIndex.getCPtr(rootIndex), nDepth), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public virtual void next() {
    pyxlibPINVOKE.PYXSpiralIterator_next(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public virtual bool end() {
    bool ret = pyxlibPINVOKE.PYXSpiralIterator_end(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual PYXIcosIndex getIndex() {
    PYXIcosIndex ret = new PYXIcosIndex(pyxlibPINVOKE.PYXSpiralIterator_getIndex(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void reset(PYXIcosIndex rootIndex, int nSpiralCount, int nExtraSpirals) {
    pyxlibPINVOKE.PYXSpiralIterator_reset__SWIG_0(swigCPtr, PYXIcosIndex.getCPtr(rootIndex), nSpiralCount, nExtraSpirals);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset(PYXIcosIndex rootIndex, int nSpiralCount) {
    pyxlibPINVOKE.PYXSpiralIterator_reset__SWIG_1(swigCPtr, PYXIcosIndex.getCPtr(rootIndex), nSpiralCount);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public int getTwoDirectionFactor() {
    int ret = pyxlibPINVOKE.PYXSpiralIterator_getTwoDirectionFactor(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getSixDirectionFactor() {
    int ret = pyxlibPINVOKE.PYXSpiralIterator_getSixDirectionFactor(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
