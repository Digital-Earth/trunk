/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class PYXSingleIterator : PYXIterator {
  private HandleRef swigCPtr;

  public PYXSingleIterator(IntPtr cPtr, bool cMemoryOwn) : base(pyxlibPINVOKE.PYXSingleIteratorUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(PYXSingleIterator obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~PYXSingleIterator() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_PYXSingleIterator(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public static SWIGTYPE_p_boost__intrusive_ptrTPYXSingleIterator_t create(PYXIcosIndex index) {
    SWIGTYPE_p_boost__intrusive_ptrTPYXSingleIterator_t ret = new SWIGTYPE_p_boost__intrusive_ptrTPYXSingleIterator_t(pyxlibPINVOKE.PYXSingleIterator_create(PYXIcosIndex.getCPtr(index)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXSingleIterator(PYXIcosIndex index) : this(pyxlibPINVOKE.new_PYXSingleIterator(PYXIcosIndex.getCPtr(index)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override void next() {
    pyxlibPINVOKE.PYXSingleIterator_next(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override bool end() {
    bool ret = pyxlibPINVOKE.PYXSingleIterator_end(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override PYXIcosIndex getIndex() {
    PYXIcosIndex ret = new PYXIcosIndex(pyxlibPINVOKE.PYXSingleIterator_getIndex(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
