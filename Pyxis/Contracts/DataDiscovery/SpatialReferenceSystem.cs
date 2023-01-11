using System;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace Pyxis.Contract.DataDiscovery
{
    /// <summary>
    /// SpatialReferenceSystem is used to specify which projection to use when importing legacy GIS files.
    /// </summary>
    public class SpatialReferenceSystem
    {
        /// <summary>
        /// Define if the SpatialReferenceSystem is projected or not.
        /// </summary>
        public enum Systems 
        {
            /// <summary>
            /// A projected coordinate system such as universal transverse Mercator (UTM), Albers Equal Area, or Robinson, all of which (along with numerous other map projection models) provide various mechanisms to project maps of the earth's spherical surface onto a two-dimensional Cartesian coordinate plane. 
            /// </summary>
            Projected,
            /// <summary>
            /// A global or spherical coordinate system such as latitude-longitude. These are often referred to as geographic coordinate systems.
            /// </summary>
            Geographical,
        }

        /// <summary>
        /// Common Datums
        /// </summary>
        public enum Datums 
        {
            /// <summary>North American Datum of 1927</summary>
            NAD27,
            /// <summary>North American Datum of 1983</summary>
            NAD83,
            /// <summary>World Geodetic System of 1972</summary>
            WGS72,
            /// <summary>World Geodetic System of 1984</summary>
            WGS84
        };

        /// <summary>
        /// Common Projections
        /// </summary>
	    public enum Projections
        {
            /// <summary>
            /// Universal Transverse Mercator
            /// </summary>
            UTM,

            /// <summary>
            /// Custom projection defined using CustomProjection string.
            /// </summary>
            Custom
        };

        /// <summary>
        /// Get or set if the SpatialReferenceSystem is projected or not.
        /// </summary>
        [JsonConverter(typeof(StringEnumConverter))]
        public Systems CoordinateSystem { get; set; }

        /// <summary>
        /// Get or set the Datum the SpatialReferenceSystem is using.
        /// </summary>
        [JsonConverter(typeof(StringEnumConverter))]
        public Datums Datum { get; set; }

        /// <summary>
        /// Get or set the Projection the SpatialReferenceSystem is using.
        /// </summary>
        [JsonConverter(typeof(StringEnumConverter))]
        public Projections? Projection { get; set;}

        /// <summary>
        /// Get or set the custom projection string. see GDAL supported formats.
        /// </summary>
        public string CustomProjection { get; set; }

        /// <summary>
        /// Get or set if the UTM projection is North or South. Relevant only if Projection is UTM.
        /// </summary>
        public bool? UtmNorth { get; set; }

        /// <summary>
        /// Get or set the zone of the UTM projection. Relevant only if Projection is UTM.
        /// </summary>
        public int? UtmZone { get; set; }

        /// <summary>
        /// Create SpatialReferenceSystem using a custom projection string.
        /// </summary>
        /// <param name="custom">Custom projection string</param>
        /// <returns>SpatialReferenceSystem object</returns>
        public static SpatialReferenceSystem CreateFromWKT(string custom) {
            return new SpatialReferenceSystem() {
                CoordinateSystem = Systems.Projected,
                Projection = Projections.Custom,
                CustomProjection = custom,
            };
        }

        /// <summary>
        /// Get default WGS84 Datum projection
        /// </summary>
        public static SpatialReferenceSystem WGS84 
        {
            get
            {
                return CreateGeographical(Datums.WGS84);
            }
        }

        /// <summary>
        /// Create SpatialReferenceSystem using a common datum
        /// </summary>
        /// <param name="datum">Datum to use</param>
        /// <returns>SpatialReferenceSystem object</returns>
        public static SpatialReferenceSystem CreateGeographical(Datums datum)
        {
            return new SpatialReferenceSystem()
            {
                CoordinateSystem = Systems.Geographical,
                Datum = datum
            };
        }

        /// <summary>
        /// Create UTM base SpatialReferenceSystem 
        /// </summary>
        /// <param name="datum">which Datum to use</param>
        /// <param name="north">true if UTM is North hemisphere, false is South hemisphere.</param>
        /// <param name="zone">UTM zone number (1..60)</param>
        /// <returns>SpatialReferenceSystem object</returns>        
        /// <exception cref="System.ArgumentOutOfRangeException">If zone is not out of range.</exception>
        public static SpatialReferenceSystem CreateUtm(Datums datum, bool north, int zone)
        {
            if (zone < 1 && zone > 60)
            {
                throw new ArgumentOutOfRangeException("zone", "UTM zone is between 1 and 60");
            }

            return new SpatialReferenceSystem()
            {
                CoordinateSystem = Systems.Projected,
                Datum = datum,
                Projection = Projections.UTM,
                UtmNorth = north,
                UtmZone = zone,
            };
        }

        /// <summary>
        /// Create UTM north base SpatialReferenceSystem 
        /// </summary>
        /// <param name="datum">which Datum to use</param>
        /// <param name="zone">UTM zone number (1..60)</param>
        /// <returns>SpatialReferenceSystem object</returns>    
        public static SpatialReferenceSystem CreateUtmNorth(Datums datum, int zone)
        {
            return CreateUtm(datum, true, zone);
        }

        /// <summary>
        /// Create UTM south base SpatialReferenceSystem 
        /// </summary>
        /// <param name="datum">which Datum to use</param>
        /// <param name="zone">UTM zone number (1..60)</param>
        /// <returns>SpatialReferenceSystem object</returns>    
        public static SpatialReferenceSystem CreateUtmSouth(Datums datum, int zone)
        {
            return CreateUtm(datum, false, zone);
        }

        /// <summary>
        /// Create a clone of the spatial reference system.
        /// </summary>
        /// <returns>The clone.</returns>
        public SpatialReferenceSystem Clone() 
        {
            return new SpatialReferenceSystem
            {
                CoordinateSystem = this.CoordinateSystem,
                Datum = this.Datum,
                Projection = this.Projection,
                UtmNorth = this.UtmNorth,
                UtmZone = this.UtmZone,
                CustomProjection = this.CustomProjection
            };
        }
    }
}
