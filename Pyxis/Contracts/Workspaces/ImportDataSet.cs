using System.Collections.Generic;
using Newtonsoft.Json;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;

namespace Pyxis.Contract.Workspaces
{
    /// <summary>
    /// A simple dataset import based on a single Url.
    /// </summary>
    public class ImportDataSet : IImport
    {
        public string Type
        {
            get { return "DataSet"; }
        }

        public string Uri { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public string InternalPath { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public string Layer { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public Style Style { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public PipelineSpecification Specification { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public string Srs { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public GeoTagMethods GeoTag { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public string Sampler { get; set; }

        public DataSet Resolve(Dictionary<string, string> domainsValues)
        {
            return new DataSet()
            {
                Uri = Uri,
                InternalPath = InternalPath,
                Layer = Layer,
                Specification = Specification
            };
        }
    }
}