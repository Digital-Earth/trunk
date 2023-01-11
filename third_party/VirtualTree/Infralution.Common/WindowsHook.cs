#region File Header
//
//      FILE:   WindowsHook.cs.
//
// COPYRIGHT:   Copyright 2007 
//              Infralution
//              6 Bruce St 
//              Mitcham Australia
//
#endregion
using System;
using System.Collections;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Threading;
namespace Infralution.Common
{
    #region Local Types

    /// <summary>
    /// Defines the arguments passed to WindowHookEvents
    /// </summary>
    public class WindowsHookEventArgs : EventArgs
    {
        /// <summary>
        /// The Windows Hook Code
        /// </summary>
        public int HookCode;	

        /// <summary>
        /// The WPARAM argument
        /// </summary>
        public IntPtr WPARAM;

        /// <summary>
        /// The LPARAM argument 
        /// </summary>
        public IntPtr LPARAM;	

        /// <summary>
        /// Create a new instance of the class
        /// </summary>
        /// <param name="hookCode">The Windows Hook Code</param>
        /// <param name="wParam">The WPARAM argument</param>
        /// <param name="lParam">The LPARAM argument</param>
        public WindowsHookEventArgs(int hookCode, IntPtr wParam, IntPtr lParam)
        {
            HookCode = hookCode;
            WPARAM = wParam;
            LPARAM = lParam;
        }

    }

    /// <summary>
    /// Defines the event signature for WindowHook events
    /// </summary>
    /// <param name="sender"></param>
    /// <param name="e"></param>
    public delegate void WindowsHookEventHandler(object sender, WindowsHookEventArgs e);
 
    /// <summary>
    /// Defines the types of windows events that can be hooked using the <see cref="WindowsHook"/> class
    /// </summary>
    public enum WindowsHookType : int
    {
        /// <summary>WH_JOURNALRECORD</summary>
        JournalRecord = 0,

        /// <summary>WH_JOURNALPLAYBACK</summary>
        JournalPlayback = 1,

        /// <summary>WH_KEYBOARD</summary>
        Keyboard = 2,

        /// <summary>WH_GETMESSAGE</summary>
        GetMessage = 3,

        /// <summary>WH_CALLWNDPROC</summary>
        CallWndProc = 4,

        /// <summary>WH_CBT</summary>
        Cbt = 5,

        /// <summary>WH_SYSMSGFILTER</summary>
        SysMsgFilter = 6,

        /// <summary>WH_MOUSE</summary>
        Mouse = 7,

        /// <summary>WH_HARDWARE</summary>
        Hardware = 8,

        /// <summary>WH_DEBUG</summary>
        Debug = 9,

        /// <summary>WH_SHELL</summary>
        Shell = 10,

        /// <summary>WH_FOREGROUNDIDLE</summary>
        ForegroundIdle = 11,

        /// <summary>WH_CALLWNDPROCRET</summary>
        CallWndProcret = 12,

        /// <summary>WH_KEYBOARD_LL</summary>
        Keyboard_ll = 13,

        /// <summary>WH_MOUSE_LL</summary>
        Mouse_ll = 14
    }

    #endregion
    
    /// <summary>
    /// Provides a mechanism for hooking windows events
    /// </summary>
    public class WindowsHook : IDisposable
    {
        #region Local Variables

        private IntPtr _hhook = IntPtr.Zero;
        private WindowsHookType _hookType;
        private HookProc _hookProc;

        #endregion

        #region Public Interface

        /// <summary>
        /// Fired when an event is hooked
        /// </summary>
        public event WindowsHookEventHandler HookInvoked;

        /// <summary>
        /// Create a new instance of a windows hook
        /// </summary>
        /// <remarks>The hook must be installed before it becomes active</remarks>
        /// <param name="hookType">The type of events to hook</param>
        public WindowsHook(WindowsHookType hookType)
        {
            _hookType = hookType;
            _hookProc = new HookProc(this.CoreHookProc);
        }

        /// <summary>
        /// Ensure the windows hook is uninstalled
        /// </summary>
        ~WindowsHook()
        {
            Uninstall();
        }

        /// <summary>
        /// Install the windows hook (so we start receiving events)
        /// </summary>
        public void Install()
        {
            if (_hhook != IntPtr.Zero) throw new InvalidOperationException("WindowsHook Already Installed");
            _hhook = SetWindowsHookEx(_hookType, _hookProc,
                                      IntPtr.Zero, AppDomain.GetCurrentThreadId());
        }

        /// <summary>
        /// Uninstall the windows hook
        /// </summary>
        public void Uninstall()
        {
            if (_hhook != IntPtr.Zero)
            {
                UnhookWindowsHookEx(_hhook);
                _hhook = IntPtr.Zero;
            }
        }

        #endregion

        #region Local Methods

        private delegate int HookProc(int code, IntPtr wParam, IntPtr lParam);

        [DllImport("user32.dll")]
        private static extern IntPtr SetWindowsHookEx(WindowsHookType code,
            HookProc func,
            IntPtr hInstance,
            int threadID);

        [DllImport("user32.dll")]
        private static extern int UnhookWindowsHookEx(IntPtr hhook);

        [DllImport("user32.dll")]
        private static extern int CallNextHookEx(IntPtr hhook,
            int code, IntPtr wParam, IntPtr lParam);

        /// <summary>
        /// The hook callback function
        /// </summary>
        /// <param name="code"></param>
        /// <param name="wParam"></param>
        /// <param name="lParam"></param>
        /// <returns></returns>
        private int CoreHookProc(int code, IntPtr wParam, IntPtr lParam)
        {
            if (code < 0)
                return CallNextHookEx(_hhook, code, wParam, lParam);

            OnHookInvoked(code, wParam, lParam);

            // Yield to the next hook in the chain
            return CallNextHookEx(_hhook, code, wParam, lParam);
        }

        /// <summary>
        /// Fire the <see cref="HookInvoked"/> event
        /// </summary>
        /// <param name="hookCode">The hook code</param>
        /// <param name="wParam">The WPARAM</param>
        /// <param name="lParam">The LPARAM</param>
        protected virtual void OnHookInvoked(int hookCode, IntPtr wParam, IntPtr lParam)
        {
            if (HookInvoked != null)
            {
                HookInvoked(this, new WindowsHookEventArgs(hookCode, wParam, lParam));
            }
        }

        #endregion
 
        #region IDisposable Members

        /// <summary>
        /// Ensure the hook is uninstalled before disposal
        /// </summary>
        public virtual void  Dispose()
        {
            Uninstall();
            GC.SuppressFinalize(this);
        }

        #endregion
}
}
