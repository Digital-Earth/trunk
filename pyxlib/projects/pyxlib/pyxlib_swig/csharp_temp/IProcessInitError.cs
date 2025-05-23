/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class IProcessInitError : PYXCOM_IUnknown {
  private HandleRef swigCPtr;

  public IProcessInitError(IntPtr cPtr, bool cMemoryOwn) : base(pyxlibPINVOKE.IProcessInitErrorUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(IProcessInitError obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~IProcessInitError() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_IProcessInitError(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public static GUID iid {
    get {
      IntPtr cPtr = pyxlibPINVOKE.IProcessInitError_iid_get();
      GUID ret = (cPtr == IntPtr.Zero) ? null : new GUID(cPtr, false);
      if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public virtual string getError() {
  // Generated from typemap(csout) const string &
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.IProcessInitError_getError(swigCPtr));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public virtual void setError(string strError) {
    pyxlibPINVOKE.IProcessInitError_setError(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strError));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public virtual string getErrorID() {
  // Generated from %typemap(csout) string
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.IProcessInitError_getErrorID(swigCPtr));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

}
