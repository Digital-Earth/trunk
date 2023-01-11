#region File Header
//
//      FILE:   SelectionChangingEventArgs.cs.
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
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines the possible changes to the selection of a <see cref="VirtualTree"/>.
    /// </summary>
    /// <seealso cref="VirtualTree"/>
    /// <seealso cref="RowSelectionList"/>
    /// <seealso cref="SelectionChangingEventArgs"/>
    public enum SelectionChange
    {
        /// <summary>
        /// The current selection will be cleared
        /// </summary>
        Clear,

        /// <summary>
        /// New rows will be added to the current selection
        /// </summary>
        Add,

        /// <summary>
        /// Rows will be removed from the current selection
        /// </summary>
        Remove,

        /// <summary>
        /// The current selection will be cleared and new rows added
        /// </summary>
        ClearAndAdd
    }

}
