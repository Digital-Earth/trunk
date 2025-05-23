/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class IWorkbookPointer : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public IWorkbookPointer(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(IWorkbookPointer obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~IWorkbookPointer() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_IWorkbookPointer(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public IWorkbookPointer() : this(pyxlibPINVOKE.new_IWorkbookPointer__SWIG_0(), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public IWorkbookPointer(IWorkbook p, bool add_ref) : this(pyxlibPINVOKE.new_IWorkbookPointer__SWIG_1(IWorkbook.getCPtr(p), add_ref), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public IWorkbookPointer(IWorkbook p) : this(pyxlibPINVOKE.new_IWorkbookPointer__SWIG_2(IWorkbook.getCPtr(p)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public IWorkbookPointer(IWorkbookPointer rhs) : this(pyxlibPINVOKE.new_IWorkbookPointer__SWIG_3(IWorkbookPointer.getCPtr(rhs)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset() {
    pyxlibPINVOKE.IWorkbookPointer_reset__SWIG_0(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset(IWorkbook rhs) {
    pyxlibPINVOKE.IWorkbookPointer_reset__SWIG_1(swigCPtr, IWorkbook.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public IWorkbook get() {
    IntPtr cPtr = pyxlibPINVOKE.IWorkbookPointer_get(swigCPtr);
    IWorkbook ret = (cPtr == IntPtr.Zero) ? null : new IWorkbook(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public IWorkbook __ref__() {
    IWorkbook ret = new IWorkbook(pyxlibPINVOKE.IWorkbookPointer___ref__(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public IWorkbook __deref__() {
    IntPtr cPtr = pyxlibPINVOKE.IWorkbookPointer___deref__(swigCPtr);
    IWorkbook ret = (cPtr == IntPtr.Zero) ? null : new IWorkbook(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void swap(IWorkbookPointer rhs) {
    pyxlibPINVOKE.IWorkbookPointer_swap(swigCPtr, IWorkbookPointer.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isNull() {
    bool ret = pyxlibPINVOKE.IWorkbookPointer_isNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isNotNull() {
    bool ret = pyxlibPINVOKE.IWorkbookPointer_isNotNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public string GetName() {
  // Generated from %typemap(csout) string
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.IWorkbookPointer_GetName(swigCPtr));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public void GetTableNames(Vector_String tableNames) {
    pyxlibPINVOKE.IWorkbookPointer_GetTableNames(swigCPtr, Vector_String.getCPtr(tableNames));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public IWorkbookTablePointer CreateTable(string tableName) {
    IWorkbookTablePointer ret = new IWorkbookTablePointer(pyxlibPINVOKE.IWorkbookPointer_CreateTable(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(tableName)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int addRef() {
    int ret = pyxlibPINVOKE.IWorkbookPointer_addRef(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int release() {
    int ret = pyxlibPINVOKE.IWorkbookPointer_release(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
