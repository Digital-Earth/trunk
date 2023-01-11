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
using LicenseServer.App_Start;
using LicenseServer.Extensions.PyxNet;
using LicenseServer.Models;
using LicenseServer.Models.Mongo;
using Pyxis.Contract;
using Pyxis.Contract.Publishing;
using PyxNet.Service;

namespace LicenseServer.Controllers
{
    [RoutePrefix("api/v1/Parameter")]
    public class ParameterController : CORSMongoApiController
    {
        public ParameterController()
        { }

        // Inject for test
        public ParameterController(TestMongoSetup setup) :
            base(setup)
        { }

        // GET api/v1/Parameter
        public PageResult<KeyValuePair<string, object>> Get(ODataQueryOptions<KeyValuePair<string, object>> options)
        {
            var result = db.GetParameters();

            var results = options.ApplyTo(result, ODataConfig.ODataQuerySettings);

            return new PageResult<KeyValuePair<string, object>>(results as IEnumerable<KeyValuePair<string, object>>, Request.ODataProperties().NextLink, Request.ODataProperties().TotalCount);
        }

        // GET api/v1/Parameter/{Key}
        [Route("{key}")]
        [ResponseType(typeof(object))]
        public HttpResponseMessage Get(string key)
        {
            var value = db.GetParameter(key);
            return value == null ? Request.CreateResponse(HttpStatusCode.NotFound) : Request.CreateResponse(HttpStatusCode.OK, value);
        }

        // POST api/v1/Parameter
        [Authorize(Roles = PyxisIdentityRoles.SiteAdmin)]
        public HttpResponseMessage Post(KeyValuePair<string, object> pair)
        {
            if (String.IsNullOrEmpty(pair.Key) || pair.Key.Any(c => char.IsWhiteSpace(c)))
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Valid keys are non-empty and contain no whitespace");
            } 
            var existingValue = db.GetParameter(pair.Key);
            if (existingValue != null)
            {
                return Request.CreateResponse(HttpStatusCode.Conflict, "Specified key already exists");
            }
            try
            {
                db.UpdateParameter(pair);
                var response = Request.CreateResponse(HttpStatusCode.Created, pair);
                response.Headers.Location = new Uri(Url.Link("DefaultApi", new { id = pair.Key }));
                return response;
            }
            catch (DataLayerException exception)
            {
                return exception.ToHttpResponse(Request);
            }
        }

        // PUT api/v1/Parameter/{Key}
        [Authorize(Roles = PyxisIdentityRoles.SiteAdmin)]
        [Route("{key}")]
        public HttpResponseMessage Put(string key, KeyValuePair<string, object> pair)
        {
            if (key != pair.Key)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "URL key doesn't match key-value pair");
            }
            try
            {
                db.UpdateParameter(pair);
                var response = Request.CreateResponse(HttpStatusCode.Created, pair);
                return response;
            }
            catch (DataLayerException exception)
            {
                return exception.ToHttpResponse(Request);
            }
        }
    }
}
