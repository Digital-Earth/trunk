using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Pyxis.Utilities;
using Quobject.SocketIoClientDotNet.Client;

namespace GeoWebCore.Services.Cluster
{
    public class Cluster
    {
        public HashRing ServersRing { get; set; }
        public HashRing ImportRing { get; set; }
        public HashRing SearchRing { get; set; }
        public HashSet<string> LocalHost { get; set; }

        
        public Cluster(string clusterUri,IEnumerable<string> localHost)
        {
            LocalHost = new HashSet<string>(FixAddresses(localHost));
            foreach (var host in LocalHost)
            {
                Console.WriteLine("localhost: " + host);
            }
            
            ServersRing = new HashRing();
            SearchRing = new HashRing();
            ImportRing = new HashRing();

            if (!String.IsNullOrEmpty(clusterUri))
            {
                var ioEndpoint = clusterUri + "/endpoint";
                Console.WriteLine("cluster uri: " + ioEndpoint);
                var socket = IO.Socket(ioEndpoint);

                socket.On(Socket.EVENT_CONNECT, () =>
                {
                    Console.WriteLine("Connected to cluster: " + ioEndpoint);
                });

                socket.On(Socket.EVENT_DISCONNECT, () =>
                {
                    Console.WriteLine("Disconnected to cluster: " + ioEndpoint);
                });

                socket.On("endpoints", (data) =>
                {
                    var jobject = data as JObject;
                    var endpoints = jobject.ToObject<Dictionary<string, Dictionary<string, List<string>>>>();

                    if (endpoints.ContainsKey("server"))
                    {
                        UpdateRing(ServersRing, "server", endpoints["server"]);
                    }
                    if (endpoints.ContainsKey("import"))
                    {
                        UpdateRing(ImportRing, "import", endpoints["import"]);
                    }
                    if (endpoints.ContainsKey("search"))
                    {
                        UpdateRing(ServersRing, "search", endpoints["search"]);
                    }
                });
            }
        }

        private void UpdateRing(HashRing ring, string service, Dictionary<string, List<string>> serviceApi)
        {
            if (serviceApi.ContainsKey("api"))
            {
                Console.WriteLine("{0} ring: [{1}]", service, string.Join(", ", serviceApi["api"]));
                ring.SetEndpoints(serviceApi["api"]);
            }
            else
            {
                Console.WriteLine("{0} ring: [{1}]", service, "");
                ring.SetEndpoints(new List<string>());
            }
        }

        /// <summary>
        /// Fix http://*:port address to be valid http host. for example http://localhost:port
        /// </summary>
        /// <param name="localHost">list of addresses to fix</param>
        /// <returns>list of fixed addresses</returns>
        private IEnumerable<string> FixAddresses(IEnumerable<string> localHost)
        {
            foreach (var host in localHost)
            {
                if (host.StartsWith("http://*:"))
                {
                    yield return host.Replace("http://*:", "http://localhost:");
                }
                else
                {
                    yield return host;
                }
            }
        }

        public bool IsLocal(HashRing ring, string host)
        {
            return ring.Count == 0 || LocalHost.Contains(host);
        }

        public string GetEndpointForGeometry(string geomtryHash)
        {
            return ServersRing.GetEndpoint(geomtryHash);
        }

        public string GetEndpointForGeoSource(Guid geoSourceId)
        {
            return GetEndpointForGeoSource(geoSourceId.ToString());
        }

        public string GetEndpointForGeoSource(string geoSourceId)
        {
            return ServersRing.GetEndpoint(geoSourceId.ToString());
        }

        public GeometryClusterResolver ResolveGeometry(string geomtryHash)
        {
            return new GeometryClusterResolver(this, geomtryHash);
        }

        public GeoSourceClusterResolver ResolveGeoSource(Guid geoSourceId)
        {
            return new GeoSourceClusterResolver(this, geoSourceId);
        }

        public async Task<List<T>> Broadcast<T>(HashRing ring, string path)
        {
            var result = new List<T>();
            using (var webClient = new WebClient())
            {
                foreach (var endpoint in ring.Endpoints.ToList())
                {
                    if (!IsLocal(ring, endpoint))
                    {
                        result.Add(JsonConvert.DeserializeObject<T>(await webClient.DownloadStringTaskAsync(endpoint.TrimEnd('/') + "/" + path.TrimStart('/'))));    
                    }
                }
            }
            return result;
        }
    }
}