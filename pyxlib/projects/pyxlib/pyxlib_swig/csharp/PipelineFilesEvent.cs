/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class PipelineFilesEvent : NotifierEvent {
  private HandleRef swigCPtr;

  public PipelineFilesEvent(IntPtr cPtr, bool cMemoryOwn) : base(pyxlibPINVOKE.PipelineFilesEventUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(PipelineFilesEvent obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~PipelineFilesEvent() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_PipelineFilesEvent(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public static PipelineFilesEvent_SPtr create(IProcess_SPtr spProc) {
    PipelineFilesEvent_SPtr ret = new PipelineFilesEvent_SPtr(pyxlibPINVOKE.PipelineFilesEvent_create(IProcess_SPtr.getCPtr(spProc)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public IProcess_SPtr getProcess() {
    IProcess_SPtr ret = new IProcess_SPtr(pyxlibPINVOKE.PipelineFilesEvent_getProcess(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool getFailed() {
    bool ret = pyxlibPINVOKE.PipelineFilesEvent_getFailed(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void setFailed(bool bFailed) {
    pyxlibPINVOKE.PipelineFilesEvent_setFailed(swigCPtr, bFailed);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public static PipelineFilesEvent dynamic_cast(NotifierEvent pSource) {
    IntPtr cPtr = pyxlibPINVOKE.PipelineFilesEvent_dynamic_cast(NotifierEvent.getCPtr(pSource));
    PipelineFilesEvent ret = (cPtr == IntPtr.Zero) ? null : new PipelineFilesEvent(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
