using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Core.IO.GeoJson
{
    /// <summary>
    /// A Pyxis.Core.IO.GeoJson.Geometry representing a line.
    /// </summary>
    public class LineStringGeometry : Geometry
    {
        /// <summary>
        /// Gets or sets the Pyxis.Core.IO.GeoJson.GeographicPosition coordinates of the line.
        /// </summary>
        [JsonProperty("coordinates")]
        public List<GeographicPosition> Coordinates { get; set; }

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.LineStringGeometry.
        /// </summary>
        public LineStringGeometry()
        {
            Type = GeoJsonObjectType.LineString;
        }

        internal LineStringGeometry(JObject json) : this()
        {
            ValidateType(json);
            Coordinates = json["coordinates"].ToObject<List<GeographicPosition>>();
        }

        internal LineStringGeometry(PYXCurveRegion_SPtr curve) : this()
        {
            int count = curve.getVerticesCount();
            Coordinates = new List<GeographicPosition>();
            for (uint i = 0; i < count; i++)
            {
                Coordinates.Add(new GeographicPosition(PointLocation.fromXYZ(curve.getVertex(i))));
            }
        }

        /// <summary>
        /// Convert the Pyxis.Core.IO.GeoJson.LineStringGeometry to a PYXIS geometry.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to perform the conversion with.</param>
        /// <param name="resolution">resolution is been ingnored by this implementation</param>
        /// <returns>The PYXIS geometry created by converting the Pyxis.Core.IO.GeoJson.LineStringGeometry.</returns>
        public override PYXGeometry_SPtr ToPyxGeometry(Engine engine, int resolution = -1)
        {
            var builder = new PYXRegionBuilder();
            foreach (var point in Coordinates)
            {
                builder.addVertex(point.ToPointLocation());
            }
            return builder.createCurveGeometry(builder.suggestResolution());
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
