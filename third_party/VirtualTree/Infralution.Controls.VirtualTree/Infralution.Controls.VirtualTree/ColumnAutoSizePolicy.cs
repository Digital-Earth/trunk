#region File Header
//
//      FILE:   ColumnAutoSizePolicy.cs.
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
    /// Defines the whether the <see cref="VirtualTree"/> will auto size a given <see cref="Column"/>.
    /// </summary>
    /// <seealso cref="VirtualTree"/>
    /// <seealso cref="Column.AutoSizePolicy">Column.AutoSizePolicy</seealso>
    public enum ColumnAutoSizePolicy
    {
        /// <summary>
        /// The width of the column is
        /// </summary>
        Manual,

        /// <summary>
        /// The column width is automatically increased if necessary to fit the displayed cell data
        /// but is not decreased
        /// </summary>
        AutoIncrease,

        /// <summary>
        /// The column width is automatically increased and decreased to fit the displayed cell data
        /// </summary>
        AutoSize
    }

}
