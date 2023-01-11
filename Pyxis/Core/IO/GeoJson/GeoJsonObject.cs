using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Core.IO.GeoJson
{
    /// <summary>
    /// Represents an geometry object that can be serialized to JSON.
    /// </summary>
    /// <remarks>
    /// Coordinate reference system (crs) and bounding box (bbox) fields in the GeoJSON specification are not supported.
    /// By default WGS84 is used as the default coordinate reference system.
    /// </remarks>
    public class GeoJsonObject
    {
        /// <summary>
        /// Gets the Pyxis.Core.IO.GeoJson.GeoJsonObjectType of the object.
        /// </summary>
        [JsonConverter(typeof(StringEnumConverter))]
        [JsonProperty("type")]
        public GeoJsonObjectType Type { get; protected set; }

        //CRS

        //BBOX
    }

    /// <summary>
    /// Defines the type of a Pyxis.Core.IO.GeoJson.GeoJsonObject
    /// </summary>
    public enum GeoJsonObjectType
    {
        /// <summary>
        /// Specifies a point geometry.
        /// </summary>
        Point,
        /// <summary>
        /// Specifies a multi-point geometry.
        /// </summary>
        MultiPoint,
        /// <summary>
        /// Specifies a line geometry.
        /// </summary>
        LineString,
        /// <summary>
        /// Specifies a multi-line geometry.
        /// </summary>
        MultiLineString,
        /// <summary>
        /// Specifies a polygon geometry.
        /// </summary>
        Polygon,
        /// <summary>
        /// Specifies a multi-polygon geometry.
        /// </summary>
        MultiPolygon,
        /// <summary>
        /// Specifies a geometry collection.
        /// </summary>
        GeometryCollection,

        /// <summary>
        /// Specifies a feature.
        /// </summary>
        Feature,
        /// <summary>
        /// Specifies a feature collection.
        /// </summary>
        FeatureCollection,

        //specialized geometry types
        /// <summary>
        /// Specifies a circle geometry (specialized, non-standard GeoJSON type).
        /// </summary>
        Circle,
        /// <summary>
        /// Specifies a projected bbox geometry (specialized, non-standard GeoJSON type).
        /// </summary>
        BBox,

        /// <summary>
        /// Specifies a cell geometry (specialized, non-standard GeoJSON type).
        /// </summary>
        PYXCell,
        // <summary>
        // Specifies a tile geometry (specialized, non-standard GeoJSON type).
        // </summary>
        //PYXTile,
        /// <summary>
        /// Specifies a tile collection geometry (specialized, non-standard GeoJSON type).
        /// </summary>
        PYXTileCollection,

        /// <summary>
        /// Specifies a feature reference geometry (specialized, non-standard GeoJSON type).
        /// </summary>
        FeatureRef,

        /// <summary>
        /// Specifies a condition based geometry (specialized, non-standard GeoJSON type).
        /// </summary>
        Condition,

        /// <summary>
        /// Specifies a boolean operation based geometry (specialized, non-standard GeoJSON type).
        /// </summary>
        Boolean,

        /// <summary>
        /// Specifies a feature group
        /// </summary>
        FeatureGroup
    }
}
