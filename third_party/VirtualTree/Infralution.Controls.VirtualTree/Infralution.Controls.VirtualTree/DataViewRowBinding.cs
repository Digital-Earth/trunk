#region File Header
//
//      FILE:   DataViewRowBinding.cs.
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
using System.Windows.Forms.Design;
using System.Drawing.Design;
using Infralution.Common;
using NS=Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree
{
    /// <summary>
    /// Defines a class for binding information displayed in a <see cref="Row"/> of a 
    /// <see cref="VirtualTree"/> and relationships between Rows to the properties of a <see cref="DataView"/>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This binding allows you to display a <see cref="DataView"/> or <see cref="DataTable"/> as the root row 
    /// of a <see cref="VirtualTree"/>.  It is also used in conjuction with the <see cref="DataSetRowBinding"/>
    /// to display a <see cref="DataSet"/> as the root row with the <see cref="DataTable">DataTables</see>
    /// displayed as child rows.
    /// </para>
    /// <para>
    /// The class is typically defined using the <see cref="VirtualTree"/> visual designer.  The user creates 
    /// <see cref="DataViewCellBinding">DataViewCellBindings</see> (using the visual designer) that define the
    /// information to be displayed in the columns of the tree for this type of row. 
    /// </para>
    /// </remarks>
    /// <seealso href="XtraDatasetBinding.html">Data Binding to ADO.NET Datasets and DataViews</seealso>
    /// <seealso cref="VirtualTree"/>
    /// <seealso cref="NS.RowBinding"/>
    /// <seealso cref="NS.DataSetRowBinding"/>
    /// <seealso cref="NS.DataRowRowBinding"/>
    /// <seealso cref="NS.DataViewCellBinding"/>
    /// <seealso cref="RowData"/>
    /// <seealso cref="DataSet"/>
    /// <seealso cref="DataView"/>
    public class DataViewRowBinding : RowBinding
	{
        #region Member Variables

        private string _table;

        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a default object.
        /// </summary>
        public DataViewRowBinding()
        {
        }

        /// <summary>
        ///  Initialise a DataViewRowBinding object.
        /// </summary>
        /// <param name="table">The table name</param>
        public DataViewRowBinding(string table)
        {
            _table = table;
        }

        /// <summary>
        /// Set/Get the name of the table that this binding applies to.  If null then
        /// the binding will apply to all tables/views.
        /// </summary>
        [Category("Data")]
        [Description("The name of the table that this binding applies to.  If null then binding applies to all tables")]
        [Editor("Infralution.Controls.VirtualTree.Design.DataTableEditor, " + DesignAssembly.Name, typeof(UITypeEditor))]
        public string Table
        {
            get { return _table; }
            set { _table = StringUtilities.BlankToNull(value); }
        }

        /// <summary>
        /// Determines whether this binding is applicable for the given
        /// item.
        /// </summary>
        /// <param name="item">The item to bind</param>
        /// <returns>True if the item is a DataView or DataTable and the table matches</returns>
        /// <remarks>If the binding TableName is null then the binding will match any table or view</remarks>
        public override bool BindsTo(object item)
        {
            if (item is DataView)
            {
                DataView dataView = (DataView)item;
               
                // if table is null if we are binding directly to a view which
                // has not yet been fully initialized
                //
                if (_table == null || dataView.Table == null) return true;     
                return _table == dataView.Table.TableName;                    
            }
            return false;
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

            DataView view = (DataView)row.Item;
            SortView(view);
            return new DataViewList(view);            
        }
            
        /// <summary>
        /// Returns the parent object for the given item.
        /// </summary>
        /// <remarks>Returns the DataSet the view is part of</remarks>
        /// <param name="item">The item to get the parent for</param>
        /// <returns>The parent item for the given item</returns>
        public override object GetParentForItem(object item)
        {
            if (UseGetParentEvent) return base.GetParentForItem(item);
            return ((DataView)item).Table.DataSet;
        }

        /// <summary>
        /// Get the text to display in the bindings editor for this binding.
        /// </summary>
        public override string DisplayName
        {
            get
            {
                string table = (_table == null) ? "All" : _table;
                return String.Format("DataView({0})", table);
            }
        }
        
        #endregion

        #region Local Methods

        /// <summary>
        /// Creates a new <see cref="DataViewCellBinding"/>
        /// </summary>
        /// <returns>A new DataViewCellBinding</returns>
        public override CellBinding CreateCellBinding()
        {
            return new DataViewCellBinding();
        }

        /// <summary>
        /// Sorts the given view based on the current sort column of the tree
        /// </summary>
        /// <param name="view">The view to sort</param>
        protected virtual void SortView(DataView view)
        {
            // handle possibility of view not being fully initialized yet
            //
            if (view.Table != null)
            {
                DataRowRowBinding rowBinding = Tree.FindDataRowBinding(view.Table);
                if (rowBinding != null)
                {
                    rowBinding.SortView(view);
                }
            }
        }


        #endregion

    }
}
