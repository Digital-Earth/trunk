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
    /// A Pyxis.Core.IO.GeoJson.Geometry representing a point.
    /// </summary>
    public class PointGeometry : Geometry
    {
        /// <summary>
        /// Gets or sets the Pyxis.Core.IO.GeoJson.GeographicPosition coordinates of the point.
        /// </summary>
        [JsonProperty("coordinates")]
        public GeographicPosition Coordinates { get; set; }

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.PointGeometry.
        /// </summary>
        public PointGeometry()
        {
            Type = GeoJsonObjectType.Point;
        }

        internal PointGeometry(JObject json) : this()
        {
            ValidateType(json);
            Coordinates = json["coordinates"].ToObject<GeographicPosition>();
        }

        internal PointGeometry(PYXVectorPointRegion_SPtr point) : this()
        {
            Coordinates = new GeographicPosition(PointLocation.fromXYZ(point.getPoint()));
        }

        /// <summary>
        /// Convert the Pyxis.Core.IO.GeoJson.PointGeometry to a PYXIS geometry.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to perform the conversion with.</param>
        /// <param name="resolution">resolution is been ingnored by this implementation</param>
        /// <returns>The PYXIS geometry created by converting the Pyxis.Core.IO.GeoJson.PointGeometry.</returns>
        public override PYXGeometry_SPtr ToPyxGeometry(Engine engine, int resolution = -1)
        {
            return pyxlib.DynamicPointerCast_PYXGeometry(
                PYXVectorGeometry2.createFromPoint(Coordinates.ToPointLocation().asXYZ(),
                    resolution == -1 ? 30 : resolution));
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
