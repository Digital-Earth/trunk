using Newtonsoft.Json;
using Pyxis.Core.IO.Core;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Core.IO.EDR
{
    /// <summary>
    /// A geometric feature.
    /// </summary>
    public class Variables : Object
    {
        /// <summary>
        /// Gets or sets the title.
        /// </summary>
        [JsonProperty("title", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public string Title { get; set; }

        /// <summary>
        /// Gets or sets the description.
        /// </summary>
        [JsonProperty("description", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public string Description { get; set; }

        /// <summary>
        /// Gets or sets the query_type.
        /// </summary>
        [JsonProperty("query_type", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public string Query_type { get; set; }

        /// <summary>
        /// Gets or sets the coords.
        /// </summary>
        [JsonProperty("coords", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public string Coords { get; set; }

        /// <summary>
        /// Gets or sets the within_units.
        /// </summary>
        [JsonProperty("within_units", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public string[] Within_units { get; set; }

        /// <summary>
        /// Gets or sets the width_units.
        /// </summary>
        [JsonProperty("width_units", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public string[] Width_units { get; set; }

        /// <summary>
        /// Gets or sets the height_units.
        /// </summary>
        [JsonProperty("height_units", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public string[] Height_units { get; set; }

        /// <summary>
        /// Gets or sets the output_formats.
        /// </summary>
        [JsonProperty("output_formats", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public string[] Output_formats { get; set; }

        /// <summary>
        /// Gets or sets the default output_formats.
        /// </summary>
        [JsonProperty("default_output_format", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public string Default_output_formats { get; set; }

        /// <summary>
        /// Gets or sets the crs.
        /// </summary>
        [JsonProperty("crs_details", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public CrsDetails Crs_details { get; set; }

    }
}
