#region File Header
//
//      FILE:   ImageDrawMode.cs.
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
namespace Infralution.Controls
{

    /// <summary>
    /// Defines how images are drawn into a rectangular area.
    /// </summary>
    public enum ImageDrawMode
    {
        /// <summary>
        /// The image is tiled to fill the available space
        /// </summary>
        Tile,

        /// <summary>
        /// The image is stretched to fill the available space
        /// </summary>
        Stretch,

        /// <summary>
        /// The image is stretched to fill the available space while maintaining aspect ratio of the image
        /// </summary>
        StretchSymmetric
    }

}
