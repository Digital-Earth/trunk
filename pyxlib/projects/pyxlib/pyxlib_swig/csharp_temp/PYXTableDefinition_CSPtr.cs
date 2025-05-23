/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class PYXTableDefinition_CSPtr : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public PYXTableDefinition_CSPtr(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(PYXTableDefinition_CSPtr obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~PYXTableDefinition_CSPtr() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_PYXTableDefinition_CSPtr(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }
	
	public System.Collections.Generic.IEnumerable<PYXFieldDefinition> FieldDefinitions
	{
		get
		{
			int count = getFieldCount();
			for(int i = 0; i < count; ++i)
			{
				yield return getFieldDefinition(i);
			}			
		}
	}

  public PYXTableDefinition_CSPtr() : this(pyxlibPINVOKE.new_PYXTableDefinition_CSPtr__SWIG_0(), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXTableDefinition_CSPtr(PYXTableDefinition p, bool add_ref) : this(pyxlibPINVOKE.new_PYXTableDefinition_CSPtr__SWIG_1(PYXTableDefinition.getCPtr(p), add_ref), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXTableDefinition_CSPtr(PYXTableDefinition p) : this(pyxlibPINVOKE.new_PYXTableDefinition_CSPtr__SWIG_2(PYXTableDefinition.getCPtr(p)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXTableDefinition_CSPtr(PYXTableDefinition_CSPtr rhs) : this(pyxlibPINVOKE.new_PYXTableDefinition_CSPtr__SWIG_3(PYXTableDefinition_CSPtr.getCPtr(rhs)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset() {
    pyxlibPINVOKE.PYXTableDefinition_CSPtr_reset__SWIG_0(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void reset(PYXTableDefinition rhs) {
    pyxlibPINVOKE.PYXTableDefinition_CSPtr_reset__SWIG_1(swigCPtr, PYXTableDefinition.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXTableDefinition get() {
    IntPtr cPtr = pyxlibPINVOKE.PYXTableDefinition_CSPtr_get(swigCPtr);
    PYXTableDefinition ret = (cPtr == IntPtr.Zero) ? null : new PYXTableDefinition(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXTableDefinition __ref__() {
    PYXTableDefinition ret = new PYXTableDefinition(pyxlibPINVOKE.PYXTableDefinition_CSPtr___ref__(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXTableDefinition __deref__() {
    IntPtr cPtr = pyxlibPINVOKE.PYXTableDefinition_CSPtr___deref__(swigCPtr);
    PYXTableDefinition ret = (cPtr == IntPtr.Zero) ? null : new PYXTableDefinition(cPtr, false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void swap(PYXTableDefinition_CSPtr rhs) {
    pyxlibPINVOKE.PYXTableDefinition_CSPtr_swap(swigCPtr, PYXTableDefinition_CSPtr.getCPtr(rhs));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool isNull() {
    bool ret = pyxlibPINVOKE.PYXTableDefinition_CSPtr_isNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool isNotNull() {
    bool ret = pyxlibPINVOKE.PYXTableDefinition_CSPtr_isNotNull(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXTableDefinition_SPtr clone() {
    PYXTableDefinition_SPtr ret = new PYXTableDefinition_SPtr(pyxlibPINVOKE.PYXTableDefinition_CSPtr_clone(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getFieldCount() {
    int ret = pyxlibPINVOKE.PYXTableDefinition_CSPtr_getFieldCount(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXFieldDefinition getFieldDefinition(int nFieldIndex) {
    PYXFieldDefinition ret = new PYXFieldDefinition(pyxlibPINVOKE.PYXTableDefinition_CSPtr_getFieldDefinition(swigCPtr, nFieldIndex), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int getFieldIndex(string strName) {
    int ret = pyxlibPINVOKE.PYXTableDefinition_CSPtr_getFieldIndex(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strName));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public Vector_String getFieldNames() {
    Vector_String ret = new Vector_String(pyxlibPINVOKE.PYXTableDefinition_CSPtr_getFieldNames(swigCPtr), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
