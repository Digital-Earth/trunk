using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Pyxis.Core.Measurements;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Core.IO.GeoJson.Specialized
{
    /// <summary>
    /// A Pyxis.Core.IO.GeoJson.Geometry representing a circle.
    /// This is implements a specialized, non-standard GeoJSON geometry.
    /// </summary>
    public class CircleGeometry : Geometry
    {
        /// <summary>
        /// Gets or sets the Pyxis.Core.IO.GeoJson.GeographicPosition coordinates of the center of the circle.
        /// </summary>
        [JsonProperty("coordinates")]
        public GeographicPosition Coordinates { get; set; }

        /// <summary>
        /// Gets or sets the radius of the circle in meters.
        /// </summary>
        [JsonProperty("radius")]        
        public SphericalDistance Radius { get; set; }

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.Specialized.CircleGeometry.
        /// </summary>
        public CircleGeometry()
        {
            Type = GeoJsonObjectType.Circle;
            Radius = 0 * SphericalDistance.Meter;
        }

        internal CircleGeometry(JObject json) : this()
        {
            ValidateType(json);
            Coordinates = json["coordinates"].ToObject<GeographicPosition>();
            Radius = json["radius"].ToObject<double>() * SphericalDistance.Meter;
        }

        internal CircleGeometry(PYXCircleRegion_SPtr circle) : this()
        {
            Coordinates = new GeographicPosition(PointLocation.fromXYZ(circle.getCenter()));
            Radius = circle.getRadius() * SphericalDistance.Radian;
        }
        
        /// <summary>
        /// Convert the Pyxis.Core.IO.GeoJson.Specialized.CircleGeometry to a PYXIS geometry.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to perform the conversion with.</param>
        /// <param name="resolution">Resolution to use when converting into PYXGeometry, -1 for default</param>
        /// <returns>The PYXIS geometry created by converting the Pyxis.Core.IO.GeoJson.Specialized.CircleGeometry.</returns>
        public override PYXGeometry_SPtr ToPyxGeometry(Engine engine, int resolution = -1)
        {
            var circle = PYXCircleRegion.create( Coordinates.ToPointLocation().asXYZ(), Radius.InRadians);
            if (resolution == -1)
            {
                resolution = Math.Min(PYXBoundingCircle.estimateResolutionFromRadius(circle.getRadius()) + 9, PYXMath.knMaxAbsResolution);
            }

            return pyxlib.DynamicPointerCast_PYXGeometry(PYXVectorGeometry2.create(pyxlib.DynamicPointerCast_IRegion(circle), resolution));
        }

        internal override JObject ToJObject()
        {
            var json = new JObject();
            json.Add("type", (JToken)Type.ToString());
            json.Add("coordinates", JToken.FromObject(Coordinates));
            json.Add("radius", (JToken)Radius.InMeters);
            return json;
        }
    }
}
