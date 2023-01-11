#region File Header
//
//      FILE:   ClipControl.cs.
//
//    AUTHOR:   Grant Frisken
//
// COPYRIGHT:   Copyright 2006 
//              Infralution
//              6 Bruce St 
//              Mitcham Australia
//
#endregion
using System;
using System.Windows.Forms;
using System.Drawing;
namespace Infralution.Controls.VirtualTree
{
	/// <summary>
	/// Defines the control used to parent editor controls.
	/// </summary>
	/// <remarks>
    /// This is a transparent control that ensures the child editor controls are
    /// clipped to the scrollable area
	/// </remarks>
    /// <seealso cref = "VirtualTree"/>
    internal class ClipControl : Control
    {
 
        #region Local Methods

        /// <summary>
        /// Set the window region to include visible child controls
        /// </summary>
        private void SetupChildWindowRegion()
        {
            // Set the window region to include the area for
            // each of the visible child controls
            //
            Region region = new Region(new Rectangle(0,0,0,0));
            foreach (Control control in this.Controls)
            {
                if (control.Visible)
                {
                    region.Union(control.Bounds);
                }
            }
            this.Region = region;
        }
 
        /// <summary>
        /// Set the window region to clip all but the child controls
        /// </summary>
        /// <param name="levent"></param>
        protected override void OnLayout(LayoutEventArgs levent)
        {
            base.OnLayout (levent);

            // only set up the child window region once this control
            // is visible (otherwise childControl.Visible will always
            // return false)
            //
            if (this.Visible)
            {
                SetupChildWindowRegion();
            }
        }

        /// <summary>
        /// Call PerformLayout when the control is made visible to ensure
        /// that the window region is set correctly for visible controls 
        /// </summary>
        /// <param name="e"></param>
        protected override void OnVisibleChanged(EventArgs e)
        {
            base.OnVisibleChanged (e);
            if (this.Visible)
            {
                SetupChildWindowRegion();
            }
        }

        /// <summary>
        /// Overridden to prevent painting background
        /// </summary>
        /// <param name="pevent"></param>
        protected override void OnPaintBackground(PaintEventArgs pevent)
        {
        }

        /// <summary>
        /// Shift focus to the parent VirtualTree control
        /// </summary>
        /// <param name="e"></param>
        protected override void OnGotFocus(EventArgs e)
        {
            Parent.Focus();
        }

        #endregion
     }
}
