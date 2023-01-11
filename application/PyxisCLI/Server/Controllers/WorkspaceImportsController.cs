using System.Collections.Generic;
using System.Net;
using System.Net.Http;
using System.Threading.Tasks;
using System.Web.Http;
using System.Web.Http.OData.Query;
using System.Web.Http.Results;
using Newtonsoft.Json.Linq;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Workspaces;
using PyxisCLI.Server.Models;
using PyxisCLI.Server.WebConfig;
using PyxisCLI.Workspaces;

namespace PyxisCLI.Server.Controllers
{
    /// <summary>
    /// Get Information about workspace
    /// </summary>
    [RoutePrefix("api/v1/Workspace")]
    public class WorkspaceImportsController : AuthorizedApiController
    {
        //private static Dictionary<string, EndpointState> s_endpointStates = new Dictionary<string, EndpointState>();

        [Route("{workspace}/Import")]
        [HttpGet]
        [TimeTrace("workspace")]
        public Dictionary<string,IImport> GetImports(string workspace)
        {
            return Workspaces.GetWorkspace(workspace).Imports;
        }

        [Route("{workspace}/Import/{import}")]
        [HttpGet]
        [TimeTrace("workspace")]
        public IImport GetImport(string workspace, string import)
        {
            return Workspaces.GetImport(Reference.FromParts(workspace, import));
        }

        [Route("{workspace}/Import/{import}")]
        [HttpPost]
        [TimeTrace("workspace")]
        public IImport PostImport(string workspace, string import, [FromBody] JObject importJson)
        {
            var newImport = WorkspaceParser.ParseImport(importJson);

            Workspaces.GetWorkspaceFile(workspace).UpdateOrInsertImport(import,newImport);
            return newImport;            
        }

        [Route("{workspace}/Import/{import}")]
        [HttpDelete]
        [TimeTrace("workspace")]
        public HttpResponseMessage DeleteImport(string workspace, string import)
        {
            Workspaces.GetWorkspaceFile(workspace).RemoveImport(import);
            return Request.CreateResponse(HttpStatusCode.Accepted);
        }

       
        [Route("{workspace}/Import/{import}/Sync")]
        [HttpGet]
        [TimeTrace("workspace")]
        public string SyncImport(string workspace, string import)
        {
            var discoverTask = HttpClusterProxy.StartImportTask(Reference.EncodeFromParts(workspace, import));
            return discoverTask.Id;
        }
    }
}
