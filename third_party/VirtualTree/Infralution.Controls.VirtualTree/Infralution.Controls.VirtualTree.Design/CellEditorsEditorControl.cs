#region File Header
//
//      FILE:   CellEditorsEditorControl.cs.
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
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;
using System.ComponentModel.Design;
using Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree.Design
{
	/// <summary>
    /// A control for design time editing of the CellEditors associated with a Virtual Tree
    /// </summary>
	internal class CellEditorsEditorControl : System.Windows.Forms.UserControl
	{
        #region Member Variables

        private VirtualTree _tree;
        private IServiceProvider _serviceProvider;
        private CellEditorList _editors;
        private IContainer  _componentContainer;

        private Infralution.Controls.EnhancedPropertyGrid _grid;
        private System.Windows.Forms.Panel _listPanel;
        private Infralution.Controls.VirtualListBox _listBox;
        private System.Windows.Forms.Panel _buttonPanel;
        private System.Windows.Forms.Button _removeButton;
        private System.Windows.Forms.Button _addButton;
        private System.Windows.Forms.Panel _spacerPanel;		
		private System.ComponentModel.Container components = null;

        #endregion

        #region Public Interface 

        /// <summary>
        /// Initialise a new instance of the CellEditorsEditorControl
        /// </summary>
		public CellEditorsEditorControl()
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();
     
            // this property is not being serialized correctly and is always set back to true
            _grid.CommandsVisibleIfAvailable = false;
        }

        /// <summary>
        /// Initialize the control.
        /// </summary>
        public void Initialize(VirtualTree tree, IServiceProvider serviceProvider)
        {
            _tree = tree;
            _serviceProvider = serviceProvider;
            _grid.Site = _tree.Site;
            _grid.ShowEventsTab(true, _tree.Site);
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Overidden to load the editors for the tree.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad (e);
            if (_tree != null)
            {
                _componentContainer = _tree.Container;
                _editors = _tree.Editors;
                _listBox.DataSource = _editors;
                if (_editors.Count > 0)
                    _listBox.SelectedItem = _editors[0];
             }
        }

        /// <summary>
        /// Update the currently selected editor in the grid.
        /// </summary>
        protected void UpdateSelectedEditors()
        {
            int count = _listBox.SelectedItems.Count;
            _removeButton.Enabled = (count > 0);
            _grid.SelectedObjects = _listBox.SelectedItems.GetItems();
        }

        /// <summary>
        /// Link the displayed properties to the currently selected editor.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnListBoxSelectedItemsChanged(object sender, System.EventArgs e)
        {
            UpdateSelectedEditors();
        }

        /// <summary>
        /// Refresh the list of editors when properties are changed (the displayed name for the editor
        /// may have changed).
        /// </summary>
        /// <param name="s"></param>
        /// <param name="e"></param>
        private void _grid_PropertyValueChanged(object s, System.Windows.Forms.PropertyValueChangedEventArgs e)
        {
            _listBox.UpdateData();
        }

        /// <summary>
        /// Remove the selected editor.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _removeButton_Click(object sender, System.EventArgs e)
        {
            int firstIndex = _editors.Count - 1;
            _editors.SuspendChangeNotification();
            foreach (CellEditor editor in _listBox.SelectedItems)
            {
                int index = _editors.IndexOf(editor);
                if (index < firstIndex)
                    firstIndex = index;
                _editors.Remove(editor);
                if (_componentContainer != null)
                {
                    _componentContainer.Remove(editor as IComponent);
                    if (editor.Control != null)
                    {
                        _componentContainer.Remove(editor.Control as IComponent);
                    }
                }
            }
            _editors.ResumeChangeNotification();
            
            if (firstIndex > 0)
                firstIndex--;
            if (_editors.Count > 0)
                _listBox.SelectedItem = _editors[firstIndex];
        }

        /// <summary>
        /// Add a new ediotr
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _addButton_Click(object sender, System.EventArgs e)
        {
            CellEditorAddForm form = new CellEditorAddForm(_serviceProvider);

            if (form.ShowDialog() == DialogResult.OK)
            {
                CellEditor editor = form.CellEditor;
                if (_componentContainer != null)
                {
                    _componentContainer.Add(editor as IComponent);
                    _componentContainer.Add(editor.Control as IComponent);
                  }
                _editors.Add(editor);
                _listBox.SelectedItem = editor;
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

		#region Component Designer generated code
		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this._grid = new Infralution.Controls.EnhancedPropertyGrid();
            this._listPanel = new System.Windows.Forms.Panel();
            this._listBox = new Infralution.Controls.VirtualListBox();
            this._buttonPanel = new System.Windows.Forms.Panel();
            this._removeButton = new System.Windows.Forms.Button();
            this._addButton = new System.Windows.Forms.Button();
            this._spacerPanel = new System.Windows.Forms.Panel();
            this._listPanel.SuspendLayout();
            this._buttonPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // _grid
            // 
            this._grid.CommandsVisibleIfAvailable = true;
            this._grid.Dock = System.Windows.Forms.DockStyle.Fill;
            this._grid.LargeButtons = false;
            this._grid.LineColor = System.Drawing.SystemColors.ScrollBar;
            this._grid.Location = new System.Drawing.Point(224, 5);
            this._grid.Name = "_grid";
            this._grid.Size = new System.Drawing.Size(235, 254);
            this._grid.TabIndex = 11;
            this._grid.Text = "PropertyGrid";
            this._grid.ViewBackColor = System.Drawing.SystemColors.Window;
            this._grid.ViewForeColor = System.Drawing.SystemColors.WindowText;
            this._grid.PropertyValueChanged += new System.Windows.Forms.PropertyValueChangedEventHandler(this._grid_PropertyValueChanged);
            // 
            // _listPanel
            // 
            this._listPanel.Controls.Add(this._listBox);
            this._listPanel.Controls.Add(this._buttonPanel);
            this._listPanel.Dock = System.Windows.Forms.DockStyle.Left;
            this._listPanel.Location = new System.Drawing.Point(5, 5);
            this._listPanel.Name = "_listPanel";
            this._listPanel.Size = new System.Drawing.Size(200, 254);
            this._listPanel.TabIndex = 12;
            // 
            // _listBox
            // 
            this._listBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this._listBox.Location = new System.Drawing.Point(0, 0);
            this._listBox.Name = "_listBox";
            this._listBox.Size = new System.Drawing.Size(200, 230);
            this._listBox.TabIndex = 8;
            this._listBox.SelectedItemsChanged += new System.EventHandler(this.OnListBoxSelectedItemsChanged);
            // 
            // _buttonPanel
            // 
            this._buttonPanel.Controls.Add(this._removeButton);
            this._buttonPanel.Controls.Add(this._addButton);
            this._buttonPanel.Dock = System.Windows.Forms.DockStyle.Bottom;
            this._buttonPanel.Location = new System.Drawing.Point(0, 230);
            this._buttonPanel.Name = "_buttonPanel";
            this._buttonPanel.Size = new System.Drawing.Size(200, 24);
            this._buttonPanel.TabIndex = 7;
            // 
            // _removeButton
            // 
            this._removeButton.Dock = System.Windows.Forms.DockStyle.Left;
            this._removeButton.Location = new System.Drawing.Point(100, 0);
            this._removeButton.Name = "_removeButton";
            this._removeButton.Size = new System.Drawing.Size(100, 24);
            this._removeButton.TabIndex = 8;
            this._removeButton.Text = "Remove";
            this._removeButton.Click += new System.EventHandler(this._removeButton_Click);
            // 
            // _addButton
            // 
            this._addButton.Dock = System.Windows.Forms.DockStyle.Left;
            this._addButton.Location = new System.Drawing.Point(0, 0);
            this._addButton.Name = "_addButton";
            this._addButton.Size = new System.Drawing.Size(100, 24);
            this._addButton.TabIndex = 7;
            this._addButton.Text = "Add";
            this._addButton.Click += new System.EventHandler(this._addButton_Click);
            // 
            // _spacerPanel
            // 
            this._spacerPanel.Dock = System.Windows.Forms.DockStyle.Left;
            this._spacerPanel.Location = new System.Drawing.Point(205, 5);
            this._spacerPanel.Name = "_spacerPanel";
            this._spacerPanel.Size = new System.Drawing.Size(19, 254);
            this._spacerPanel.TabIndex = 13;
            // 
            // CellEditorsEditorControl
            // 
            this.Controls.Add(this._grid);
            this.Controls.Add(this._spacerPanel);
            this.Controls.Add(this._listPanel);
            this.DockPadding.All = 5;
            this.Name = "CellEditorsEditorControl";
            this.Size = new System.Drawing.Size(464, 264);
            this._listPanel.ResumeLayout(false);
            this._buttonPanel.ResumeLayout(false);
            this.ResumeLayout(false);

        }
		#endregion


	}
}
