/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class IEnumClassObject : PYXCOM_IUnknown {
  private HandleRef swigCPtr;

  public IEnumClassObject(IntPtr cPtr, bool cMemoryOwn) : base(pyxlibPINVOKE.IEnumClassObjectUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(IEnumClassObject obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~IEnumClassObject() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_IEnumClassObject(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public static GUID iid {
    get {
      IntPtr cPtr = pyxlibPINVOKE.IEnumClassObject_iid_get();
      GUID ret = (cPtr == IntPtr.Zero) ? null : new GUID(cPtr, false);
      if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public virtual int Next(uint nElem, SWIGTYPE_p_p_PYXCOM_IUnknown ppObject, SWIGTYPE_p_unsigned_long pnFetched) {
    int ret = pyxlibPINVOKE.IEnumClassObject_Next(swigCPtr, nElem, SWIGTYPE_p_p_PYXCOM_IUnknown.getCPtr(ppObject), SWIGTYPE_p_unsigned_long.getCPtr(pnFetched));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual int Skip(uint nElem) {
    int ret = pyxlibPINVOKE.IEnumClassObject_Skip(swigCPtr, nElem);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual int Reset() {
    int ret = pyxlibPINVOKE.IEnumClassObject_Reset(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual int Clone(SWIGTYPE_p_p_IEnumClassObject ppObject) {
    int ret = pyxlibPINVOKE.IEnumClassObject_Clone(swigCPtr, SWIGTYPE_p_p_IEnumClassObject.getCPtr(ppObject));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
