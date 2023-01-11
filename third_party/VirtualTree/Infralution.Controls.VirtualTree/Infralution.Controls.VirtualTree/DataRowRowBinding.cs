#region File Header
//
//      FILE:   DataRowRowBinding.cs.
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
using System.Data;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Windows.Forms.Design;
using System.Drawing.Design;
using System.Diagnostics;
using Infralution.Common;
using NS=Infralution.Controls.VirtualTree;
using Infralution.Controls.VirtualTree.Properties;
namespace Infralution.Controls.VirtualTree
{
    /// <summary>
    /// Defines a class for binding information displayed in a <see cref="Row"/> of a 
    /// <see cref="VirtualTree"/> and relationships between Rows to the properties of a <see cref="DataRow"/>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This class is typically defined using the <see cref="VirtualTree"/> visual designer.  It is used
    /// when the <see cref="VirtualTree.DataSource"/> is set to be a <see cref="DataSet"/>, <see cref="DataView"/> or
    /// <see cref="DataTable"/>.   The <see cref="Table"/> property specifies the name of the <see cref="Table"/> 
    /// within the <see cref="VirtualTree.DataSource"/> that this binding applies to.   The user creates 
    /// <see cref="DataRowCellBinding">DataRowCellBindings</see> (using the visual designer) that map the 
    /// <see cref="DataRow"/> items to <see cref="NS.VirtualTree.Columns"/> of the <see cref="VirtualTree"/>. 
    /// </para>
    /// <para>
    /// This binding is always used in conjuction with <see cref="DataViewRowBinding"/> and optionally  
    /// <see cref="DataSetRowBinding"/>.  These define the bindings for the root row of the tree when the 
    /// <see cref="VirtualTree.DataSource"/> is set to be a <see cref="DataView"/>, <see cref="DataTable"/> 
    /// or <see cref="DataSet"/> 
    /// </para>
    /// <para>
    /// The <see cref="ChildRelation"/> property specifies the name of a <see cref="DataRelation"/> associated with
    /// the <see cref="Table"/> that, given a <see cref="DataRow"/>, can generate a child <see cref="DataView"/> containing
    /// the child DataRows.  This is used by the <see cref="VirtualTree"/> to locate the <see cref="Row">Rows</see> to
    /// display is children of a given row.
    /// </para>
    /// <para>
    /// The <see cref="ParentRelation"/> property specifies the name of a <see cref="DataRelation"/> associated with
    /// the <see cref="Table"/> that given a <see cref="DataRow"/> can be used to locate the parent <see cref="DataRow"/>.
    /// </para>
    /// </remarks>
    /// <seealso href="XtraDatasetBinding.html">Data Binding to ADO.NET Datasets and DataViews</seealso>
    /// <seealso cref="VirtualTree"/>
    /// <seealso cref="NS.RowBinding"/>
    /// <seealso cref="NS.DataSetRowBinding"/>
    /// <seealso cref="NS.DataViewRowBinding"/>
    /// <seealso cref="NS.DataRowCellBinding"/>
    /// <seealso cref="NS.DataViewRowBinding"/>
    /// <seealso cref="RowData"/>
    /// <seealso cref="DataRow"/>
    /// <seealso cref="DataRelation"/>
    /// <seealso cref="DataSet"/>
    public class DataRowRowBinding : RowBinding
    {

        #region Member Variables

        private string _table;
        private string _parentRelation;
        private string _childRelation;
        
        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a default object.
        /// </summary>
        public DataRowRowBinding()
        {
        }

        /// <summary>
        ///  Initialise a DataRowRowBinding object.
        /// </summary>
        /// <param name="table">The name of the <see cref="DataTable"/> to bind to</param>
        /// <param name="parentRelation">The name of the parent <see cref="DataRelation"/> (may be null)</param>
        /// <param name="childRelation">The name of the child <see cref="DataRelation"/> (may be null)</param>
        public DataRowRowBinding(string table, 
                                 string parentRelation, 
                                 string childRelation)
        {
            _table = table;
            _parentRelation = parentRelation;
            _childRelation = childRelation;
        }

        /// <summary>
        /// Set/Get the name of the <see cref="DataTable"/> that this binding applies to.
        /// </summary>
        [Category("Data")]
        [Description("The DataTable that this binding applies to")]
        [Editor("Infralution.Controls.VirtualTree.Design.DataTableEditor, " + DesignAssembly.Name, typeof(UITypeEditor))]
        public string Table
        {
            get { return _table; }
            set { _table = StringUtilities.BlankToNull(value); }
        }

