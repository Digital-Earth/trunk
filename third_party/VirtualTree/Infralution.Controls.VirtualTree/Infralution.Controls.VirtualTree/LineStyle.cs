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
    /// Defines the type of line used to draw the connections between <see cref="Row">Rows</see> of 
    /// the <see cref="VirtualTree"/>.
    /// </summary>
    public enum LineStyle
    {
        /// <summary>
        /// Use the default line style for the <see cref="StyleTemplate"/>
        /// </summary>
        Default,

        /// <summary>
        /// Connections bewteen rows are not drawn
        /// </summary>
        None,

        /// <summary>
        /// Connection between rows are drawn in solid color
        /// </summary>
        Solid,

        /// <summary>
        /// Connection between rows are drawn using a dot pattern
        /// </summary>
        Dot,

        /// <summary>
        /// Connection between rows are drawn using a dash pattern
        /// </summary>
        Dash,

        /// <summary>
        /// Connection between rows are drawn using a dash-dot pattern
        /// </summary>
        DashDot,

        /// <summary>
        /// Connection between rows are drawn using a dash-dot-dot pattern
        /// </summary>
        DashDotDot
    }

}
