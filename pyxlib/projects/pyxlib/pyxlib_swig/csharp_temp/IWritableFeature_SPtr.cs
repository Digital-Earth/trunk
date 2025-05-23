/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class IWritableFeature_SPtr : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public IWritableFeature_SPtr(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(IWritableFeature_SPtr obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~IWritableFeature_SPtr() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_IWritableFeature_SPtr(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public IWritableFeature_SPtr() : this(pyxlibPINVOKE.new_IWritableFeature_SPtr__SWIG_0(), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public IWritableFeature_SPtr(IWritableFeature p, bool add_ref) : this(pyxlibPINVOKE.new_IWritableFeature_SPtr__SWIG_1(IWritableFeature.getCPtr(p), add_ref), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public IWritableFeature_SPtr(IWritableFeature p) : this(pyxlibPINVOKE.new_IWritableFeature_SPtr__SWIG_2(IWritableFeature.getCPtr(p)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public IWritableFeature_SPtr(IWritableFeature_SPtr rhs) : this(pyxlibPINVOKE.new_IWritableFeature_SPtr__SWIG_3(IWritableFeature_SPtr.getCPtr(rhs)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset() {
    pyxlibPINVOKE.IWritableFeature_SPtr_reset__SWIG_0(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset(IWritableFeature rhs) {
    pyxlibPINVOKE.IWritableFeature_SPtr_reset__SWIG_1(swigCPtr, IWritableFeature.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public IWritableFeature get() {
    IntPtr cPtr = pyxlibPINVOKE.IWritableFeature_SPtr_get(swigCPtr);
    IWritableFeature ret = (cPtr == IntPtr.Zero) ? null : new IWritableFeature(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public IWritableFeature __ref__() {
    IWritableFeature ret = new IWritableFeature(pyxlibPINVOKE.IWritableFeature_SPtr___ref__(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public IWritableFeature __deref__() {
    IntPtr cPtr = pyxlibPINVOKE.IWritableFeature_SPtr___deref__(swigCPtr);
    IWritableFeature ret = (cPtr == IntPtr.Zero) ? null : new IWritableFeature(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void swap(IWritableFeature_SPtr rhs) {
    pyxlibPINVOKE.IWritableFeature_SPtr_swap(swigCPtr, IWritableFeature_SPtr.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isNull() {
    bool ret = pyxlibPINVOKE.IWritableFeature_SPtr_isNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isNotNull() {
    bool ret = pyxlibPINVOKE.IWritableFeature_SPtr_isNotNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public GUID iid {
    get {
      IntPtr cPtr = pyxlibPINVOKE.IWritableFeature_SPtr_iid_get(swigCPtr);
      GUID ret = (cPtr == IntPtr.Zero) ? null : new GUID(cPtr, false);
      if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public void setID(string strID) {
    pyxlibPINVOKE.IWritableFeature_SPtr_setID(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strID));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void setGeometryName(string strName) {
    pyxlibPINVOKE.IWritableFeature_SPtr_setGeometryName(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strName));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void setGeometry(PYXGeometry_SPtr spGeom) {
    pyxlibPINVOKE.IWritableFeature_SPtr_setGeometry(swigCPtr, PYXGeometry_SPtr.getCPtr(spGeom));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void setIsWritAble(bool bWritable) {
    pyxlibPINVOKE.IWritableFeature_SPtr_setIsWritAble(swigCPtr, bWritable);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void setStyle(string style) {
    pyxlibPINVOKE.IWritableFeature_SPtr_setStyle(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(style));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void setMetaDataDefinition(PYXTableDefinition_SPtr spDef) {
    pyxlibPINVOKE.IWritableFeature_SPtr_setMetaDataDefinition(swigCPtr, PYXTableDefinition_SPtr.getCPtr(spDef));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isWritable() {
    bool ret = pyxlibPINVOKE.IWritableFeature_SPtr_isWritable(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public string getID() {
  // Generated from typemap(csout) const string &
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.IWritableFeature_SPtr_getID(swigCPtr));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public PYXGeometry_SPtr getGeometry() {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.IWritableFeature_SPtr_getGeometry(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXGeometry_CSPtr getGeometry_const() {
    PYXGeometry_CSPtr ret = new PYXGeometry_CSPtr(pyxlibPINVOKE.IWritableFeature_SPtr_getGeometry_const(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public string getStyle() {
  // Generated from %typemap(csout) string
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.IWritableFeature_SPtr_getStyle__SWIG_0(swigCPtr));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public string getStyle(string strStyleToGet) {
  // Generated from %typemap(csout) string
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.IWritableFeature_SPtr_getStyle__SWIG_1(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strStyleToGet)));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public PYXTableDefinition_CSPtr getDefinition() {
    PYXTableDefinition_CSPtr ret = new PYXTableDefinition_CSPtr(pyxlibPINVOKE.IWritableFeature_SPtr_getDefinition__SWIG_0(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXValue getFieldValue(int nFieldIndex) {
    PYXValue ret = new PYXValue(pyxlibPINVOKE.IWritableFeature_SPtr_getFieldValue(swigCPtr, nFieldIndex), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void setFieldValue(PYXValue value, int nFieldIndex) {
    pyxlibPINVOKE.IWritableFeature_SPtr_setFieldValue(swigCPtr, PYXValue.getCPtr(value), nFieldIndex);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXValue getFieldValueByName(string strName) {
    PYXValue ret = new PYXValue(pyxlibPINVOKE.IWritableFeature_SPtr_getFieldValueByName(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strName)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void setFieldValueByName(PYXValue value, string strName) {
    pyxlibPINVOKE.IWritableFeature_SPtr_setFieldValueByName(swigCPtr, PYXValue.getCPtr(value), pyxlibPINVOKE.UnmanagedString.Create(strName));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public Vector_Value getFieldValues() {
    Vector_Value ret = new Vector_Value(pyxlibPINVOKE.IWritableFeature_SPtr_getFieldValues(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void setFieldValues(Vector_Value vecValues) {
    pyxlibPINVOKE.IWritableFeature_SPtr_setFieldValues(swigCPtr, Vector_Value.getCPtr(vecValues));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void addField(string strName, PYXFieldDefinition.eContextType nContext, PYXValue.eType nType, int nCount, PYXValue value) {
    pyxlibPINVOKE.IWritableFeature_SPtr_addField__SWIG_0(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strName), (int)nContext, (int)nType, nCount, PYXValue.getCPtr(value));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void addField(string strName, PYXFieldDefinition.eContextType nContext, PYXValue.eType nType, int nCount) {
    pyxlibPINVOKE.IWritableFeature_SPtr_addField__SWIG_1(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strName), (int)nContext, (int)nType, nCount);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void addField(string strName, PYXFieldDefinition.eContextType nContext, PYXValue.eType nType) {
    pyxlibPINVOKE.IWritableFeature_SPtr_addField__SWIG_2(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strName), (int)nContext, (int)nType);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public uint AddRef() {
    uint ret = pyxlibPINVOKE.IWritableFeature_SPtr_AddRef(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public uint Release() {
    uint ret = pyxlibPINVOKE.IWritableFeature_SPtr_Release(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
