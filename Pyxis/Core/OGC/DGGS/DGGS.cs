using Newtonsoft.Json;
using Pyxis.Core.IO.Core;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Core.IO.DGGS
{
    /// <summary>
    /// A geometric feature.
    /// </summary>
    public class DGGS : Object
    {
        /// <summary>
        /// Gets or sets the id.
        /// </summary>
        [JsonProperty("id")]
        public string Id { get; set; }

        /// <summary>
        /// Gets or sets the title.
        /// </summary>
        [JsonProperty("title", NullValueHandling = NullValueHandling.Ignore)]
        public string Title { get; set; }

        /// <summary>
        /// Gets or sets the Description.
        /// </summary>
        [JsonProperty("description", NullValueHandling = NullValueHandling.Ignore)]
        public string Description { get; set; }

        /// <summary>
        /// Gets or sets the links.
        /// </summary>
        [JsonProperty("links")]
        public Link[] Links { get; set; }

        /// <summary>
        /// Gets or sets the extent.
        /// </summary>
        [JsonProperty("uri", NullValueHandling = NullValueHandling.Ignore)]
        public string Uri { get; set; }

        /// <summary>
        /// Gets or sets the crs.
        /// </summary>
        [JsonProperty("crs", NullValueHandling = NullValueHandling.Ignore)]
        public string Crs { get; set; }

    }
}
