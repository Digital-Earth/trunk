using System;
using System.Collections.Generic;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;

namespace Pyxis.Contract.Workspaces
{
    /// <summary>
    /// GlobeTemplate is a representation the state of the Application: loaded layers, camera location, selection etc
    /// </summary>
    public class GlobeTemplate
    {
        public class Layer : ReferenceOrExpression
        {
            [JsonProperty("active")]
            public bool Active { get; set; }

            [JsonProperty("name", NullValueHandling = NullValueHandling.Ignore)]
            public string Name { get; set; }

            [JsonProperty("style", NullValueHandling = NullValueHandling.Ignore)]
            public Style Style { get; set; }

            [JsonProperty("specification", NullValueHandling = NullValueHandling.Ignore)]
            public PipelineSpecification Specification { get; set; }
        }

        [JsonProperty("name")]
        public string Name { get; set; }

        [JsonProperty("camera")]
        public Camera Camera { get; set; }

        [JsonProperty("timestamp", NullValueHandling = NullValueHandling.Ignore)]
        public DateTime? Timestamp { get; set; }

        [JsonProperty("layers")]
        public List<Layer> Layers { get; set; }

        [JsonProperty("selection")]
        public dynamic Selection { get; set; }

        [JsonProperty("theme", NullValueHandling = NullValueHandling.Ignore)]
        public string Theme { get; set; }
    }
}
