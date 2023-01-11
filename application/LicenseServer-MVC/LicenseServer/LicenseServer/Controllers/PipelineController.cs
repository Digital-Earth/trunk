/******************************************************************************
PipelineController.cs

begin		: June. 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Http;
using System.Web.Http;
using System.Web.Http.Description;
using System.Web.Http.OData.Query;
using LicenseServer.App_Utilities;
using LicenseServer.DTOs;
using LicenseServer.Extensions;
using LicenseServer.Models;
using LicenseServer.Models.Mongo;
using Microsoft.AspNet.Identity;
using Pyxis.Contract.Publishing;
using Pipeline = LicenseServer.Models.Mongo.Pipeline;

namespace LicenseServer.Controllers
{
    [RoutePrefix("api/v1/Pipeline")]
    public class PipelineController : CORSMongoApiController
    {
        public PipelineController() 
        { }

        // Inject for test
        public PipelineController(TestMongoSetup setup) :
            base(setup)
        { }

        // GET api/v1/Pipeline
        public IEnumerable<PipelineTaggedInfoNoDefinitionDTO> GetPipelineInfoes()
        {
            return db.GetPipelines();
        }

        // GET api/v1/Pipeline/5
        [Route("{procRef}")]
        [ResponseType(typeof(PipelineTaggedInfoDTO))]
        public HttpResponseMessage GetPipelineInfo(string procRef)
        {
            var pipelineTaggedInfoDTO = db.GetPipeline(procRef);
            if (pipelineTaggedInfoDTO != null)
            {
                return Request.CreateResponse(HttpStatusCode.OK, pipelineTaggedInfoDTO);
            }
            return Request.CreateResponse<PipelineTaggedInfoDTO>(HttpStatusCode.NotFound, null);
        }

        // GET api/v1/Pipeline/Details/5
        [Route("Details/{procRef}")]
        [ResponseType(typeof(PublishedPipelineDetailsDTO))]
        public HttpResponseMessage GetPipelineDetails(string procRef)
        {
            try
            {
                var result = db.GetPipelineDetails(procRef);
                return Request.CreateResponse(HttpStatusCode.OK, result);
            }
            catch (DataLayerException exception)
            {
                return exception.ToHttpResponse(Request);
            }
        }

        // GET api/v1/Pipeline/Resource/ProcRef/5
        [Route("Resource/ProcRef/{procRef}")]
        [ResponseType(typeof(Resource))]
        public HttpResponseMessage GetPipelineResource(string procRef)
        {
            try
            {
                var result = db.GetPipelineByProcRef(procRef);
                return Request.CreateResponse(HttpStatusCode.OK, result);
            }
            catch (DataLayerException exception)
            {
                return exception.ToHttpResponse(Request);
            }
        }

        // GET api/v1/Pipeline/Resource/5?Version={Version}
        [Route("Resource/{id}")]
        [ResponseType(typeof(Resource))]
        public HttpResponseMessage GetPipelineResource(Guid id, Guid version)
        {
            try
            {
                var result = db.GetPipelineByIdAndVersion(id, version);
                return Request.CreateResponse(HttpStatusCode.OK, result);
            }
            catch (DataLayerException exception)
            {
                return exception.ToHttpResponse(Request);
            }
        }

        // TODO move to BaseResourceController when migrating old pipeline controller clients
        // POST api/v1/Pipeline
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public HttpResponseMessage Post(Pipeline pipeline)
        {
            if (pipeline.ProcRef == null || pipeline.Definition == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Pipeline ProcRef, and Definition must be specified");
            }
            if (db.GetPipelineByProcRef(pipeline.ProcRef) != null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "A Resource identified by the specified ProcRef already exists");
            }
            if (pipeline.Metadata == null || pipeline.Metadata.Name == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Resource.Metadata.Name must be specified");
            }
            if (pipeline.Metadata.Providers != null && pipeline.Metadata.Providers.Count > 0)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Pipeline cannot have a provider");
            }
            if (!CurrentUserIdentity.ResourceId.HasValue)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Complete user profile before requesting");
            }
            if (pipeline.Metadata.User == null || !CurrentUserIdentity.IsInPyxisAdminRole())
            {
                pipeline.Metadata.User = new Pyxis.Contract.Publishing.UserInfo { Id = CurrentUserIdentity.ResourceId.Value, Name = CurrentUserIdentity.ProfileName };
            }
            if (pipeline.Id == Guid.Empty)
            {
                pipeline.Id = Guid.NewGuid();
            }
            if (pipeline.Version == Guid.Empty)
            {
                pipeline.Version = Guid.NewGuid();
            }
            pipeline.Type = ResourceTypeResolver.Resolve<Pipeline>().Value;
            pipeline.Metadata.Created = DateTime.UtcNow;
            pipeline.Metadata.Updated = pipeline.Metadata.Created;

            FillReferenceTypeNulls(pipeline);

            HttpResponseMessage response;
            try
            {
                db.InsertResource<Pipeline>(pipeline);
                response = Request.CreateResponse(HttpStatusCode.Created, pipeline);
                response.Headers.Location = new Uri(Url.Link("DefaultApi", new { id = pipeline.Id }));
            }
            catch (DataLayerException exception)
            {
                response = exception.ToHttpResponse(Request);
            }
            return response;
        }

        // TODO move to BaseResourceController when migrating old pipeline controller clients
        private static void FillReferenceTypeNulls(Pipeline pipeline)
        {
            // prevent nulls from being inserted
            if (pipeline.Licenses == null) { pipeline.Licenses = new List<Pyxis.Contract.Publishing.LicenseReference>(); }
            if (pipeline.Metadata.Providers == null) { pipeline.Metadata.Providers = new List<Pyxis.Contract.Publishing.Provider>(); }
            if (pipeline.Metadata.Comments == null) { pipeline.Metadata.Comments = new LinkedList<Pyxis.Contract.Publishing.AggregateComment>(); }
            if (pipeline.Metadata.Ratings == null) { pipeline.Metadata.Ratings = new Pyxis.Contract.Publishing.AggregateRatings(); }
            if (pipeline.Metadata.ExternalUrls == null) { pipeline.Metadata.ExternalUrls = new List<Pyxis.Contract.Publishing.ExternalUrl>(); }
            if (pipeline.Metadata.Tags == null) { pipeline.Metadata.Tags = new List<string>(); }
            if (pipeline.Metadata.SystemTags == null) { pipeline.Metadata.SystemTags = new List<string>(); }
            if (pipeline.Metadata.Visibility == null) { pipeline.Metadata.Visibility = Pyxis.Contract.Publishing.VisibilityType.Public; }

            if (pipeline.BasedOn == null) { pipeline.BasedOn = new List<Pyxis.Contract.Publishing.ResourceReference>(); }
            if (pipeline.Specification == null) { pipeline.Specification = new PipelineSpecification(); }
        }
    }
}