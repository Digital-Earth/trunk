/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class PYXFeature : IWritableFeature {
  private HandleRef swigCPtr;

  public PYXFeature(IntPtr cPtr, bool cMemoryOwn) : base(pyxlibPINVOKE.PYXFeatureUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(PYXFeature obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~PYXFeature() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_PYXFeature(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public static GUID clsid {
    get {
      IntPtr cPtr = pyxlibPINVOKE.PYXFeature_clsid_get();
      GUID ret = (cPtr == IntPtr.Zero) ? null : new GUID(cPtr, false);
      if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public static GUID aiid {
    get {
      IntPtr cPtr = pyxlibPINVOKE.PYXFeature_aiid_get();
      GUID ret = (cPtr == IntPtr.Zero) ? null : new GUID(cPtr, false);
      if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public static int niid {
    get {
      int ret = pyxlibPINVOKE.PYXFeature_niid_get();
      if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public override uint AddRef() {
    uint ret = pyxlibPINVOKE.PYXFeature_AddRef(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override uint Release() {
    uint ret = pyxlibPINVOKE.PYXFeature_Release(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override bool isWritable() {
    bool ret = pyxlibPINVOKE.PYXFeature_isWritable(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override string getID() {
  // Generated from typemap(csout) const string &
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.PYXFeature_getID(swigCPtr));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public override PYXGeometry_CSPtr getGeometry_const() {
    PYXGeometry_CSPtr ret = new PYXGeometry_CSPtr(pyxlibPINVOKE.PYXFeature_getGeometry_const(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override PYXGeometry_SPtr getGeometry() {
    PYXGeometry_SPtr ret = new PYXGeometry_SPtr(pyxlibPINVOKE.PYXFeature_getGeometry(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override string getStyle() {
  // Generated from %typemap(csout) string
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.PYXFeature_getStyle__SWIG_0(swigCPtr));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public override string getStyle(string strStyleToGet) {
  // Generated from %typemap(csout) string
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.PYXFeature_getStyle__SWIG_1(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strStyleToGet)));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public override PYXTableDefinition_CSPtr getDefinition() {
    PYXTableDefinition_CSPtr ret = new PYXTableDefinition_CSPtr(pyxlibPINVOKE.PYXFeature_getDefinition__SWIG_0(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override PYXValue getFieldValue(int nFieldIndex) {
    PYXValue ret = new PYXValue(pyxlibPINVOKE.PYXFeature_getFieldValue(swigCPtr, nFieldIndex), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override void setFieldValue(PYXValue value, int nFieldIndex) {
    pyxlibPINVOKE.PYXFeature_setFieldValue(swigCPtr, PYXValue.getCPtr(value), nFieldIndex);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override PYXValue getFieldValueByName(string strName) {
    PYXValue ret = new PYXValue(pyxlibPINVOKE.PYXFeature_getFieldValueByName(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strName)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override void setFieldValueByName(PYXValue value, string strName) {
    pyxlibPINVOKE.PYXFeature_setFieldValueByName(swigCPtr, PYXValue.getCPtr(value), pyxlibPINVOKE.UnmanagedString.Create(strName));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override Vector_Value getFieldValues() {
    Vector_Value ret = new Vector_Value(pyxlibPINVOKE.PYXFeature_getFieldValues(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public override void setFieldValues(Vector_Value vecValues) {
    pyxlibPINVOKE.PYXFeature_setFieldValues(swigCPtr, Vector_Value.getCPtr(vecValues));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override void addField(string strName, PYXFieldDefinition.eContextType nContext, PYXValue.eType nType, int nCount, PYXValue value) {
    pyxlibPINVOKE.PYXFeature_addField__SWIG_0(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strName), (int)nContext, (int)nType, nCount, PYXValue.getCPtr(value));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override void addField(string strName, PYXFieldDefinition.eContextType nContext, PYXValue.eType nType, int nCount) {
    pyxlibPINVOKE.PYXFeature_addField__SWIG_1(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strName), (int)nContext, (int)nType, nCount);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override void addField(string strName, PYXFieldDefinition.eContextType nContext, PYXValue.eType nType) {
    pyxlibPINVOKE.PYXFeature_addField__SWIG_2(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strName), (int)nContext, (int)nType);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override void setID(string strID) {
    pyxlibPINVOKE.PYXFeature_setID(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strID));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override void setGeometryName(string strName) {
    pyxlibPINVOKE.PYXFeature_setGeometryName(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strName));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override void setGeometry(PYXGeometry_SPtr spGeom) {
    pyxlibPINVOKE.PYXFeature_setGeometry(swigCPtr, PYXGeometry_SPtr.getCPtr(spGeom));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override void setIsWritAble(bool bWritable) {
    pyxlibPINVOKE.PYXFeature_setIsWritAble(swigCPtr, bWritable);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override void setStyle(string style) {
    pyxlibPINVOKE.PYXFeature_setStyle(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(style));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public override void setMetaDataDefinition(PYXTableDefinition_SPtr spDef) {
    pyxlibPINVOKE.PYXFeature_setMetaDataDefinition(swigCPtr, PYXTableDefinition_SPtr.getCPtr(spDef));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXFeature() : this(pyxlibPINVOKE.new_PYXFeature__SWIG_0(), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXFeature(PYXGeometry_SPtr spGeom, string strId, string strStyle, bool isWritable, PYXTableDefinition_SPtr spTableDef, string strGeomName) : this(pyxlibPINVOKE.new_PYXFeature__SWIG_1(PYXGeometry_SPtr.getCPtr(spGeom), pyxlibPINVOKE.UnmanagedString.Create(strId), pyxlibPINVOKE.UnmanagedString.Create(strStyle), isWritable, PYXTableDefinition_SPtr.getCPtr(spTableDef), pyxlibPINVOKE.UnmanagedString.Create(strGeomName)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXFeature(PYXGeometry_SPtr spGeom, string strId, string strStyle, bool isWritable, PYXTableDefinition_SPtr spTableDef) : this(pyxlibPINVOKE.new_PYXFeature__SWIG_2(PYXGeometry_SPtr.getCPtr(spGeom), pyxlibPINVOKE.UnmanagedString.Create(strId), pyxlibPINVOKE.UnmanagedString.Create(strStyle), isWritable, PYXTableDefinition_SPtr.getCPtr(spTableDef)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

}
