using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web;
using System.Web.Http;
using System.Web.Http.Description;
using System.Web.Http.OData;
using System.Web.Http.OData.Extensions;
using System.Web.Http.OData.Query;
using LicenseServer.Extensions;
using LicenseServer.Models;
using LicenseServer.Models.Mongo;
using Microsoft.AspNet.Identity;
using Pyxis.Contract.Publishing;
using License = LicenseServer.Models.Mongo.License;

namespace LicenseServer.Controllers
{

    [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
    [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
    [RoutePrefix("api/v1/Agreement")]
    public class AgreementController : CORSMongoApiController
    {
        public AgreementController() 
        { }

        // Inject for test
        public AgreementController(TestMongoSetup setup) :
            base(setup)
        { }

        // GET api/v1/Agreement
        public PageResult<dynamic> Get(ODataQueryOptions<Agreement> options)
        {
            return GetAllAgreements(null, null, options);
        }

        // GET api/v1/Agreement?From={DateTime}&Until={DateTime}
        public PageResult<dynamic> Get(DateTime from, DateTime until, ODataQueryOptions<Agreement> options)
        {
            return GetAllAgreements(from, until, options);
        }

        // GET api/v1/Agreement/License/{licenseId}
        [Route("License/{licenseId}")]
        public PageResult<dynamic> GetLicense(Guid licenseId, ODataQueryOptions<Agreement> options)
        {
            return GetLicenseAgreements(licenseId, null, null, options);
        }

        // GET api/v1/Agreement/License/{licenseId}?From={DateTime}&Until={DateTime}
        [Route("License/{licenseId}")]
        public PageResult<dynamic> GetResource(Guid licenseId, DateTime from, DateTime until, ODataQueryOptions<Agreement> options)
        {
            return GetLicenseAgreements(licenseId, from, until, options);
        }

        // GET api/v1/Agreement/License/{licenseId}?Version={licenseVersion}
        [Route("License/{licenseId}")]
        public PageResult<dynamic> GetLicense(Guid licenseId, Guid licenseVersion, ODataQueryOptions<Agreement> options)
        {
            return GetLicenseAgreements(licenseId, licenseVersion, null, null, options);
        }

        // GET api/v1/Agreement/License/{licenseId}?Version={licenseVersion}&From={DateTime}&Until={DateTime}
        [Route("License/{licenseId}")]
        public PageResult<dynamic> GetResource(Guid licenseId, Guid licenseVersion, DateTime from, DateTime until, ODataQueryOptions<Agreement> options)
        {
            return GetLicenseAgreements(licenseId, licenseVersion, from, until, options);
        }

        // GET api/v1/Agreement/User
        [Route("User")]
        public PageResult<dynamic> GetCurrentUser(ODataQueryOptions<Agreement> options)
        {
            return GetAgreementsForCurrentUser(null, null, options);
        }

        // GET api/v1/Agreement/User?From={DateTime}&Until={DateTime}
        [Route("User")]
        public PageResult<dynamic> GetCurrentUser(DateTime from, DateTime until, ODataQueryOptions<Agreement> options)
        {
            return GetAgreementsForCurrentUser(from, until, options);
        }

        // GET api/v1/Agreement/User/{UserId}
        [Route("User/{userId}")]
        public PageResult<dynamic> GetUser(Guid userId, ODataQueryOptions<Agreement> options)
        {
            return GetAgreementsForUser(userId, null, null, options);
        }

        // GET api/v1/Agreement/User/{UserId}?From={DateTime}&Until={DateTime}
        [Route("User/{userId}")]
        public PageResult<dynamic> GetUser(Guid userId, DateTime from, DateTime until, ODataQueryOptions<Agreement> options)
        {
            return GetAgreementsForUser(userId, from, until, options);
        }

        // GET api/v1/Agreement/User?LicenseId={LicenseId}&LicenseVersion={LicenseVersion}
        [Route("User")]
        [ResponseType(typeof(Agreement))]
        public HttpResponseMessage GetLatestAgreementForCurrentUser(Guid licenseId, Guid licenseVersion, ODataQueryOptions<Agreement> options)
        {
            RequireCompletedProfile();
            var agreement = db.GetLatestAgreementByUserAndIdAndVersion(CurrentUserIdentity.ResourceId.Value, licenseId, licenseVersion);
            if (agreement == null)
            {
                return Request.CreateResponse(HttpStatusCode.NotFound);
            }
            else
            {
                return Request.CreateResponse(HttpStatusCode.OK, agreement);
            }
        }

        // GET api/v1/Agreement/User/LicensedAccess?ResourceId={ResourceId}
        [Route("User/LicensedAccess")]
        [ResponseType(typeof(LicensedAccess))]
        public HttpResponseMessage GetResourceAccess(Guid resourceId, ODataQueryOptions<LicensedAccess> options)
        {
            RequireCompletedProfile();
            try
            {
                var licensedAccess = Retry.Execute(() => db.GetLicensedAccessToResource(CurrentUserIdentity.ResourceId.Value, resourceId));
                return Request.CreateResponse(HttpStatusCode.OK, licensedAccess);
            }
            catch(DataLayerException exception)
            {
                return exception.ToHttpResponse(Request);
            }
        }

        // POST api/v1/Agreement/License/{licenseId}?LicenseVersion={Version}&Decision={DecisionType}
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [Route("License/{licenseId}")]
        public HttpResponseMessage PostAgreement(Guid licenseId, Guid licenseVersion, DecisionType decision)
        {
            RequireCompletedProfile();
            var license = db.GetResourceById<License>(licenseId);
            if (license == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "No license with the specified Id");
            }
            if (license.Version != licenseVersion)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Attempt to agree to an old version of a license.  Please refer to the latest version of the license before agreeing.");
            }
            if (license.LicenseType == LicenseType.Trial)
            {
                if (db.GetAgreementsById(license.Id).Any(a => a.Decision == DecisionType.Agree))
                {
                    return Request.CreateResponse(HttpStatusCode.BadRequest, "Disallowing attempt to agree to a trial license more than once.");
                }
            }
            var expiration = license.LicenseType == LicenseType.Full
                ? new DateTime(DateTime.UtcNow.Year, DateTime.UtcNow.Month, 1, 0, 0, 0, DateTimeKind.Utc).AddMonths(Properties.Settings.Default.AgreementMonthsUntilExpiration) 
                : DateTime.UtcNow + TimeSpan.FromDays(Properties.Settings.Default.TrialAgreementDaysUntilExpiration);
            if (license.Limitations.Time != null)
            {
                expiration = DateTime.UtcNow + license.Limitations.Time.Value;
            }
            var agreement = new Agreement(VersionedLicenseReference.VersionedReferenceFromLicense(license), 
                CurrentUserIdentity.UserInfo, decision, expiration);

            try
            {
                db.InsertAgreement(agreement);
            }
            catch (DataLayerException exception)
            {
                return exception.ToHttpResponse(Request);
            }

            return Request.CreateResponse(HttpStatusCode.OK);
        }

        private PageResult<dynamic> GetAllAgreements(DateTime? from, DateTime? until, ODataQueryOptions<Agreement> options)
        {
            RequireCompletedProfile();
            if (!CurrentUserIdentity.IsInPyxisAdminRole())
            {
                throw new HttpException((int)HttpStatusCode.Unauthorized, "Not authorized to access all agreements");
            }
            return (from.HasValue && until.HasValue) ? GetAllAgreementPageResults(from.Value, until.Value, options) : GetAllAgreementPageResults(options);
        }

        private PageResult<dynamic> GetLicenseAgreements(Guid licenseId, DateTime? from, DateTime? until, ODataQueryOptions<Agreement> options)
        {
            RequireCompletedProfile();
            var resource = db.GetResourceById(licenseId);
            if (resource == null)
            {
                throw new HttpException((int)HttpStatusCode.BadRequest, "No license with the specified Id");
            }
            if (resource.Metadata.User.Id != CurrentUserIdentity.ResourceId.Value && !CurrentUserIdentity.IsInPyxisAdminRole())
            {
                throw new HttpException((int)HttpStatusCode.Unauthorized, "Not authorized to access requested License's agreements");
            }

            var result = (from.HasValue && until.HasValue) ? db.GetAgreementsByLicenseId(licenseId, from.Value, until.Value) : db.GetAgreementsById(licenseId);
            return result.ToPageResult(Request, options);
        }

        private PageResult<dynamic> GetLicenseAgreements(Guid licenseId, Guid licenseVersion, DateTime? from, DateTime? until, ODataQueryOptions<Agreement> options)
        {
            RequireCompletedProfile();
            var resource = db.GetResourceByIdAndVersion(licenseId, licenseVersion);
            if (resource == null)
            {
                throw new HttpException((int)HttpStatusCode.BadRequest, "No license with the specified Id and Version");
            }
            if (resource.Metadata.User.Id != CurrentUserIdentity.ResourceId.Value && !CurrentUserIdentity.IsInPyxisAdminRole())
            {
                throw new HttpException((int)HttpStatusCode.Unauthorized, "Not authorized to access requested license's agreements");
            }

            var result = (from.HasValue && until.HasValue) ? db.GetAgreementsByLicenseVersionedId(licenseId, licenseVersion, from.Value, until.Value) : db.GetAgreementsByIdAndVersion(licenseId, licenseVersion);
            return result.ToPageResult(Request, options);
        }

        private PageResult<dynamic> GetAgreementsForCurrentUser(DateTime? from, DateTime? until, ODataQueryOptions<Agreement> options)
        {
            RequireCompletedProfile();
            return (from.HasValue && until.HasValue) ? GetUserAgreementPageResults(CurrentUserIdentity.ResourceId.Value, from.Value, until.Value, options) : GetUserAgreementPageResults(CurrentUserIdentity.ResourceId.Value, options);
        }

        private PageResult<dynamic> GetAgreementsForUser(Guid userId, DateTime? from, DateTime? until, ODataQueryOptions<Agreement> options)
        {
            RequireCompletedProfile();
            if (CurrentUserIdentity.ResourceId.Value != userId && !CurrentUserIdentity.IsInPyxisAdminRole())
            {
                throw new HttpException((int)HttpStatusCode.Unauthorized, "Not authorized to access requested User's agreements");
            }
            return (from.HasValue && until.HasValue) ? GetUserAgreementPageResults(userId, from.Value, until.Value, options) : GetUserAgreementPageResults(userId, options);
        }

        private PageResult<dynamic> GetUserAgreementPageResults(Guid userId, ODataQueryOptions<Agreement> options)
        {
            return db.GetAgreementsByUser(userId).ToPageResult(Request, options);
        }

        private PageResult<dynamic> GetUserAgreementPageResults(Guid userId, DateTime from, DateTime until, ODataQueryOptions<Agreement> options)
        {
            return db.GetAgreementsByUser(userId, from, until).ToPageResult(Request, options);
        }

        private PageResult<dynamic> GetAllAgreementPageResults(ODataQueryOptions<Agreement> options)
        {
            return db.GetAgreements().ToPageResult(Request, options);
        }

        private PageResult<dynamic> GetAllAgreementPageResults(DateTime from, DateTime until, ODataQueryOptions<Agreement> options)
        {
            return db.GetAgreements(from, until).ToPageResult(Request, options);
        }
    }
}