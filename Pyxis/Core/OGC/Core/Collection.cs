using Newtonsoft.Json;
using Pyxis.Core.IO.EDR;
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
    public class Collection : Object
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
        [JsonProperty("extent")]
        public Extent Extent { get; set; }

        /// <summary>
        /// Gets or sets the parameter names.
        /// </summary>
        [JsonProperty("parameter_names", NullValueHandling = NullValueHandling.Ignore)]
        public Object Parameter_names { get; set; }

        /// <summary>
        /// Gets or sets the data queries.
        /// </summary>
        [JsonProperty("data_queries", NullValueHandling = NullValueHandling.Ignore)]
        public DataQueries Data_queries { get; set; }
    }
}
