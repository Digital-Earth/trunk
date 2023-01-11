#region File Header
//
//      FILE:   DropDownForm.cs.
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
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Windows.Forms.VisualStyles;
using System.Runtime.InteropServices;
using System.Threading;
namespace Infralution.Controls
{
	/// <summary>
	/// Defines a form used for displaying drop down lists and controls.
	/// </summary>
	/// <remarks>
	/// The form provides a popup window for displaying a single control (set using the 
	/// <see cref="ContainedControl"/> property).   The form is displayed by the owning 
	/// control using the overloaded <see cref="Show"/> method.  The form is automatically
	/// hidden offscreen when the user clicks outside the form.  The <see cref="Displayed"/> 
	/// property can be used to determine whether the form is currently visible to the user.
	/// </remarks>
	[ToolboxItem(false)]
	public class DropDownForm : System.Windows.Forms.Form
	{
        #region Member Variables

		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

        /// <summary>
        /// Should the form dispose of the contained control.
        /// </summary>
        private bool _manageContainedControlDisposal = true;

        /// <summary>
        /// Is the form currently active.
        /// </summary>
        private bool _displayed = false;

        #endregion

        #region Public Interface

        /// <summary>
        /// Default constructor
        /// </summary>
		public DropDownForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();
		}

        /// <summary>
        /// Positions the form relative to the specified bounds of the given owner
        /// control and displays it.   
        /// </summary>
        /// <remarks>
        /// The position of the form is adjusted to ensure that it is fully onscreen.
        /// </remarks>
        /// <param name="owner">The control that owns the dropdown</param>
        public virtual void Show(Control owner)
        {
            this.RightToLeft = owner.RightToLeft;

            int width = Width;
            int height = Height;

            // account for .NET changing the width/height of borderless forms the first
            // time they are shown
            //
            if (!this.Created)
            {
                Width += 2;
                Height += 2;
            }

            Rectangle bounds = owner.Bounds;
            Point location = owner.Parent.PointToScreen(bounds.Location);
            location.X += bounds.Width - width;
            location.Y += bounds.Height;

            // Get the screen working area
            //
            Rectangle workArea = Screen.FromControl(owner).WorkingArea;
            
            // Adjust the X position to ensure that the drop down is fully on the screen
            //
            location.X = Math.Min(workArea.Right - width, Math.Max(workArea.X, location.X));
                  
            // If the the dropdown would flow off the bottom of the screen then
            // flip it up instead of down
            //
            if (location.Y + Height > workArea.Bottom)
            {
                location.Y = location.Y - bounds.Height - height + 1;
            }           
            Location = location;             
            Show();
            Activate();
        }

        /// <summary>
        /// Show the form and wait until it is deactivated
        /// </summary>
        /// <remarks>
        /// The position of the form is adjusted to ensure that it is fully onscreen.
        /// </remarks>
        /// <param name="owner">The control that owns the dropdown</param>
        public virtual void ShowModal(Control owner)
        {
            Show(owner);
            while (Displayed)
            {
                Application.DoEvents();
                Thread.Sleep(20);
            }
        }

        /// <summary>
        /// Set/Get the control contained within the drop down form
        /// </summary>
        [Browsable(false)]
        public Control ContainedControl
        {
            get 
            {
                if (Controls.Count > 0)
                {
                    return Controls[0];
                }
                return null;
            }
            set 
            {
                Controls.Clear();
                if (value != null)
                {
                    value.Dock = DockStyle.Fill;
                    Controls.Add(value);
                }
            }
        }

        /// <summary>
        /// Is the form currently displayed.  This should be used instead of the Visible
        /// property to determine whether a dropdown is active.
        /// </summary>
        /// <remarks>
        /// The form is "hidden" when the user clicks outside the form (in the OnDeactivate 
        /// method).   Unfortunately hiding the form from within the OnDeactivate method
        /// causes events to be lost.   Instead the form is hidden by moving it offscreen
        /// and setting a flag which indicates that the form is no longer displayed.
        /// </remarks>
        [Browsable(false)]
        public bool Displayed
        {
            get { return _displayed; }
        }

        /// <summary>
        /// Get/Set whether the drop down form should dispose of the
        /// contained control when the form is disposed
        /// </summary>
        /// <remarks>
        /// When using the drop down for UITypeEditors this should be
        /// set to false because the UITypeEditor caches and reuses the 
        /// control
        /// </remarks>
        [Browsable(false)]
        public bool ManageContainedControlDisposal
        {
            get { return _manageContainedControlDisposal; }
            set { _manageContainedControlDisposal = value; }
        }

        #endregion

        #region Local Methods / Overrides

        /// <summary>
        /// Sets the <see cref="Displayed"/> property to true when the control is activated.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnActivated(EventArgs e)
        {
            base.OnActivated (e);
            _displayed = true;
        }

        [DllImport("user32.dll")]
        private static extern IntPtr GetWindowDC(IntPtr hWnd);

        /// <summary>
        /// Handle a WM_NCPAINT message
        /// </summary>
        protected virtual void WmNonClientPaint(ref Message m)
        {
            if (Application.RenderWithVisualStyles)
            {
                DrawingUtilities.DrawThemeBorder(this);
            }
            else
            {
                base.WndProc(ref m);
            }
        }

        /// <summary>
        /// Handles painting of non-client area when using Visual Styles
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
        /// Create the form window with the correct border adornment
        /// </summary>
        protected override CreateParams CreateParams
        {
            get
            {
                CreateParams cp = base.CreateParams;
                cp.Style = cp.Style | 0x800000;
                return cp;
            }
        }
         
        /// <summary>
        /// Hides the DropDownForm if the user clicks outside the form
        /// </summary>
        /// <remarks>
        /// Hiding/Closing the drop down form from within OnDeactivate method causes
        /// the event that caused the deactivation to be lost (ie if the user clicks in
        /// another control that control will not receive the click.   For this reason
        /// we simply move the form offscreen here and allow the client application to 
        /// handle closing the form at a later stage.
        /// </remarks>
        /// <param name="e"></param>
        protected override void OnDeactivate(EventArgs e)
        {
            base.OnDeactivate (e);
            _displayed = false;
            Left = -10000;
        }

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
            _displayed = false;
			if( disposing )
			{
                if (!ManageContainedControlDisposal)
                {
                    Controls.Clear();
                }

				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

        #endregion

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            // 
            // DropDownForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(120, 16);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.MinimumSize = new System.Drawing.Size(1, 1);
            this.Name = "DropDownForm";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.Manual;
            this.Text = "DropDownForm";
            this.TopMost = true;

        }


		#endregion
	}
}
