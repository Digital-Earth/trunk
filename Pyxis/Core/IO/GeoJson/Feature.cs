using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Core.IO.GeoJson
{
    /// <summary>
    /// A geometric feature.
    /// </summary>
    public class Feature : GeoJsonObject
    {
        /// <summary>
        /// Gets or sets the Pyxis.Core.IO.GeoJson.Feature identifier.
        /// </summary>
        [JsonProperty("id")]
        public string Id { get; set; }

        /// <summary>
        /// Gets or sets the properties of the Pyxis.Core.IO.GeoJson.Feature.
        /// </summary>
        [JsonProperty("properties")]
        public Dictionary<string, object> Properties { get; set; }

        /// <summary>
        /// Gets or sets the Pyxis.Core.IO.GeoJson.Geometry associated with the Pyxis.Core.IO.GeoJson.Feature.
        /// </summary>
        [JsonProperty("geometry")]
        public Geometry Geometry { get; set; }

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.Feature.
        /// </summary>
        public Feature()
        {
            Type = GeoJsonObjectType.Feature;
            Properties = new Dictionary<string, object>();
        }

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.Feature from an identifier, Pyxis.Core.IO.GeoJson.Geometry and properties.
        /// </summary>
        /// <param name="id">Identifier used to initialize the Pyxis.Core.IO.GeoJson.Feature.</param>
        /// <param name="geometry">Geometry used to initialize the Pyxis.Core.IO.GeoJson.Feature.</param>
        /// <param name="properties">Properties used to initialize the Pyxis.Core.IO.GeoJson.Feature.</param>
        public Feature(string id, Geometry geometry, Dictionary<string, object> properties)
        {
            Type = GeoJsonObjectType.Feature;
            Id = id;
            Geometry = geometry;
            Properties = properties ?? new Dictionary<string, object>();
        }

        /// <summary>
        /// Create a Pyxis.Core.IO.GeoJson.Feature from a PYXIS IFeature.
        /// </summary>
        /// <param name="pyxFeature">The PYXIS IFeature to use to create the Pyxis.Core.IO.GeoJson.Feature.</param>
        /// <param name="extractionFlags">Define which parts of the feature to extract.</param>
        /// <returns>The created Pyxis.Core.IO.GeoJson.Feature.</returns>
        public static Feature FromIFeature(IFeature_SPtr pyxFeature, FeatureExtractionFlags extractionFlags = FeatureExtractionFlags.All)
        {
            var properties = new Dictionary<string, object>();

            if (extractionFlags.HasFlag(FeatureExtractionFlags.Fields))
            {
                foreach (var field in pyxFeature.getDefinition().FieldDefinitions)
                {
                    var name = field.getName();
                    var value = pyxFeature.getFieldValueByName(name);
                    properties[name] = value.ToDotNetObject();
                }
            }

            Geometry geoJsonGeometry = null;
            if (extractionFlags.HasFlag(FeatureExtractionFlags.Geometry))
            {
                var geometry = pyxFeature.getGeometry();            
                geoJsonGeometry = Geometry.FromPYXGeometry(geometry);
            }

            return new Feature(pyxFeature.getID(), geoJsonGeometry, properties);
        }
    }
}
