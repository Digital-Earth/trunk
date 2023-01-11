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
    /// A Pyxis.Core.IO.GeoJson.Geometry representing a cell.
    /// This is implements a specialized, non-standard GeoJSON geometry.
    /// </summary>
    public class CellGeometry : Geometry
    {
        /// <summary>
        /// Gets or sets the PYXIS index.
        /// </summary>
        [JsonProperty("index")]
        public string Index { get; set; }

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.Specialized.CellGeometry.
        /// </summary>
        public CellGeometry()
        {
            Type = GeoJsonObjectType.PYXCell;
        }

        internal CellGeometry(JObject json) : this()
        {
            ValidateType(json);
            Index = json["index"].ToObject<string>();
        }

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.Specialized.CellGeometry using a PYXIS index.
        /// </summary>
        /// <param name="index">The PYXIS index to initialize the cell with.</param>
        public CellGeometry(PYXIcosIndex index) : this()
        {
            Index = index.toString();
        }
        
        /// <summary>
        /// Convert the Pyxis.Core.IO.GeoJson.Specialized.CellGeometry to a PYXIS geometry.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to perform the conversion with.</param>
        /// <param name="resolution">resolution is been ingnored by this implementation</param>
        /// <returns>The PYXIS geometry created by converting the Pyxis.Core.IO.GeoJson.Specialized.CellGeometry.</returns>
        public override PYXGeometry_SPtr ToPyxGeometry(Engine engine, int resolution = -1)
        {
            return pyxlib.DynamicPointerCast_PYXGeometry(PYXCell.create(new PYXIcosIndex(Index)));
        }

        internal override JObject ToJObject()
        {
            var json = new JObject();
            json.Add("type", (JToken)Type.ToString());
            json.Add("index", (JToken)Index);
            return json;
        }

        /// <summary>
        /// Create a Pyxis.Core.IO.GeoJson.GeographicPosition corresponding to the center of the cell.
        /// </summary>
        /// <returns>A Pyxis.Core.IO.GeoJson.GeographicPosition corresponding to the center of the cell.</returns>
        public GeographicPosition AsGeographicPosition()
        {
            return GeographicPosition.FromIndex(Index);
        }
    }
}
