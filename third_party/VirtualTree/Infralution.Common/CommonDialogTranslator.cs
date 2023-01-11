#region File Header
//
//      FILE:   CommonDialogTranslator.cs.
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
using System.Windows.Forms;
using System.Resources;
using System.Diagnostics;
using System.Globalization;
namespace Infralution.Common
{
    /// <summary>
    /// Defines a windows hook that translates the text for common dialogs and
    /// message boxes when using a different culture to that of the installed OS.
    /// </summary>
    /// <remarks>
    /// Changing the <see cref="CultureInfo.CurrentUICulture"/> for your .NET application
    /// does not change the text displayed for Message Box buttons and other common dialogs 
    /// such as the OpenFileDialog.   These are displayed in the language specified by
    /// the <see cref="CultureInfo.InstalledUICulture"/>.   This class provides a 
    /// mechanism for translating these dialogs in this instance.  You must call 
    /// <see cref="WindowsHook.Install"/> to install the windows hook that performs
    /// the translation.
    /// </remarks>
    /// <seealso cref="CbtHook"/>
    public class CommonDialogTranslator : WindowsHook 
    {

        #region Member Variables

        private ResourceManager _rm;
        private static EnumChildWindowsProc _translateChildWindow;

        #endregion

        #region Public Interface

        /// <summary>
        /// Create a new common dialog translator using the default translations
        /// </summary>
        public CommonDialogTranslator()
            : base(WindowsHookType.CallWndProcret)
        {
            _rm = Properties.Resources.ResourceManager;
            _translateChildWindow = new EnumChildWindowsProc(TranslateChildWindow);
        }

        /// <summary>
        /// Create a new common dialog translator using the given resource manager
        /// to translate the common dialog strings
        /// </summary>
        /// <param name="resourceManager"></param>
        public CommonDialogTranslator(ResourceManager resourceManager)
            : base(WindowsHookType.CallWndProcret)
        {
            if (resourceManager == null) throw new ArgumentNullException("resourceManager");
            _rm = resourceManager;
        }

        #endregion

        #region Local Methods

        [StructLayout(LayoutKind.Sequential)]
        private struct CWPRETSTRUCT
        {
            public IntPtr lResult;
            public IntPtr lParam;
            public IntPtr wParam;
            public uint message;
            public IntPtr hwnd;
        };

        [DllImport("user32.dll")]
        private static extern bool EnumChildWindows(IntPtr hWndParent, EnumChildWindowsProc lpEnumFunc, IntPtr lParam);

        [DllImport("user32.dll")]
        private static extern int GetDlgCtrlID(IntPtr hwndCtl);

        private delegate bool EnumChildWindowsProc(IntPtr hWnd, IntPtr lParam);

        [return: MarshalAs(UnmanagedType.Bool)]
        [DllImport("user32.dll", SetLastError = true)]
        static extern bool PostMessage(IntPtr hWnd, uint Msg, IntPtr wParam, IntPtr lParam);

        private const int WM_INITDIALOG = 0x0110;
        private const int WM_ACTIVATE = 0x0006;
        private const int WM_SHOWWINDOW = 0x0018;
        private const int WM_WINDOWPOSCHANGED = 0x0047;
 
        /// <summary>
        /// Handle the hook
        /// </summary>
        /// <param name="hookCode"></param>
        /// <param name="wParam"></param>
        /// <param name="lParam"></param>
        protected override void OnHookInvoked(int hookCode, IntPtr wParam, IntPtr lParam)
        {
            CWPRETSTRUCT msg = (CWPRETSTRUCT)Marshal.PtrToStructure(lParam, typeof(CWPRETSTRUCT));
            if (msg.message == WM_SHOWWINDOW)
            {
                if (WindowUtilities.IsDialog(msg.hwnd))
                {
                    TranslateDialog(msg.hwnd);
                }
            }
            base.OnHookInvoked(hookCode, wParam, lParam);
        }

        /// <summary>
        /// Translate the given dialog
        /// </summary>
        /// <param name="hwnd">The handle to the dialog</param>
        protected virtual void TranslateDialog(IntPtr hwnd)
        {
            EnumChildWindows(hwnd, _translateChildWindow, IntPtr.Zero);
        }

        /// <summary>
        /// Translate the text for the given child window of the dialog
        /// </summary>
        /// <param name="hwnd"></param>
        /// <param name="lParam"></param>
        /// <returns></returns>
        protected virtual bool TranslateChildWindow(IntPtr hwnd, IntPtr lParam)
        {
            string originalText = WindowUtilities.GetWindowText(hwnd);
            if (!string.IsNullOrEmpty(originalText))
            {
                int id = GetDlgCtrlID(hwnd);
                string resourceKey = string.Format("Dlg0x{0:X3}_{1}", id, StringUtilities.Strip(originalText, "& :-.,"));
                int length = Math.Min(resourceKey.Length, 30);
                resourceKey = resourceKey.Substring(0, length);
                string text = GetDialogText(resourceKey, originalText);
                if (!string.IsNullOrEmpty(text))
                {
                    WindowUtilities.SetWindowText(hwnd, text);
                }
            }
            return true;
        }

        /// <summary>
        /// Return the text to display for the given resource name in the current locale
        /// </summary>
        /// <param name="resourceName">The resource name</param>
        /// <param name="originalText">The original text</param>
        /// <returns>The text to display</returns>
        protected virtual string GetDialogText(string resourceName, string originalText)
        {
            // Debug.WriteLine(resourceName + ": " + originalText);
            return _rm.GetString(resourceName, Thread.CurrentThread.CurrentUICulture);
        }

        #endregion

    }
}
