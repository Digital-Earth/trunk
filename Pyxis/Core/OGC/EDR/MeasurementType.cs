using Newtonsoft.Json;
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
    public class MeasurementType : Object
    {
        /// <summary>
        /// Gets or sets the method.
        /// </summary>
        [JsonProperty("method")]
        public string Method { get; set; }

        /// <summary>
        /// Gets or sets the duration.
        /// </summary>
        [JsonProperty("duration")]
        public string Duration { get; set; }

    }
}
