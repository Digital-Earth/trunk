using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Pyxis.Core.Measurements
{
    /// <summary>
    /// Describes the type of quantity being measured.
    /// </summary>
    public enum MeasurementType
    {
        /// <summary>
        /// Distance on the surface of a sphere.
        /// </summary>
        SphericalDistance,      
        /// <summary>
        /// Area on the surface of a sphere.
        /// </summary>
        SphericalArea,
        /// <summary>
        /// Time elapsed.
        /// </summary>
        Time,
        /// <summary>
        /// Count of the number of occurrences.
        /// </summary>
        Count
    }
}
