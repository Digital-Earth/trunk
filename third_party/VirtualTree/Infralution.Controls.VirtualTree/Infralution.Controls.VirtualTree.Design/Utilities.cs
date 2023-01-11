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
    /// Defines general design time utility methods
    /// </summary>
    internal sealed class Utilities 
    {

        /// <summary>
        /// Get the DataSet the tree is bound to (if any)
        /// </summary>
        /// <param name="tree"></param>
        /// <returns></returns>
        public static DataSet GetDataSet(VirtualTree tree)
        {
            object rootItem = tree.RootItem;
            DataSet dataSet = null;
            if (rootItem is DataSet)
                dataSet = (DataSet)rootItem;
            else if (rootItem is DataTable)
                dataSet = (rootItem as DataTable).DataSet;
            else if (rootItem is DataView)
                dataSet = (rootItem as DataView).Table.DataSet;
            return dataSet;
        }

    }

}


