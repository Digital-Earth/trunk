#region File Header
//
//      FILE:   CellEditorUpdateValueEventEditor.cs.
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
using System.Reflection;
using System.ComponentModel;
using Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree.Design
{

    /// <summary>
    /// Defines the editor for selecting the UpdateValueEvent for a CellEditor.
    /// </summary>
    internal class CellEditorUpdateValueEventEditor : Infralution.Controls.ListUITypeEditor  
    {

        /// <summary>
        /// Return the list of items to display
        /// </summary>
        /// <param name="context">The context for the editor</param>
        /// <returns>The items - or null if the context is not supported</returns>
        protected override IList GetItems(ITypeDescriptorContext context)
        {
            // check that the container is a CellEditor
            //
            if (!(context.Instance is CellEditor)) return null;            
            CellEditor cellEditor = (CellEditor)context.Instance;
            if (cellEditor.Control == null) return null;

            Type type = cellEditor.Control.GetType();
            return type.GetEvents();
        }

        /// <summary>
        /// Return the string to display for the given item
        /// </summary>
        /// <param name="item">The item to get the display string for</param>
        /// <returns>The display string</returns>
        protected override string GetDisplayString(object item)
        {
            return (item as EventInfo).Name;
        }


    }
}


