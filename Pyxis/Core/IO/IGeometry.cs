using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Core.IO
{
    /// <summary>
    /// Describes a geometry on the globe.
    /// </summary>
    [JsonConverter(typeof(GeoJson.GeoJsonGeometryConverter))]    
    public interface IGeometry
    {
        /// <summary>
        /// Convert to an instance of a PYXIS geometry.
        /// </summary>
        /// <param name="engine">A running Pyxis.Core.Engine to use for creating the PYXIS geometry.</param>
        /// <param name="resolution">Resolution to use when converting into PYXGeometry, -1 for default</param>
        /// <returns>Pointer to the PYXIS geometry.</returns>
        PYXGeometry_SPtr ToPyxGeometry(Engine engine, int resolution = -1);

        /// <summary>
        /// Calculate the area the geometry covers on earth.
        /// </summary>
        /// <param name="engine">A running Pyxis.Core.Engine to use for calculating the PYXIS geometry.</param>
        /// <returns>Area of the geometry.</returns>
        Measurements.SphericalArea GetArea(Engine engine);
    }
}
