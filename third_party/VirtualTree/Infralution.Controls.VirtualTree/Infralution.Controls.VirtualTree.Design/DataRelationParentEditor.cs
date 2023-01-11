#region File Header
//
//      FILE:   DataRelationParentEditor.cs.
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
    /// Defines the <see cref="UITypeEditor"/> for editing parent relation names for 
    /// a <see cref="DataRowRowBinding"/>
    /// </summary>
    internal class DataRelationParentEditor : Infralution.Controls.ListUITypeEditor  
    {

        /// <summary>
        /// Return the list of items to display
        /// </summary>
        /// <param name="context">The context for the editor</param>
        /// <returns>The items - or null if the context is not supported</returns>
        protected override IList GetItems(ITypeDescriptorContext context)
        { 
            // check that the object is a DataRowRowBinding
            //
            if (!(context.Instance is DataRowRowBinding)) return null;
                        
            // find the data set associated with the tree (if any)
            //
            DataRowRowBinding binding = (DataRowRowBinding)context.Instance;
            DataSet dataSet = Utilities.GetDataSet(binding.Tree);
            if (dataSet == null) return null;
          
            DataTable table = dataSet.Tables[binding.Table];
            if (table == null) return null;
            return new ArrayList(table.ParentRelations);
        }

        /// <summary>
        /// Return the value to set for the given item
        /// </summary>
        /// <param name="item">The item to get the value for</param>
        protected override object GetValue(object item)
        {
            return (item as DataRelation).RelationName;
        }

    }
}


