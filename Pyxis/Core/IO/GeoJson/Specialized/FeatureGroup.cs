using System.Collections.Generic;
using Newtonsoft.Json;

namespace Pyxis.Core.IO.GeoJson.Specialized
{
    /// <summary>
    /// Represents a collection of features.
    /// </summary>
    public class FeatureGroup : GeoJsonObject
    {
        /// <summary>
        /// Gets or sets the collection of Pyxis.Core.IO.GeoJson.Feature.
        /// </summary>
        [JsonProperty("features")]
        public List<Feature> Features { get; set; }

        /// <summary>
        /// Gets or sets the collection of Pyxis.Core.IO.GeoJson.Feature.
        /// </summary>
        [JsonProperty("groups")]
        public List<Feature> Groups { get; set; }

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.Specialized.FeatureGroup.
        /// </summary>
        public FeatureGroup()
        {
            Type = GeoJsonObjectType.FeatureGroup;
        }
    }
}