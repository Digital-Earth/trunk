using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Http;
using System.Threading.Tasks;
using System.Web.Http;
using System.Web.Http.OData.Query;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Workspaces;
using PyxisCLI.Server.Utilities;
using PyxisCLI.Server.WebConfig;
using PyxisCLI.State;
using PyxisCLI.Workspaces;

namespace PyxisCLI.Server.Controllers
{
    /// <summary>
    /// Get Information about workspace
    /// </summary>
    [RoutePrefix("api/v1/Workspace")]
    public class WorkspaceEndpointsController : AuthorizedApiController
    {
        private static Dictionary<string, EndpointState> s_endpointStates = new Dictionary<string, EndpointState>();

        [Route("{workspace}/Endpoint")]
        [HttpGet]
        [TimeTrace("workspace")]
        [AllowAnonymous]
        public Dictionary<string,Endpoint> Endpoints(string workspace)
        {
            return Workspaces.GetWorkspace(workspace).Endpoints;
        }

        [Route("{workspace}/Endpoint/{endpoint}")]
        [HttpGet]
        [TimeTrace("workspace")]
        [AllowAnonymous]
        public Endpoint Endpoint2(string workspace, string endpoint)
        {
            return Workspaces.GetEndpoint(Reference.FromParts(workspace, endpoint));
        }

        [Route("{workspace}/Endpoint/{endpoint}")]
        [HttpPost]
        [TimeTrace("workspace")]
        public Endpoint Endpoint3(string workspace, string endpoint, string uri)
        {
            if (uri.Contains("https://"))
            {
                string proxyPort = ClusterConfiguration.ProxyPort;
                string proxyhost = ClusterConfiguration.ProxyHost;

                var newProxyURl = "http://" + proxyhost + ":" + proxyPort + "/external/https/";
                uri = uri.Replace("https://", newProxyURl);
            }
            
            var newEndpoint = new Endpoint() {Uri = uri};
            var ws = Workspaces.GetWorkspaceFile(workspace);
            ws.UpdateOrInsertEndpoint(endpoint,newEndpoint);
            return newEndpoint;
        }

        [Route("{workspace}/Endpoint/{endpoint}")]
        [HttpDelete]
        [TimeTrace("workspace")]        
        public HttpResponseMessage DeleteEndpoint(string workspace, string endpoint)
        {
            var ep = Workspaces.GetEndpoint(Reference.FromParts(workspace, endpoint));
            if (ep == null)
            {
                return Request.CreateResponse(HttpStatusCode.NotFound);
            }

            var wsFile = Workspaces.GetWorkspaceFile(workspace);
            wsFile.RemoveEndpoint(endpoint);
            return Request.CreateResponse(HttpStatusCode.Accepted);
        }

        [Route("{workspace}/Endpoint/{endpoint}/Search")]
        [HttpGet]
        [TimeTrace("workspace")]
        [AllowAnonymous]
        public IEnumerable<dynamic> Search3(string workspace, string endpoint, ODataQueryOptions<DataSet> odataParameters, string query = null)
        {
            var endpointState = GetEndpointState(workspace, endpoint);
            var datasets = endpointState.Search(query ?? "");
            string proxyPort = ClusterConfiguration.ProxyPort;
            string proxyhost = ClusterConfiguration.ProxyHost;

            var newProxyURl = "http://" + proxyhost + ":" + proxyPort + "/external/https/";
            foreach (var d in datasets)
            {
                if (d.Uri.Contains(newProxyURl))
                {
                    d.Uri = d.Uri.Replace(newProxyURl, "https://");
                }
            }
            return odataParameters.ApplyTo(datasets);
        }

        [Route("{workspace}/Endpoint/{endpoint}/DiscoveryReport")]
        [HttpGet]
        [TimeTrace("workspace")]
        [AllowAnonymous]
        public UrlDiscoveryReport DiscoveryReport(string workspace, string endpoint)
        {
            var endpointState = GetEndpointState(workspace, endpoint);
            return endpointState.DiscoveryReport;
        }

        [Route("{workspace}/Endpoint/{endpoint}/Discover")]
        [HttpGet]
        [TimeTrace("workspace")]
        public string Discover(string workspace, string endpoint)
        {
            var ep = Endpoint2(workspace, endpoint);
            if (ep == null)
            {
                throw new HttpResponseException(HttpStatusCode.NotFound);
            }

            var discoverTask = HttpClusterProxy.StartDiscoverTask(Reference.EncodeFromParts(workspace, endpoint));
            return discoverTask.Id;
        }

        private EndpointState GetEndpointState(string workspace, string endpoint)
        {
            var endpointObject = Endpoint2(workspace, endpoint);
            if (endpointObject.Uri.Contains("https://"))
            {
                string proxyPort = ClusterConfiguration.ProxyPort;
                string proxyhost = ClusterConfiguration.ProxyHost;

                var newProxyURl = "http://" + proxyhost + ":" + proxyPort + "/external/https/";
                endpointObject.Uri = endpointObject.Uri.Replace("https://", newProxyURl);
            }
            var key = workspace + "/" + endpoint;
            EndpointState endpointState;
            if (s_endpointStates.ContainsKey(key))
            {
                endpointState = s_endpointStates[key];
            }
            else
            {
                endpointState = new EndpointState(endpointObject);
                s_endpointStates[key] = endpointState;
            }
            return endpointState;
        }
    }
}
