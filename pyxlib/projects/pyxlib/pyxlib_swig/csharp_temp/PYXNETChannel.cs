/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class PYXNETChannel : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public PYXNETChannel(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(PYXNETChannel obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~PYXNETChannel() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_PYXNETChannel(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public void publish() {
    pyxlibPINVOKE.PYXNETChannel_publish(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void unpublish() {
    pyxlibPINVOKE.PYXNETChannel_unpublish(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isPublished() {
    bool ret = pyxlibPINVOKE.PYXNETChannel_isPublished(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool wasFoundRemotely() {
    bool ret = pyxlibPINVOKE.PYXNETChannel_wasFoundRemotely(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public ProcRef getProcRef() {
    ProcRef ret = new ProcRef(pyxlibPINVOKE.PYXNETChannel_getProcRef(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public string getDataCode() {
  // Generated from typemap(csout) const string &
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.PYXNETChannel_getDataCode(swigCPtr));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public int getHandle() {
    int ret = pyxlibPINVOKE.PYXNETChannel_getHandle(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public SWIGTYPE_p_std__auto_ptrTPYXConstWireBuffer_t getKey(string key) {
    SWIGTYPE_p_std__auto_ptrTPYXConstWireBuffer_t ret = new SWIGTYPE_p_std__auto_ptrTPYXConstWireBuffer_t(pyxlibPINVOKE.PYXNETChannel_getKey(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(key)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public SWIGTYPE_p_boost__intrusive_ptrTPYXTaskSourceWithResultTboost__intrusive_ptrTPYXConstBuffer_t_t_t requestKey(string key) {
    SWIGTYPE_p_boost__intrusive_ptrTPYXTaskSourceWithResultTboost__intrusive_ptrTPYXConstBuffer_t_t_t ret = new SWIGTYPE_p_boost__intrusive_ptrTPYXTaskSourceWithResultTboost__intrusive_ptrTPYXConstBuffer_t_t_t(pyxlibPINVOKE.PYXNETChannel_requestKey(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(key)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void attachLocalProvider(PYXNETChannelKeyProvider_SPtr provider) {
    pyxlibPINVOKE.PYXNETChannel_attachLocalProvider(swigCPtr, PYXNETChannelKeyProvider_SPtr.getCPtr(provider));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

}
