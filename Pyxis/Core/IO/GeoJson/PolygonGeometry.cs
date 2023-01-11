using System;
using System.Collections.Generic;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace Pyxis.Core.IO.GeoJson
{
    /// <summary>
    /// A Pyxis.Core.IO.GeoJson.Geometry representing a polygon.
    /// </summary>
    public class PolygonGeometry : Geometry
    {
        /// <summary>
        /// Gets or sets the Pyxis.Core.IO.GeoJson.GeographicPosition coordinates of the polygon.
        /// </summary>
        [JsonProperty("coordinates")]
        public List<List<GeographicPosition>> Coordinates { get; set; }

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.PolygonGeometry.
        /// </summary>
        public PolygonGeometry()
        {
            Type = GeoJsonObjectType.Polygon;
        }

        internal PolygonGeometry(JObject json) : this()
        {
            ValidateType(json);
            Coordinates = json["coordinates"].ToObject <List<List<GeographicPosition>>>();
        }

        /// <summary>
        /// Convert the Pyxis.Core.IO.GeoJson.PolygonGeometry to a PYXIS geometry.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to perform the conversion with.</param>
        /// <param name="resolution">resolution is been ingnored by this implementation</param>
        /// <returns>The PYXIS geometry created by converting the Pyxis.Core.IO.GeoJson.PolygonGeometry.</returns>
        public override PYXGeometry_SPtr ToPyxGeometry(Engine engine, int resolution = -1)
        {            
            var builder = new PYXRegionBuilder();
            foreach (var curve in Coordinates)
            {
                if (curve.Count < 3)
                {
                    throw new Exception("Curve must contain at least 3 point");
                }

                foreach (var point in curve)
                {
                    builder.addVertex(point.ToPointLocation());
                }
                builder.closeCurve();
            }

            return builder.createPolyonGeometry(builder.suggestResolution());
        }

        internal override JObject ToJObject()
        {
            var json = new JObject();
            json.Add("type", (JToken)Type.ToString());
            json.Add("coordinates", JToken.FromObject(Coordinates));
            return json;
        }
    }
}
