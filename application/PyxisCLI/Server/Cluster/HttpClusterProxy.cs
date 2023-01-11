using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using ApplicationUtility;
using Nest;
using Newtonsoft.Json;
using PyxisCLI.State;

namespace PyxisCLI.Server.Cluster
{
    public class HttpClusterProxy
    {
        public Cluster Cluster { get; set; }
        public string AuthorizationToken { get; set; }

        public HttpClusterProxy(Cluster cluster, string token = null)
        {
            Cluster = cluster;
            AuthorizationToken = token;
        }

        private WebClient CreateWebClient()
        {
            var webClient = new WebClient();
            if (AuthorizationToken.HasContent())
            {
                webClient.Headers[HttpRequestHeader.Authorization] = "Bearer " + AuthorizationToken;
            }
            return webClient;
        }

        public async Task<T> Forward<T>(HashRing ring, string key, string path)
        {
            using (var webClient = CreateWebClient())
            {
                var endpoint = ring.GetEndpoint(key);
                return
                    JsonConvert.DeserializeObject<T>(
                        await webClient.DownloadStringTaskAsync(endpoint.TrimEnd('/') + "/" + path.TrimStart('/')));
            }
        }

        public async Task<T> Forward<T>(HashRing ring, string key, string path, object body)
        {
            using (var webClient = CreateWebClient())
            {
                var endpoint = ring.GetEndpoint(key);
                webClient.Headers[HttpRequestHeader.ContentType] = "application/json";
                return
                    JsonConvert.DeserializeObject<T>(
                        await webClient.UploadStringTaskAsync(endpoint.TrimEnd('/') + "/" + path.TrimStart('/'), JsonConvert.SerializeObject(body)));
            }
        }

        public async Task<List<T>> Broadcast<T>(HashRing ring, string path)
        {
            var result = new List<T>();
            using (var webClient = CreateWebClient())
            {
                foreach (var endpoint in ring.Endpoints.ToList())
                {
                    if (!Cluster.IsLocal(ring, endpoint))
                    {
                        result.Add(JsonConvert.DeserializeObject<T>(await webClient.DownloadStringTaskAsync(endpoint.TrimEnd('/') + "/" + path.TrimStart('/'))));
                    }
                }
            }
            return result;
        }

        /// <summary>
        /// Start a Discovery task on a cluster
        /// </summary>
        /// <param name="reference">endpoint reference</param>
        /// <returns>RemoteTask to track the progress of the discovery</returns>
        public RemoteTask StartDiscoverTask(string reference)
        {
            using (var webClient = CreateWebClient())
            {
                var newDiscoverTaskUrl = Cluster.Uri + "/_cluster/jobs/" +
                                         ClusterConfiguration.JobId +
                                         "/discover?reference=" +
                                         System.Uri.EscapeDataString(reference);

                var newTaskJson = webClient.DownloadString(newDiscoverTaskUrl);

                var taskDetails = JsonConvert.DeserializeObject<RemoteTask.RemoteTaskStatus>(newTaskJson);

                return new RemoteTask(taskDetails);
            }
        }

        /// <summary>
        /// Start an import reference task on a cluster
        /// </summary>
        /// <param name="reference">import reference</param>
        /// <returns>RemoteTask to track the progress of the import process</returns>
        public RemoteTask StartImportTask(string reference)
        {
            using (var webClient = CreateWebClient())
            {
                var newImportTaskUrl = Cluster.Uri + "/_cluster/jobs/" +
                                       ClusterConfiguration.JobId +
                                       "/import?reference=" +
                                       System.Uri.EscapeDataString(reference);

                var newTaskJson = webClient.DownloadString(newImportTaskUrl);

                var taskDetails = JsonConvert.DeserializeObject<RemoteTask.RemoteTaskStatus>(newTaskJson);

                return new RemoteTask(taskDetails);
            }
        }
    }
}
