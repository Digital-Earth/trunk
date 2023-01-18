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
    public class CrsDetails : Object
    {
        /// <summary>
        /// Gets or sets the crs.
        /// </summary>
        [JsonProperty("crs")]
        public string Crs { get; set; }
        
        /// <summary>
        /// Gets or sets the wkt.
        /// </summary>
        [JsonProperty("wkt")]
        public string Wkt { get; set; }

    }
}
