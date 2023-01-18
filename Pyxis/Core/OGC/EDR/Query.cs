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
    public class Query : Object
    {
        /// <summary>
        /// Gets or sets the links.
        /// </summary>
        [JsonProperty("link")]
        public Link Link { get; set; }

    }
}
