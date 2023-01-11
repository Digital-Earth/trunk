#region File Header
//
//      FILE:  AutoGenerateBindingsForm.cs.
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
using System.ComponentModel.Design;
using System.Windows.Forms;
using System.Reflection;
using Infralution.Common;
using Infralution.Controls.Design;
using Infralution.Controls.VirtualTree;
using Infralution.Controls.VirtualTree.Design.Properties;
namespace Infralution.Controls.VirtualTree.Design
{
	/// <summary>
	/// Allows the user to add a new cell editor and select the type of control to use.
	/// </summary>
	internal class AutoGenerateBindingsForm : System.Windows.Forms.Form
	{
        private Type _autoGenerateType;
        private IServiceProvider _serviceProvider;
        private System.Windows.Forms.ComboBox _typeCombo;
        private System.Windows.Forms.Label _typeLabel;
        private System.Windows.Forms.Button _okButton;
        private System.Windows.Forms.Button _cancelButton;

        /// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

        /// <summary>
        /// Default constructor
        /// </summary>
		public AutoGenerateBindingsForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

		}

        /// <summary>
        /// Intialize the form passing the service provider
        /// </summary>
        public AutoGenerateBindingsForm(IServiceProvider serviceProvider)
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();
            _serviceProvider = serviceProvider;
        }

        /// <summary>
        /// Return the new cell editor
        /// </summary>
        public Type AutoGenerateType
        {
            get { return _autoGenerateType; }
        }

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
        /// Handle OK button click
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _okButton_Click(object sender, System.EventArgs e)
        {
            _autoGenerateType = ReflectionUtilities.ResolveType(_typeCombo.Text);
            if (_autoGenerateType == null)
            {
                MessageBoxEx.ShowError(Resources.InvalidTypeNameTitle, Resources.InvalidTypeNameMsg);
                return;
            }
            DialogResult = DialogResult.OK;
        }

        /// <summary>
        /// Update the list of possible controls to use
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _typeCombo_DropDown(object sender, System.EventArgs e)
        {
            this.Cursor = Cursors.WaitCursor;
            _typeCombo.DataSource = TypeUtilities.GetCodeModelTypeNames(_serviceProvider);
            this.Cursor = Cursors.Default;       
        }

        #region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(AutoGenerateBindingsForm));
            this._typeCombo = new System.Windows.Forms.ComboBox();
            this._typeLabel = new System.Windows.Forms.Label();
            this._okButton = new System.Windows.Forms.Button();
            this._cancelButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // _typeCombo
            // 
            this._typeCombo.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right)));
            this._typeCombo.Location = new System.Drawing.Point(13, 58);
            this._typeCombo.Name = "_typeCombo";
            this._typeCombo.Size = new System.Drawing.Size(328, 21);
            this._typeCombo.TabIndex = 0;
            this._typeCombo.DropDown += new System.EventHandler(this._typeCombo_DropDown);
            // 
            // _typeLabel
            // 
            this._typeLabel.Location = new System.Drawing.Point(10, 16);
            this._typeLabel.Name = "_typeLabel";
            this._typeLabel.Size = new System.Drawing.Size(334, 34);
            this._typeLabel.TabIndex = 1;
            this._typeLabel.Text = "Enter the fully qualified name of the type to generate columns and bindings for, " +
                "or select from the drop down list.";
            // 
            // _okButton
            // 
            this._okButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this._okButton.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this._okButton.Location = new System.Drawing.Point(274, 93);
            this._okButton.Name = "_okButton";
            this._okButton.Size = new System.Drawing.Size(64, 24);
            this._okButton.TabIndex = 2;
            this._okButton.Text = "OK";
            this._okButton.Click += new System.EventHandler(this._okButton_Click);
            // 
            // _cancelButton
            // 
            this._cancelButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this._cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this._cancelButton.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this._cancelButton.Location = new System.Drawing.Point(210, 93);
            this._cancelButton.Name = "_cancelButton";
            this._cancelButton.Size = new System.Drawing.Size(64, 24);
            this._cancelButton.TabIndex = 3;
            this._cancelButton.Text = "Cancel";
            // 
            // AutoGenerateBindingsForm
            // 
            this.AcceptButton = this._okButton;
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.CancelButton = this._cancelButton;
            this.ClientSize = new System.Drawing.Size(352, 130);
            this.Controls.Add(this._cancelButton);
            this.Controls.Add(this._okButton);
            this.Controls.Add(this._typeLabel);
            this.Controls.Add(this._typeCombo);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(800, 164);
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(360, 164);
            this.Name = "AutoGenerateBindingsForm";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Auto Generate Bindings";
            this.ResumeLayout(false);

        }
		#endregion

	}


}
