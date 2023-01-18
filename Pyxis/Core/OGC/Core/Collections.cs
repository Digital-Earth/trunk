using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Core.IO.Core
{
    /// <summary>
    /// A geometric feature.
    /// </summary>
    public class Collections : Object
    {
        /// <summary>
        /// Gets or sets the links.
        /// </summary>
        [JsonProperty("links")]
        public Link[] Links { get; set; }

        /// <summary>
        /// Gets or sets the collections.
        /// </summary>
        [JsonProperty("collections")]
        public Collection[] Collection { get; set; }
    }
}
