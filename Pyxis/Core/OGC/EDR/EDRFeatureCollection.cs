using Newtonsoft.Json;
using Pyxis.Core.IO.Core;
using Pyxis.Core.IO.EDR;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Core.IO.GeoJson
{
    /// <summary>
    /// Represents a collection of EDR features.
    /// </summary>
    public class EDRFeatureCollection : GeoJsonObject
    {
        /// <summary>
        /// Gets or sets the collection of Pyxis.Core.IO.GeoJson.Feature.
        /// </summary>
        [JsonProperty("features")]
        public List<Feature> Features { get; set; }

        /// <summary>
        /// Gets or sets the collection of Pyxis.Core.IO.GeoJson.Feature.
        /// </summary>
        [JsonProperty("links", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public List<Link> Links { get; set; }

        /// <summary>
        /// Gets or sets the timestamp.
        /// </summary>
        [JsonProperty("timestamp", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public string Timestamp { get; set; }

        /// <summary>
        /// Gets or sets the number matched.
        /// </summary>
        [JsonProperty("numberMatched", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public int NumberMatched { get; set; }

        /// <summary>
        /// Gets or sets the number returned.
        /// </summary>
        [JsonProperty("numberReturned", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public int NumberReturned { get; set; }

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.FeatureCollection.
        /// </summary>
        public EDRFeatureCollection()
        {
            Type = GeoJsonObjectType.FeatureCollection;
        }

    }
}
