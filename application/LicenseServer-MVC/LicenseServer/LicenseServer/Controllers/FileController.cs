using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web.Http;
using LicenseServer.Models.Mongo;
using Microsoft.AspNet.Identity;

namespace LicenseServer.Controllers
{
    [RoutePrefix("api/v1/File")]
    public class FileController : BaseResourceController<File>
    {
        public FileController() 
        { }

        // Inject for test
        public FileController(TestMongoSetup setup) :
            base(setup)
        { }

        // POST api/v1/File
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public override HttpResponseMessage Post(File file)
        {
            if (file.FileStamp == null || file.MimeType == null || file.Size == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "File filestamp, MimeType, and Size must be specified");
            }
            return base.Post(file);
        }
    }
}
