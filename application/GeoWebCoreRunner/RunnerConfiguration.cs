using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GeoWebCoreRunner
{
    public class RunnerConfiguration
    {
        public string GwcUser { get; set; }
        public string GwcKey { get; set; }

        public class NodeInfo
        {
            public List<string> Tags { get; set; }
            public Guid NodeId { get; set; }
            public string Url { get; set; }
        }

        public int MasterNodes { get; set; }
        public int ServersNodes { get; set; }
        public int ImportNodes { get; set; }

        public List<NodeInfo> Nodes { get; set; }

        public class EnvironmentInfo
        {
            public string ApiUrl { get; set; }
            public string GwcDir { get; set; }
            public string GwcFileName { get; set; }
            public string CacheFolder { get; set; }
            public string FilesFolder { get; set; }
            
            /// <summary>
            /// Memory limit that will cause GWC Runner to restart an instance if idle long enough
            /// 
            /// TODO: change into json conditions: Alerts: [ { Trigger: { Memory: { gt: 1500 }, Idle: { gt: 5 } }, Action: "restart", Log: "Memory exceeded limits" } , ... ]
            /// </summary>
            public long MemoryLimitMB { get; set; }


            /// <summary>
            /// Define if the GWC should consider it self as production server
            /// </summary>
            public bool Production { get; set; }
        }

        public EnvironmentInfo Environment { get; set; }

        public RunnerConfiguration()
        {
        }

        public RunnerConfiguration(string host, int startPort, int endPort)
        {
            Environment = new EnvironmentInfo()
            {
                ApiUrl = Pyxis.Publishing.ApiUrl.ProductionLicenseServerRestAPI,
                MemoryLimitMB = 1500,
                Production = false,
            };

            ServersNodes = 0;
            Nodes = new List<NodeInfo>();

            var middle = (startPort + endPort) / 2;
            for(var port = startPort; port <= endPort; port++)
            {
                //create half of nodes as server and half as tasks
                Nodes.Add(new NodeInfo()
                {
                    NodeId = Guid.NewGuid(),
                    Url = host + ":" + port,
                    Tags = new List<string>{ port < middle ? "Server" : "Task" }
                });
            }
        }
    }
}
