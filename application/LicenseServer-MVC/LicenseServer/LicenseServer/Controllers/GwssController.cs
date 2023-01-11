/******************************************************************************
GwssController.cs

begin		: June. 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web.Http;
using System.Web.Http.Description;
using System.Web.Http.OData;
using System.Web.Http.OData.Extensions;
using System.Web.Http.OData.Query;
using LicenseServer.App_Utilities;
using LicenseServer.DTOs;
using LicenseServer.Extensions;
using LicenseServer.Models.Mongo;
using Pyxis.Contract.Operations;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Services.GeoWebStreamService;

namespace LicenseServer.Controllers
{
    public class GwssController : CORSMongoApiController
    {
        public GwssController() 
        { }

        // Inject for test
        public GwssController(TestMongoSetup setup) :
            base(setup)
        { }

        // GET api/v1/Gwss
        [HttpGet]
        [ResponseType(typeof(GwssSummaryDTO))]
        public PageResult<dynamic> GetServers(ODataQueryOptions<GwssSummaryDTO> options)
        {
            var gwssSummaries = new List<GwssSummaryDTO>();
            var servers = db.GetGwssesStatuses();
            foreach (var server in servers)
            {
                var gwssSummary = GenerateGwssSummary(server);
                gwssSummaries.Add(gwssSummary);
            }

            return gwssSummaries.AsQueryable().ToPageResult(Request, options);
        }

        // GET api/v1/Gwss/5
        public GwssSummaryDTO GetServer(Guid id)
        {
            var server = db.GetGwssStatusById(id);
            if (server == null)
            {
                throw new HttpResponseException(Request.CreateResponse(HttpStatusCode.NotFound));
            }

            return GenerateGwssSummary(server);
        }

        private static GwssSummaryDTO GenerateGwssSummary(Gwss server)
        {
            var gwssSummary = new GwssSummaryDTO { 
                AvailableDiskSpaceMB = server.Status.ServerStatus.AvailableDiskSpaceMB, 
                Name = server.Status.Name, 
                NodeID = server.Id.ToString(), 
                LastHeard = server.LastHeard 
            };
            gwssSummary.PipelineStatuses.AddRange(server.Status.PipelinesStatuses.
               Select(x => new PipelineServerStatusDTO { 
                   ProcRef = x.ProcRef, 
                   Status = x.StatusCode.ToString(), 
                   NodeId = server.Id, 
                   OperationStatus = (server.Status.OperationsStatuses == null ? null 
                        : server.Status.OperationsStatuses.FirstOrDefault(operStatus => 
                            {
                                string procRef;
                                var found = (operStatus.Operation.Parameters.TryGetValue("ProcRef", out procRef) && procRef == x.ProcRef);
                                return found;
                            }))
               }).
               OrderBy(x => x.Status == PipelineStatusCode.Published.ToString() ? 2 : 1).
               ThenBy(x => x.OperationStatus == null ? 2 : 1).
               ThenBy(x => x.Status)
               );
            return gwssSummary;
        }
    }
}