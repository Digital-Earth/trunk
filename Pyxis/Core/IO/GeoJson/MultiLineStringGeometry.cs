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
    /// A Pyxis.Core.IO.GeoJson.Geometry representing a multi-line.
    /// </summary>
    public class MultiLineStringGeometry : Geometry
    {
        /// <summary>
        /// Gets or sets the Pyxis.Core.IO.GeoJson.GeographicPosition coordinates of the multi-line.
        /// </summary>
        [JsonProperty("coordinates")]
        public List<List<GeographicPosition>> Coordinates { get; set; }

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.MultiLineStringGeometry.
        /// </summary>
        public MultiLineStringGeometry ()
        {
            Type = GeoJsonObjectType.MultiLineString;
        }

        internal MultiLineStringGeometry(JObject json)
            : this()
        {
            ValidateType(json);
            Coordinates = json["coordinates"].ToObject <List<List<GeographicPosition>>>();
        }

        internal MultiLineStringGeometry(PYXMultiCurveRegion_SPtr multiCurve)
            : this()
        {            
            Coordinates = new List<List<GeographicPosition>>();
            var curveCount = multiCurve.getCurveCount();

            for (uint curveIndex = 0; curveIndex < curveCount; curveIndex++)
            {
                var newJsonCurve = new List<GeographicPosition>();
                var curveVertexCount = multiCurve.getCurveVerticesCount(curveIndex);
                for(uint vertexIndex = 0; vertexIndex < curveVertexCount; vertexIndex ++ )
                {
                    newJsonCurve.Add(new GeographicPosition(PointLocation.fromXYZ(multiCurve.getCurveVertex(curveIndex,vertexIndex))));
                }

                Coordinates.Add(newJsonCurve);
            }            
        }

        /// <summary>
        /// Convert the Pyxis.Core.IO.GeoJson.MultiLineStringGeometry to a PYXIS geometry.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to perform the conversion with.</param>
        /// <param name="resolution">resolution is been ingnored by this implementation</param>
        /// <returns>The PYXIS geometry created by converting the Pyxis.Core.IO.GeoJson.MultiLineStringGeometry.</returns>
        public override PYXGeometry_SPtr ToPyxGeometry(Engine engine, int resolution = -1)
        {
            var builder = new PYXRegionBuilder();
            foreach (var curve in Coordinates)
            {
                foreach (var point in curve)
                {
                    builder.addVertex(point.ToPointLocation());
                }
                builder.endCurve();
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
