#region File Header
//
//      FILE:   RowBindingsEditorControl.cs.
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
using System.Reflection;
using Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree.Design
{
	/// <summary>
	/// A control for design time editing of the <see cref="VirtualTree.RowBindings"/> 
	/// </summary>
	internal class RowBindingsEditorControl : System.Windows.Forms.UserControl
	{
        #region Member Variables

        private VirtualTree _tree;
        private IContainer  _componentContainer;
        private RowBindingList _bindings;

        private Infralution.Controls.EnhancedPropertyGrid _grid;
        private System.Windows.Forms.Panel _separatorPanel;
        private System.Windows.Forms.Button _upButton;
        private System.Windows.Forms.Button _downButton;
        private System.Windows.Forms.ContextMenu _addMenu;
        private Infralution.Controls.VirtualTree.Column _bindingColumn;
        private System.Windows.Forms.MenuItem _menuSeparator;
        private System.Windows.Forms.MenuItem _addCellBindingItem;
        private System.Windows.Forms.MenuItem _addCellBindingsItem;
        private System.Windows.Forms.ContextMenu _contextMenu;
        private System.Windows.Forms.MenuItem _separatorItem2;
        public System.Windows.Forms.MenuItem _deleteItem;
        private System.Windows.Forms.MenuItem _addCellBindingItem2;
        private System.Windows.Forms.MenuItem _addCellBindingsItem2;
        private System.Windows.Forms.Panel _buttonPanel;
        private System.Windows.Forms.Button _removeButton;
        private System.Windows.Forms.Button _addButton;
        private Infralution.Controls.VirtualTree.VirtualTree _bindingsTree;
        private System.Windows.Forms.Button _generateButton;
        private System.Windows.Forms.Button _refreshButton;

        #endregion
        private IContainer components;

        #region Public Interface

        /// <summary>
        /// Fired when the user clicks the autogenerate button
        /// </summary>
        public event System.EventHandler AutoGenerate;

        /// <summary>
        /// Initialise a new instance of the control.
        /// </summary>
        public RowBindingsEditorControl()
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
            _grid.ShowEventsTab(true, _tree.Site);
        }

        #endregion

        #region Local Methods


        /// <summary>
        /// Overidden to load the binding list for the tree.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad (e);
            if (_tree != null)
            {
                _grid.Site = _tree.Site;
                _componentContainer = _tree.Container;
                _bindings = _tree.RowBindings;
                InitializeBindingsTree();
                InitializeAddMenu();
                UpdateSelectedBindings();
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

        /// <summary>
        /// Setup the data binding for the tree used to display the row bindings
        /// </summary>
        /// <remarks>
        /// RowBindings are added to the bindings tree manually - to use the editor
        /// for this would require pulling yourself up by your bootstraps
        /// </remarks>
        protected void InitializeBindingsTree()
        {
            ObjectRowBinding rowBinding;
            ObjectCellBinding cellBinding;

            // Add the root binding for the list
            //
            rowBinding = new ObjectRowBinding(typeof(RowBindingList));
            rowBinding.ChildProperty = "this";
            cellBinding = new ObjectCellBinding(_bindingColumn, null);
            cellBinding.Format = "Bindings";
            rowBinding.CellBindings.Add(cellBinding);
            _bindingsTree.RowBindings.Add(rowBinding);

            // add the bindings for RowBindings
            //
            rowBinding = new ObjectRowBinding(typeof(RowBinding));
            rowBinding.ChildProperty = "CellBindings";
            cellBinding = new ObjectCellBinding(_bindingColumn, "DisplayName");
            rowBinding.CellBindings.Add(cellBinding);
            rowBinding.AllowDrag = true;
            rowBinding.AllowDropOnRow = true;
            rowBinding.AllowDropAboveRow = true;
            rowBinding.AllowDropBelowRow = true;

            _bindingsTree.RowBindings.Add(rowBinding);

            // add the bindings for CellBindings
            //
            rowBinding = new ObjectRowBinding(typeof(CellBinding));
            rowBinding.ParentProperty = "RowBinding";
            cellBinding = new ObjectCellBinding(_bindingColumn, "DisplayName");
            rowBinding.CellBindings.Add(cellBinding);
            rowBinding.AllowDrag = true;
            rowBinding.AllowDropAboveRow = true;
            rowBinding.AllowDropBelowRow = true;
            _bindingsTree.RowBindings.Add(rowBinding);

            _bindingsTree.DataSource = _bindings;
        }

        /// <summary>
        /// Add an "Add" menu item for each registered RowBindings type
        /// </summary>
        protected void InitializeAddMenu()
        {
            int index = 0;
            foreach(Type type in RowBinding.RegisteredRowBindingTypes)
            {
                string name = RowBinding.RegisteredRowBindingName(type);
                MenuItem menuItem = new MenuItem(name, new System.EventHandler(this.OnAddRowBindingMenuClick));
                menuItem.Tag = type;
                this._addMenu.MenuItems.Add(index++, menuItem);
            }
        }

        /// <summary>
        /// Update the state of the buttons based on the currently selected item.
        /// </summary>
        protected void UpdateSelectedBindings()
        {
            IList selectedItems = _bindingsTree.SelectedItems;
            _removeButton.Enabled = (selectedItems.Count > 0);
            _deleteItem.Enabled = (selectedItems.Count > 0);
            _addCellBindingItem.Enabled = (selectedItems.Count == 1);
            _addCellBindingsItem.Enabled = (selectedItems.Count == 1);
            _addCellBindingItem2.Enabled = (selectedItems.Count == 1);
            _addCellBindingsItem2.Enabled = (selectedItems.Count == 1);

            object[] selectedObjects = new object[selectedItems.Count];
            selectedItems.CopyTo(selectedObjects, 0);
            _grid.SelectedObjects = selectedObjects;        

            object item = _bindingsTree.SelectedItem;
            if (item is RowBinding && selectedItems.Count == 1)
            {
                int index = _bindings.IndexOf((RowBinding)item);
                _upButton.Enabled = (index > 0);
                _downButton.Enabled = (index >= 0)&&(index < (_bindings.Count - 1));
            }
            else 
            {
                _upButton.Enabled = false;
                _downButton.Enabled = false;
            }
        }

        /// <summary>
        /// Link the displayed properties to the currently selected binding.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _bindingsTree_SelectionChanged(object sender, System.EventArgs e)
        {
            if (_bindings != null)
                UpdateSelectedBindings();
        
        }

        /// <summary>
        /// Refresh the bindings tree row data (the displayed name for the binding
        /// may have changed).
        /// </summary>
        /// <param name="s"></param>
        /// <param name="e"></param>
        private void _grid_PropertyValueChanged(object s, System.Windows.Forms.PropertyValueChangedEventArgs e)
        {
            _bindingsTree.UpdateRowData();
        }

        /// <summary>
        /// Remove the selected binding.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _removeButton_Click(object sender, System.EventArgs e)
        {
            Row[] selectedRows = _bindingsTree.SelectedRows.GetRows();
            foreach (Row row in selectedRows)
            {
                if (row.Item is RowBinding)
                {
                    RowBinding binding = (RowBinding)row.Item;
                    _bindings.Remove(binding); 
                    if (_componentContainer != null)
                        _componentContainer.Remove(binding as IComponent);
                }        
                else if (row.Item is CellBinding)
                {
                    CellBinding binding = (CellBinding)row.Item;
                    binding.RowBinding.CellBindings.Remove(binding);
                }
            }
            _tree.UpdateRows();
        }

        /// <summary>
        /// Move the selected binding toward the top of the list
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _upButton_Click(object sender, System.EventArgs e)
        {
            object item = _bindingsTree.SelectedItem;
            if (item is RowBinding)
            {
                RowBinding binding = (RowBinding)item;
                int index = _bindings.IndexOf(binding);
                _bindings.SetIndexOf(binding, index-1);
                Row row = _bindingsTree.FindRow(binding);
                row.EnsureVisible();
                UpdateSelectedBindings();
            }            
        }

        /// <summary>
        /// Move the selected binding toward the bottom of the list
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _downButton_Click(object sender, System.EventArgs e)
        {
            object item = _bindingsTree.SelectedItem;
            if (item is RowBinding)
            {
                RowBinding binding = (RowBinding)item;
                int index = _bindings.IndexOf(binding);
                _bindings.SetIndexOf(binding, index+1);
                Row row = _bindingsTree.FindRow(binding);
                row.EnsureVisible();
                UpdateSelectedBindings();
            }            
        }

        /// <summary>
        /// Display the context menu to allow adding of new bindings
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _addButton_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            _addMenu.Show(_addButton, new Point(0, _addButton.Height));        
        }

        /// <summary>
        /// Add the given binding to the list of bindings and to the 
        /// component container (if not null)
        /// </summary>
        /// <param name="rowBinding">The row binding to add</param>
        private void AddRowBinding(RowBinding rowBinding)
        {
            // Create cell bindings for each non-context sensitive column
            //
            foreach (Column column in _tree.Columns)
            {
                if (!column.ContextSensitive)
                {
                    CellBinding cellBinding = rowBinding.CreateCellBinding();
                    cellBinding.Column = column;
                    rowBinding.CellBindings.Add(cellBinding);
                }
            }

            _bindings.Add(rowBinding);
            if (_componentContainer != null)
                _componentContainer.Add(rowBinding as IComponent);
           
            _bindingsTree.SuspendLayout();
            _bindingsTree.FindRow(rowBinding).Expanded = true;
            _bindingsTree.SelectedItem = rowBinding;
            _bindingsTree.ResumeLayout();
            _tree.UpdateRows();
        }

        /// <summary>
        /// Add a new RowBinding
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnAddRowBindingMenuClick(object sender, System.EventArgs e)
        {
            MenuItem menuItem = sender as MenuItem;
            Type type = menuItem.Tag as Type;
            ConstructorInfo constructor = type.GetConstructor(Type.EmptyTypes);
            if (constructor != null)
            {
                RowBinding binding = constructor.Invoke(null) as RowBinding;
                AddRowBinding(binding);
            }
        }
 
        /// <summary>
        /// Return the currently selected RowBinding
        /// </summary>
        /// <returns></returns>
        private RowBinding GetCurrentRowBinding()
        {
            object item = _bindingsTree.SelectedItem;
            RowBinding rowBinding = null;
            if (item is RowBinding)
                rowBinding = (RowBinding) item;
            else if (item is CellBinding)
                rowBinding = (item as CellBinding).RowBinding;
            return rowBinding;
        }

        /// <summary>
        /// Add a new cell binding to the currently selected RowBinding
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _addCellBindingItem_Click(object sender, System.EventArgs e)
        {
            RowBinding rowBinding = GetCurrentRowBinding();
            if (rowBinding != null)
            {
                CellBinding cellBinding = rowBinding.CreateCellBinding();
                rowBinding.CellBindings.Add(cellBinding);
                _bindingsTree.SelectedItem = cellBinding;
                _tree.UpdateRows();
            }                   
        }

        /// <summary>
        /// Add cell bindings for each Column to the currently selected RowBinding
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _addCellBindingsItem_Click(object sender, System.EventArgs e)
        {
            RowBinding rowBinding = GetCurrentRowBinding();
            if (rowBinding != null)
            {
                // Create cell bindings for each column that doesn't already have
                // a binding
                //
                foreach (Column column in _tree.Columns)
                {
                    if (rowBinding.CellBindings[column] == null)
                    {
                        CellBinding cellBinding = rowBinding.CreateCellBinding();
                        cellBinding.Column = column;
                        rowBinding.CellBindings.Add(cellBinding);
                    }
                }        
                _tree.UpdateRows();
            }
        }

        /// <summary>
        /// Check the type of the bindings being dropped are compatible
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _bindingsTree_GetAllowedRowDropLocations(object sender, GetAllowedRowDropLocationsEventArgs e)
        {
            // get the default drop locations
            //
            RowBinding binding = _bindingsTree.GetBindingForRow(e.Row);
            e.AllowedDropLocations = binding.GetAllowedDropLocations(e.Row, e.Data); 
    
            Row dropRow = (Row)e.Data.GetData(typeof(Row));
            if (dropRow != null && dropRow.Item is CellBinding)
            {
                CellBinding cellBinding = (CellBinding)dropRow.Item;
                if (e.Row.Item is RowBinding)
                {
                    // dropping a cell binding onto a row binding - check that the row
                    // binding is of the correct type
                    //
                    if (cellBinding.RowBinding.GetType() != e.Row.Item.GetType())
                    {
                        e.AllowedDropLocations = RowDropLocation.None;
                    }
                }
                else if (e.Row.Item is CellBinding)
                {
                    // dropping above or below another cell binding - check that the cellbinding
                    // is of the same type
                    //
                    if (cellBinding.GetType() != e.Row.Item.GetType())
                    {
                        e.AllowedDropLocations = RowDropLocation.None;
                    }
                }
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
        }
  
        /// <summary>
        /// Update the designer display following changes made to the bindings
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _refreshButton_Click(object sender, System.EventArgs e)
        {
            _tree.UpdateRows();
        }

        #endregion

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(RowBindingsEditorControl));
            this._grid = new Infralution.Controls.EnhancedPropertyGrid();
            this._separatorPanel = new System.Windows.Forms.Panel();
            this._downButton = new System.Windows.Forms.Button();
            this._upButton = new System.Windows.Forms.Button();
            this._bindingColumn = new Infralution.Controls.VirtualTree.Column();
            this._contextMenu = new System.Windows.Forms.ContextMenu();
            this._addCellBindingItem2 = new System.Windows.Forms.MenuItem();
            this._addCellBindingsItem2 = new System.Windows.Forms.MenuItem();
            this._separatorItem2 = new System.Windows.Forms.MenuItem();
            this._deleteItem = new System.Windows.Forms.MenuItem();
            this._addMenu = new System.Windows.Forms.ContextMenu();
            this._menuSeparator = new System.Windows.Forms.MenuItem();
            this._addCellBindingItem = new System.Windows.Forms.MenuItem();
            this._addCellBindingsItem = new System.Windows.Forms.MenuItem();
            this._buttonPanel = new System.Windows.Forms.Panel();
            this._refreshButton = new System.Windows.Forms.Button();
            this._generateButton = new System.Windows.Forms.Button();
            this._removeButton = new System.Windows.Forms.Button();
            this._addButton = new System.Windows.Forms.Button();
            this._bindingsTree = new Infralution.Controls.VirtualTree.VirtualTree();
            this._separatorPanel.SuspendLayout();
            this._buttonPanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this._bindingsTree)).BeginInit();
            this.SuspendLayout();
            // 
            // _grid
            // 
            this._grid.Dock = System.Windows.Forms.DockStyle.Fill;
            this._grid.LineColor = System.Drawing.SystemColors.ScrollBar;
            this._grid.Location = new System.Drawing.Point(237, 5);
            this._grid.Name = "_grid";
            this._grid.Size = new System.Drawing.Size(242, 267);
            this._grid.TabIndex = 1;
            this._grid.PropertyValueChanged += new System.Windows.Forms.PropertyValueChangedEventHandler(this._grid_PropertyValueChanged);
            // 
            // _separatorPanel
            // 
            this._separatorPanel.Controls.Add(this._downButton);
            this._separatorPanel.Controls.Add(this._upButton);
            this._separatorPanel.Dock = System.Windows.Forms.DockStyle.Left;
            this._separatorPanel.Location = new System.Drawing.Point(205, 5);
            this._separatorPanel.Name = "_separatorPanel";
            this._separatorPanel.Size = new System.Drawing.Size(32, 267);
            this._separatorPanel.TabIndex = 10;
            // 
            // _downButton
            // 
            this._downButton.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this._downButton.Image = ((System.Drawing.Image)(resources.GetObject("_downButton.Image")));
            this._downButton.Location = new System.Drawing.Point(4, 132);
            this._downButton.Name = "_downButton";
            this._downButton.Size = new System.Drawing.Size(25, 21);
            this._downButton.TabIndex = 1;
            this._downButton.Click += new System.EventHandler(this._downButton_Click);
            // 
            // _upButton
            // 
            this._upButton.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this._upButton.Image = ((System.Drawing.Image)(resources.GetObject("_upButton.Image")));
            this._upButton.Location = new System.Drawing.Point(4, 111);
            this._upButton.Name = "_upButton";
            this._upButton.Size = new System.Drawing.Size(25, 21);
            this._upButton.TabIndex = 0;
            this._upButton.Click += new System.EventHandler(this._upButton_Click);
            // 
            // _bindingColumn
            // 
            this._bindingColumn.AutoSizePolicy = Infralution.Controls.VirtualTree.ColumnAutoSizePolicy.AutoSize;
            this._bindingColumn.Caption = "Bindings";
            this._bindingColumn.Movable = false;
            this._bindingColumn.Name = "_bindingColumn";
            this._bindingColumn.Sortable = false;
            this._bindingColumn.Width = 30;
            // 
            // _contextMenu
            // 
            this._contextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this._addCellBindingItem2,
            this._addCellBindingsItem2,
            this._separatorItem2,
            this._deleteItem});
            // 
            // _addCellBindingItem2
            // 
            this._addCellBindingItem2.Index = 0;
            this._addCellBindingItem2.Text = "Add Cell Binding";
            this._addCellBindingItem2.Click += new System.EventHandler(this._addCellBindingItem_Click);
            // 
            // _addCellBindingsItem2
            // 
            this._addCellBindingsItem2.Index = 1;
            this._addCellBindingsItem2.Text = "Add Cell Bindings (All Columns)";
            this._addCellBindingsItem2.Click += new System.EventHandler(this._addCellBindingsItem_Click);
            // 
            // _separatorItem2
            // 
            this._separatorItem2.Index = 2;
            this._separatorItem2.Text = "-";
            // 
            // _deleteItem
            // 
            this._deleteItem.Index = 3;
            this._deleteItem.Text = "Delete";
            this._deleteItem.Click += new System.EventHandler(this._removeButton_Click);
            // 
            // _addMenu
            // 
            this._addMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this._menuSeparator,
            this._addCellBindingItem,
            this._addCellBindingsItem});
            // 
            // _menuSeparator
            // 
            this._menuSeparator.Index = 0;
            this._menuSeparator.Text = "-";
            // 
            // _addCellBindingItem
            // 
            this._addCellBindingItem.Index = 1;
            this._addCellBindingItem.Text = "Cell Binding";
            this._addCellBindingItem.Click += new System.EventHandler(this._addCellBindingItem_Click);
            // 
            // _addCellBindingsItem
            // 
            this._addCellBindingsItem.Index = 2;
            this._addCellBindingsItem.Text = "Cell Bindings (All Columns)";
            this._addCellBindingsItem.Click += new System.EventHandler(this._addCellBindingsItem_Click);
            // 
            // _buttonPanel
            // 
            this._buttonPanel.Controls.Add(this._refreshButton);
            this._buttonPanel.Controls.Add(this._generateButton);
            this._buttonPanel.Controls.Add(this._removeButton);
            this._buttonPanel.Controls.Add(this._addButton);
            this._buttonPanel.Dock = System.Windows.Forms.DockStyle.Bottom;
            this._buttonPanel.Location = new System.Drawing.Point(5, 272);
            this._buttonPanel.Name = "_buttonPanel";
            this._buttonPanel.Padding = new System.Windows.Forms.Padding(0, 5, 0, 0);
            this._buttonPanel.Size = new System.Drawing.Size(474, 30);
            this._buttonPanel.TabIndex = 11;
            // 
            // _refreshButton
            // 
            this._refreshButton.Dock = System.Windows.Forms.DockStyle.Right;
            this._refreshButton.Location = new System.Drawing.Point(278, 5);
            this._refreshButton.Name = "_refreshButton";
            this._refreshButton.Size = new System.Drawing.Size(96, 25);
            this._refreshButton.TabIndex = 2;
            this._refreshButton.Text = "Update Display";
            this._refreshButton.Click += new System.EventHandler(this._refreshButton_Click);
            // 
            // _generateButton
            // 
            this._generateButton.Dock = System.Windows.Forms.DockStyle.Right;
            this._generateButton.Location = new System.Drawing.Point(374, 5);
            this._generateButton.Name = "_generateButton";
            this._generateButton.Size = new System.Drawing.Size(100, 25);
            this._generateButton.TabIndex = 3;
            this._generateButton.Text = "Auto Generate";
            this._generateButton.Click += new System.EventHandler(this._generateButton_Click);
            // 
            // _removeButton
            // 
            this._removeButton.Dock = System.Windows.Forms.DockStyle.Left;
            this._removeButton.Location = new System.Drawing.Point(100, 5);
            this._removeButton.Name = "_removeButton";
            this._removeButton.Size = new System.Drawing.Size(100, 25);
            this._removeButton.TabIndex = 1;
            this._removeButton.Text = "Remove";
            this._removeButton.Click += new System.EventHandler(this._removeButton_Click);
            // 
            // _addButton
            // 
            this._addButton.Dock = System.Windows.Forms.DockStyle.Left;
            this._addButton.Image = ((System.Drawing.Image)(resources.GetObject("_addButton.Image")));
            this._addButton.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
            this._addButton.Location = new System.Drawing.Point(0, 5);
            this._addButton.Name = "_addButton";
            this._addButton.Size = new System.Drawing.Size(100, 25);
            this._addButton.TabIndex = 0;
            this._addButton.Text = "Add";
            this._addButton.MouseDown += new System.Windows.Forms.MouseEventHandler(this._addButton_MouseDown);
            // 
            // _bindingsTree
            // 
            this._bindingsTree.AllowDrop = true;
            this._bindingsTree.Columns.Add(this._bindingColumn);
            this._bindingsTree.ContextMenu = this._contextMenu;
            this._bindingsTree.Dock = System.Windows.Forms.DockStyle.Left;
            this._bindingsTree.HeaderHeight = 0;
            this._bindingsTree.Location = new System.Drawing.Point(5, 5);
            this._bindingsTree.Name = "_bindingsTree";
            this._bindingsTree.SelectionMode = Infralution.Controls.VirtualTree.SelectionMode.MainCellText;
            this._bindingsTree.ShowColumnHeaders = false;
            this._bindingsTree.ShowRootRow = false;
            this._bindingsTree.Size = new System.Drawing.Size(200, 267);
            this._bindingsTree.TabIndex = 0;
            this._bindingsTree.GetAllowedRowDropLocations += new Infralution.Controls.VirtualTree.GetAllowedRowDropLocationsHandler(this._bindingsTree_GetAllowedRowDropLocations);
            this._bindingsTree.SelectionChanged += new System.EventHandler(this._bindingsTree_SelectionChanged);
            // 
            // RowBindingsEditorControl
            // 
            this.Controls.Add(this._grid);
            this.Controls.Add(this._separatorPanel);
            this.Controls.Add(this._bindingsTree);
            this.Controls.Add(this._buttonPanel);
            this.Name = "RowBindingsEditorControl";
            this.Padding = new System.Windows.Forms.Padding(5);
            this.Size = new System.Drawing.Size(484, 307);
            this._separatorPanel.ResumeLayout(false);
            this._buttonPanel.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this._bindingsTree)).EndInit();
            this.ResumeLayout(false);

        }
        #endregion

    }
}
