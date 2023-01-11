using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Net.Http.Formatting;
using System.Net.Http.Headers;
using System.Text;
using System.Web.Http;
using System.Web.Http.Description;
using LicenseServer.Extensions;
using LicenseServer.Models;
using LicenseServer.Models.Mongo;
using Microsoft.AspNet.Identity;
using Pyxis.Contract.Publishing;
using GeoSource = LicenseServer.Models.Mongo.GeoSource;
using License = LicenseServer.Models.Mongo.License;

namespace LicenseServer.Controllers
{
    [RoutePrefix("api/v1/License")]
    public class LicenseController : BaseResourceController<License>
    {
        private static string s_licenseTerms = Properties.Settings.Default.TermsOfUse;

        public LicenseController()
        { }

        // Inject for test
        public LicenseController(TestMongoSetup setup) :
            base(setup)
        { }

        // GET /api/v1/License/Terms
        [Route("Terms")]
        [ResponseType(typeof(LicenseTerms))]
        public HttpResponseMessage GetTerms()
        {
            return Request.CreateResponse(HttpStatusCode.OK, new LicenseTerms { Text = s_licenseTerms });
        }

        // POST api/v1/License
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public override HttpResponseMessage Post(License license)
        {
            if (license.LicenseType == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "LicenseType (Full or Trial) must be specified");
            }
            if (license.LicenseType == LicenseType.Trial
                && (license.Limitations == null || license.Limitations.Area == null || license.Limitations.Resolution == null || license.Limitations.Time == null || license.Limitations.Watermark == null))
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Trial licenses must specify a limitation.");
            }
            if(license.PublishingType == null)
            {
                license.PublishingType = PublishingType.Open;
            }

            return base.Post(license);
        }

        // PUT /api/v1/License/5?ResourceId={resourceId}
        [HttpPut]
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public HttpResponseMessage Attach(Guid id, Guid resourceId)
        {
            RequireCompletedProfile();
            var resources = db.GetResourcesByIds(new List<Guid> { id, resourceId }).ToList();
            var license = resources.FirstOrDefault(r => r.Id == id && r.Type == ResourceType.License) as License;
            var resource = resources.FirstOrDefault(r => r.Id == resourceId && r.Type == ResourceType.GeoSource) as GeoSource;
            var preconditionsFailedResponse = LicensingPreconditionsFailedResponse(license, resource);
            if (preconditionsFailedResponse != null)
            {
                return preconditionsFailedResponse;
            }
            if (resource.Licenses.FirstOrDefault(l => l.Id == id) != null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "The specified License is already attached to the specified Resource."); 
            }

            var licenseReference = LicenseReference.ReferenceFromLicense(license);
            var updates = new GeoSource { Licenses = new List<LicenseReference>(resource.Licenses) };
            updates.Licenses.Add(licenseReference);

            try
            {
                db.UpdateResource(resource.Id, resource.Version, updates);
                return Request.CreateResponse(HttpStatusCode.OK);
            }
            catch (DataLayerException exception)
            {
                return exception.ToHttpResponse(Request);
            }
        }

        // DELETE /api/v1/License/5?ResourceId={resourceId}
        [HttpDelete]
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public HttpResponseMessage Detach(Guid id, Guid resourceId)
        {
            RequireCompletedProfile();
            var resources = db.GetResourcesByIds(new List<Guid> { id, resourceId }).ToList();
            var license = resources.FirstOrDefault(r => r.Id == id && r.Type == ResourceType.License) as License;
            var resource = resources.FirstOrDefault(r => r.Id == resourceId && r.Type == ResourceType.GeoSource) as GeoSource;
            var preconditionsFailedResponse = LicensingPreconditionsFailedResponse(license, resource);
            if (preconditionsFailedResponse != null)
            {
                return preconditionsFailedResponse;
            }
            if (resource.Licenses.FirstOrDefault(l => l.Id == id) == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "The specified License is not attached to the specified Resource.");
            }
            if (license.LicenseType == LicenseType.Full && resource.Licenses.Count(x => x.LicenseType == LicenseType.Full) <= 1)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Disallowing detachment of the only full License attached to the specified Resource.  Attach another full License before retrying.");
            }

            var updates = new GeoSource { Licenses = new List<LicenseReference>(resource.Licenses) };
            updates.Licenses.RemoveAll(l => l.Id == id);

            try
            {
                db.UpdateResource(resource.Id, resource.Version, updates);
                return Request.CreateResponse(HttpStatusCode.OK);
            }
            catch (DataLayerException exception)
            {
                return exception.ToHttpResponse(Request);
            }
        }

        private HttpResponseMessage LicensingPreconditionsFailedResponse(License license, GeoSource resource)
        {
            if (license == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Specified License does not exist.");
            }
            if (!CurrentUserIdentity.IsInPyxisAdminRole() && !ValidateOwnership(license))
            {
                return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to attach the specified license.");
            }
            if (resource == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Specified GeoSource does not exist.  Only GeoSources can be licensed.");
            }
            if (!CurrentUserIdentity.IsInPyxisAdminRole() && !ValidateOwnership(resource))
            {
                return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to attach a license to the specified Resource.");
            }
            return null;
        }
    }
}
