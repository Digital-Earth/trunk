using System.Collections.Generic;
using System.Net;
using System.Net.Http;
using System.Threading.Tasks;
using System.Web.Http;
using System.Web.Http.OData.Query;
using Newtonsoft.Json.Linq;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Workspaces;
using PyxisCLI.Server.WebConfig;
using PyxisCLI.Workspaces;

namespace PyxisCLI.Server.Controllers
{
    /// <summary>
    /// Get Information about workspace
    /// </summary>
    [RoutePrefix("api/v1/Workspace")]
    public class WorkspaceGlobesController : AuthorizedApiController
    {

        [Route("{workspace}/Globe")]
        [HttpGet]
        [TimeTrace("workspace")]
        [AllowAnonymous]
        public Dictionary<string, GlobeTemplate> GetGlobes(string workspace)
        {
            return Workspaces.GetWorkspace(workspace).Globes;
        }

        [Route("{workspace}/Globe/{globe}")]
        [HttpGet]
        [TimeTrace("workspace")]
        [AllowAnonymous]
        public GlobeTemplate GetGlobe(string workspace, string globe)
        {
            return Workspaces.GetGlobe(Reference.FromParts(workspace, globe));
        }

        [Route("{workspace}/Globe/{globe}")]
        [HttpPost]
        [TimeTrace("workspace")]
        public GlobeTemplate PostGlobe(string workspace, string globe, [FromBody] JObject globeJson)
        {
            var newGlobe = globeJson.ToObject<GlobeTemplate>();
            Workspaces.GetWorkspaceFile(workspace).UpdateOrInsertGlobe(globe, newGlobe);
            return newGlobe;
        }

        [Route("{workspace}/Globe/{globe}")]
        [HttpDelete]
        [TimeTrace("workspace")]
        public HttpResponseMessage DeleteGlobe(string workspace, string globe)
        {
            Workspaces.GetWorkspaceFile(workspace).RemoveGlobe(globe);
            return Request.CreateResponse(HttpStatusCode.Accepted);
        }
    }
}
