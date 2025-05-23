/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class BitmapServerProvider_SPtr : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public BitmapServerProvider_SPtr(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(BitmapServerProvider_SPtr obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~BitmapServerProvider_SPtr() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_BitmapServerProvider_SPtr(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public BitmapServerProvider_SPtr() : this(pyxlibPINVOKE.new_BitmapServerProvider_SPtr__SWIG_0(), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public BitmapServerProvider_SPtr(BitmapServerProvider p, bool add_ref) : this(pyxlibPINVOKE.new_BitmapServerProvider_SPtr__SWIG_1(BitmapServerProvider.getCPtr(p), add_ref), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public BitmapServerProvider_SPtr(BitmapServerProvider p) : this(pyxlibPINVOKE.new_BitmapServerProvider_SPtr__SWIG_2(BitmapServerProvider.getCPtr(p)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public BitmapServerProvider_SPtr(BitmapServerProvider_SPtr rhs) : this(pyxlibPINVOKE.new_BitmapServerProvider_SPtr__SWIG_3(BitmapServerProvider_SPtr.getCPtr(rhs)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset() {
    pyxlibPINVOKE.BitmapServerProvider_SPtr_reset__SWIG_0(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset(BitmapServerProvider rhs) {
    pyxlibPINVOKE.BitmapServerProvider_SPtr_reset__SWIG_1(swigCPtr, BitmapServerProvider.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public BitmapServerProvider get() {
    IntPtr cPtr = pyxlibPINVOKE.BitmapServerProvider_SPtr_get(swigCPtr);
    BitmapServerProvider ret = (cPtr == IntPtr.Zero) ? null : new BitmapServerProvider(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public BitmapServerProvider __ref__() {
    BitmapServerProvider ret = new BitmapServerProvider(pyxlibPINVOKE.BitmapServerProvider_SPtr___ref__(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public BitmapServerProvider __deref__() {
    IntPtr cPtr = pyxlibPINVOKE.BitmapServerProvider_SPtr___deref__(swigCPtr);
    BitmapServerProvider ret = (cPtr == IntPtr.Zero) ? null : new BitmapServerProvider(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void swap(BitmapServerProvider_SPtr rhs) {
    pyxlibPINVOKE.BitmapServerProvider_SPtr_swap(swigCPtr, BitmapServerProvider_SPtr.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isNull() {
    bool ret = pyxlibPINVOKE.BitmapServerProvider_SPtr_isNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isNotNull() {
    bool ret = pyxlibPINVOKE.BitmapServerProvider_SPtr_isNotNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public string loadIcon(string iconStyle) {
  // Generated from %typemap(csout) string
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.BitmapServerProvider_SPtr_loadIcon(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(iconStyle)));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public string loadResource(string resourceName) {
  // Generated from %typemap(csout) string
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.BitmapServerProvider_SPtr_loadResource(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(resourceName)));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public string loadBitmap(string path) {
  // Generated from %typemap(csout) string
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.BitmapServerProvider_SPtr_loadBitmap(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(path)));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public string forceRGB(string path) {
  // Generated from %typemap(csout) string
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.BitmapServerProvider_SPtr_forceRGB(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(path)));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public int release() {
    int ret = pyxlibPINVOKE.BitmapServerProvider_SPtr_release(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int addRef() {
    int ret = pyxlibPINVOKE.BitmapServerProvider_SPtr_addRef(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public BitmapServerProvider_SPtr getBitmapServerProvider() {
    BitmapServerProvider_SPtr ret = new BitmapServerProvider_SPtr(pyxlibPINVOKE.BitmapServerProvider_SPtr_getBitmapServerProvider(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void setBitmapServerProvider(BitmapServerProvider_SPtr spProvider) {
    pyxlibPINVOKE.BitmapServerProvider_SPtr_setBitmapServerProvider(swigCPtr, BitmapServerProvider_SPtr.getCPtr(spProvider));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

}
