

#region File Header
//
//      FILE:   DataTableEditor.cs.
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
using Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree.Design
{

    /// <summary>
    /// Defines the UITypeEditor for editing table names for a <see cref="DataViewRowBinding"/> 
    /// or <see cref="DataRowRowBinding"/>.
    /// </summary>
    internal class DataTableEditor : Infralution.Controls.ListUITypeEditor
    {

        /// <summary>
        /// Return the list of items to display
        /// </summary>
        /// <param name="context">The context for the editor</param>
        /// <returns>The items - or null if the context is not supported</returns>
        protected override IList GetItems(ITypeDescriptorContext context)
        {
            // check that the object is a RowBinding
            //
            if (!(context.Instance is RowBinding)) return null;
            RowBinding binding = (RowBinding)context.Instance;

            DataSet dataSet = Utilities.GetDataSet(binding.Tree);
            if (dataSet == null) return null;
            return new ArrayList(dataSet.Tables);
        }

        /// <summary>
        /// Return the value to set for the given item
        /// </summary>
        /// <param name="item">The item to get the value for</param>
        protected override object GetValue(object item)
        {
            return (item as DataTable).TableName;
        }

    }

}
