using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Pyxis.Core.Measurements;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Core.IO.GeoJson
{
    /// <summary>
    /// GeoJSON WGS84 utility class.
    /// </summary>
    [JsonConverter(typeof(GeographicPositionJsonConverter))]
    public class GeographicPosition
    {
        /// <summary>
        /// Gets or sets the longitude.
        /// </summary>
        public double Longitude { get; set; }
        /// <summary>
        /// Gets or sets the latitude.
        /// </summary>
        public double Latitude { get; set; }       

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.GeographicPosition.
        /// </summary>
        public GeographicPosition()
        {
        }
        
        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.GeographicPosition from a PYXIS PointLocation.
        /// </summary>
        /// <param name="point">The PYXIS PointLocation to initialize the Pyxis.Core.IO.GeoJson.GeographicPosition with.</param>
        public GeographicPosition(PointLocation point)
        {
            var coord = point.asWGS84();

            //please note the PointLocation have mixed coordinates - we might want to fix that
            Latitude = coord.y();
            Longitude = coord.x();
        }

        /// <summary>
        /// Convert the Pyxis.Core.IO.GeoJson.GeographicPosition to a PYXIS PointLocation.
        /// </summary>
        /// <returns>A PYXIS PointLocation corresponding to the Pyxis.Core.IO.GeoJson.GeographicPosition.</returns>
        public PointLocation ToPointLocation()
        {
            return PointLocation.fromWGS84(Latitude, Longitude);
        }

        /// <summary>
        /// Create a Pyxis.Core.IO.GeoJson.GeographicPosition from a latitude and longitude.
        /// </summary>
        /// <param name="latitude">The latitude to create the Pyxis.Core.IO.GeoJson.GeographicPosition with.</param>
        /// <param name="longitute">The longitude to create the Pyxis.Core.IO.GeoJson.GeographicPosition with.</param>
        /// <returns>The created Pyxis.Core.IO.GeoJson.GeographicPosition.</returns>
        public static GeographicPosition FromWgs84LatLon(double latitude, double longitute)
        {
            return new GeographicPosition(PointLocation.fromWGS84(latitude, longitute));
        }

        /// <summary>
        /// Create a Pyxis.Core.IO.GeoJson.GeographicPosition from a PYXIS index string.
        /// </summary>
        /// <param name="index">The PYXIS index string to create the Pyxis.Core.IO.GeoJson.GeographicPosition with.</param>
        /// <returns>The created Pyxis.Core.IO.GeoJson.GeographicPosition.</returns>
        /// <exception cref="System.ArgumentNullException">The index string is null or cannot be converted to a PYXIS index.</exception>
        public static GeographicPosition FromIndex(string index)
        {
            if (String.IsNullOrEmpty(index))
            {
                throw new ArgumentNullException("index", "can't create GeographicPosition from an empty index");
            }
            var icosIndex = new PYXIcosIndex(index);

            if (icosIndex.isNull())
            {
                throw new ArgumentNullException("index", "can't create GeographicPosition from an empty index");
            }

            return new GeographicPosition(PointLocation.fromPYXIndex(icosIndex));
        }

        /// <summary>
        /// Calculates the Pyxis.Core.Measurements.SphericalDistance from this position to another.
        /// </summary>
        /// <param name="other">The position to calculate the distance to.</param>
        /// <returns>The Pyxis.Core.Measurements.SphericalDistance to the other position.</returns>
        public SphericalDistance DistanceTo(GeographicPosition other)
        {
            return ToPointLocation().distance(other.ToPointLocation()) * SphericalDistance.Meter;
        }
    }

    /// <summary>
    /// A JsonConverter to serialize Pyxis.Core.IO.GeoJson.GeographicPosition with.
    /// </summary>
    public class GeographicPositionJsonConverter : JsonConverter
    {
        /// <summary>
        /// Determine if a System.Type can be converted using the converter.
        /// </summary>
        /// <param name="objectType">The System.Type to determine if conversion is possible.</param>
        /// <returns>true if the System.Type can be converted; otherwise, false.</returns>
        public override bool CanConvert(Type objectType)
        {
            return objectType == typeof(GeographicPosition);
        }

        /// <summary>
        /// Read JSON data into a System.Object boxing a Pyxis.Core.IO.GeoJson.GeographicPosition.
        /// </summary>
        /// <param name="reader">JSON reader.</param>
        /// <param name="objectType">System.Type of the object.</param>
        /// <param name="existingValue">The existing value of the object being read.</param>
        /// <param name="serializer">The calling JSON serializer.</param>
        /// <returns>A System.Object boxing the read Pyxis.Core.IO.GeoJson.GeographicPosition.</returns>
        /// <exception cref="System.Exception">Failed to deserialize the JSON data into a Pyxis.Core.IO.GeoJson.GeographicPosition.</exception>
        public override object ReadJson(JsonReader reader, Type objectType, object existingValue, JsonSerializer serializer)
        {
            var coords = serializer.Deserialize<JArray>(reader);

            if (coords == null || coords.Count < 2)
            {
                throw new Exception("failed to parse coordinate: " + coords.ToString()); 
            }

            return new GeographicPosition()
            {
                Longitude = (double)coords[0],
                Latitude = (double)coords[1]                
            };
        }

        /// <summary>
        /// Write the boxed Pyxis.Core.IO.GeoJson.GeographicPosition value to JSON.
        /// </summary>
        /// <param name="writer">JSON writer.</param>
        /// <param name="value">The boxed Pyxis.Core.IO.GeoJson.GeographicPosition.</param>
        /// <param name="serializer">The calling JSON serializer.</param>
        public override void WriteJson(JsonWriter writer, object value, JsonSerializer serializer)
        {
            var position = value as GeographicPosition;

            if (position != null)
            {
                var coords = new JArray(position.Longitude, position.Latitude);
                serializer.Serialize(writer, coords);
            }
            else
            {
                serializer.Serialize(writer, null);
            }
        }
    }
}
