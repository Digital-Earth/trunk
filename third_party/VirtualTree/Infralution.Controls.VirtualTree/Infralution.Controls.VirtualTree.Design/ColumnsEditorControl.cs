#region File Header
//
//      FILE:   ColumnsEditorControl.cs.
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
using Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree.Design
{
	/// <summary>
    /// A control for design time editing of the <see cref="Column">Columns</see> of 
    /// a <see cref="VirtualTree"/>
    /// </summary>
	internal class ColumnsEditorControl : System.Windows.Forms.UserControl
	{
        #region Member Variables

        private VirtualTree _tree;
        private ColumnList  _columns;
        private IContainer  _componentContainer;

        private Infralution.Controls.EnhancedPropertyGrid _grid;
        private System.Windows.Forms.Panel _separatorPanel;
        private System.Windows.Forms.Button _downButton;
        private System.Windows.Forms.Button _upButton;
        private System.Windows.Forms.Panel _buttonPanel;
        private System.Windows.Forms.Button _removeButton;
        private System.Windows.Forms.Button _addButton;
        private Infralution.Controls.VirtualListBox _listBox;
        private System.Windows.Forms.Button _generateButton;		
		private System.ComponentModel.Container components = null;

        #endregion

        #region Public Interface 

        /// <summary>
        /// Fired when the user clicks the autogenerate button
        /// </summary>
        public event System.EventHandler AutoGenerate;

        /// <summary>
        /// Initialise a new instance of the ColumnsEditorControl
        /// </summary>
		public ColumnsEditorControl()
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

            // this property is not being serialized correctly and is always set back to true
            _grid.CommandsVisibleIfAvailable = false;
        }

        /// <summary>
        /// Initialize the control.
        /// </summary>
        public void Initialize(VirtualTree tree)
        {
            _tree = tree;
            _grid.Site = _tree.Site;
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Overidden to load the columns for the tree.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad (e);
            if (_tree != null)
            {
                _componentContainer = _tree.Container;
                _columns = _tree.Columns;
                _listBox.DataSource = _columns;
                if (_columns.Count > 0)
                    _listBox.SelectedItem = _columns[0];
            }
        }

        /// <summary>
        /// Update the state of the buttons based on the currently selected item.
        /// </summary>
        protected void UpdateButtonState()
        {
            int index = _listBox.Row(_listBox.SelectedItem);
            int count = _listBox.SelectedItems.Count;
            _removeButton.Enabled = (count >= 0);
            _upButton.Enabled = (index > 0) && (count == 1);
            _downButton.Enabled = (index >= 0)&&(index < (_columns.Count - 1)) & (count == 1);
        }

        /// <summary>
        /// Update the currently selected column in the property editor.
        /// </summary>
        protected void UpdateSelectedColumns()
        {
            _grid.SelectedObjects = _listBox.SelectedItems.GetItems();
            UpdateButtonState();
        }

        /// <summary>
        /// Link the displayed properties to the currently selected column.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnListBoxSelectedItemsChanged(object sender, System.EventArgs e)
        {
            if (_columns != null)
                UpdateSelectedColumns();
        }

        /// <summary>
        /// Refresh the list of columns in case the design name for the binding has changed.
        /// </summary>
        /// <param name="s"></param>
        /// <param name="e"></param>
        private void _grid_PropertyValueChanged(object s, System.Windows.Forms.PropertyValueChangedEventArgs e)
        {
            _listBox.UpdateData();
        }

        /// <summary>
        /// Remove any cell bindings for the given column
        /// </summary>
        /// <param name="column"></param>
        private void RemoveCellBindings(Column column)
        {
            foreach (RowBinding rowBinding in _tree.RowBindings)
            {
                CellBinding cellBinding = rowBinding.CellBindings[column];
                if (cellBinding != null)
                {
                    rowBinding.CellBindings.Remove(cellBinding);
                }
            }
        }

        /// <summary>
        /// Remove the selected column(s).
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _removeButton_Click(object sender, System.EventArgs e)
        {
            object[] removeItems = _listBox.SelectedItems.GetItems();
            int firstIndex = _columns.Count - 1;
            _columns.SuspendChangeNotification();
            foreach (Column column in removeItems)
            {
                int index = _columns.IndexOf(column);
                if (index < firstIndex)
                    firstIndex = index;
                _columns.Remove(column);
                if (_componentContainer != null)
                {
                    _componentContainer.Remove(column);
                }
                RemoveCellBindings(column);
            }
            _columns.ResumeChangeNotification();
            
            if (firstIndex > 0)
                firstIndex--;

            if (_columns.Count > 0)
                _listBox.SelectedItem = _columns[firstIndex];
        }

        /// <summary>
        /// Add a new column
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _addButton_Click(object sender, System.EventArgs e)
        {
            Column column = _tree.CreateColumn();
            if (_componentContainer != null)
                _componentContainer.Add(column as IComponent);
            _columns.Add(column);
            _listBox.SelectedItem = column;
        }

        /// <summary>
        /// Move the selected column toward the top of the list
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _upButton_Click(object sender, System.EventArgs e)
        {
            int index = _columns.IndexOf(_listBox.SelectedItem as Column);
            if (index > 0) 
            {
                Column column = _columns[index];
                _columns.SetIndexOf(column, index - 1);
                UpdateButtonState();
            }
        }

        /// <summary>
        /// Move the selected column toward the bottom of the list
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _downButton_Click(object sender, System.EventArgs e)
        {
            int index = _columns.IndexOf(_listBox.SelectedItem as Column);
            if (index < (_columns.Count - 1)) 
            {
                Column column = _columns[index];
                _columns.SetIndexOf(column, index+1);
                UpdateButtonState();
            }        
        }

        /// <summary>
        /// Auto generate columns and bindings for the current data source
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _generateButton_Click(object sender, System.EventArgs e)
        {
            if (AutoGenerate != null)
            {
                AutoGenerate(this, new EventArgs());
            }
            UpdateSelectedColumns();
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ColumnsEditorControl));
            this._grid = new Infralution.Controls.EnhancedPropertyGrid();
            this._separatorPanel = new System.Windows.Forms.Panel();
            this._downButton = new System.Windows.Forms.Button();
            this._upButton = new System.Windows.Forms.Button();
            this._buttonPanel = new System.Windows.Forms.Panel();
            this._generateButton = new System.Windows.Forms.Button();
            this._removeButton = new System.Windows.Forms.Button();
            this._addButton = new System.Windows.Forms.Button();
            this._listBox = new Infralution.Controls.VirtualListBox();
            this._separatorPanel.SuspendLayout();
            this._buttonPanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this._listBox)).BeginInit();
            this.SuspendLayout();
            // 
            // _grid
            // 
            this._grid.Dock = System.Windows.Forms.DockStyle.Fill;
            this._grid.LineColor = System.Drawing.SystemColors.ScrollBar;
            this._grid.Location = new System.Drawing.Point(237, 5);
            this._grid.Name = "_grid";
            this._grid.Size = new System.Drawing.Size(230, 232);
            this._grid.TabIndex = 11;
            this._grid.PropertyValueChanged += new System.Windows.Forms.PropertyValueChangedEventHandler(this._grid_PropertyValueChanged);
            // 
            // _separatorPanel
            // 
            this._separatorPanel.Controls.Add(this._downButton);
            this._separatorPanel.Controls.Add(this._upButton);
            this._separatorPanel.Dock = System.Windows.Forms.DockStyle.Left;
            this._separatorPanel.Location = new System.Drawing.Point(205, 5);
            this._separatorPanel.Name = "_separatorPanel";
            this._separatorPanel.Size = new System.Drawing.Size(32, 232);
            this._separatorPanel.TabIndex = 13;
            // 
            // _downButton
            // 
            this._downButton.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this._downButton.Image = ((System.Drawing.Image)(resources.GetObject("_downButton.Image")));
            this._downButton.Location = new System.Drawing.Point(4, 115);
            this._downButton.Name = "_downButton";
            this._downButton.Size = new System.Drawing.Size(25, 21);
            this._downButton.TabIndex = 1;
            this._downButton.Click += new System.EventHandler(this._downButton_Click);
            // 
            // _upButton
            // 
            this._upButton.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this._upButton.Image = ((System.Drawing.Image)(resources.GetObject("_upButton.Image")));
            this._upButton.Location = new System.Drawing.Point(4, 94);
            this._upButton.Name = "_upButton";
            this._upButton.Size = new System.Drawing.Size(25, 21);
            this._upButton.TabIndex = 0;
            this._upButton.Click += new System.EventHandler(this._upButton_Click);
            // 
            // _buttonPanel
            // 
            this._buttonPanel.Controls.Add(this._generateButton);
            this._buttonPanel.Controls.Add(this._removeButton);
            this._buttonPanel.Controls.Add(this._addButton);
            this._buttonPanel.Dock = System.Windows.Forms.DockStyle.Bottom;
            this._buttonPanel.Location = new System.Drawing.Point(5, 237);
            this._buttonPanel.Name = "_buttonPanel";
            this._buttonPanel.Padding = new System.Windows.Forms.Padding(0, 5, 0, 0);
            this._buttonPanel.Size = new System.Drawing.Size(462, 30);
            this._buttonPanel.TabIndex = 14;
            // 
            // _generateButton
            // 
            this._generateButton.Dock = System.Windows.Forms.DockStyle.Right;
            this._generateButton.Location = new System.Drawing.Point(362, 5);
            this._generateButton.Name = "_generateButton";
            this._generateButton.Size = new System.Drawing.Size(100, 25);
            this._generateButton.TabIndex = 9;
            this._generateButton.Text = "Auto Generate";
            this._generateButton.Click += new System.EventHandler(this._generateButton_Click);
            // 
            // _removeButton
            // 
            this._removeButton.Dock = System.Windows.Forms.DockStyle.Left;
            this._removeButton.Location = new System.Drawing.Point(100, 5);
            this._removeButton.Name = "_removeButton";
            this._removeButton.Size = new System.Drawing.Size(100, 25);
            this._removeButton.TabIndex = 8;
            this._removeButton.Text = "Remove";
            this._removeButton.Click += new System.EventHandler(this._removeButton_Click);
            // 
            // _addButton
            // 
            this._addButton.Dock = System.Windows.Forms.DockStyle.Left;
            this._addButton.Location = new System.Drawing.Point(0, 5);
            this._addButton.Name = "_addButton";
            this._addButton.Size = new System.Drawing.Size(100, 25);
            this._addButton.TabIndex = 7;
            this._addButton.Text = "Add";
            this._addButton.Click += new System.EventHandler(this._addButton_Click);
            // 
            // _listBox
            // 
            this._listBox.Dock = System.Windows.Forms.DockStyle.Left;
            this._listBox.Location = new System.Drawing.Point(5, 5);
            this._listBox.Name = "_listBox";
            this._listBox.Size = new System.Drawing.Size(200, 232);
            this._listBox.TabIndex = 15;
            this._listBox.UseCurrencyManager = false;
            this._listBox.SelectedItemsChanged += new System.EventHandler(this.OnListBoxSelectedItemsChanged);
            // 
            // ColumnsEditorControl
            // 
            this.Controls.Add(this._grid);
            this.Controls.Add(this._separatorPanel);
            this.Controls.Add(this._listBox);
            this.Controls.Add(this._buttonPanel);
            this.Name = "ColumnsEditorControl";
            this.Padding = new System.Windows.Forms.Padding(5);
            this.Size = new System.Drawing.Size(472, 272);
            this._separatorPanel.ResumeLayout(false);
            this._buttonPanel.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this._listBox)).EndInit();
            this.ResumeLayout(false);

        }
		#endregion

 
	}
}
