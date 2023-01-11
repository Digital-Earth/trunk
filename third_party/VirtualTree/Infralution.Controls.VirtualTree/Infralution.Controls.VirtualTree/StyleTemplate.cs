#region File Header
//
//      FILE:   LineStyle.cs.
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
    /// Determines the default styles, expansion icons and linestyle for <see cref="VirtualTree"/>.
    /// </summary>
    public enum StyleTemplate
    {
        /// <summary>
        /// Classic Windows 98/2000 Explorer appearance (solid tree connection lines and 3D bordered headers)  
        /// </summary>
        Classic,

        /// <summary>
        /// Classic Windows Explorer appearance but with XP Themed headers (if available)
        /// </summary>
        ClassicXP,

        /// <summary>
        /// XP Windows Explorer appearance (no tree connection lines, themed headers)
        /// </summary>
        XP,

        /// <summary>
        /// Windows Vista Explorer appearance (gradient headers, Vista style expansion icons and rounded selection styles)
        /// </summary>
        Vista
    }

}
