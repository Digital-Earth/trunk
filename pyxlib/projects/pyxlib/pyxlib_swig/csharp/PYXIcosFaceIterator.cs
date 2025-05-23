/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class PYXIcosFaceIterator : PYXIterator {
  private HandleRef swigCPtr;

  public PYXIcosFaceIterator(IntPtr cPtr, bool cMemoryOwn) : base(pyxlibPINVOKE.PYXIcosFaceIteratorUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(PYXIcosFaceIterator obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~PYXIcosFaceIterator() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_PYXIcosFaceIterator(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public PYXIcosFaceIterator(int nResolution) : this(pyxlibPINVOKE.new_PYXIcosFaceIterator__SWIG_0(nResolution), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXIcosFaceIterator() : this(pyxlibPINVOKE.new_PYXIcosFaceIterator__SWIG_1(), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override void next() {
    pyxlibPINVOKE.PYXIcosFaceIterator_next(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override bool end() {
    bool ret = pyxlibPINVOKE.PYXIcosFaceIterator_end(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override PYXIcosIndex getIndex() {
    PYXIcosIndex ret = new PYXIcosIndex(pyxlibPINVOKE.PYXIcosFaceIterator_getIndex(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
