/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class IRecordCollection_CSPtr : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public IRecordCollection_CSPtr(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(IRecordCollection_CSPtr obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~IRecordCollection_CSPtr() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_IRecordCollection_CSPtr(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }
	
	public System.Collections.Generic.IEnumerable<IRecord_SPtr> GetRecordsEnumerator()
	{
		RecordIterator_SPtr iterator = getIterator();
		if (iterator.isNull())
		{
			yield break;
		}
		while(!iterator.end())
		{
			yield return iterator.getRecord();
			iterator.next();
		}
	}

  public IRecordCollection_CSPtr() : this(pyxlibPINVOKE.new_IRecordCollection_CSPtr__SWIG_0(), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public IRecordCollection_CSPtr(IRecordCollection p, bool add_ref) : this(pyxlibPINVOKE.new_IRecordCollection_CSPtr__SWIG_1(IRecordCollection.getCPtr(p), add_ref), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public IRecordCollection_CSPtr(IRecordCollection p) : this(pyxlibPINVOKE.new_IRecordCollection_CSPtr__SWIG_2(IRecordCollection.getCPtr(p)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public IRecordCollection_CSPtr(IRecordCollection_CSPtr rhs) : this(pyxlibPINVOKE.new_IRecordCollection_CSPtr__SWIG_3(IRecordCollection_CSPtr.getCPtr(rhs)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset() {
    pyxlibPINVOKE.IRecordCollection_CSPtr_reset__SWIG_0(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset(IRecordCollection rhs) {
    pyxlibPINVOKE.IRecordCollection_CSPtr_reset__SWIG_1(swigCPtr, IRecordCollection.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public IRecordCollection get() {
    IntPtr cPtr = pyxlibPINVOKE.IRecordCollection_CSPtr_get(swigCPtr);
    IRecordCollection ret = (cPtr == IntPtr.Zero) ? null : new IRecordCollection(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public IRecordCollection __ref__() {
    IRecordCollection ret = new IRecordCollection(pyxlibPINVOKE.IRecordCollection_CSPtr___ref__(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public IRecordCollection __deref__() {
    IntPtr cPtr = pyxlibPINVOKE.IRecordCollection_CSPtr___deref__(swigCPtr);
    IRecordCollection ret = (cPtr == IntPtr.Zero) ? null : new IRecordCollection(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void swap(IRecordCollection_CSPtr rhs) {
    pyxlibPINVOKE.IRecordCollection_CSPtr_swap(swigCPtr, IRecordCollection_CSPtr.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isNull() {
    bool ret = pyxlibPINVOKE.IRecordCollection_CSPtr_isNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isNotNull() {
    bool ret = pyxlibPINVOKE.IRecordCollection_CSPtr_isNotNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public GUID iid {
    get {
      IntPtr cPtr = pyxlibPINVOKE.IRecordCollection_CSPtr_iid_get(swigCPtr);
      GUID ret = (cPtr == IntPtr.Zero) ? null : new GUID(cPtr, false);
      if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public RecordIterator_SPtr getIterator() {
    RecordIterator_SPtr ret = new RecordIterator_SPtr(pyxlibPINVOKE.IRecordCollection_CSPtr_getIterator(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXTableDefinition_SPtr getRecordDefinition() {
    PYXTableDefinition_SPtr ret = new PYXTableDefinition_SPtr(pyxlibPINVOKE.IRecordCollection_CSPtr_getRecordDefinition(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public IRecord_SPtr getRecord(string strRecordID) {
    IRecord_SPtr ret = new IRecord_SPtr(pyxlibPINVOKE.IRecordCollection_CSPtr_getRecord(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strRecordID)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXTableDefinition_CSPtr getDefinition() {
    PYXTableDefinition_CSPtr ret = new PYXTableDefinition_CSPtr(pyxlibPINVOKE.IRecordCollection_CSPtr_getDefinition__SWIG_0(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXValue getFieldValue(int nFieldIndex) {
    PYXValue ret = new PYXValue(pyxlibPINVOKE.IRecordCollection_CSPtr_getFieldValue(swigCPtr, nFieldIndex), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXValue getFieldValueByName(string strName) {
    PYXValue ret = new PYXValue(pyxlibPINVOKE.IRecordCollection_CSPtr_getFieldValueByName(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strName)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public Vector_Value getFieldValues() {
    Vector_Value ret = new Vector_Value(pyxlibPINVOKE.IRecordCollection_CSPtr_getFieldValues(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
