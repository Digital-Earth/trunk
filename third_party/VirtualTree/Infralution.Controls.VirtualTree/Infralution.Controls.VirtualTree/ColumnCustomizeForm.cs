#region File Header
//
//      FILE:   ColumnCustomizeForm.cs.
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

namespace Infralution.Controls.VirtualTree
{
    /// <summary>
    /// Defines the form used to display non-active <see cref="Column">Columns</see> when the user
    /// customizes a <see cref="VirtualTree"/>.
    /// </summary>
    /// <remarks>
    /// This form can be used or extended by applications to provide specialized handling of customization.
    /// Uses the <see cref="ColumnCustomizeControl"/> to do the bulk of the work
    /// </remarks>
    /// <seealso cref = "VirtualTree"/>
    /// <seealso cref = "Column"/>
    /// <seealso cref = "ColumnCustomizeControl"/>
    public class ColumnCustomizeForm : System.Windows.Forms.Form
	{
        private ColumnCustomizeControl _customizeControl;
        private CultureManager _cultureManager;
        private IContainer components;


		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ColumnCustomizeForm));
            this._customizeControl = new Infralution.Controls.VirtualTree.ColumnCustomizeControl();
            this._cultureManager = new Infralution.Controls.CultureManager(this.components);
            this.SuspendLayout();
            // 
            // _customizeControl
            // 
            this._customizeControl.AllowDrop = true;
            this._customizeControl.BorderStyle = System.Windows.Forms.BorderStyle.None;
            resources.ApplyResources(this._customizeControl, "_customizeControl");
            this._customizeControl.Name = "_customizeControl";
            // 
            // _cultureManager
            // 
            this._cultureManager.ManagedControl = this;
            // 
            // ColumnCustomizeForm
            // 
            resources.ApplyResources(this, "$this");
            this.Controls.Add(this._customizeControl);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "ColumnCustomizeForm";
            this.ShowInTaskbar = false;
            this.ResumeLayout(false);

        }
		#endregion

        #region Public Interface

        /// <summary>
        /// Default Constuctor
        /// </summary>
        public ColumnCustomizeForm()
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();
        }

        /// <summary>
        /// Get/Set the Virtual Tree control that is to have its columns customized
        /// </summary>
        [Category("Data"),
        DefaultValue(null),
        Description("The Virtual Tree control that is to have its columns customized")] 
        public VirtualTree Tree
        {
            get { return _customizeControl.Tree; }
            set 
            { 
                _customizeControl.Tree = value; 
            }
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        protected override void Dispose( bool disposing )
        {
            if( disposing )
            {
                if(components != null)
                {
                    components.Dispose();
                }
            }
            base.Dispose( disposing );
        }

        /// <summary>
        /// Hide the form when the user clicks the close cross (rather than disposing of it)
        /// This allows the form position and size to be retained.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnClosing(CancelEventArgs e)
        {
            base.OnClosing (e);
            e.Cancel = true;
            this.Hide();
        }


        #endregion


	}
}
