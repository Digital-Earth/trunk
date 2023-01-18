using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web.Http;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;
using PyxisCLI.Server.Cluster;
using PyxisCLI.Server.Models;
using PyxisCLI.Server.Services;
using PyxisCLI.Server.WebConfig;
using PyxisCLI.State;
using PyxisCLI.Workspaces;

namespace PyxisCLI.Server.Controllers
{
    /// <summary>
    /// Get Information about workspace
    /// </summary>
    [RoutePrefix("api/v1/Workspace")]
    public class WorkspaceController : AuthorizedApiController
    {
        [Route("")]
        [HttpGet]
        [TimeTrace]
        [AllowAnonymous]
        public List<string> List()
        {
            return Workspaces.Names.ToList();
        }

        [Route("{workspace}")]
        [HttpGet]
        [TimeTrace("workspace")]
        [AllowAnonymous]
        public Workspace Workspace(string workspace)
        {
            return Workspaces.GetWorkspace(workspace);
        }

        [Route("{workspace}")]
        [HttpPost]
        [TimeTrace("workspace")]
        public Workspace AddWorkspace(string workspace)
        {
            if (!Workspaces.WorkspaceExists(workspace))
            {
                Workspaces.CreateWorkspace(workspace);

                //notify other nodes about new empty workspace
                var notifications = HttpClusterProxy.Broadcast<Workspace>(Program.Cluster.ServersRing, "/api/v1/Workspace/" + workspace).Result;
            }
            return Workspaces.GetWorkspace(workspace);
        }

        [Route("{workspace}")]
        [HttpDelete]
        [TimeTrace("workspace")]
        public HttpResponseMessage DeleteWorkspace(string workspace)
        {
            if (Workspaces.WorkspaceExists(workspace))
            {
                Workspaces.DeleteWorkspace(workspace);
            }
            return Request.CreateResponse(HttpStatusCode.OK);
        }

        [Route("{workspace}/Status")]
        [HttpGet]
        [TimeTrace("workspace")]
        [AllowAnonymous]
        public WorkspaceWithStatus WorkspaceStatus(string workspace)
        {
            var ws = Workspaces.GetWorkspace(workspace);

            var result = new WorkspaceWithStatus()
            {
                Endpoints = new Dictionary<string, WorkspaceWithStatus.ItemAndStatus>(),
                Imports = new Dictionary<string, WorkspaceWithStatus.ItemAndStatus>(),
                Globes = new Dictionary<string, WorkspaceWithStatus.ItemAndStatus>()
            };

            string proxyPort = ClusterConfiguration.ProxyPort;
            string proxyhost = ClusterConfiguration.ProxyHost;

            var newProxyURl = "http://" + proxyhost + ":" + proxyPort + "/external/https/";

            foreach (var endpoint in ws.Endpoints)
            {
                if (endpoint.Value.Uri.Contains(newProxyURl))
                {
                    endpoint.Value.Uri = endpoint.Value.Uri.Replace(newProxyURl, "https://");
                }
                
                result.Endpoints[endpoint.Key] = new WorkspaceWithStatus.ItemAndStatus()
                {
                    Item = endpoint.Value,
                    Status = StatusFactory.Create(endpoint.Value)
                };

            }

            foreach (var import in ws.Imports)
            {
                result.Imports[import.Key] = new WorkspaceWithStatus.ItemAndStatus()
                {
                    Item = import.Value,
                    Status = StatusFactory.Create(import.Value)
                };
            }

            foreach (var globe in ws.Globes)
            {
                result.Globes[globe.Key] = new WorkspaceWithStatus.ItemAndStatus()
                {
                    Item = globe.Value,
                    Status = StatusFactory.Create(globe.Value)
                };
            }

            return result;
        }

        [Route("Import")]
        [HttpGet]
        [TimeTrace("reference")]
        [AllowAnonymous]
        public IImport ResolveImport(string reference)
        {
            return Workspaces.GetImport(new Reference(reference));
        }

        [Route("Globe")]
        [HttpGet]
        [TimeTrace("reference")]
        [AllowAnonymous]
        public GlobeTemplate ResolveGlobe(string reference)
        {
            return Workspaces.GetGlobe(new Reference(reference));
        }

        [Route("Resolve")]
        [HttpGet]
        [TimeTrace("reference")]
        [AllowAnonymous]
        public GeoSource Resolve2(string reference, bool broadcast = true, bool forceImport = false )
        {
            var geoSource = Workspaces.ResolveGeoSource(new ReferenceOrExpression() { Reference = reference }, forceImport);

            if (geoSource != null)
            {
                if (broadcast && !Program.Cluster.IsLocalGeoSource(geoSource.Id))
                {
                    var host = Program.Cluster.GetEndpointForGeoSource(geoSource.Id);

                    Console.WriteLine("GeoSource is not local, notify remote host: " + host);

                    var uri =
                        new UriBuilder(host).AddPath("api/v1/WorkSpace/Resolve")
                            .AddQuery("reference", reference)
                            .AddQuery("forceImport", "false")
                            .AddQuery("broadcast", "false")
                            .ToString().Replace(host,"");

                    var remoteGeoSource = HttpClusterProxy.Forward<GeoSource>(Program.Cluster.ServersRing, geoSource.Id.ToString(), uri).Result;

                    return remoteGeoSource;
                }
                else
                {
                    GeoSourceInitializer.Initialize(geoSource);    
                }
            }

            return geoSource;
        }

        [HttpPost]
        [Route("Calculate")]
        [TimeTrace]
        [AllowAnonymous]
        public GeoSource ResolveExpression([FromBody] ReferenceOrExpression request, int pull = 30)
        {
            var geoSource = Workspaces.ResolveGeoSource(request);

            NotifyNewGeoSource(geoSource);
            
            return geoSource;
        }

        private void NotifyNewGeoSource(GeoSource geoSource)
        {
            if (geoSource == null)
            {
                return;
            }

            if (!Program.Cluster.IsLocalGeoSource(geoSource.Id))
            {
                var host = Program.Cluster.GetEndpointForGeoSource(geoSource.Id);

                Console.WriteLine("GeoSource is not local, notify remote host: " + host);

                var uri =
                    new UriBuilder(host).AddPath("api/v1/GeoSource/" + geoSource.Id).ToString().Replace(host, "");

                var remoteGeoSource = HttpClusterProxy.Forward<GeoSource>(Program.Cluster.ServersRing, geoSource.Id.ToString(), uri, geoSource).Result;
            }
            else
            {
                GeoSourceInitializer.Initialize(geoSource);
            }
        }
    }
}
