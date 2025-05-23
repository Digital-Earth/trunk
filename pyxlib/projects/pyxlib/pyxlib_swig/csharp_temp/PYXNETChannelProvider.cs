/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class PYXNETChannelProvider : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public PYXNETChannelProvider(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(PYXNETChannelProvider obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~PYXNETChannelProvider() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        pyxlibPINVOKE.delete_PYXNETChannelProvider(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public PYXNETChannel_SPtr getOrCreateChannel(ProcRef processProcRef, string code) {
    PYXNETChannel_SPtr ret = new PYXNETChannel_SPtr(pyxlibPINVOKE.PYXNETChannelProvider_getOrCreateChannel(swigCPtr, ProcRef.getCPtr(processProcRef), pyxlibPINVOKE.UnmanagedString.Create(code)), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  protected virtual int createChannel(ProcRef processProcRef, string code) {
    int ret = ((this.GetType() == typeof(PYXNETChannelProvider)) ? pyxlibPINVOKE.PYXNETChannelProvider_createChannel(swigCPtr, ProcRef.getCPtr(processProcRef), pyxlibPINVOKE.UnmanagedString.Create(code)) : pyxlibPINVOKE.PYXNETChannelProvider_createChannelSwigExplicitPYXNETChannelProvider(swigCPtr, ProcRef.getCPtr(processProcRef), pyxlibPINVOKE.UnmanagedString.Create(code)));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  protected virtual void removeChannel(int channelId) {
    if (this.GetType() == typeof(PYXNETChannelProvider)) pyxlibPINVOKE.PYXNETChannelProvider_removeChannel(swigCPtr, channelId); else pyxlibPINVOKE.PYXNETChannelProvider_removeChannelSwigExplicitPYXNETChannelProvider(swigCPtr, channelId);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  protected virtual void requestKey(int channelId, string key) {
    if (this.GetType() == typeof(PYXNETChannelProvider)) pyxlibPINVOKE.PYXNETChannelProvider_requestKey(swigCPtr, channelId, pyxlibPINVOKE.UnmanagedString.Create(key)); else pyxlibPINVOKE.PYXNETChannelProvider_requestKeySwigExplicitPYXNETChannelProvider(swigCPtr, channelId, pyxlibPINVOKE.UnmanagedString.Create(key));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  protected virtual void cancelRequestKey(int channelId, string key) {
    if (this.GetType() == typeof(PYXNETChannelProvider)) pyxlibPINVOKE.PYXNETChannelProvider_cancelRequestKey(swigCPtr, channelId, pyxlibPINVOKE.UnmanagedString.Create(key)); else pyxlibPINVOKE.PYXNETChannelProvider_cancelRequestKeySwigExplicitPYXNETChannelProvider(swigCPtr, channelId, pyxlibPINVOKE.UnmanagedString.Create(key));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  protected virtual void addLocalProvider(int channelId, PYXNETChannelKeyProvider_SPtr provider) {
    if (this.GetType() == typeof(PYXNETChannelProvider)) pyxlibPINVOKE.PYXNETChannelProvider_addLocalProvider(swigCPtr, channelId, PYXNETChannelKeyProvider_SPtr.getCPtr(provider)); else pyxlibPINVOKE.PYXNETChannelProvider_addLocalProviderSwigExplicitPYXNETChannelProvider(swigCPtr, channelId, PYXNETChannelKeyProvider_SPtr.getCPtr(provider));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  protected virtual void publishChannel(int channelId) {
    if (this.GetType() == typeof(PYXNETChannelProvider)) pyxlibPINVOKE.PYXNETChannelProvider_publishChannel(swigCPtr, channelId); else pyxlibPINVOKE.PYXNETChannelProvider_publishChannelSwigExplicitPYXNETChannelProvider(swigCPtr, channelId);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  protected virtual void unpublishChannel(int channelId) {
    if (this.GetType() == typeof(PYXNETChannelProvider)) pyxlibPINVOKE.PYXNETChannelProvider_unpublishChannel(swigCPtr, channelId); else pyxlibPINVOKE.PYXNETChannelProvider_unpublishChannelSwigExplicitPYXNETChannelProvider(swigCPtr, channelId);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void keyProvidedFailed(int channelId, string key) {
    pyxlibPINVOKE.PYXNETChannelProvider_keyProvidedFailed(swigCPtr, channelId, pyxlibPINVOKE.UnmanagedString.Create(key));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void keyProvided(int channelId, string key, string dataBase64) {
    pyxlibPINVOKE.PYXNETChannelProvider_keyProvided(swigCPtr, channelId, pyxlibPINVOKE.UnmanagedString.Create(key), pyxlibPINVOKE.UnmanagedString.Create(dataBase64));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public virtual int release() {
    int ret = ((this.GetType() == typeof(PYXNETChannelProvider)) ? pyxlibPINVOKE.PYXNETChannelProvider_release(swigCPtr) : pyxlibPINVOKE.PYXNETChannelProvider_releaseSwigExplicitPYXNETChannelProvider(swigCPtr));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual int addRef() {
    int ret = ((this.GetType() == typeof(PYXNETChannelProvider)) ? pyxlibPINVOKE.PYXNETChannelProvider_addRef(swigCPtr) : pyxlibPINVOKE.PYXNETChannelProvider_addRefSwigExplicitPYXNETChannelProvider(swigCPtr));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static PYXNETChannelProvider_SPtr getInstance() {
    PYXNETChannelProvider_SPtr ret = new PYXNETChannelProvider_SPtr(pyxlibPINVOKE.PYXNETChannelProvider_getInstance(), true);
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static void setInstance(PYXNETChannelProvider_SPtr spProvider) {
    pyxlibPINVOKE.PYXNETChannelProvider_setInstance(PYXNETChannelProvider_SPtr.getCPtr(spProvider));
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PYXNETChannelProvider() : this(pyxlibPINVOKE.new_PYXNETChannelProvider(), true) {
    if (pyxlibPINVOKE.SWIGPendingException.Pending) throw pyxlibPINVOKE.SWIGPendingException.Retrieve();
    SwigDirectorConnect();
  }

  private void SwigDirectorConnect() {
    if (SwigDerivedClassHasMethod("createChannel", swigMethodTypes0))
      swigDelegate0 = new SwigDelegatePYXNETChannelProvider_0(SwigDirectorcreateChannel);
    if (SwigDerivedClassHasMethod("removeChannel", swigMethodTypes1))
      swigDelegate1 = new SwigDelegatePYXNETChannelProvider_1(SwigDirectorremoveChannel);
    if (SwigDerivedClassHasMethod("requestKey", swigMethodTypes2))
      swigDelegate2 = new SwigDelegatePYXNETChannelProvider_2(SwigDirectorrequestKey);
    if (SwigDerivedClassHasMethod("cancelRequestKey", swigMethodTypes3))
      swigDelegate3 = new SwigDelegatePYXNETChannelProvider_3(SwigDirectorcancelRequestKey);
    if (SwigDerivedClassHasMethod("addLocalProvider", swigMethodTypes4))
      swigDelegate4 = new SwigDelegatePYXNETChannelProvider_4(SwigDirectoraddLocalProvider);
    if (SwigDerivedClassHasMethod("publishChannel", swigMethodTypes5))
      swigDelegate5 = new SwigDelegatePYXNETChannelProvider_5(SwigDirectorpublishChannel);
    if (SwigDerivedClassHasMethod("unpublishChannel", swigMethodTypes6))
      swigDelegate6 = new SwigDelegatePYXNETChannelProvider_6(SwigDirectorunpublishChannel);
    if (SwigDerivedClassHasMethod("release", swigMethodTypes7))
      swigDelegate7 = new SwigDelegatePYXNETChannelProvider_7(SwigDirectorrelease);
    if (SwigDerivedClassHasMethod("addRef", swigMethodTypes8))
      swigDelegate8 = new SwigDelegatePYXNETChannelProvider_8(SwigDirectoraddRef);
    pyxlibPINVOKE.PYXNETChannelProvider_director_connect(swigCPtr, swigDelegate0, swigDelegate1, swigDelegate2, swigDelegate3, swigDelegate4, swigDelegate5, swigDelegate6, swigDelegate7, swigDelegate8);
  }

  private bool SwigDerivedClassHasMethod(string methodName, Type[] methodTypes) {
    System.Reflection.MethodInfo methodInfo = this.GetType().GetMethod(methodName, System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance, null, methodTypes, null);
    bool hasDerivedMethod = methodInfo.DeclaringType.IsSubclassOf(typeof(PYXNETChannelProvider));
    return hasDerivedMethod;
  }

  private int SwigDirectorcreateChannel(IntPtr processProcRef, IntPtr code) {
    return createChannel(new ProcRef(processProcRef, false), pyxlibPINVOKE.UnmanagedString.Consume(code));
  }

  private void SwigDirectorremoveChannel(int channelId) {
    removeChannel(channelId);
  }

  private void SwigDirectorrequestKey(int channelId, IntPtr key) {
    requestKey(channelId, pyxlibPINVOKE.UnmanagedString.Consume(key));
  }

  private void SwigDirectorcancelRequestKey(int channelId, IntPtr key) {
    cancelRequestKey(channelId, pyxlibPINVOKE.UnmanagedString.Consume(key));
  }

  private void SwigDirectoraddLocalProvider(int channelId, IntPtr provider) {
    addLocalProvider(channelId, new PYXNETChannelKeyProvider_SPtr(provider, false));
  }

  private void SwigDirectorpublishChannel(int channelId) {
    publishChannel(channelId);
  }

  private void SwigDirectorunpublishChannel(int channelId) {
    unpublishChannel(channelId);
  }

  private int SwigDirectorrelease() {
    return release();
  }

  private int SwigDirectoraddRef() {
    return addRef();
  }

  public delegate int SwigDelegatePYXNETChannelProvider_0(IntPtr processProcRef, IntPtr code);
  public delegate void SwigDelegatePYXNETChannelProvider_1(int channelId);
  public delegate void SwigDelegatePYXNETChannelProvider_2(int channelId, IntPtr key);
  public delegate void SwigDelegatePYXNETChannelProvider_3(int channelId, IntPtr key);
  public delegate void SwigDelegatePYXNETChannelProvider_4(int channelId, IntPtr provider);
  public delegate void SwigDelegatePYXNETChannelProvider_5(int channelId);
  public delegate void SwigDelegatePYXNETChannelProvider_6(int channelId);
  public delegate int SwigDelegatePYXNETChannelProvider_7();
  public delegate int SwigDelegatePYXNETChannelProvider_8();

  private SwigDelegatePYXNETChannelProvider_0 swigDelegate0;
  private SwigDelegatePYXNETChannelProvider_1 swigDelegate1;
  private SwigDelegatePYXNETChannelProvider_2 swigDelegate2;
  private SwigDelegatePYXNETChannelProvider_3 swigDelegate3;
  private SwigDelegatePYXNETChannelProvider_4 swigDelegate4;
  private SwigDelegatePYXNETChannelProvider_5 swigDelegate5;
  private SwigDelegatePYXNETChannelProvider_6 swigDelegate6;
  private SwigDelegatePYXNETChannelProvider_7 swigDelegate7;
  private SwigDelegatePYXNETChannelProvider_8 swigDelegate8;

  private static Type[] swigMethodTypes0 = new Type[] { typeof(ProcRef), typeof(string) };
  private static Type[] swigMethodTypes1 = new Type[] { typeof(int) };
  private static Type[] swigMethodTypes2 = new Type[] { typeof(int), typeof(string) };
  private static Type[] swigMethodTypes3 = new Type[] { typeof(int), typeof(string) };
  private static Type[] swigMethodTypes4 = new Type[] { typeof(int), typeof(PYXNETChannelKeyProvider_SPtr) };
  private static Type[] swigMethodTypes5 = new Type[] { typeof(int) };
  private static Type[] swigMethodTypes6 = new Type[] { typeof(int) };
  private static Type[] swigMethodTypes7 = new Type[] {  };
  private static Type[] swigMethodTypes8 = new Type[] {  };
}
