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
    public class DataQueries : Object
    {
        /// <summary>
        /// Gets or sets the position.
        /// </summary>
        [JsonProperty("position", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public Query Position { get; set; }

        /// <summary>
        /// Gets or sets the radius.
        /// </summary>
        [JsonProperty("radius", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public Query Radius { get; set; }

        /// <summary>
        /// Gets or sets the area.
        /// </summary>
        [JsonProperty("area", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public Query Area { get; set; }

        /// <summary>
        /// Gets or sets the cube.
        /// </summary>
        [JsonProperty("cube", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public Query Cube { get; set; }

        /// <summary>
        /// Gets or sets the trajectory.
        /// </summary>
        [JsonProperty("trajectory", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public Query Trajectory { get; set; }

        /// <summary>
        /// Gets or sets the corridor.
        /// </summary>
        [JsonProperty("corridor", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public Query Corridor { get; set; }

        /// <summary>
        /// Gets or sets the item.
        /// </summary>
        [JsonProperty("item", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public Query Item { get; set; }

        /// <summary>
        /// Gets or sets the location.
        /// </summary>
        [JsonProperty("location", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public Query Location { get; set; }

    }
}
