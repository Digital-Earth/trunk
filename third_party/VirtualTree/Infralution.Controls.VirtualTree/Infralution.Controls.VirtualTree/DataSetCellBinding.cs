#region File Header
//
//      FILE:   DataSetCellBinding.cs.
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
    /// Defines a class for binding information displayed in a cell of a <see cref="VirtualTree"/> 
    /// (defined by a <see cref="Row"/> and <see cref="NS.Column"/>) to the properties of a <see cref="DataSet"/>.
    /// </summary>
    /// <remarks>
    /// This class is typically created/added using the <see cref="VirtualTree"/> visual designer.  If the
    /// <see cref="VirtualTree.DataSource"/> is set to be a <see cref="DataSet"/> then the root row of the tree
    /// will correspond to the <see cref="DataSet"/>.  The class allows the user to define the information to 
    /// display in a particular column for the <see cref="DataSet"/> row.  The default behaviour is to display
    /// the <see cref="DataSet.DataSetName"/>.
    /// </remarks>
    /// <seealso href="XtraDatasetBinding.html">Data Binding to ADO.NET Datasets and DataViews</seealso>
    /// <seealso cref="VirtualTree"/>
    /// <seealso cref="NS.CellBinding"/>
    /// <seealso cref="NS.DataSetRowBinding"/>
    /// <seealso cref="NS.Column"/>
    /// <seealso cref="Row"/>
    /// <seealso cref="CellData"/>
    [TypeConverter("Infralution.Controls.VirtualTree.Design.DataSetCellBindingConverter, " + DesignAssembly.Name)]
    public class DataSetCellBinding : CellBinding
    {

        /// <summary>
        /// Initialise a default object
        /// </summary>
        public DataSetCellBinding()
        {
        }

        /// <summary>
        /// Initialise an Data Set Cell Binding for the given column and field.
        /// </summary>
        public DataSetCellBinding(Column column)
        {
            Column = column;
        }

        /// <summary>
        /// Get the data to be displayed in the given cell.
        /// </summary>
        /// <param name="row">The row that the cell belongs to</param>
        /// <param name="cellData">The data to be displayed in the cell</param>
        public override void GetCellData(Row row, CellData cellData)
        {
            base.GetCellData(row, cellData);
            DataSet ds = (DataSet)row.Item;
            cellData.Value = ds.DataSetName;
        }
  
    }
}
