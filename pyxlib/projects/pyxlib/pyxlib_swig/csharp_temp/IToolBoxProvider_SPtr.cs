/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class IToolBoxProvider_SPtr : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public IToolBoxProvider_SPtr(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(IToolBoxProvider_SPtr obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~IToolBoxProvider_SPtr() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_IToolBoxProvider_SPtr(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public IToolBoxProvider_SPtr() : this(pyxlibPINVOKE.new_IToolBoxProvider_SPtr__SWIG_0(), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public IToolBoxProvider_SPtr(IToolBoxProvider p, bool add_ref) : this(pyxlibPINVOKE.new_IToolBoxProvider_SPtr__SWIG_1(IToolBoxProvider.getCPtr(p), add_ref), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public IToolBoxProvider_SPtr(IToolBoxProvider p) : this(pyxlibPINVOKE.new_IToolBoxProvider_SPtr__SWIG_2(IToolBoxProvider.getCPtr(p)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public IToolBoxProvider_SPtr(IToolBoxProvider_SPtr rhs) : this(pyxlibPINVOKE.new_IToolBoxProvider_SPtr__SWIG_3(IToolBoxProvider_SPtr.getCPtr(rhs)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset() {
    pyxlibPINVOKE.IToolBoxProvider_SPtr_reset__SWIG_0(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset(IToolBoxProvider rhs) {
    pyxlibPINVOKE.IToolBoxProvider_SPtr_reset__SWIG_1(swigCPtr, IToolBoxProvider.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public IToolBoxProvider get() {
    IntPtr cPtr = pyxlibPINVOKE.IToolBoxProvider_SPtr_get(swigCPtr);
    IToolBoxProvider ret = (cPtr == IntPtr.Zero) ? null : new IToolBoxProvider(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public IToolBoxProvider __ref__() {
    IToolBoxProvider ret = new IToolBoxProvider(pyxlibPINVOKE.IToolBoxProvider_SPtr___ref__(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public IToolBoxProvider __deref__() {
    IntPtr cPtr = pyxlibPINVOKE.IToolBoxProvider_SPtr___deref__(swigCPtr);
    IToolBoxProvider ret = (cPtr == IntPtr.Zero) ? null : new IToolBoxProvider(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void swap(IToolBoxProvider_SPtr rhs) {
    pyxlibPINVOKE.IToolBoxProvider_SPtr_swap(swigCPtr, IToolBoxProvider_SPtr.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isNull() {
    bool ret = pyxlibPINVOKE.IToolBoxProvider_SPtr_isNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isNotNull() {
    bool ret = pyxlibPINVOKE.IToolBoxProvider_SPtr_isNotNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public GUID iid {
    get {
      IntPtr cPtr = pyxlibPINVOKE.IToolBoxProvider_SPtr_iid_get(swigCPtr);
      GUID ret = (cPtr == IntPtr.Zero) ? null : new GUID(cPtr, false);
      if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public ProcessList_SPtr getProcessList() {
    ProcessList_SPtr ret = new ProcessList_SPtr(pyxlibPINVOKE.IToolBoxProvider_SPtr_getProcessList(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void setProcessList(ProcessList_SPtr spProcList) {
    pyxlibPINVOKE.IToolBoxProvider_SPtr_setProcessList(swigCPtr, ProcessList_SPtr.getCPtr(spProcList));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public uint AddRef() {
    uint ret = pyxlibPINVOKE.IToolBoxProvider_SPtr_AddRef(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public uint Release() {
    uint ret = pyxlibPINVOKE.IToolBoxProvider_SPtr_Release(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
