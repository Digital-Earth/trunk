using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Core.IO.Core
{
    /// <summary>
    /// A geometric Extent.
    /// </summary>
    public class Extent : Object
    {
        /// <summary>
        /// Gets or sets the properties of the extent.
        /// </summary>
        [JsonProperty("spatial")]
        public SpatialExtent spatial { get; set; }
    }

    public class SpatialExtent : Object
    {
        /// <summary>
        /// Gets or sets the properties of the extent.
        /// </summary>
        [JsonProperty("bbox")]
        public double[] Bbox { get; set; }

        /// <summary>
        /// Gets or sets the properties of the extent.
        /// </summary>
        [JsonProperty("crs")]
        public string Crs { get; set; }
    }
}
