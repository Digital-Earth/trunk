using System;
using System.Collections.Generic;
using ApplicationUtility;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Pyxis.Contract.DataDiscovery;

namespace Pyxis.Core.IO.GeoJson.Specialized
{
    /// <summary>
    /// A Pyxis.Core.IO.GeoJson.Geometry representing a projected bounding box.
    /// </summary>
    public class BBoxGeometry : Geometry
    {
        /// <summary>
        /// Gets or sets the Pyxis.Core.IO.SpatialReferenceSystem used for this bounding box
        /// </summary>
        [JsonProperty("srs")]
        public SpatialReferenceSystem SpatialReferenceSystem { get; set; }

        /// <summary>
        /// Gets or sets the Pyxis.Core.IO.GeoJson.GeographicPosition coordinates of the bounding box.
        /// </summary>
        [JsonProperty("coordinates")]
        public double [][] Coordinates { get; set; }

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.Specialized.BBoxGeometry.
        /// </summary>
        public BBoxGeometry()
        {
            Type = GeoJsonObjectType.BBox;
        }

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.Specialized.BBoxGeometry.
        /// </summary>
        internal BBoxGeometry(PYXXYBoundsGeometry_SPtr geometry) 
            : this()
        {
            var coordConverter = geometry.getCoordConverter();
            if (coordConverter.isNull())
            {
                throw new Exception("Failed to find projection for BBox Geometry");
            }
            var srs = PYXCOMFactory.CreateSRSFromCoordConverter(coordConverter);
            if (srs.isNull())
            {
                throw new Exception("Failed to find projection for BBox Geometry");
            }

            SpatialReferenceSystem = SpatialReferenceSystem.CreateFromWKT(srs.getWKT()).Normalize();
            
            var bounds = geometry.getBounds();
            Coordinates = new double[][] {
                new double[] { bounds.xMin(),bounds.yMin() },
                new double[] { bounds.xMax(),bounds.yMax() }
            };
        }

        internal BBoxGeometry(JObject json)
            : this()
        {
            ValidateType(json);
            Coordinates = json["coordinates"].ToObject<double[][]>();
            SpatialReferenceSystem = json["srs"].ToObject<SpatialReferenceSystem>();
        }

        /// <summary>
        /// Convert the Pyxis.Core.IO.GeoJson.Specialized.BBoxGeometry to a PYXIS geometry.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to perform the conversion with.</param>
        /// <param name="resolution">Resolution to use when converting into PYXGeometry, -1 for default</param>
        /// <returns>The PYXIS geometry created by converting the Pyxis.Core.IO.GeoJson.Specialized.BBoxGeometry.</returns>
        public override PYXGeometry_SPtr ToPyxGeometry(Engine engine, int resolution = -1)
        {
            if (Coordinates.Length != 2)
            {
                throw new InvalidOperationException("Must have exactly 2 coordinates to define a bbox");
            }

            var coordConverter = SpatialReferenceSystem.CreateCoordConverter();

            var point1 = Coordinates[0];
            var point2 = Coordinates[1];
            var minX = Math.Min(point1[0], point2[0]);
            var maxX = Math.Max(point1[0], point2[0]);
            var minY = Math.Min(point1[1], point2[1]);
            var maxY = Math.Max(point1[1], point2[1]);

            var bbox = new PYXRect2DDouble(minX, minY, maxX, maxY);

            return pyxlib.DynamicPointerCast_PYXGeometry(PYXXYBoundsGeometry.create(bbox, coordConverter.get(), resolution == -1 ? 24 : resolution));
        }

        internal override JObject ToJObject()
        {
            var json = new JObject();
            json.Add("type", (JToken)Type.ToString());
            json.Add("coordinates", JToken.FromObject(Coordinates));
            json.Add("srs", JToken.FromObject(SpatialReferenceSystem));
            return json;
        }
    }
}
