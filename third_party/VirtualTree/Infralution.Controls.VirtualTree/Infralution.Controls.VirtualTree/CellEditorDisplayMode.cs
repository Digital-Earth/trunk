#region File Header
//
//      FILE:   CellEditorMode.cs.
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
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines when a cell editor will be displayed <see cref="CellEditor"/>.
    /// </summary>
    /// <seealso cref="VirtualTree"/>
    /// <seealso cref="CellEditor"/>
    /// <seealso cref="CellWidget"/>
    public enum CellEditorDisplayMode
    {
        /// <summary>
        /// The editor is always displayed
        /// </summary>
        Always,

        /// <summary>
        /// The editor is displayed when the user clicks on the <see cref="CellWidget"/> or tabs to
        /// the cell
        /// </summary>
        OnEdit
    }

}
