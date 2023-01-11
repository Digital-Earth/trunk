#region File Header
//
//      FILE:   WindowUtilities.cs.
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
using System.Text;
namespace Infralution.Common
{
    /// <summary>
    /// Provides utilities for dealing with raw window handles
    /// </summary>
    public static class WindowUtilities
    {

        #region Public Interface

        /// <summary>
        /// Return the class name of the given window 
        /// </summary>
        /// <param name="hwnd">The window handle</param>
        /// <returns>The class name of the window</returns>
        public static string GetClassName(IntPtr hwnd)
        {
            StringBuilder sb = new StringBuilder(80);
            GetClassName(hwnd, sb, 80);
            return sb.ToString();
        }

        /// <summary>
        /// Return true if the given window is a dialog
        /// </summary>
        /// <param name="hwnd">The window handle</param>
        /// <returns>True if the window is a dialog</returns>
        public static bool IsDialog(IntPtr hwnd)
        {
            return GetClassName(hwnd) == "#32770";
        }

        /// <summary>
        /// Return the text of the given window 
        /// </summary>
        /// <param name="hwnd">The window handle</param>
        /// <returns>The text of the window</returns>
        public static string GetWindowText(IntPtr hwnd)
        {
            StringBuilder sb = new StringBuilder(256);
            GetWindowText(hwnd, sb, 256);
            return sb.ToString();
        }

        /// <summary>
        /// Set the text of the given window
        /// </summary>
        /// <param name="hwnd">The window handle</param>
        /// <param name="text">The text for the window</param>
        /// <returns>True if the function succeeds</returns>
        [DllImport("user32.dll")]
        public static extern bool SetWindowText(IntPtr hwnd, string text);

        /// <summary>
        /// Return the window handle for the dialog item with the given ID 
        /// </summary>
        /// <param name="hwnd">The handle to the dialog window</param>
        /// <param name="itemID">The ID of the dialog to get</param>
        /// <returns></returns>
        [DllImport("user32.dll")]
        public static extern IntPtr GetDlgItem(IntPtr hwnd, int itemID);



        #endregion

        #region Local Methods

        [DllImport("user32.dll")]
        private static extern int GetClassName(IntPtr hwnd, StringBuilder lpClassName, int nMaxCount);

        [DllImport("user32.dll")]
        private static extern int GetWindowText(IntPtr hwnd, StringBuilder lpString, int nMaxCount);

        #endregion
    }
}
