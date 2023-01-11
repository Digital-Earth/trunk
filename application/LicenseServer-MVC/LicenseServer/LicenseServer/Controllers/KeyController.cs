using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web;
using System.Web.Http;
using System.Web.Http.Description;
using LicenseServer.Extensions.PyxNet;
using LicenseServer.Models.Mongo;
using Pyxis.Contract;
using Pyxis.Contract.Publishing;
using PyxNet.Service;

namespace LicenseServer.Controllers
{
    [RoutePrefix("api/v1/Permit/Key")]
    public class KeyController : CORSMongoApiController
    {
        private static string s_bingKey = "AoB4BSMMQ6rX2eDUTQogtISxyNlYxjx9Ebf9-bxpwq0Aszlqeothmc1DT6iCY58F";
        
        public KeyController() 
        { }

        // Inject for test
        public KeyController(TestMongoSetup setup) :
            base(setup)
        { }

        // POST api/v1/Permit/Key/{keyGroup}/Request
        [Authorize]
        [Route("{keyGroup}/Request")]
        [ResponseType(typeof(KeyPermissionGrant))]
        public HttpResponseMessage RequestKey(Guid keyGroup)
        {
            var issuedTime = DateTime.UtcNow;
            var permit = new KeyPermit
            {
                Issued = issuedTime,
                Expires = issuedTime + TimeSpan.FromMinutes(5),
                ResourceId = Guid.Empty,
                Key = s_bingKey
            };
            var grant = new KeyPermissionGrant { Permits = new List<KeyPermit> { permit }, NotGranted = new List<DeniedPermit>() };
            return Request.CreateResponse(HttpStatusCode.OK, grant);
        }

        // POST api/v1/Permit/Key/{keyGroup}/Release
        [Authorize]
        [Route("{keyGroup}/Release")]
        public HttpResponseMessage ReleaseKey(Guid keyGroup)
        {
            return Request.CreateResponse(HttpStatusCode.OK);
        }
    }
}
