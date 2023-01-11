using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Web.Http;
using System.Web.Http.OData;
using System.Web.Http.OData.Query;
using ApplicationUtility;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;
using Pyxis.Contract.Workspaces.Domains;
using Pyxis.Core.DERM;
using Pyxis.Core.IO;
using Pyxis.Core.IO.GeoJson;
using PyxisCLI.Server.WebConfig;

namespace PyxisCLI.Server.Controllers
{
    [RoutePrefix("api/v1/Slice")]
    public class SliceController : ApiController
    {
        public class RangeDetails
        {
            public object Start { get; set; }
            public object End { get; set; }
            public object Step { get; set; }
        }

        public class Inputs
        {
            public string Reference { get; set; }
            public string As { get; set; }
        }

        public class SliceRequest
        {
            public string Cell { get; set;  }
            public List<string> Cells { get; set; }
            public string Timestamp { get; set; }
            public RangeDetails TimestampRange { get; set; }

            public List<Inputs> Inputs { get; set; }
        }

        public class CellResult
        {
            public string Index { get; set; }
            public Dictionary<string, object> Values { get; set; }
        }

        [Route("")]
        [HttpPost]
        public List<CellResult> Slice([FromBody] SliceRequest request)
        {
            if (request.Cell.HasContent() && request.Cells == null)
            {
                request.Cells = new List<string> { request.Cell };
                request.Cell = null;
            }

            var result = new List<CellResult>();

            foreach (var cell in request.Cells)
            {
                result.Add(CreateCellResult(cell, request));
            }

            return result;
        }

        private CellResult CreateCellResult(string index, SliceRequest request)
        {
            var result = new CellResult
            {
                Index = index,
                Values = new Dictionary<string, object>()
            };

            foreach (var input in request.Inputs)
            {
                var geoSources = ResolveInputs(input, request);
                
                var values = geoSources.AsParallel().AsOrdered().Select(
                    (geoSource) => Program.Cluster.ResolveGeoSource(geoSource).GetCellValue(index)).ToList();
                                    
                result.Values[input.As ?? input.Reference] = values;
            }

            return result;
        }

        private IEnumerable<GeoSource> ResolveInputs(Inputs input, SliceRequest request)
        {
            var import = Program.Workspaces.GetImport(new Reference(input.Reference));

            if (import is ImportTemplate)
            {
                var templateImport = import as ImportTemplate;

                if (templateImport.Domains != null && templateImport.Domains.ContainsKey("time"))
                {
                    var domains = new Dictionary<string,string>();

                    if (request.Timestamp.HasContent())
                    {
                        domains["time"] = request.Timestamp;
                        yield return Program.Workspaces.ResolveGeoSource(import, domains);
                    }
                    else if (request.TimestampRange != null)
                    {
                        var range = new DateRangeDomain(
                               (DateTime)request.TimestampRange.Start,
                               (DateTime)request.TimestampRange.End,
                               request.TimestampRange.Step.ToString()
                               );

                        foreach (var value in range.Values)
                        {
                            domains["time"] = value;
                            yield return Program.Workspaces.ResolveGeoSource(import, domains);
                        }
                    }
                }
            }
            else
            {
                yield return Program.Workspaces.ResolveGeoSource(import);
            }
        }
    }
}