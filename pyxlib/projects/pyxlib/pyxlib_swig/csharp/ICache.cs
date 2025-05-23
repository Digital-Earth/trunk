/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class ICache : PYXCOM_IUnknown {
  private HandleRef swigCPtr;

  public ICache(IntPtr cPtr, bool cMemoryOwn) : base(pyxlibPINVOKE.ICacheUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(ICache obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~ICache() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_ICache(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public static GUID iid {
    get {
      IntPtr cPtr = pyxlibPINVOKE.ICache_iid_get();
      GUID ret = (cPtr == IntPtr.Zero) ? null : new GUID(cPtr, false);
      if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public virtual void initCacheDir() {
    pyxlibPINVOKE.ICache_initCacheDir(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public virtual void setCacheDir(string strDir) {
    pyxlibPINVOKE.ICache_setCacheDir(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strDir));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public virtual string getCacheDir() {
  // Generated from %typemap(csout) string
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.ICache_getCacheDir(swigCPtr));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public virtual void setCacheTileDepth(int nDepth) {
    pyxlibPINVOKE.ICache_setCacheTileDepth(swigCPtr, nDepth);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public virtual int getCacheTileDepth() {
    int ret = pyxlibPINVOKE.ICache_getCacheTileDepth(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual void setCacheCellResolution(int nCellResolution) {
    pyxlibPINVOKE.ICache_setCacheCellResolution(swigCPtr, nCellResolution);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public virtual int getCacheCellResolution() {
    int ret = pyxlibPINVOKE.ICache_getCacheCellResolution(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual void setCachePersistence(bool bPersistent) {
    pyxlibPINVOKE.ICache_setCachePersistence(swigCPtr, bPersistent);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public virtual bool openReadOnly(string strDir) {
    bool ret = pyxlibPINVOKE.ICache_openReadOnly(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strDir));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual bool openReadWrite(string strDir) {
    bool ret = pyxlibPINVOKE.ICache_openReadWrite__SWIG_0(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strDir));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual void setCacheMaxTileCount(int nMaxTiles) {
    pyxlibPINVOKE.ICache_setCacheMaxTileCount(swigCPtr, nMaxTiles);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public virtual bool openReadWrite(string strDir, PYXTableDefinition defn, Vector_Value vecValues, PYXTableDefinition coverageDefn, int nCellResolution, int nTileResolution) {
    bool ret = pyxlibPINVOKE.ICache_openReadWrite__SWIG_1(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strDir), PYXTableDefinition.getCPtr(defn), Vector_Value.getCPtr(vecValues), PYXTableDefinition.getCPtr(coverageDefn), nCellResolution, nTileResolution);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual bool openReadWrite(string strDir, PYXTableDefinition defn, Vector_Value vecValues, PYXTableDefinition coverageDefn, int nCellResolution) {
    bool ret = pyxlibPINVOKE.ICache_openReadWrite__SWIG_2(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strDir), PYXTableDefinition.getCPtr(defn), Vector_Value.getCPtr(vecValues), PYXTableDefinition.getCPtr(coverageDefn), nCellResolution);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual void close() {
    pyxlibPINVOKE.ICache_close(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public virtual void persistAllTiles() {
    pyxlibPINVOKE.ICache_persistAllTiles(swigCPtr);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public virtual string toFileName(PYXTile tile) {
  // Generated from %typemap(csout) string
  string ret = pyxlibPINVOKE.UnmanagedString.Consume(pyxlibPINVOKE.ICache_toFileName(swigCPtr, PYXTile.getCPtr(tile)));
  
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  return ret;
}

  public virtual void setGreedyCache(bool bGreedy) {
    pyxlibPINVOKE.ICache_setGreedyCache(swigCPtr, bGreedy);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public virtual void addTileFile(string strFileName, PYXTile_SPtr spTile, ProcessDataChangedEvent.ChangeTrigger trigger) {
    pyxlibPINVOKE.ICache_addTileFile(swigCPtr, pyxlibPINVOKE.UnmanagedString.Create(strFileName), PYXTile_SPtr.getCPtr(spTile), (int)trigger);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public virtual bool forceCoverageTile(PYXTile tile) {
    bool ret = pyxlibPINVOKE.ICache_forceCoverageTile(swigCPtr, PYXTile.getCPtr(tile));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual Notifier getNeedATileNotifier() {
    Notifier ret = new Notifier(pyxlibPINVOKE.ICache_getNeedATileNotifier(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual Notifier getCacheChangedNotifier() {
    Notifier ret = new Notifier(pyxlibPINVOKE.ICache_getCacheChangedNotifier(swigCPtr), false);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}
