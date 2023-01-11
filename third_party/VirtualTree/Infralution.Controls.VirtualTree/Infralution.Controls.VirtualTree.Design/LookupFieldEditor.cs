#region File Header
//
//      FILE:   LookupFieldEditor.cs.
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
    /// Defines the <see cref="UITypeEditor"/> for LookupField names for 
    /// a <see cref="DataRowCellBinding"/>
    /// </summary>
    internal class LookupFieldEditor : Infralution.Controls.ListUITypeEditor  
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
                        
            // find the data set associated with the tree (if any)
            //
            DataRowCellBinding binding = (DataRowCellBinding)context.Instance;
            DataSet dataSet = Utilities.GetDataSet(binding.RowBinding.Tree);
            if (dataSet == null) return null;
          
            DataTable table = dataSet.Tables[binding.RowBinding.Table];
            if (table == null) return null;

            // find the lookup relation on this column (if any)
            //
            DataRelation lookupRelation = null;
            foreach (DataRelation relation in table.ParentRelations)
            {
                if (relation.ChildColumns[0].ColumnName == binding.Field)
                {
                    lookupRelation = relation;
                    break;
                }
            }
            if (lookupRelation == null) return null;

            return new ArrayList(lookupRelation.ParentTable.Columns);
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


