#region File Header
//
//      FILE:   DataViewCellBinding.cs.
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
    /// (defined by a <see cref="Row"/> and <see cref="NS.Column"/>) to the properties of a <see cref="DataView"/>.
    /// </summary>
    /// <remarks>
    /// This class is typically created/added using the <see cref="VirtualTree"/> visual designer.  If the
    /// <see cref="VirtualTree.DataSource"/> is set to be a <see cref="DataView"/> or <see cref="DataTable"/> 
    /// then the root node of the tree is displayed using this binding.  The class allows the user to define the 
    /// information to display in a particular column for a <see cref="DataView"/> or <see cref="DataTable"/>.  
    /// The default behaviour is to display the <see cref="DataTable.TableName"/>.
    /// </remarks>
    /// <seealso href="XtraDatasetBinding.html">Data Binding to ADO.NET Datasets and DataViews</seealso>
    /// <seealso cref="VirtualTree"/>
    /// <seealso cref="NS.CellBinding"/>
    /// <seealso cref="NS.DataViewRowBinding"/>
    /// <seealso cref="NS.Column"/>
    /// <seealso cref="Row"/>
    /// <seealso cref="CellData"/>
    [TypeConverter("Infralution.Controls.VirtualTree.Design.DataViewCellBindingConverter, " + DesignAssembly.Name)]
    public class DataViewCellBinding : CellBinding
    {

        /// <summary>
        /// Initialise a default object
        /// </summary>
        public DataViewCellBinding()
        {
        }

        /// <summary>
        /// Initialise an Data View Cell Binding for the given column.
        /// </summary>
        public DataViewCellBinding(Column column)
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
            DataView dv = (DataView)row.Item;

            // handle possibility of view not being fully initialized yet
            //
            if (dv.Table != null)
            {
                cellData.Value = dv.Table.TableName;
            }
        }
  
    }
}