        /// <summary>
        /// Set/Get the name of the <see cref="DataRelation"/> within the <see cref="DataSet"/> to be used to find the
        /// parent row for this table.
        /// </summary>
        /// <remarks>
        /// This property allows the <see cref="NS.VirtualTree"/> to locate the row that uniquely corresponds to a
        /// given item in the tree.  It must be set if the binding is to support the simplified methods 
        /// for locating and selecting items in the <see cref="VirtualTree"/>.  This includes 
        /// <see cref="NS.VirtualTree.FindRow(object)"/>, <see cref="NS.VirtualTree.SelectedItem"/> and 
        /// <see cref="NS.VirtualTree.SelectedItems"/>.   If the tree hierarchy is such that an item may
        /// appear multiple times within the tree then these simplified methods cannot be used and this property
        /// does not need to be set.
        /// </remarks>
        [Category("Data")]
        [Description("The DataRelation used to find parents of this item type")]
        [Editor("Infralution.Controls.VirtualTree.Design.DataRelationParentEditor, " + DesignAssembly.Name, typeof(UITypeEditor))]
        public string ParentRelation
        {
            get { return _parentRelation; }
            set { _parentRelation = StringUtilities.BlankToNull(value); }
        }

        /// <summary>
        /// Set/Get the name of the <see cref="DataRelation"/> within the  <see cref="DataSet"/> to be used to find the
        /// child rows for this table.
        /// </summary>
        [Category("Data")]
        [Description("The DataRelation used to find children of this item type")]
        [Editor("Infralution.Controls.VirtualTree.Design.DataRelationChildEditor, " + DesignAssembly.Name, typeof(UITypeEditor))]
        public string ChildRelation
        {
            get { return _childRelation; }
            set { _childRelation = StringUtilities.BlankToNull(value); }
        }

        /// <summary>
        /// Determines whether this binding is applicable for the given item.
        /// </summary>
        /// <param name="item">The item to bind</param>
        /// <returns>True if the item is a DataRow associated with the specified table</returns>
        public override bool BindsTo(object item)
        {
            DataRow dataRow = item as DataRow;
            if (dataRow != null)
            {
                return dataRow.Table.TableName == _table;
            }
            return false;
        }

        /// <summary>
        /// Return the row data for the given DataRow
        /// </summary>
        /// <param name="row">The tree row to get the data for</param>
        /// <param name="rowData">The row data to populate</param>
        public override void GetRowData(Row row, RowData rowData)
        {
            base.GetRowData (row, rowData);
            DataRow dr = (DataRow)row.Item;
            rowData.Error = dr.RowError;
        }

        /// <summary>
        /// Returns the list of children for the given row
        /// </summary>
        /// <remarks>Returns the DataView IList interface wrapped in a DataViewList</remarks>
        /// <param name="row">The row to get the children for</param>
        /// <returns>The list of children for the row</returns>
        public override IList GetChildrenForRow(Row row)
        {
            if (UseGetChildrenEvent) return base.GetChildrenForRow(row);
            if (_childRelation == null) return null;
            DataRow dr = (DataRow)row.Item;
            DataRelation relation = dr.Table.ChildRelations[_childRelation];
            if (relation == null) 
            {
                DisplayErrorMessage(string.Format(Resources.InvalidChildRelationMsg,  Table, _childRelation));
                return null;
            }
            DataRowView drv = GetDataRowView(dr);
            if (drv == null) 
            {
                DisplayErrorMessage(string.Format(Resources.NoDataRowViewMsg,  Table));
                return null;
            }
            DataView childView = drv.CreateChildView(relation);
            SortChildView(childView);
            return new DataViewList(childView);                        
        }

        /// <summary>
        /// Returns the parent object for the given item.
        /// </summary>
        /// <param name="item">The item to get the parent for</param>
        /// <returns>The parent item for the given item</returns>
        public override object GetParentForItem(object item)
        {
            if (UseGetParentEvent) return base.GetParentForItem(item);
            DataRow dr = (DataRow)item;
                 
            // if no parent has been specified for this row then it must belong to the
            // root row 
            //
            if (_parentRelation == null) return Tree.RootItem;

            DataRelation relation = dr.Table.ParentRelations[_parentRelation];
            if (relation == null)
            {
                DisplayErrorMessage(string.Format(Resources.InvalidParentRelationMsg,  Table, _childRelation));
                return null;
            }

            DataRow parentRow = dr.GetParentRow(relation);

            // if there is no parent row then this must be the root item 
            // for a recursive database definition
            //
            return (parentRow == null) ? Tree.RootItem : parentRow;
        }
                    
