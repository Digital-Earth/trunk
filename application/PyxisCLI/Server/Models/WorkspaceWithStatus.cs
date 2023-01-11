using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Pyxis.Contract.Workspaces;

namespace PyxisCLI.Server.Models
{
    public class WorkspaceWithStatus
    {
        public class ItemAndStatus
        {
            [JsonProperty("item")]
            public object Item { get; set; }
            
            [JsonProperty("status")]
            public Dictionary<string, object> Status { get; set; }
        }

        [JsonProperty("endpoints")]
        public Dictionary<string, ItemAndStatus> Endpoints;

        [JsonProperty("imports")]
        public Dictionary<string, ItemAndStatus> Imports;

        [JsonProperty("globes")]
        public Dictionary<string, ItemAndStatus> Globes;
    }
}
