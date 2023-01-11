#region File Header
//
//      FILE:   BorderedControl.cs.
//
//    AUTHOR:   Grant Frisken
//
// COPYRIGHT:   Copyright 2004 
//              Infralution
//              6 Bruce St 
//              Mitcham Australia
//
#endregion
using System;
using System.Windows.Forms;
using System.ComponentModel;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Windows.Forms.VisualStyles;
namespace Infralution.Controls
{
	/// <summary>
	/// Defines a light weight control (ie not derived from UserControl) which
	/// uses VisualStyles (if enabled) to draw its border.
	/// </summary>
    [ToolboxItem(false)]
    public class BorderedControl : Control
	{
        #region Member Variables

        /// <summary>
        /// The current border style
        /// </summary>
        private BorderStyle _borderStyle = BorderStyle.Fixed3D;
     
        #endregion

        #region Public Interface

        /// <summary>
        /// Set/Get the border style for this control
        /// </summary>
        [Category("Appearance"),
         DefaultValue(BorderStyle.Fixed3D),
         Description("Set/Get the border style for this control")]
        public virtual BorderStyle BorderStyle
        {
            get { return _borderStyle; }
            set
            {
                if (_borderStyle != value)
                {
                    _borderStyle = value;
                    RecreateHandle();
                }
            }
        }

        #endregion

        #region Local Methods

        [DllImport("user32.dll")]
        private static extern IntPtr GetWindowDC(IntPtr hWnd);

        /// <summary>
        /// Handle a WM_NCPAINT message
        /// </summary>
        protected virtual void WmNonClientPaint(ref Message m)
        {
            if (BorderStyle != BorderStyle.None && Application.RenderWithVisualStyles)
            {
                DrawingUtilities.DrawThemeBorder(this);
            }
            else
            {
                base.WndProc(ref m);
            }
        }

        /// <summary>
        /// Handle painting of non-client area for XP Themes
        /// </summary>
        /// <param name="m"></param>
        protected override void WndProc(ref Message m)
        {
            const int WM_NCPAINT = 0x85;
 
            switch (m.Msg)
            {
                case WM_NCPAINT:
                    WmNonClientPaint(ref m);
                    break;
                default:
                    base.WndProc (ref m);
                    break;
            }
        }

        /// <summary>
        /// Creates the control window with the correct border adornment
        /// </summary>
        protected override CreateParams CreateParams
        {
            get
            {
                CreateParams cp = base.CreateParams;
                switch (_borderStyle)
                {
                    case BorderStyle.Fixed3D:
                        if (Application.RenderWithVisualStyles)
                            cp.Style = cp.Style | 0x800000;
                        else
                            cp.ExStyle = cp.ExStyle | 0x200;
                        break;
                    case BorderStyle.FixedSingle:
                        cp.Style = cp.Style | 0x800000;
                        break;
                    case BorderStyle.None:
                        break;
                }
                return cp;
            }
        }


        #endregion
	}
}
