#region File Header
//
//      FILE:   DataFieldEditor.cs.
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
using System.Drawing.Design;
using System.Windows.Forms.Design;
using System.Windows.Forms;
using System.Data;
using System.ComponentModel;
namespace Infralution.Controls.VirtualTree.Design
{

    /// <summary>
    /// Defines the <see cref="UITypeEditor"/> for editing field names for 
    /// a <see cref="DataRowRowBinding"/>.
    /// </summary>
    internal class DataFieldEditor : Infralution.Controls.ListUITypeEditor  
    {

        /// <summary>
        /// Return the list of items to display
        /// </summary>
        /// <param name="context">The context for the editor</param>
        /// <returns>The items - or null if the context is not supported</returns>
        protected override IList GetItems(ITypeDescriptorContext context)
        {
            // check that the object is a DataRowCellBinding
            //
            if (!(context.Instance is DataRowCellBinding)) return null;
            
            DataRowCellBinding binding = (DataRowCellBinding)context.Instance;
            
            // find the data set associated with the tree (if any)
            //
            DataSet dataSet = Utilities.GetDataSet(binding.RowBinding.Tree);
            if (dataSet == null) return null;

            // find the table this row is bound to
            //
            DataTable table = dataSet.Tables[binding.RowBinding.Table];
            if (table == null) return null;

            return new ArrayList(table.Columns);
        }

        /// <summary>
        /// Return the value to set for the given item
        /// </summary>
        /// <param name="item">The item to get the value for</param>
        protected override object GetValue(object item)
        {
            return (item as DataColumn).ColumnName;
        }

    }

}


