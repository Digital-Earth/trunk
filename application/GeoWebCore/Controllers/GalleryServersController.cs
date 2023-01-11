using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web.Http;
using System.Web.Http.OData;
using System.Web.Http.OData.Extensions;
using System.Web.Http.OData.Query;
using GeoWebCore.WebConfig;
using GeoWebCore.Services.Storage;
using Pyxis.Contract.DataDiscovery;
using Pyxis.IO.Import;

namespace GeoWebCore.Controllers
{

    /// <summary>
    /// GalleryController enables the user to upload files to a private storage and manage its file system structure.
    /// Also, the files then can be used for performing import operations.
    /// </summary>
    [RoutePrefix("api/v1")]
    public class GalleryServersController : ApiController
    {
        /// <summary>
        /// Settings to control OData pagination.
        /// </summary>
        private static readonly ODataQuerySettings s_ODataQuerySettings = new ODataQuerySettings() { PageSize = 50, EnsureStableOrdering = false };

        /// <summary>
        /// link a server to a gallery
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="uri">server uri</param>
        /// <returns>Status code: 200 - operation succeeded</returns>
        /// <response code="400">Gallery id invalid, directory path not provided, or file with path already exists</response>
        /// <response code="401">Invalid gallery</response>
        /// <response code="500">Unable to create directory</response>
        [HttpPost]
        [Route("Galleries/{id}/Servers")]
        [AuthorizeGallery]
        [TimeTrace("id,uri")]
        public HttpResponseMessage CreateServer(string id, string uri)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id");
            }

            // Verify the argument
            if (!GalleryController.IsValidUri(uri))
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "uri server is not valid");
            }

            try
            {
                var catalog = Program.Engine.BuildCatalog(uri);

                if (catalog == null)
                {
                    return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "uri is not supported");
                }
                m_userServers.SaveServer(galleryId,catalog);                
                return Request.CreateResponse(HttpStatusCode.OK, catalog);
            }
            catch (Exception e)
            {
                return Request.CreateErrorResponse(HttpStatusCode.InternalServerError, e.Message);
            }
        }

        /// <summary>
        /// Get the servers linked to a gallery
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="options">Pagination options</param>
        /// <returns>Status code: 200 - operation succeeded</returns>
        /// <response code="400">Invalid gallery id</response>
        /// <response code="401">Invalid gallery</response>
        /// <response code="500">Unable to get servers</response>
        [HttpGet]
        [Route("Galleries/{id}/Servers")]
        [AuthorizeGallery]
        [TimeTrace("id")]
        public PageResult<dynamic> GetServers(string id, ODataQueryOptions<DataSetCatalog> options)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id"));
            }

            try
            {
                // Get contents of the folder
                var result = m_userServers.GetAllServers(galleryId).AsQueryable();
                
                // Send the information in JSON format as an array of directories
                var results = options.ApplyTo(result, s_ODataQuerySettings);
                return new PageResult<dynamic>(results as IEnumerable<dynamic>, Request.ODataProperties().NextLink, Request.ODataProperties().TotalCount);
            }
            catch (Exception e)
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.InternalServerError, e.Message));
            }
        }

        /// <summary>
        /// unlink a server from a gallery
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="uri">server uri</param>
        /// <returns>Status code: 202 - operation succeeded</returns>
        /// <response code="204">no server found to delete</response>
        /// <response code="400">Invalid gallery id</response>
        /// <response code="500">Unable to get servers</response>
        [HttpDelete]
        [Route("Galleries/{id}/Servers")]
        [AuthorizeGallery]
        [TimeTrace("id")]
        public HttpResponseMessage DeleteServer(string id, string uri)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id"));
            }

            try
            {
                // Get contents of the folder
                var foundServer = m_userServers.GetAllServers(galleryId).FirstOrDefault(server => server.Uri == uri);

                if (foundServer != null)
                {
                    if (m_userServers.DeleteServer(galleryId, foundServer))
                    {
                        return Request.CreateResponse(HttpStatusCode.Accepted);
                    }
                }

                return Request.CreateResponse(HttpStatusCode.NoContent);
            }
            catch (Exception e)
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.InternalServerError, e.Message));
            }
        }

        /// <summary>
        /// Manages storage of user files
        /// </summary>
        private readonly UserServersStorage m_userServers = new UserServersStorage();
    }
}
