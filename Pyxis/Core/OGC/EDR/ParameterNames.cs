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
    public class ParameterNames : Object
    {
        /// <summary>
        /// Gets or sets the id.
        /// </summary>
        [JsonProperty("id")]
        public string Id { get; set; }

        /// <summary>
        /// Gets or sets the observed property.
        /// </summary>
        [JsonProperty("observedProperty")]
        public ObservedProperty ObservedProperty { get; set; }

        /// <summary>
        /// Gets or sets the rel.
        /// </summary>
        [JsonProperty("type")]
        public string Type { get; set; }

        /// <summary>
        /// Gets or sets the label.
        /// </summary>
        [JsonProperty("label", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public string Label { get; set; }

        /// <summary>
        /// Gets or sets the description.
        /// </summary>
        [JsonProperty("description", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public string Description { get; set; }

        /// <summary>
        /// Gets or sets the data-type.
        /// </summary>
        [JsonProperty("data_type", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public string Data_type { get; set; }

        /// <summary>
        /// Gets or sets the unit.
        /// </summary>
        [JsonProperty("unit", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public Unit Unit { get; set; }

        /// <summary>
        /// Gets or sets the extent.
        /// </summary>
        [JsonProperty("extent", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public Extent Extent { get; set; }

        /// <summary>
        /// Gets or sets the measurementType.
        /// </summary>
        [JsonProperty("measurementType", DefaultValueHandling = DefaultValueHandling.Ignore)]
        public MeasurementType MeasurementType { get; set; }
    }
}
