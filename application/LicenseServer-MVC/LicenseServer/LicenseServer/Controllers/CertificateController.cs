using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web;
using System.Web.Http;
using System.Web.Http.Description;
using LicenseServer.Extensions.PyxNet;
using Microsoft.AspNet.Identity;
using Pyxis.Contract.Publishing;
using PyxNet.Service;

namespace LicenseServer.Controllers
{
    [RoutePrefix("api/v1/Permit/Certificate")]
    public class CertificateController : CORSMongoApiController
    {
        private static readonly TrustedNodes s_trustedNodes;

        static CertificateController()
        {
            s_trustedNodes = new TrustedNodes
            {
                Nodes = new List<PyxNetNodeId> { PyxNetConfig.PrimaryLicenseService.NodeId.ToPyxNetNodeId(), PyxNetConfig.SecondaryLicenseService.NodeId.ToPyxNetNodeId() }
            };
        }

        public CertificateController() 
        { }

        // Inject for test
        public CertificateController(TestMongoSetup setup) :
            base(setup)
        { }

        // GET api/v1/Permit/Certificate/Trusted
        [Route("Trusted")]
        [ResponseType(typeof(TrustedNodes))]
        public HttpResponseMessage GetTrustedNodes()
        {
            return Request.CreateResponse(HttpStatusCode.OK, s_trustedNodes);
        }

        // POST api/v1/Permit/Certificate/Request
        [Authorize]
        [Route("Request")]
        [ResponseType(typeof(CertificatePermissionGrant))]
        public HttpResponseMessage RequestCertificate(PyxNetPermissionRequest permissionRequest)
        {
            RequireCompletedProfile();
            if (permissionRequest == null || permissionRequest.NodeId == null || permissionRequest.ResourceIds == null)
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Empty or incorrectly formatted request");
            }
            if (!permissionRequest.Format.Equals(PermissionFormats.PyxNetV1))
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Only application/pyxnet-v1 format type is supported");
            }
            if (!permissionRequest.ResourceIds.Any())
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "No ResourceIds specified to request permission for");
            }
            var geoSourceIds = db.GetResourcesByIds<Models.Mongo.GeoSource>(permissionRequest.ResourceIds.Distinct().ToList())
                .Select(g => new GeoSourceIdentifiers { Id = g.Id, Name = g.Metadata.Name, ProcRef = g.ProcRef, Licenses = g.Licenses }).ToList();
            if (!geoSourceIds.Any())
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "No GeoSources found with the specified ResourceIds");
            }
            var geoSourceLicenses = geoSourceIds.SelectMany(g => g.Licenses).ToList();
            var agreedLicenses = new List<Agreement>();
            if (geoSourceLicenses.Any())
            {
                var notGranted = new List<DeniedPermit>();
                agreedLicenses = db.GetActiveAgreementsByUserAndLicenseReferences(CurrentUserIdentity.ResourceId.Value, geoSourceLicenses).ToList();
                foreach (var geoSource in geoSourceIds)
                {
                    if (geoSource.Licenses.Any() && !geoSource.Licenses.Any(l => agreedLicenses.Any(a => l.Id == a.ResourceId)))
                    {
                        notGranted.Add(new DeniedPermit { ResourceId = geoSource.Id, Message = "You must accept a license agreement for GeoSource: " + geoSource.Name });
                    }
                }
                if (notGranted.Any())
                {
                    return Request.CreateResponse(HttpStatusCode.Unauthorized, new CertificatePermissionGrant { Permits = new List<CertificatePermit>(), NotGranted = notGranted });
                }
            }

            var permissionGrant = GenerateGeoSourcePermissionGrant(permissionRequest, geoSourceIds, agreedLicenses);
            return Request.CreateResponse(HttpStatusCode.OK, permissionGrant);
        }

        private CertificatePermissionGrant GenerateGeoSourcePermissionGrant(PyxNetPermissionRequest permissionRequest, List<GeoSourceIdentifiers> geoSourceIds, List<Agreement> agreements)
        {
            var permits = new List<CertificatePermit>();
            foreach (var geoSource in geoSourceIds)
            {
                var userResourceId = (CurrentUserIdentity != null && CurrentUserIdentity.ResourceId != null) ? new ResourceId(CurrentUserIdentity.ResourceId.Value) : new ResourceId(Guid.Empty);
                var nodeId = permissionRequest.NodeId.ToNodeId();
                var agreement = agreements.FirstOrDefault(a => geoSource.Licenses.Any(l => l.Id == a.ResourceId));
                var geoSourcePermissionFact =
                    (agreement == null
                    ? new GeoSourcePermissionFact(new ResourceId(geoSource.Id), userResourceId, nodeId, geoSource.ProcRef, null, null)
                    : new GeoSourcePermissionFact(new ResourceId(geoSource.Id), userResourceId, nodeId, geoSource.ProcRef, null, agreement.Limitations.ToPermissionLimitations()));
                var issuedTime = DateTime.UtcNow;
                var expiration = issuedTime + TimeSpan.FromDays(1);
                if (agreement != null && expiration > agreement.Expiration)
                {
                    expiration = agreement.Expiration;
                }
                var certificate = CertifiableFactCertificateHelper.Create(PyxNetConfig.PrimaryLicenseService.PrivateKey, PyxNetConfig.PrimaryLicenseService.ServiceInstance,
                    expiration, geoSourcePermissionFact);
                var permit = new CertificatePermit
                {
                    Issued = issuedTime,
                    Expires = expiration,
                    Format = PermissionFormats.PyxNetV1,
                    ResourceId = geoSource.Id,
                    Certificate = certificate.SerializationString
                };
                permits.Add(permit);
            }
            return new CertificatePermissionGrant { Permits = permits, NotGranted = new List<DeniedPermit>() };
        }

        private class GeoSourceIdentifiers
        {
            public Guid Id { get; set; }
            public string Name { get; set; }
            public string ProcRef { get; set; }
            public List<LicenseReference> Licenses { get; set; }
        }
    }
}