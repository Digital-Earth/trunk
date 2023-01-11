#region File Header
//
//      FILE:   TreeEditorForm.cs.
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
using System.Windows.Forms.Design;
using System.ComponentModel.Design;
using Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree.Design
{
	/// <summary>
	/// Provides the user interface for design time editing of properties.of a <see cref="VirtualTree"/> 
	/// </summary>
	public class TreeEditorForm : System.Windows.Forms.Form
	{
        /// <summary>
        /// Defines named indexes for the TabPages of the tree editor
        /// </summary>
        public enum Tabs
        {
            /// <summary>
            /// Defines the index for the TabPage used to edit the Columns
            /// </summary>
            Columns = 0,

            /// <summary>
            /// Defines the index for the TabPage used to edit the RowBindings
            /// </summary>
            Bindings = 1,

            /// <summary>
            /// Defines the index for the TabPage used to edit the Editors
            /// </summary>
            Editors = 2
        }

        #region Member Variables

        /// <summary>
        /// Remembers the last tab used when editing in the designer
        /// </summary>
        private static int _currentTabIndex = 0;

        private Infralution.Controls.VirtualTree.VirtualTree _tree;
        private System.Windows.Forms.TabControl _tabControl;
        private System.Windows.Forms.TabPage _bindingsTab;
        private RowBindingsEditorControl _bindingsControl;
        private System.Windows.Forms.TabPage _columnsTab;
        private ColumnsEditorControl _columnsControl;
        private System.Windows.Forms.TabPage _editorsTab;
        private CellEditorsEditorControl _editorsControl;
        private System.ComponentModel.Container components = null;
        private IServiceProvider _serviceProvider;

        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the form.
        /// </summary>
		public TreeEditorForm()
		{
			InitializeComponent();
        }

        /// <summary>
        /// Initialise a new instance of the form at design time using the given designer service provider.
        /// </summary>
        /// <param name="tree">The Virtual Tree to be edited</param>
        /// <param name="provider">Designer services</param>
        public TreeEditorForm(VirtualTree tree, IServiceProvider provider)
        {
            InitializeComponent();
            _tree = tree;
            _serviceProvider = provider;
            _bindingsControl.Initialize(tree);
            _columnsControl.Initialize(tree);
            _editorsControl.Initialize(tree, provider);
            _tabControl.SelectedIndex = _currentTabIndex; 
        }

        /// <summary>
        /// Set/Get the active tab for the editor
        /// </summary>
        public Tabs ActiveTab
        {
            get { return (Tabs)_tabControl.SelectedIndex; }
            set { _tabControl.SelectedIndex = (int)value; }
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Handle user clicking on AutoGenerate
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnAutoGenerate(object sender, EventArgs e)
        {
            if (_tree.DataSource == null)
            {
                AutoGenerateBindingsForm form = new AutoGenerateBindingsForm(_serviceProvider);
                form.ShowDialog();
                if (form.AutoGenerateType != null)
                {
                    _tree.AutoGenerateBindings(form.AutoGenerateType);
                }
            }
            else
            {
                _tree.AutoGenerateBindings();      
            }
            _tree.UpdateRows();
        }

        /// <summary>
        /// Uses the <see cref="IComponentChangeService"/> to notify the <see cref="VirtualTree"/> of changes
        /// to edited properties.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnClosed(EventArgs e)
        {
            base.OnClosed (e);
            _currentTabIndex = _tabControl.SelectedIndex;

            // force the designer to serialize the edited properties after the editor is closed
            // otherwise for some nested properties (eg CellBinding PreviewSize) it decides it won't 
            // serialize when the property is reset
            //
            if (_serviceProvider != null)
            {
                PropertyDescriptorCollection properties = TypeDescriptor.GetProperties(_tree);
                IComponentChangeService changeService;
                changeService = (IComponentChangeService)_serviceProvider.GetService(typeof(IComponentChangeService));
                changeService.OnComponentChanged(_tree, properties["RowBindings"], _tree.RowBindings, _tree.RowBindings);
                changeService.OnComponentChanged(_tree, properties["Columns"], _tree.Columns, _tree.Columns);
                changeService.OnComponentChanged(_tree, properties["Editors"], _tree.Editors, _tree.Editors);
            }
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
 
        #endregion

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(TreeEditorForm));
            this._tabControl = new System.Windows.Forms.TabControl();
            this._columnsTab = new System.Windows.Forms.TabPage();
            this._columnsControl = new Infralution.Controls.VirtualTree.Design.ColumnsEditorControl();
            this._bindingsTab = new System.Windows.Forms.TabPage();
            this._bindingsControl = new Infralution.Controls.VirtualTree.Design.RowBindingsEditorControl();
            this._editorsTab = new System.Windows.Forms.TabPage();
            this._editorsControl = new Infralution.Controls.VirtualTree.Design.CellEditorsEditorControl();
            this._tabControl.SuspendLayout();
            this._columnsTab.SuspendLayout();
            this._bindingsTab.SuspendLayout();
            this._editorsTab.SuspendLayout();
            this.SuspendLayout();
            // 
            // _tabControl
            // 
            this._tabControl.Controls.Add(this._columnsTab);
            this._tabControl.Controls.Add(this._bindingsTab);
            this._tabControl.Controls.Add(this._editorsTab);
            this._tabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this._tabControl.Location = new System.Drawing.Point(0, 0);
            this._tabControl.Name = "_tabControl";
            this._tabControl.SelectedIndex = 0;
            this._tabControl.Size = new System.Drawing.Size(555, 439);
            this._tabControl.TabIndex = 0;
            // 
            // _columnsTab
            // 
            this._columnsTab.Controls.Add(this._columnsControl);
            this._columnsTab.Location = new System.Drawing.Point(4, 22);
            this._columnsTab.Name = "_columnsTab";
            this._columnsTab.Size = new System.Drawing.Size(547, 413);
            this._columnsTab.TabIndex = 1;
            this._columnsTab.Text = "Columns";
            // 
            // _columnsControl
            // 
            this._columnsControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this._columnsControl.DockPadding.All = 5;
            this._columnsControl.Location = new System.Drawing.Point(0, 0);
            this._columnsControl.Name = "_columnsControl";
            this._columnsControl.Size = new System.Drawing.Size(547, 413);
            this._columnsControl.TabIndex = 1;
            this._columnsControl.AutoGenerate += new System.EventHandler(this.OnAutoGenerate);
            // 
            // _bindingsTab
            // 
            this._bindingsTab.Controls.Add(this._bindingsControl);
            this._bindingsTab.Location = new System.Drawing.Point(4, 22);
            this._bindingsTab.Name = "_bindingsTab";
            this._bindingsTab.Size = new System.Drawing.Size(547, 413);
            this._bindingsTab.TabIndex = 0;
            this._bindingsTab.Text = "Data Binding";
            // 
            // _bindingsControl
            // 
            this._bindingsControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this._bindingsControl.DockPadding.All = 5;
            this._bindingsControl.Location = new System.Drawing.Point(0, 0);
            this._bindingsControl.Name = "_bindingsControl";
            this._bindingsControl.Size = new System.Drawing.Size(547, 413);
            this._bindingsControl.TabIndex = 0;
            this._bindingsControl.AutoGenerate += new System.EventHandler(this.OnAutoGenerate);
            // 
            // _editorsTab
            // 
            this._editorsTab.Controls.Add(this._editorsControl);
            this._editorsTab.Location = new System.Drawing.Point(4, 22);
            this._editorsTab.Name = "_editorsTab";
            this._editorsTab.Size = new System.Drawing.Size(547, 413);
            this._editorsTab.TabIndex = 2;
            this._editorsTab.Text = "Editors";
            // 
            // _editorsControl
            // 
            this._editorsControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this._editorsControl.DockPadding.All = 5;
            this._editorsControl.Location = new System.Drawing.Point(0, 0);
            this._editorsControl.Name = "_editorsControl";
            this._editorsControl.Size = new System.Drawing.Size(547, 413);
            this._editorsControl.TabIndex = 0;
            // 
            // TreeEditorForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(555, 439);
            this.Controls.Add(this._tabControl);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(430, 300);
            this.Name = "TreeEditorForm";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Infralution Virtual Tree Editor";
            this._tabControl.ResumeLayout(false);
            this._columnsTab.ResumeLayout(false);
            this._bindingsTab.ResumeLayout(false);
            this._editorsTab.ResumeLayout(false);
            this.ResumeLayout(false);

        }
		#endregion

 
 	}
}
