using Newtonsoft.Json;

namespace Pyxis.Contract.Workspaces
{
    public class OnwerDetails
    {
        [JsonProperty("name")]
        public string Name { get; set; }

        [JsonProperty("id")]
        public string Id { get; set; }
    }
}