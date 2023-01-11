using System;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Collections.Generic;
using System.Web.Http;
using System.Web.Http.Description;
using System.Web.Http.OData;
using System.Web.Http.OData.Query;
using LicenseServer.DTOs;
using LicenseServer.Models.Mongo;
using Microsoft.AspNet.Identity;
using LicenseServer.Models;
using MongoDB.Bson.IO;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using GeoSource = LicenseServer.Models.Mongo.GeoSource;
using MultiDomainGeoSource = LicenseServer.Models.Mongo.MultiDomainGeoSource;
using User = LicenseServer.Models.Mongo.User;

namespace LicenseServer.Controllers
{
    [RoutePrefix("api/v1/GeoSource")]
    public class GeoSourceController : BaseResourceController<MultiDomainGeoSource>
    {
        private static readonly LicenseReference s_commonLicense = new LicenseReference
        {
            Id = new Guid(Properties.Settings.Default.CommonFreeLicenseId), 
            LicenseType = LicenseType.Full, 
            Limitations = new LicenseTrialLimitations()
        };

        public GeoSourceController() 
        { }

        // Inject for test
        public GeoSourceController(TestMongoSetup setup) :
            base(setup)
        { }

        // Get api/v1/GeoSource/{id}/Expanded
        [HttpGet]
        [Route("{id}/Expanded")]
        [ResponseType(typeof(GeoSourceExpandedDTO))]
        public HttpResponseMessage GetExpandedGeoSource(Guid id)
        {
            var geoSourceResponse = base.Get(id);
            MultiDomainGeoSource geoSource;
            if (geoSourceResponse.TryGetContentValue(out geoSource))
            {
                var publisher = db.GetResourceById<User>(geoSource.Metadata.User.Id);
                var geoSourceExpandedDTO = GeoSourceExpandedFactory.Create(geoSource, publisher);
                return Request.CreateResponse(HttpStatusCode.OK, geoSourceExpandedDTO);
            }
            return Request.CreateResponse<GeoSourceExpandedDTO>(HttpStatusCode.NotFound, null);
        }

        // Get api/v1/GeoSource/{id}/Status
        [HttpGet]
        [Route("{id}/Status")]
        [ResponseType(typeof(GeoSourceStatusDTO))]
        public HttpResponseMessage GetGeoSourceStatus(Guid id)
        {
            var geoSourceResponse = base.Get(id);
            MultiDomainGeoSource geoSource;
            if (geoSourceResponse.TryGetContentValue(out geoSource))
            {
                GeoSourceStatusDTO geoSourceStatus;
                if (geoSource.State == PipelineDefinitionState.Removed)
                {
                    geoSourceStatus = GeoSourceStatusDTO.RemovedGeoSourceStatus();
                }
                else if (geoSource.Metadata.SystemTags.FirstOrDefault(t => t == "WebService" || t == "OGC") == null)
                {
                    geoSourceStatus = GeoSourceStatusDTO.FromPublishedPipelineDetails(db.GetPipelineDetails(geoSource.ProcRef));
                }
                else
                {
                    geoSourceStatus = GeoSourceStatusDTO.WebServiceGeoSourceStatus();
                }
                return Request.CreateResponse(HttpStatusCode.OK, geoSourceStatus);
            }
            else
            {
                return Request.CreateResponse(HttpStatusCode.NotFound);
            }
        }

        // POST api/v1/GeoSource
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public override HttpResponseMessage Post(MultiDomainGeoSource geoSource)
        {
            RequireCompletedProfile();
            if (geoSource.ProcRef == null || geoSource.Definition == null || geoSource.DataSize == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "GeoSource ProcRef, Definition, and DataSize must be specified");
            }
            if (db.GetPipelineByProcRef(geoSource.ProcRef) != null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "A Resource identified by the specified ProcRef already exists");
            }
            if (geoSource.State == null)
            {
                geoSource.State = PipelineDefinitionState.Active;
            }
            var storageLimit = GetUserStorageLimit();
            var currentStorageUsage = db.GetStorageByUserId(CurrentUserIdentity.ResourceId.Value);
            if (currentStorageUsage + geoSource.DataSize > storageLimit)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Storage quota (" + storageLimit + "B) exceeded by " + (currentStorageUsage + geoSource.DataSize - storageLimit) + "B");
            }
            geoSource.Licenses = new List<LicenseReference> { s_commonLicense };
            return base.Post(geoSource);
        }

        // PUT api/v1/Resource/5?Version={version}
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public override HttpResponseMessage Put(Guid id, Guid version, MultiDomainGeoSource geoSource)
        {
            List<Guid> providerGalleryIds = null;
            if (geoSource.State.HasValue && geoSource.State.Value == PipelineDefinitionState.Removed)
            {
                var existingGeoSource = db.GetResourceById<MultiDomainGeoSource>(id);
                if (existingGeoSource != null)
                {
                    providerGalleryIds = RemoveGalleryProviders(existingGeoSource, geoSource);
                }
                else
                {
                    return Request.CreateResponse(HttpStatusCode.BadRequest, "No GeoSource found with the given Id.");
                }
            }

            var response = base.Put(id, version, geoSource);

            if (response.StatusCode == HttpStatusCode.OK && providerGalleryIds != null && providerGalleryIds.Any())
            {
                response = RemoveResourceFromProviderGalleries(providerGalleryIds, id);
            }
            return response;
        }
    }
}
