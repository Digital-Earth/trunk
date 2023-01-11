using System;

namespace Pyxis.Core.IO.GeoJson
{
    /// <summary>
    /// Defines what information should be extracted from a feature
    /// </summary>
    [Flags]
    public enum FeatureExtractionFlags
    {
        /// <summary>
        /// Extract geometry only
        /// </summary>
        Geometry = 0x01,

        /// <summary>
        /// Extract fields only
        /// </summary>
        Fields = 0x02,

        /// <summary>
        /// Extract both geometry and fields
        /// </summary>
        All = 0x03
    }
}
