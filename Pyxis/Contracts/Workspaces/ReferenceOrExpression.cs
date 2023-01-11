using System.Collections.Generic;
using Newtonsoft.Json;

namespace Pyxis.Contract.Workspaces
{
    /// <summary>
    /// ReferenceOrExpression allow to simply point to a resource or an expresion built on several resources.
    /// </summary>
    public class ReferenceOrExpression
    {
        [JsonProperty("reference", NullValueHandling = NullValueHandling.Ignore)]
        public string Reference { get; set; }

        [JsonProperty("expression", NullValueHandling = NullValueHandling.Ignore)]
        public string Expression { get; set; }

        [JsonProperty("symbols", NullValueHandling = NullValueHandling.Ignore)]
        public Dictionary<string, ReferenceOrExpression> Symbols { get; set; }
    }
}