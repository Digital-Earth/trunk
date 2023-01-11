using Newtonsoft.Json;
using Pyxis.Contract.Publishing;

namespace Pyxis.Contract.DataDiscovery
{
    /// <summary>
    /// DataSetWithResource allow to represt a GeoSource that was created from this result
    /// </summary>
    public class DataSetWithResource : DataSet
    {
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public ResourceReference Resource { get; set; }
    }
}