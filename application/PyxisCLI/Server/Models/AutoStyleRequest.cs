using Pyxis.Contract.Publishing;
using Pyxis.Core.IO;

namespace PyxisCLI.Server.Models
{
    /// <summary>
    /// Generate a Style for a GeoSource based on values. The Request can specific field and an optional geometry to sample values from, and a palette to be used.
    /// </summary>
    public class AutoStyleRequest
    {
        /// <summary>
        /// Current style used by the GeoSource.
        /// </summary>
        public Style Style { get; set; }

        /// <summary>
        /// Field to use for auto styling.
        /// </summary>
        public string Field { get; set; }

        /// <summary>
        /// Palette to use for auto styling.
        /// </summary>
        public Palette Palette { get; set; }

        /// <summary>
        /// If not null, Geometry to use to sample values of the GeoSource
        /// </summary>
        public IGeometry Geometry { get; set; }
    }

    /// <summary>
    /// Generate a Style for a GeoSource based on values. The Request can specific field and an optional geometry to sample values from, and a palette to be used.
    /// </summary>
    public class AutoStyleRequestLegacy
    {
        /// <summary>
        /// GeoSource to style
        /// </summary>
        public GeoSource GeoSource { get; set; }

        /// <summary>
        /// Current style used by the GeoSource.
        /// </summary>
        public Style Style { get; set; }

        /// <summary>
        /// Field to use for auto styling.
        /// </summary>
        public string Field { get; set; }

        /// <summary>
        /// Palette to use for auto styling.
        /// </summary>
        public Palette Palette { get; set; }

        /// <summary>
        /// If not null, Geometry to use to sample values of the GeoSource
        /// </summary>
        public IGeometry Geometry { get; set; }
    }
}