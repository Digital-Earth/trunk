#region File Header
//
//      FILE:   RowDropLocation.cs.
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
    /// Defines possible drag and drop locations for a <see cref="Row"/> within a <see cref="VirtualTree"/>.
    /// </summary>
    /// <remarks>
    /// The allowed drop locations for a <see cref="Row"/> can be set by:
    /// <list type="bullet">
    /// <item>handling the <see cref="VirtualTree.GetAllowedRowDropLocations"/> event; or</item>
    /// <item>overriding the <see cref="VirtualTree.AllowedRowDropLocations"/> method; or</item>
    /// <item>setting the <see cref="RowBinding"/> AllowDrop properties</item>
    /// </list>
    /// </remarks>
    /// <seealso cref="VirtualTree.GetAllowedRowDropLocations"/>
    /// <seealso cref="VirtualTree.AllowedRowDropLocations"/>
    /// <seealso cref="RowBinding.AllowDropOnRow"/>
    /// <seealso cref="RowBinding.AllowDropAboveRow"/>
    /// <seealso cref="RowBinding.AllowDropBelowRow"/>
    [System.Flags] 
    public enum RowDropLocation
    {
        /// <summary>
        /// There is no drop operation
        /// </summary>
        None = 0,

        /// <summary>
        /// The data is being dropped onto the target row
        /// </summary>
        OnRow = 1,

        /// <summary>
        /// The data is being dropped above the target row
        /// </summary>
        AboveRow = 2,

        /// <summary>
        /// The data is being dropped below the target row
        /// </summary>
        BelowRow = 4,

        /// <summary>
        /// The data is being dropped onto the empty row space
        /// </summary>
        EmptyRowSpace = 8

    }

}
