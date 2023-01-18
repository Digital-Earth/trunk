using Newtonsoft.Json;
using Pyxis.Core.IO.Core;
using System;
namespace Pyxis.Core.IO.DGGS
{
    /// <summary>
    /// A geometric feature.
    /// </summary>
    public class DGGSs : Object
    {
        /// <summary>
        /// Gets or sets the links.
        /// </summary>
        [JsonProperty("links")]
        public Link[] Links { get; set; }

        /// <summary>
        /// Gets or sets the collections.
        /// </summary>
        [JsonProperty("dggs")]
        public DGGS[] Dggs { get; set; }
    }
}
