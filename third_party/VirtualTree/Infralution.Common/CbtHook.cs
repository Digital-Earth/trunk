#region File Header
//
//      FILE:   CbtWindowsHook.cs.
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
    /// Defines the possible type of Windows CBT Events
    /// </summary>
	public enum CbtHookAction : int
	{
        /// <summary>HCBT_MOVESIZE</summary>
		MoveSize = 0,

        /// <summary>HCBT_MINMAX</summary>
        MinMax = 1,

        /// <summary>HCBT_QS</summary>
        QS = 2,

        /// <summary>HCBT_CREATEWND</summary>
        CreateWnd = 3,

        /// <summary>HCBT_DESTROYWND</summary>
        DestroyWnd = 4,

        /// <summary>HCBT_ACTIVATE</summary>
        Activate = 5,

        /// <summary>HCBT_CLICKSKIPPED</summary>
		ClickSkipped = 6,

        /// <summary>HCBT_KEYSKIPPED</summary>
		KeySkipped = 7,

        /// <summary>HCBT_SYSCOMMAND</summary>
		SysCommand = 8,

        /// <summary>HCBT_SETFOCUS</summary>
		SetFocus = 9
	}

    /// <summary>
    /// Defines the arguments passed to CbtHookEvents
    /// </summary>
    public class CbtHookEventArgs : EventArgs
    {
        /// <summary>
        /// Win32 handle of the window
        /// </summary>
 		public IntPtr Handle;	

        /// <summary>
        /// The LPARAM argument 
        /// </summary>
        public IntPtr LPARAM;	

        /// <summary>
        /// Create a new instance of the class
        /// </summary>
         /// <param name="handle">The window handle</param>
        /// <param name="lParam">The LPARAM argument</param>
        public CbtHookEventArgs(IntPtr handle, IntPtr lParam)
        {
            Handle = handle;
            LPARAM = lParam;
        }
    }

    /// <summary>
    /// Defines the event signature for CbtHook events
    /// </summary>
    /// <param name="sender"></param>
    /// <param name="e"></param>
    public delegate void CbtHookEventHandler(object sender, CbtHookEventArgs e);
 

    #endregion
    
    /// <summary>
    /// Provides a mechanism for hooking windows CBT events (window creation, activation, destruction etc) 
    /// </summary>
    /// <remarks>The hook must be installed before it becomes active</remarks>
    /// <seealso cref="WindowsHook"/>
    public class CbtHook : WindowsHook
    {

        #region Public Interface

        /// <summary>
        /// Fired when a window is created
        /// </summary>
        public event CbtHookEventHandler CreateWindow;

        /// <summary>
        /// Fired when a window is destroyed
        /// </summary>
        public event CbtHookEventHandler DestroyWindow;

        /// <summary>
        /// Fired when a window is activated
        /// </summary>
        public event CbtHookEventHandler Activate;

        /// <summary>
        /// Fired when a window has focus set to it
        /// </summary>
        public event CbtHookEventHandler SetFocus;

        /// <summary>
        /// Fired when a window is moved/resized
        /// </summary>
        public event CbtHookEventHandler MoveSize;

        /// <summary>
        /// Fired when a window is minimized/maximized
        /// </summary>
        public event CbtHookEventHandler MinMax;

        /// <summary>
        /// Create a new instance of a CBT hook
        /// </summary>
        /// <remarks>The hook must be installed before it becomes active</remarks>
        public CbtHook() 
            : base(WindowsHookType.Cbt)
        {
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Handle hooked events and translate CBT events into calls to raise the
        /// corresponding event on the class
        /// </summary>
        /// <param name="hookCode"></param>
        /// <param name="wParam"></param>
        /// <param name="lParam"></param>
        protected override void OnHookInvoked(int hookCode, IntPtr wParam, IntPtr lParam)
        {
            CbtHookAction code = (CbtHookAction)hookCode;
            switch (code)
            {
                case CbtHookAction.CreateWnd:
                    OnCreateWindow(wParam, lParam);
                    break;
                case CbtHookAction.DestroyWnd:
                    OnDestroyWindow(wParam, lParam);
                    break;
                case CbtHookAction.Activate:
                    OnActivate(wParam, lParam);
                    break;
                case CbtHookAction.SetFocus:
                    OnSetFocus(wParam, lParam);
                    break;
                case CbtHookAction.MoveSize:
                    OnMoveSize(wParam, lParam);
                    break;
                case CbtHookAction.MinMax:
                    OnMinMax(wParam, lParam);
                    break;
            }
        }

        /// <summary>
        /// Fire the <see cref="CreateWindow"/> event
        /// </summary>
        /// <param name="handle">The window handle</param>
        /// <param name="lParam">The LPARAM</param>
        protected virtual void OnCreateWindow(IntPtr handle, IntPtr lParam)
        {
            if (CreateWindow != null)
            {
                CreateWindow(this, new CbtHookEventArgs(handle, lParam));
            }
        }

        /// <summary>
        /// Fire the <see cref="DestroyWindow"/> event
        /// </summary>
        /// <param name="handle">The window handle</param>
        /// <param name="lParam">The LPARAM</param>
        protected virtual void OnDestroyWindow(IntPtr handle, IntPtr lParam)
        {
            if (DestroyWindow != null)
            {
                DestroyWindow(this, new CbtHookEventArgs(handle, lParam));
            }
        }

        /// <summary>
        /// Fire the <see cref="Activate"/> event
        /// </summary>
        /// <param name="handle">The window handle</param>
        /// <param name="lParam">The LPARAM</param>
        protected virtual void OnActivate(IntPtr handle, IntPtr lParam)
        {
            if (Activate != null)
            {
                Activate(this, new CbtHookEventArgs(handle, lParam));
            }
        }

        /// <summary>
        /// Fire the <see cref="SetFocus"/> event
        /// </summary>
        /// <param name="handle">The window handle</param>
        /// <param name="lParam">The LPARAM</param>
        protected virtual void OnSetFocus(IntPtr handle, IntPtr lParam)
        {
            if (SetFocus != null)
            {
                SetFocus(this, new CbtHookEventArgs(handle, lParam));
            }
        }

        /// <summary>
        /// Fire the <see cref="MoveSize"/> event
        /// </summary>
        /// <param name="handle">The window handle</param>
        /// <param name="lParam">The LPARAM</param>
        protected virtual void OnMoveSize(IntPtr handle, IntPtr lParam)
        {
            if (MoveSize != null)
            {
                MoveSize(this, new CbtHookEventArgs(handle, lParam));
            }
        }

        /// <summary>
        /// Fire the <see cref="MinMax"/> event
        /// </summary>
        /// <param name="handle">The window handle</param>
        /// <param name="lParam">The LPARAM</param>
        protected virtual void OnMinMax(IntPtr handle, IntPtr lParam)
        {
            if (MinMax != null)
            {
                MinMax(this, new CbtHookEventArgs(handle, lParam));
            }
        }

        #endregion
 
}
}
