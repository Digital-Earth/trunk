/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class PYXCoord2DInt : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public PYXCoord2DInt(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(PYXCoord2DInt obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~PYXCoord2DInt() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_PYXCoord2DInt(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public PYXCoord2DInt(int x, int y) : this(pyxlibPINVOKE.new_PYXCoord2DInt__SWIG_0(x, y), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXCoord2DInt(int x) : this(pyxlibPINVOKE.new_PYXCoord2DInt__SWIG_1(x), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXCoord2DInt() : this(pyxlibPINVOKE.new_PYXCoord2DInt__SWIG_2(), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXCoord2DInt(PYXCoord2DInt pt) : this(pyxlibPINVOKE.new_PYXCoord2DInt__SWIG_3(PYXCoord2DInt.getCPtr(pt)), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool equal(PYXCoord2DInt pt) {
    bool ret = pyxlibPINVOKE.PYXCoord2DInt_equal__SWIG_0(swigCPtr, PYXCoord2DInt.getCPtr(pt));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool equal(PYXCoord2DInt pt, int precision) {
    bool ret = pyxlibPINVOKE.PYXCoord2DInt_equal__SWIG_1(swigCPtr, PYXCoord2DInt.getCPtr(pt), precision);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int x() {
    int ret = pyxlibPINVOKE.PYXCoord2DInt_x(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void setX(int x) {
    pyxlibPINVOKE.PYXCoord2DInt_setX(swigCPtr, x);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public int y() {
    int ret = pyxlibPINVOKE.PYXCoord2DInt_y(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void setY(int y) {
    pyxlibPINVOKE.PYXCoord2DInt_setY(swigCPtr, y);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public double distance(PYXCoord2DInt pt) {
    double ret = pyxlibPINVOKE.PYXCoord2DInt_distance(swigCPtr, PYXCoord2DInt.getCPtr(pt));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public PYXCoord2DInt midpoint(PYXCoord2DInt pt) {
    PYXCoord2DInt ret = new PYXCoord2DInt(pyxlibPINVOKE.PYXCoord2DInt_midpoint(swigCPtr, PYXCoord2DInt.getCPtr(pt)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void scale(int value) {
    pyxlibPINVOKE.PYXCoord2DInt_scale(swigCPtr, value);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void normalize() {
    pyxlibPINVOKE.PYXCoord2DInt_normalize(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public double dot(PYXCoord2DInt v) {
    double ret = pyxlibPINVOKE.PYXCoord2DInt_dot(swigCPtr, PYXCoord2DInt.getCPtr(v));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
