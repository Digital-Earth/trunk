/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class FileEvent_SPtr : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public FileEvent_SPtr(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(FileEvent_SPtr obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~FileEvent_SPtr() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_FileEvent_SPtr(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public FileEvent_SPtr() : this(pyxlibPINVOKE.new_FileEvent_SPtr__SWIG_0(), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public FileEvent_SPtr(FileEvent p, bool add_ref) : this(pyxlibPINVOKE.new_FileEvent_SPtr__SWIG_1(FileEvent.getCPtr(p), add_ref), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public FileEvent_SPtr(FileEvent p) : this(pyxlibPINVOKE.new_FileEvent_SPtr__SWIG_2(FileEvent.getCPtr(p)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public FileEvent_SPtr(FileEvent_SPtr rhs) : this(pyxlibPINVOKE.new_FileEvent_SPtr__SWIG_3(FileEvent_SPtr.getCPtr(rhs)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset() {
    pyxlibPINVOKE.FileEvent_SPtr_reset__SWIG_0(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset(FileEvent rhs) {
    pyxlibPINVOKE.FileEvent_SPtr_reset__SWIG_1(swigCPtr, FileEvent.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public FileEvent get() {
    IntPtr cPtr = pyxlibPINVOKE.FileEvent_SPtr_get(swigCPtr);
    FileEvent ret = (cPtr == IntPtr.Zero) ? null : new FileEvent(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public FileEvent __ref__() {
    FileEvent ret = new FileEvent(pyxlibPINVOKE.FileEvent_SPtr___ref__(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public FileEvent __deref__() {
    IntPtr cPtr = pyxlibPINVOKE.FileEvent_SPtr___deref__(swigCPtr);
    FileEvent ret = (cPtr == IntPtr.Zero) ? null : new FileEvent(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void swap(FileEvent_SPtr rhs) {
    pyxlibPINVOKE.FileEvent_SPtr_swap(swigCPtr, FileEvent_SPtr.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isNull() {
    bool ret = pyxlibPINVOKE.FileEvent_SPtr_isNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isNotNull() {
    bool ret = pyxlibPINVOKE.FileEvent_SPtr_isNotNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public FileEvent_SPtr create(IPath_SPtr spPath, int index) {
    FileEvent_SPtr ret = new FileEvent_SPtr(pyxlibPINVOKE.FileEvent_SPtr_create__SWIG_0(swigCPtr, IPath_SPtr.getCPtr(spPath), index), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public FileEvent_SPtr create(IPath_SPtr spPath) {
    FileEvent_SPtr ret = new FileEvent_SPtr(pyxlibPINVOKE.FileEvent_SPtr_create__SWIG_1(swigCPtr, IPath_SPtr.getCPtr(spPath)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getIndex() {
    int ret = pyxlibPINVOKE.FileEvent_SPtr_getIndex(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public IPath_SPtr getPath() {
    IPath_SPtr ret = new IPath_SPtr(pyxlibPINVOKE.FileEvent_SPtr_getPath(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool getFailed() {
    bool ret = pyxlibPINVOKE.FileEvent_SPtr_getFailed(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void setFailed(bool bFailed) {
    pyxlibPINVOKE.FileEvent_SPtr_setFailed(swigCPtr, bFailed);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public string getLocalPath() {
  // Generated from typemap(csout) const string &
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.FileEvent_SPtr_getLocalPath(swigCPtr));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public void setLocalPath(string value) {
    pyxlibPINVOKE.FileEvent_SPtr_setLocalPath(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(value));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public FileEvent dynamic_cast(NotifierEvent pSource) {
    IntPtr cPtr = pyxlibPINVOKE.FileEvent_SPtr_dynamic_cast(swigCPtr, NotifierEvent.getCPtr(pSource));
    FileEvent ret = (cPtr == IntPtr.Zero) ? null : new FileEvent(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
