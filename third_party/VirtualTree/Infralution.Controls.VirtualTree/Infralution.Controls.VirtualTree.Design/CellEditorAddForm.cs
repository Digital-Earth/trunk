#region File Header
//
//      FILE:   CellEditorAddForm.cs.
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
using System.Collections.Generic;
using System.ComponentModel;
using System.ComponentModel.Design;
using System.Windows.Forms;
using System.Reflection;
using Infralution.Common;
using Infralution.Controls.VirtualTree;
using Infralution.Controls.Design;
using Infralution.Controls.VirtualTree.Design.Properties;
namespace Infralution.Controls.VirtualTree.Design
{
	/// <summary>
	/// Allows the user to add a new cell editor and select the type of control to use.
	/// </summary>
	internal class CellEditorAddForm : System.Windows.Forms.Form
	{
        private CellEditor _cellEditor;
        private IServiceProvider _serviceProvider;
        private System.Windows.Forms.ComboBox _typeCombo;
        private System.Windows.Forms.Label _typeLabel;
        private System.Windows.Forms.Button _okButton;
        private System.Windows.Forms.Button _cancelButton;

        /// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

        private static LicenseContext _licenseContext = new DesigntimeLicenseContext();
        /// <summary>
        /// Default constructor
        /// </summary>
		public CellEditorAddForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

		}

        /// <summary>
        /// Initialize with the service provider used to get referenced types
        /// </summary>
        public CellEditorAddForm(IServiceProvider serviceProvider)
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
        public CellEditor CellEditor
        {
            get { return _cellEditor; }
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
            try
            { 
                Type controlType = Type.GetType(_typeCombo.Text);
                if (controlType == null)
                {
                    MessageBoxEx.ShowError(Resources.InvalidCellEditorTitle, Resources.InvalidCellEditorNullTypeMsg);
                    return;
                }

                ConstructorInfo constructor = controlType.GetConstructor(Type.EmptyTypes);
                if (constructor == null)
                {   
                    MessageBoxEx.ShowError(Resources.InvalidCellEditorTitle, Resources.InvalidCellEditorNoConstructorMsg);
                    return;
                }

                object controlObject = LicenseManager.CreateWithContext(controlType, _licenseContext);

                if (!(controlObject is Control))
                {
                    MessageBoxEx.ShowError(Resources.InvalidCellEditorTitle, Resources.InvalidCellEditorNotControlMsg);
                    return;
                }
                if (controlObject is Form)
                {
                    MessageBoxEx.ShowError(Resources.InvalidCellEditorTitle, Resources.InvalidCellEditorIsFormMsg);
                    return;
                }

                // set the visibility of the control to false by default
                //
                Control control = controlObject as Control;
                _cellEditor = new CellEditor(control);

                if (controlObject is CheckBox)
                    SetCheckBoxDefaults();
                else if (controlObject is TextBox)
                    SetTextBoxDefaults();

                DialogResult = DialogResult.OK;
            }
            catch (Exception ex)
            {
                MessageBoxEx.ShowError(Resources.CellEditorAddErrorTitle, Resources.CellEditorAddErrorMsg,
                                     ex.GetType().FullName, ex.Message, ex.StackTrace);
            }
        }

        /// <summary>
        /// Set the default settings for check box control editors
        /// </summary>
        private void SetCheckBoxDefaults()
        {
            CheckBox checkBox = (CheckBox) _cellEditor.Control;

            _cellEditor.DisplayMode = CellEditorDisplayMode.Always;
            _cellEditor.UseCellHeight = false;
            _cellEditor.UseCellWidth = false;
            _cellEditor.UsesLeftRightKeys = false;
            _cellEditor.UsesUpDownKeys = false;
            checkBox.Width = 13;
            checkBox.Height = 13;
        }

        /// <summary>
        /// Set the default settings for text box editors
        /// </summary>
        private void SetTextBoxDefaults()
        {
            TextBox textBox = (TextBox)_cellEditor.Control;
            textBox.Multiline = true;   // allows the textbox to size heightwise
        }

        /// <summary>
        /// Update the list of possible controls to use
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _typeCombo_DropDown(object sender, System.EventArgs e)
        {
            ArrayList typeList = new ArrayList();
            Type controlType = typeof(Control);
            Type formType = typeof(Form);
            this.Cursor = Cursors.WaitCursor;
            List<Assembly> assemblies = TypeUtilities.GetReferencedAssemblies(_serviceProvider);
            foreach (Assembly assembly in assemblies) 
            {
                foreach(Type type in assembly.GetTypes()) 
                {
                    if (controlType.IsAssignableFrom(type) && 
                        !formType.IsAssignableFrom(type) && type.IsPublic)
                    {
                        typeList.Add(type.FullName);
                    }
                } 
            }
            typeList.Sort();
            _typeCombo.DataSource = typeList;
            this.Cursor = Cursors.Default;
        
        }

        #region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(CellEditorAddForm));
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
            this._typeCombo.Text = "Infralution.Controls.UniversalEditBox";
            this._typeCombo.DropDown += new System.EventHandler(this._typeCombo_DropDown);
            // 
            // _typeLabel
            // 
            this._typeLabel.Location = new System.Drawing.Point(10, 16);
            this._typeLabel.Name = "_typeLabel";
            this._typeLabel.Size = new System.Drawing.Size(334, 34);
            this._typeLabel.TabIndex = 1;
            this._typeLabel.Text = "Enter the fully qualified type name of the control you wish to use as an editor, " +
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
            // CellEditorAddForm
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
            this.Name = "CellEditorAddForm";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "New Cell Editor";
            this.ResumeLayout(false);

        }
		#endregion

	}


}
