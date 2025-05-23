/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class ProcessLifeTimeIndication : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public ProcessLifeTimeIndication(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(ProcessLifeTimeIndication obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~ProcessLifeTimeIndication() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_ProcessLifeTimeIndication(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public static SWIGTYPE_p_boost__intrusive_ptrTProcessLifeTimeIndication_t create(IProcess process) {
    SWIGTYPE_p_boost__intrusive_ptrTProcessLifeTimeIndication_t ret = new SWIGTYPE_p_boost__intrusive_ptrTProcessLifeTimeIndication_t(pyxlibPINVOKE.ProcessLifeTimeIndication_create(IProcess.getCPtr(process)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public string getProcName() {
  // Generated from typemap(csout) const string &
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.ProcessLifeTimeIndication_getProcName(swigCPtr));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public ProcRef getProcRef() {
    ProcRef ret = new ProcRef(pyxlibPINVOKE.ProcessLifeTimeIndication_getProcRef(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public ProcessSpec_SPtr getProcSpec() {
    ProcessSpec_SPtr ret = new ProcessSpec_SPtr(pyxlibPINVOKE.ProcessLifeTimeIndication_getProcSpec(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
