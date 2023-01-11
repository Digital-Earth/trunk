#region File Header
//
//      FILE:   DataSetRowBinding.cs.
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
using NS=Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree
{
    /// <summary>
    /// Defines a class for binding information displayed in a <see cref="Row"/> of a 
    /// <see cref="VirtualTree"/> and relationships between Rows to the properties of a <see cref="DataSet"/>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This binding allows you to display a <see cref="DataSet"/> as the root row of a <see cref="VirtualTree"/>.
    /// The <see cref="DataTable">DataTables</see> within the <see cref="DataSet"/> are displayed as child rows. 
    /// This class is typically defined using the <see cref="VirtualTree"/> visual designer.  It is used
    /// when the <see cref="VirtualTree.DataSource"/> is set to be a <see cref="DataSet"/>.   The user creates 
    /// <see cref="DataSetCellBinding">DataSetCellBindings</see> (using the visual designer) that define the
    /// information to be displayed in the columns for the root row. 
    /// </para>
    /// <para>
    /// This class will typically be used in conjunction with a <see cref="DataViewRowBinding"/> (which defines
    /// how the <see cref="DataTable"/> rows will be displayed) and <see cref="DataRowRowBinding">DataRowRowBindings</see>
    /// (which define the information to be displayed for each <see cref="DataRow"/> in the tables.
    /// </para>
    /// <para>
    /// If you want to bind the root row of the tree to a single <see cref="DataTable"/> then set the 
    /// <see cref="VirtualTree.DataSource"/> to be a <see cref="DataTable"/> or <see cref="DataView"/> from
    /// the <see cref="DataSet"/> and use the <see cref="DataViewRowBinding"/> class to bind the root row.
    /// </para>
    /// </remarks>
    /// <seealso href="XtraDatasetBinding.html">Data Binding to ADO.NET Datasets and DataViews</seealso>
    /// <seealso cref="VirtualTree"/>
    /// <seealso cref="NS.RowBinding"/>
    /// <seealso cref="NS.DataViewRowBinding"/>
    /// <seealso cref="NS.DataRowRowBinding"/>
    /// <seealso cref="NS.DataSetCellBinding"/>
    /// <seealso cref="RowData"/>
    /// <seealso cref="DataSet"/>
    public class DataSetRowBinding : RowBinding
    {
        #region Public Interface

        /// <summary>
        /// Determines whether this binding is applicable for the given
        /// item.
        /// </summary>
        /// <param name="item">The item to bind</param>
        /// <returns>True if the item is a DataSet</returns>
        public override bool BindsTo(object item)
        {
            return (item is DataSet);
        }

        /// <summary>
        /// Returns the list of children for the given row
        /// </summary>
        /// <remarks>Returns the list of default views for tables in the data set</remarks>
        /// <param name="row">The item to get the children for</param>
        /// <returns>The list of children for the row</returns>
        public override IList GetChildrenForRow(Row row)
        {
            if (UseGetChildrenEvent) return base.GetChildrenForRow(row);

            DataSet dataSet = (DataSet)row.Item;
            DataView[] dataViews = new DataView[dataSet.Tables.Count];
            for (int i=0; i < dataSet.Tables.Count; i++)
            {
                dataViews[i] = dataSet.Tables[i].DefaultView;
            }
            return dataViews;            
        }
            
        /// <summary>
        /// Get the text to display in the bindings editor for this binding.
        /// </summary>
        public override string DisplayName
        {
            get
            {
                return "DataSet";
            }
        }
        
        #endregion

        #region Local Methods

        /// <summary>
        /// Creates a new <see cref="DataSetCellBinding"/>.
        /// </summary>
        /// <returns>A new DataSetCellBinding</returns>
        public override CellBinding CreateCellBinding()
        {
            return new DataSetCellBinding();
        }

        #endregion

    }
}
