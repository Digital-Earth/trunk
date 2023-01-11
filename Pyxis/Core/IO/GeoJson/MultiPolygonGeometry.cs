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
    /// A Pyxis.Core.IO.GeoJson.Geometry representing a multi-polygon.
    /// </summary>
    public class MultiPolygonGeometry : Geometry
    {
        /// <summary>
        /// Gets or sets the Pyxis.Core.IO.GeoJson.GeographicPosition coordinates of the multi-polygon. 
        /// </summary>
        [JsonProperty("coordinates")]
        public List<List<List<GeographicPosition>>> Coordinates { get; set; }

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.MultiPolygonGeometry.
        /// </summary>
        public MultiPolygonGeometry()
        {
            Type = GeoJsonObjectType.MultiPolygon;
        }

        internal MultiPolygonGeometry(JObject json) : this()
        {
            ValidateType(json);
            Coordinates = json["coordinates"].ToObject <List<List<List<GeographicPosition>>>>();
        }

        /// <summary>
        /// Convert the Pyxis.Core.IO.GeoJson.MultiPolygonGeometry to a PYXIS geometry.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to perform the conversion with.</param>
        /// <param name="resolution">resolution is been ingnored by this implementation</param>
        /// <returns>The PYXIS geometry created by converting the Pyxis.Core.IO.GeoJson.MultiPolygonGeometry.</returns>
        public override PYXGeometry_SPtr ToPyxGeometry(Engine engine, int resolution = -1)
        {
            var builder = new PYXRegionBuilder();
            foreach (var polygon in Coordinates)
            {
                foreach (var curve in polygon)
                {
                    foreach (var point in curve)
                    {
                        builder.addVertex(point.ToPointLocation());
                    }
                    builder.closeCurve();
                }
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

        internal static Geometry FromPYXGeometry(PYXMultiPolygonRegion_SPtr multiPolygon)
        {
            var polygons = new List<List<List<GeographicPosition>>>();
            var polygon = new List<List<GeographicPosition>>();
                        
            PYXMultiPolygonRegion_SPtr polygonSoFar = null;
            var pyxRegionBuilder = new PYXRegionBuilder();

            var ringsCount = multiPolygon.getRingsCount();

            for (uint ringIndex = 0; ringIndex < ringsCount; ringIndex++)
            {
                var newJsonCurve = new List<GeographicPosition>();
                var ringVertexCount = multiPolygon.getRingVerticesCount(ringIndex);

                var isOuterRing = (
                    //if no polygon built so far
                    polygonSoFar == null ||
                    //the first vertex is not contained by the polygon we built so far
                    !polygonSoFar.isPointContained(multiPolygon.getRingVertex(ringIndex, 0), 0));

                //if this is outer ring - add the current polygon into our multi polygon list and start a new one
                if (isOuterRing && polygon.Count > 0)
                {
                    polygons.Add(polygon);
                    polygon = new List<List<GeographicPosition>>();                    
                    pyxRegionBuilder = new PYXRegionBuilder();
                }

                for (uint vertexIndex = 0; vertexIndex < ringVertexCount; vertexIndex++)
                {
                    var point = PointLocation.fromXYZ(multiPolygon.getRingVertex(ringIndex, vertexIndex));
                    pyxRegionBuilder.addVertex(point);
                    newJsonCurve.Add(new GeographicPosition(point));
                }

                polygon.Add(newJsonCurve);
                
                pyxRegionBuilder.closeCurve();
                polygonSoFar = pyxlib.DynamicPointerCast_PYXMultiPolygonRegion(pyxRegionBuilder.createPolygonRegion());                
            }

            if (polygons.Count>0)
            {
                polygons.Add(polygon);
                return new MultiPolygonGeometry() { Coordinates = polygons };
            }
            else 
            {
                return new PolygonGeometry() { Coordinates = polygon };
            }
        }
    }
}
