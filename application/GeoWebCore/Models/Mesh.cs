using System;
using System.Collections.Generic;

namespace GeoWebCore.Models
{
    /// <summary>
    /// Simple mesh object for Rhombuses
    /// </summary>
    [Obsolete("client now generate meshses locally")]
    public class Mesh
    {
        /// <summary>
        /// List over vertices in the meash. each verices is an array double[3] (x,y,z)
        /// </summary>
        public double[][] Vertices { get; set; }
    }
}
