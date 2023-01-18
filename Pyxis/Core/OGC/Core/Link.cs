using Newtonsoft.Json;
using Pyxis.Core.IO.EDR;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Core.IO.Core
{
    /// <summary>
    /// A geometric feature.
    /// </summary>
    public class Link : Object
    {
        /// <summary>
        /// Gets or sets the href.
        /// </summary>
        [JsonProperty("href")]
        public string Href { get; set; }

        /// <summary>
        /// Gets or sets the rel.
        /// </summary>
        [JsonProperty("rel")]
        public string Rel { get; set; }

        /// <summary>
        /// Gets or sets the type.
        /// </summary>
        [JsonProperty("type", DefaultValueHandling=DefaultValueHandling.Ignore)]
        public string Type { get; set; }
        
        /// <summary>
        /// Gets or sets the hreflang.
        /// </summary>
        [JsonProperty("hreflang", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public string Hreflang { get; set; }
        
        /// <summary>
        /// Gets or sets the title.
        /// </summary>
        [JsonProperty("title", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public string Title { get; set; }
       
        /// <summary>
        /// Gets or sets the length.
        /// </summary>
        [JsonProperty("length", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public string Length { get; set; }
        
        /// <summary>
        /// Gets or sets the templated.
        /// </summary>
        [JsonProperty("templated", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public bool Templated { get; set; }
        
        /// <summary>
        /// Gets or sets the variables.
        /// </summary>
        [JsonProperty("variables", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public Variables Variables { get; set; }


    }
}
