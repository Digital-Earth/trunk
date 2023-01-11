#region File Header
//
//      FILE:   RowChildPolicy.cs.
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
    /// Defines the policy used by the <see cref="VirtualTree"/> when loading children for a given item.
    /// </summary>
    /// <remarks>
    /// The child policy for a <see cref="Row"/> can be set by:
    /// <list type="bullet">
    /// <item>handling the <see cref="VirtualTree.GetChildPolicy"/> event; or</item>
    /// <item>overriding the <see cref="VirtualTree.GetChildPolicyForRow"/> method; or</item>
    /// <item>setting the <see cref="RowBinding.ChildPolicy">RowBinding.ChildPolicy</see></item>
    /// </list>
    /// </remarks>
    /// <seealso cref="VirtualTree.GetChildPolicy"/>
    /// <seealso cref="VirtualTree.GetChildPolicyForRow"/>
    /// <seealso cref="RowBinding"/>
    public enum RowChildPolicy
    {
        /// <summary>
        /// Children are loaded when the item is first displayed
        /// </summary>
        Normal,

        /// <summary>
        /// Children are loaded and the item is expanded when it is first displayed
        /// </summary>
        AutoExpand,

        /// <summary>
        /// Children are not loaded until the user clicks on the expansion indicator.  If there
        /// are no children the expansion indicator is then removed.
        /// </summary>
        LoadOnExpand
    }

}