        /// <summary>
        /// Return the allowed drop locations for the given row and dropped data
        /// </summary>
        /// <param name="row">The row being dropped on</param>
        /// <param name="data">The data being dropped</param>
        /// <returns>The allowed drop locations</returns>
        public override RowDropLocation GetAllowedDropLocations(Row row, IDataObject data)
        {
            if (!AllowDropOnRow) return RowDropLocation.None;

            // check that the item being dropped is a row selection
            //            
            Row[] dropRows = (Row[])data.GetData(typeof(Row[]));
            if (dropRows == null) return RowDropLocation.None;
            
            // check the validity of each row in the selection
            //
            foreach (Row dropRow in dropRows)
            {
                // can't a row drop on itself or it's children
                //
                if (row == dropRow || row.IsDescendant(dropRow))
                    return RowDropLocation.None;

                // can only handle dropping data rows
                //
                if (!(dropRow.Item is DataRow)) return RowDropLocation.None;

                // check that the dropped data row is parented by this
                // data row
                //
                DataRow dropDr = (DataRow)dropRow.Item;
                DataTable dropTable = dropDr.Table;
                DataRow dr = (DataRow)row.Item;
                DataTable table = dr.Table;
                DataRelation childRelation = table.ChildRelations[ChildRelation];
                if (childRelation == null) return RowDropLocation.None;
              
                // check that the dropped row is parented by this type of row
                if (childRelation.ChildTable != dropTable) return RowDropLocation.None;

            }
            return RowDropLocation.OnRow;
        }

        /// <summary>
        /// Return the drop effect for the given row, drop location and dropped data
        /// </summary>
        /// <param name="row">The row being dropped on</param>
        /// <param name="dropLocation">The location to drop</param>
        /// <param name="data">The data being dropped</param>
        /// <returns>The drop effect</returns>
        public override DragDropEffects GetDropEffect(Row row, RowDropLocation dropLocation, IDataObject data)
        {
            if (Control.ModifierKeys == Keys.Control) 
                return DragDropEffects.Copy;   
            else
                return DragDropEffects.Move;
        }

        /// <summary>
        /// Handle dropping of data onto a row of this type
        /// </summary>
        /// <param name="row">The row being dropped on</param>
        /// <param name="dropLocation">The drop location</param>
        /// <param name="data">The data being dropped</param>
        /// <param name="dropEffect">The type of drop operation</param>
        public override void OnDrop(Row row, RowDropLocation dropLocation, IDataObject data, DragDropEffects dropEffect)
        {
            // we can only support dropping onto the row (can't control the order etc)
            //
            if (dropLocation != RowDropLocation.OnRow) return;
            
            // check that the item being dropped is an array of rows
            //            
            Row[] dropRows = (Row[])data.GetData(typeof(Row[]));
            if (dropRows == null) return;
            
            Tree.SuspendLayout();
            Tree.SuspendDataUpdate();
            ArrayList selectDataRows = new ArrayList();

            try
            {

                // ensure the row being dropped on is expanded
                //
                row.Expanded = true;

                // Get the data rows that have been dropped
                //
                ArrayList droppedDataRows = new ArrayList();
                foreach (Row dropRow in dropRows)
                {
                    DataRow dataRow = (dropRow.Item as DataRow);
                    if (dataRow != null)
                    {
                        droppedDataRows.Add(dataRow);
                    }
                }

                // Now move/copy the data rows
                //
                DataRow parentDataRow = (DataRow)row.Item;

                foreach (DataRow dataRow in droppedDataRows)
                {
                    DataRow selectRow = null;
                    if (dropEffect == DragDropEffects.Copy)
                    {
                        selectRow = CopyChildRow(parentDataRow, dataRow);
                    }
                    else if (dropEffect == DragDropEffects.Move)
                    {
                        MoveChildRow(parentDataRow, dataRow);
                        selectRow = dataRow;
                    }
                    selectDataRows.Add(selectRow);
                }

            }
            finally
            {
                Tree.ResumeDataUpdate();
                Tree.ResumeLayout();
            }

            // Set the selection
            //
            Tree.SelectedItems = selectDataRows;

            // Set the first item in the selection as the new focus row - this ensures that
            // it is visible.  Have to do this following resume layout so that EnsureVisible will
            // work properly
            //
            if (Tree.SelectedRows.Count > 0)
                Tree.FocusRow = Tree.SelectedRows[0];
        }

        /// <summary>
        /// Get the text to display in the bindings editor for this binding.
        /// </summary>
        public override string DisplayName
        {
            get
            {
                string table = (_table == null) ? "Unknown" : _table;
                return String.Format("DataRow({0})", table);
            }
        }
        
        /// <summary>
        /// Return the cell binding to use for the given column name
        /// </summary>
        /// <param name="column">The column to get the cell binding for</param>
        /// <returns>The cell binding to use</returns>
        public new DataRowCellBinding CellBinding(Column column)
        {
            return (DataRowCellBinding)base.CellBinding(column);
        }

