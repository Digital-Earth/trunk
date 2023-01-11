using System.Collections.Generic;
using System.Web.Http;
using System.Web.Http.OData;
using System.Web.Http.OData.Query;
using Pyxis.Contract.Workspaces;
using PyxisCLI.Server.WebConfig;

namespace PyxisCLI.Server.Controllers
{
    [RoutePrefix("api/v1/Search")]
    public class SearchController : ApiController
    {
        public class SearchResult
        {
            public string Reference { get; set; }
            public string Name { get; set; }
            public string Workspace { get; set; }
            public string Type { get; set; }

            public float Score { get; set; }

            public object Data { get; set; }
        }

        [EnableQuery]
        [Route("")]
        [HttpGet]
        [TimeTrace("query")]
        public IEnumerable<dynamic> Search(ODataQueryOptions<SearchResult> odataParameters, string query = "")
        {
            query = query.ToLower();

            var result = new List<SearchResult>();
            foreach (var name in Program.Workspaces.Names)
            {
                var workspace = Program.Workspaces.GetWorkspace(name);

                foreach (var import in workspace.Imports)
                {
                    if (import.Key.ToLower().Contains(query))
                    {
                        var searchResult = new SearchResult()
                        {
                            Reference = name + "/" + import.Key,
                            Name = import.Key,
                            Workspace = name,
                            Type = "Import",
                            Score = 1.0f,
                        };

                        var importTemplate = import.Value as ImportTemplate;
                        if (importTemplate != null)
                        {
                            searchResult.Data = new
                            {
                                Style = importTemplate.Style,
                                Specification = importTemplate.Specification,
                                Domains = importTemplate.Domains
                            };
                        }
                        else
                        {
                            searchResult.Data = new
                            {
                                Style = import.Value.Style,
                                Specification = import.Value.Specification,
                            };
                        }

                        result.Add(searchResult);    
                    }
                }
            }

            return odataParameters.ApplyTo(result);
        }
    }
}