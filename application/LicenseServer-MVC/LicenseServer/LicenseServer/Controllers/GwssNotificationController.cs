using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Net.Sockets;
using System.Web.Http;
using LicenseServer.DTOs;
using LicenseServer.Models;
using LicenseServer.Models.Mongo;
using MongoDB.Driver;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Controllers
{
    [RoutePrefix("api/v1/GwssNotification")]
    public class GwssNotificationController : CORSMongoApiController
    {
        public GwssNotificationController() 
        { }

        // Inject for test
        public GwssNotificationController(TestMongoSetup setup) :
            base(setup)
        { }

        // GET api/v1/GwssNotification
        public IQueryable<Gwss> Get()
        {
            return db.GetGwsses();
        }

        // GET api/v1/GwssNotification/5
        public Gwss Get(Guid id)
        {
            return db.GetGwssById(id);
        }

        // POST api/v1/GwssNotification/5
        public HttpResponseMessage Post(Guid id, GwssStatus gwssStatus)
        {
            Retry.Execute(() => db.UpdateGwssStatus(id, gwssStatus));
            var lsRequest = Retry.Execute(() => GenerateLsRequest(id));

            var response = Request.CreateResponse(HttpStatusCode.Created, lsRequest);
            response.Headers.Location = new Uri(Url.Link("DefaultApi", new { id = id }));
            return response;
        }

        // DELETE api/v1/GwssNotification/5
        public HttpResponseMessage Delete(Guid id)
        {
            db.RemoveGwss(id);

            return Request.CreateResponse(HttpStatusCode.OK);
        }

        private LsStatus GenerateLsRequest(Guid id)
        {
            var gwss = db.GetGwssById(id);
            if (gwss != null && gwss.Request != null)
            {
                // Other events will trigger changes to the server request
                return gwss.Request;
            }
            // Create an empty request
            return new LsStatus();
        }
    }
}
