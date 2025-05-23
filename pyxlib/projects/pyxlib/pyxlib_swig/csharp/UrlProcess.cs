/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class UrlProcess : IUrl {
  private HandleRef swigCPtr;

  public UrlProcess(IntPtr cPtr, bool cMemoryOwn) : base(pyxlibPINVOKE.UrlProcessUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(UrlProcess obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~UrlProcess() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_UrlProcess(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public static GUID clsid {
    get {
      IntPtr cPtr = pyxlibPINVOKE.UrlProcess_clsid_get();
      GUID ret = (cPtr == IntPtr.Zero) ? null : new GUID(cPtr, false);
      if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public static GUID aiid {
    get {
      IntPtr cPtr = pyxlibPINVOKE.UrlProcess_aiid_get();
      GUID ret = (cPtr == IntPtr.Zero) ? null : new GUID(cPtr, false);
      if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public static int niid {
    get {
      int ret = pyxlibPINVOKE.UrlProcess_niid_get();
      if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public static string kStrUrlKey {
  // Generated from %typemap(csvarout) const string &
  get {
    string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.UrlProcess_kStrUrlKey_get());
    
      if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  }

  public static void test() {
    pyxlibPINVOKE.UrlProcess_test();
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public UrlProcess() : this(pyxlibPINVOKE.new_UrlProcess(), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override uint AddRef() {
    uint ret = pyxlibPINVOKE.UrlProcess_AddRef(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override uint Release() {
    uint ret = pyxlibPINVOKE.UrlProcess_Release(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static ProcessSpec_SPtr getSpecStatic() {
    ProcessSpec_SPtr ret = new ProcessSpec_SPtr(pyxlibPINVOKE.UrlProcess_getSpecStatic(), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual ProcessSpec_SPtr getSpec() {
    ProcessSpec_SPtr ret = new ProcessSpec_SPtr(pyxlibPINVOKE.UrlProcess_getSpec(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual IUnknown_CSPtr getOutput() {
    IUnknown_CSPtr ret = new IUnknown_CSPtr(pyxlibPINVOKE.UrlProcess_getOutput__SWIG_0(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual string getAttributeSchema() {
  // Generated from %typemap(csout) string
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.UrlProcess_getAttributeSchema(swigCPtr));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public virtual Attribute_Map getAttributes() {
    Attribute_Map ret = new Attribute_Map(pyxlibPINVOKE.UrlProcess_getAttributes(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual void setAttributes(Attribute_Map mapAttr) {
    pyxlibPINVOKE.UrlProcess_setAttributes(swigCPtr, Attribute_Map.getCPtr(mapAttr));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override string getUrl() {
  // Generated from %typemap(csout) string
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.UrlProcess_getUrl(swigCPtr));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public override bool setUrl(string url) {
    bool ret = pyxlibPINVOKE.UrlProcess_setUrl(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(url));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override string getManifest() {
  // Generated from %typemap(csout) string
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.UrlProcess_getManifest(swigCPtr));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public override bool isLocalFile() {
    bool ret = pyxlibPINVOKE.UrlProcess_isLocalFile(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