        /// <summary>
        /// Creates a new <see cref="DataRowCellBinding"/>
        /// </summary>
        /// <returns>A new DataRowCellBinding</returns>
        public override CellBinding CreateCellBinding()
        {
            return new DataRowCellBinding();
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Move the given child row to a new parent 
        /// </summary>
        /// <param name="parentRow">The new parent row</param>
        /// <param name="childRow">The child row to move</param>
        public virtual void MoveChildRow(DataRow parentRow, DataRow childRow)
        {
            DataTable parentTable = parentRow.Table;
            DataTable childTable = childRow.Table;
            DataRelation childRelation = parentTable.ChildRelations[ChildRelation];
            if ((childRelation != null) && (childRelation.ChildTable == childTable))
            {        
                DataColumn parentColumn = childRelation.ParentColumns[0];
                DataColumn childColumn = childRelation.ChildColumns[0];
                childRow.BeginEdit();
                childRow[childColumn] = parentRow[parentColumn];
                childRow.EndEdit();
            }
        }

        /// <summary>
        /// Copy the given child row (and all its descendants) to a new parent 
        /// </summary>
        /// <param name="parentRow">The new parent row</param>
        /// <param name="childRow">The child row to move</param>
        /// <returns>The copied data row</returns>
        public virtual DataRow CopyChildRow(DataRow parentRow, DataRow childRow)
        {
            DataRow copyRow = null;
            DataTable childTable = childRow.Table;
            DataTable parentTable = parentRow.Table;
            DataRelation childRelation = parentTable.ChildRelations[ChildRelation];
            if ((childRelation != null) && (childRelation.ChildTable == childTable))
            {        
                DataColumn parentColumn = childRelation.ParentColumns[0];
                DataColumn childColumn = childRelation.ChildColumns[0];

                // Create a new row in the child table
                copyRow = childTable.NewRow();

                // set the parent field
                copyRow[childColumn] = parentRow[parentColumn];
                
                // copy each of the other non-unique fields
                //
                foreach (DataColumn column in childTable.Columns)
                {
                    if (column != childColumn && !column.Unique)
                    {
                        copyRow[column] = childRow[column];
                    }
                }
                childTable.Rows.Add(copyRow);

                // now recursively copy the child rows of the copied row
                //
                DataRowRowBinding childBinding = (DataRowRowBinding)Tree.GetBindingForItem(childRow);
                DataRelation relation = childTable.ChildRelations[childBinding.ChildRelation];
                if (relation != null)
                {
                    DataRow[] grandChildRows = childRow.GetChildRows(relation);  
                    foreach (DataRow grandChildRow in grandChildRows)
                    {
                        childBinding.CopyChildRow(copyRow, grandChildRow);
                    }
                }
            }
            return copyRow;
        }

        /// <summary>
        /// Sorts the given child view based on the current sort column of the tree
        /// </summary>
        /// <param name="view">The view to sort</param>
        protected virtual void SortChildView(DataView view)
        {
            DataRowRowBinding rowBinding = Tree.FindDataRowBinding(view.Table);
            if (rowBinding != null)
            {
                rowBinding.SortView(view);
            }
        }
       
        /// <summary>
        /// Sorts the given view based on the current sort column of the tree
        /// </summary>
        /// <param name="view">The view to sort</param>
        public virtual void SortView(DataView view)
        {
            if (view.Table.TableName != this.Table) 
                throw new ArgumentException("This RowBinding does not bind to the given view");

            Column sortColumn = Tree.SortColumn;
            string sortClause = null;

            if (sortColumn != null)
            {
                // get the cell binding corresponding to the sort column (if any)
                //
                DataRowCellBinding cellBinding = CellBinding(sortColumn);
                if (cellBinding != null)
                {
                    sortClause = cellBinding.Field;
                    if (sortClause != null)
                    {
                        if (sortColumn.SortDirection == ListSortDirection.Descending)
                            sortClause += " DESC";
                    }
                }
            }

            try
            {
                view.Sort = sortClause;
            }
            catch
            {
                DisplayErrorMessage(string.Format(Resources.InvalidSortFieldMsg, Table, sortClause));
                if (!SuppressBindingExceptions) throw;
            }
        }


        #endregion

        #region Static Public Methods

        /// <summary>
        /// Return a DataRowView in the default DataView for the given dataRow
        /// </summary>
        /// <param name="dataRow">The item this binding is bound to</param>
        /// <returns>The DataRowView in the default view for the given DataRow</returns>
        static public DataRowView GetDataRowView(DataRow dataRow)
        {
            DataView view = dataRow.Table.DefaultView;
            foreach (DataRowView dataRowView in view)
            {
                if (dataRowView.Row == dataRow) return dataRowView;
            }
            return null;
        }

        #endregion

  
    }
}
