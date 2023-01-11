using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Core.IO.GeoJson.Specialized
{
    /// <summary>
    /// A Pyxis.Core.IO.GeoJson.Geometry representing a collection of tiles.
    /// This is implements a specialized, non-standard GeoJSON geometry.
    /// </summary>
    public class TileCollectionGeometry : Geometry
    {
        /// <summary>
        /// Gets or sets a base 64 encoding of the tile collection.
        /// </summary>
        [JsonProperty("base64")]
        public string Base64 { get; set; }

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.Specialized.TileCollectionGeometry.
        /// </summary>
        public TileCollectionGeometry()
        {
            Type = GeoJsonObjectType.PYXTileCollection;
        }

        internal TileCollectionGeometry(JObject json) : this()
        {
            ValidateType(json);
            Base64 = json["base64"].ToObject<string>();
        }

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.Specialized.TileCollectionGeometry from the specified PYXIS tile collection.
        /// </summary>
        /// <param name="tileCollection">The PYXIS tile collection from with to create the instance of Pyxis.Core.IO.GeoJson.Specialized.TileCollectionGeometry.</param>
        public TileCollectionGeometry(PYXTileCollection_SPtr tileCollection)
            : this()
        {
            Base64 = PYXGeometrySerializer.serializeToBase64(tileCollection.__deref__());
        }

        /// <summary>
        /// Convert the Pyxis.Core.IO.GeoJson.Specialized.TileCollectionGeometry to a PYXIS geometry.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to perform the conversion with.</param>
        /// <param name="resolution">resolution is been ingnored by this implementation</param>
        /// <returns>The PYXIS geometry created by converting the Pyxis.Core.IO.GeoJson.Specialized.TileCollectionGeometry.</returns>
        public override PYXGeometry_SPtr ToPyxGeometry(Engine engine, int resolution = -1)
        {
            return PYXGeometrySerializer.deserializeFromBase64(Base64);
        }

        internal override JObject ToJObject()
        {
            var json = new JObject();
            json.Add("type", (JToken)Type.ToString());
            json.Add("base64", (JToken)Base64);
            return json;
        }
    }
}
