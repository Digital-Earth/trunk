/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class PYXValueTile : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public PYXValueTile(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(PYXValueTile obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~PYXValueTile() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_PYXValueTile(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public static void test() {
    pyxlibPINVOKE.PYXValueTile_test();
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public static PYXValueTile_SPtr create(SWIGTYPE_p_std__istream arg0) {
    PYXValueTile_SPtr ret = new PYXValueTile_SPtr(pyxlibPINVOKE.PYXValueTile_create__SWIG_0(SWIGTYPE_p_std__istream.getCPtr(arg0)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static PYXValueTile_SPtr create(PYXIcosIndex index, int nRes, PYXTableDefinition_CSPtr spDefn) {
    PYXValueTile_SPtr ret = new PYXValueTile_SPtr(pyxlibPINVOKE.PYXValueTile_create__SWIG_1(PYXIcosIndex.getCPtr(index), nRes, PYXTableDefinition_CSPtr.getCPtr(spDefn)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static PYXValueTile_SPtr create(PYXValueTile source) {
    PYXValueTile_SPtr ret = new PYXValueTile_SPtr(pyxlibPINVOKE.PYXValueTile_create__SWIG_2(PYXValueTile.getCPtr(source)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXValueTile(PYXIcosIndex index, int nRes, PYXTableDefinition_CSPtr spDefn) : this(pyxlibPINVOKE.new_PYXValueTile__SWIG_0(PYXIcosIndex.getCPtr(index), nRes, PYXTableDefinition_CSPtr.getCPtr(spDefn)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public static PYXValueTile_SPtr create(PYXTile tile, PYXTableDefinition_CSPtr spDefn) {
    PYXValueTile_SPtr ret = new PYXValueTile_SPtr(pyxlibPINVOKE.PYXValueTile_create__SWIG_3(PYXTile.getCPtr(tile), PYXTableDefinition_CSPtr.getCPtr(spDefn)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXValueTile(PYXTile tile, PYXTableDefinition_CSPtr spDefn) : this(pyxlibPINVOKE.new_PYXValueTile__SWIG_1(PYXTile.getCPtr(tile), PYXTableDefinition_CSPtr.getCPtr(spDefn)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public static PYXValueTile_SPtr create(PYXTile tile, SWIGTYPE_p_std__vectorTPYXValue__eType_t vecTypes) {
    PYXValueTile_SPtr ret = new PYXValueTile_SPtr(pyxlibPINVOKE.PYXValueTile_create__SWIG_4(PYXTile.getCPtr(tile), SWIGTYPE_p_std__vectorTPYXValue__eType_t.getCPtr(vecTypes)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXValueTile(PYXTile tile, SWIGTYPE_p_std__vectorTPYXValue__eType_t vecTypes) : this(pyxlibPINVOKE.new_PYXValueTile__SWIG_2(PYXTile.getCPtr(tile), SWIGTYPE_p_std__vectorTPYXValue__eType_t.getCPtr(vecTypes)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public static PYXValueTile_SPtr create(PYXTile tile, SWIGTYPE_p_std__vectorTPYXValue__eType_t vecTypes, Vector_Int vecCounts) {
    PYXValueTile_SPtr ret = new PYXValueTile_SPtr(pyxlibPINVOKE.PYXValueTile_create__SWIG_5(PYXTile.getCPtr(tile), SWIGTYPE_p_std__vectorTPYXValue__eType_t.getCPtr(vecTypes), Vector_Int.getCPtr(vecCounts)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXValueTile(PYXTile tile, SWIGTYPE_p_std__vectorTPYXValue__eType_t vecTypes, Vector_Int vecCounts) : this(pyxlibPINVOKE.new_PYXValueTile__SWIG_3(PYXTile.getCPtr(tile), SWIGTYPE_p_std__vectorTPYXValue__eType_t.getCPtr(vecTypes), Vector_Int.getCPtr(vecCounts)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXValueTile(PYXValueTile arg0) : this(pyxlibPINVOKE.new_PYXValueTile__SWIG_4(PYXValueTile.getCPtr(arg0)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXValueTile(PYXValueTile orig, int nFieldIndex) : this(pyxlibPINVOKE.new_PYXValueTile__SWIG_5(PYXValueTile.getCPtr(orig), nFieldIndex), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isValueTileCompatible(PYXTableDefinition_CSPtr spDefn) {
    bool ret = pyxlibPINVOKE.PYXValueTile_isValueTileCompatible(swigCPtr, PYXTableDefinition_CSPtr.getCPtr(spDefn));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXValue getTypeCompatibleValue(int nChannelIndex) {
    PYXValue ret = new PYXValue(pyxlibPINVOKE.PYXValueTile_getTypeCompatibleValue(swigCPtr, nChannelIndex), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void serialize(SWIGTYPE_p_std__ostream arg0) {
    pyxlibPINVOKE.PYXValueTile_serialize(swigCPtr, SWIGTYPE_p_std__ostream.getCPtr(arg0));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void deserialize(SWIGTYPE_p_std__istream arg0) {
    pyxlibPINVOKE.PYXValueTile_deserialize(swigCPtr, SWIGTYPE_p_std__istream.getCPtr(arg0));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public int getHeapBytes() {
    int ret = pyxlibPINVOKE.PYXValueTile_getHeapBytes(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public SWIGTYPE_p_CacheStatus getCacheStatus() {
    IntPtr cPtr = pyxlibPINVOKE.PYXValueTile_getCacheStatus(swigCPtr);
    SWIGTYPE_p_CacheStatus ret = (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_CacheStatus(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getNumberOfDataChannels() {
    int ret = pyxlibPINVOKE.PYXValueTile_getNumberOfDataChannels(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getNumberOfCells() {
    int ret = pyxlibPINVOKE.PYXValueTile_getNumberOfCells(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXValue.eType getDataChannelType(int nChannelIndex) {
    PYXValue.eType ret = (PYXValue.eType)pyxlibPINVOKE.PYXValueTile_getDataChannelType(swigCPtr, nChannelIndex);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getDataChannelCount(int nChannelIndex) {
    int ret = pyxlibPINVOKE.PYXValueTile_getDataChannelCount(swigCPtr, nChannelIndex);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXValue getValue(PYXIcosIndex cellIndex, int nChannelIndex, SWIGTYPE_p_bool pbInitialized) {
    PYXValue ret = new PYXValue(pyxlibPINVOKE.PYXValueTile_getValue__SWIG_0(swigCPtr, PYXIcosIndex.getCPtr(cellIndex), nChannelIndex, SWIGTYPE_p_bool.getCPtr(pbInitialized)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXValue getValue(PYXIcosIndex cellIndex, int nChannelIndex) {
    PYXValue ret = new PYXValue(pyxlibPINVOKE.PYXValueTile_getValue__SWIG_1(swigCPtr, PYXIcosIndex.getCPtr(cellIndex), nChannelIndex), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool getValue(PYXIcosIndex cellIndex, int nChannelIndex, PYXValue pValue) {
    bool ret = pyxlibPINVOKE.PYXValueTile_getValue__SWIG_2(swigCPtr, PYXIcosIndex.getCPtr(cellIndex), nChannelIndex, PYXValue.getCPtr(pValue));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXValue getValue(int nOffset, int nChannelIndex, SWIGTYPE_p_bool pbInitialized) {
    PYXValue ret = new PYXValue(pyxlibPINVOKE.PYXValueTile_getValue__SWIG_3(swigCPtr, nOffset, nChannelIndex, SWIGTYPE_p_bool.getCPtr(pbInitialized)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXValue getValue(int nOffset, int nChannelIndex) {
    PYXValue ret = new PYXValue(pyxlibPINVOKE.PYXValueTile_getValue__SWIG_4(swigCPtr, nOffset, nChannelIndex), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool getValue(int nOffset, int nChannelIndex, PYXValue pValue) {
    bool ret = pyxlibPINVOKE.PYXValueTile_getValue__SWIG_5(swigCPtr, nOffset, nChannelIndex, PYXValue.getCPtr(pValue));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void setValue(PYXIcosIndex cellIndex, int nChannelIndex, PYXValue value) {
    pyxlibPINVOKE.PYXValueTile_setValue__SWIG_0(swigCPtr, PYXIcosIndex.getCPtr(cellIndex), nChannelIndex, PYXValue.getCPtr(value));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void setValue(int nOffset, int nChannelIndex, PYXValue value) {
    pyxlibPINVOKE.PYXValueTile_setValue__SWIG_1(swigCPtr, nOffset, nChannelIndex, PYXValue.getCPtr(value));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isDirty() {
    bool ret = pyxlibPINVOKE.PYXValueTile_isDirty(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isComplete() {
    bool ret = pyxlibPINVOKE.PYXValueTile_isComplete(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void setIsComplete(bool isComplete) {
    pyxlibPINVOKE.PYXValueTile_setIsComplete(swigCPtr, isComplete);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXTile getTile() {
    PYXTile ret = new PYXTile(pyxlibPINVOKE.PYXValueTile_getTile(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void setTile(PYXTile tile) {
    pyxlibPINVOKE.PYXValueTile_setTile(swigCPtr, PYXTile.getCPtr(tile));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXIterator_SPtr getIterator() {
    PYXIterator_SPtr ret = new PYXIterator_SPtr(pyxlibPINVOKE.PYXValueTile_getIterator(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXValueTile_SPtr cloneFieldTile(int nFieldIndex) {
    PYXValueTile_SPtr ret = new PYXValueTile_SPtr(pyxlibPINVOKE.PYXValueTile_cloneFieldTile(swigCPtr, nFieldIndex), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXValueTile_SPtr clone() {
    PYXValueTile_SPtr ret = new PYXValueTile_SPtr(pyxlibPINVOKE.PYXValueTile_clone(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXValueTile_SPtr zoomIn(int nFieldIndex) {
    PYXValueTile_SPtr ret = new PYXValueTile_SPtr(pyxlibPINVOKE.PYXValueTile_zoomIn(swigCPtr, nFieldIndex), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXValueTileStatistics calcStatistics() {
    PYXValueTileStatistics ret = new PYXValueTileStatistics(pyxlibPINVOKE.PYXValueTile_calcStatistics(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static bool isTileFileComplete(string tileFilename) {
    bool ret = pyxlibPINVOKE.PYXValueTile_isTileFileComplete(pyxlibPINVOKE.UnmanagedString.Create(tileFilename));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static PYXValueTile_SPtr createFromFile(string tileFilename) {
    PYXValueTile_SPtr ret = new PYXValueTile_SPtr(pyxlibPINVOKE.PYXValueTile_createFromFile(pyxlibPINVOKE.UnmanagedString.Create(tileFilename)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static PYXValueTile_SPtr createFromBase64String(string base64) {
    PYXValueTile_SPtr ret = new PYXValueTile_SPtr(pyxlibPINVOKE.PYXValueTile_createFromBase64String(pyxlibPINVOKE.UnmanagedString.Create(base64)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public string toBase64String() {
  // Generated from %typemap(csout) string
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.PYXValueTile_toBase64String(swigCPtr));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

}
