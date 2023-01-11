#region File Header
//
//      FILE:   SelectionMode.cs.
//
//    AUTHOR:   Grant Frisken
//
// COPYRIGHT:   Copyright 2005 
//              Infralution
//              6 Bruce St 
//              Mitcham Australia
//
#endregion
using System;
namespace Infralution.Controls.VirtualTree
{
    /// <summary>
    /// Defines how row selections are handled by Virtual Tree
    /// </summary>
    /// <seealso cref="VirtualTree.SelectionMode"/>
    public enum SelectionMode
    {
        /// <summary>
        /// The user can select rows only (the full row is shown as selected)
        /// </summary>
        FullRow,

        /// <summary>
        /// The user can select rows only (only the text in the main cell is shown as selected)
        /// </summary>
        MainCellText,

        /// <summary>
        /// The user can select individual cells.   The column of the selected cell(s) is given
        /// by <see cref="VirtualTree.SelectedColumn"/>
        /// </summary>
        Cell
    }
}
