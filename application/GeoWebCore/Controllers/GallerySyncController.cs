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
using Pyxis.Contract.Publishing;
using Pyxis.IO.Import;

namespace GeoWebCore.Controllers
{

    /// <summary>
    /// GallerySyncController enables the user to keep remote files in sync with the current directory.
    /// </summary>
    [RoutePrefix("api/v1")]
    public class GallerySyncController : ApiController
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
        [Route("Galleries/{id}/Sync")]
        [AuthorizeGallery]
        [TimeTrace("id")]
        public HttpResponseMessage CreateSync(string id, [FromBody] SyncDetails syncDetails)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id");
            }

            // Verify the argument
            if (!GalleryController.IsValidUri(syncDetails.Uri))
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "uri server is not valid");
            }

            //create a new sync instance
            syncDetails.Id = Guid.NewGuid();
            syncDetails.Status = SyncStatus.Created;
            syncDetails.Created = syncDetails.Updated = DateTime.UtcNow;

            try
            {
                m_userSyncs.SaveSync(galleryId, syncDetails);                
                return Request.CreateResponse(HttpStatusCode.OK, syncDetails);
            }
            catch (Exception e)
            {
                return Request.CreateErrorResponse(HttpStatusCode.InternalServerError, e.Message);
            }
        }

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
        [Route("Galleries/{id}/Sync/{syncId}")]
        [AuthorizeGallery]
        [TimeTrace("id,syncId")]
        public HttpResponseMessage CreateSync(string id, string syncId, [FromBody] SyncDetails syncDetails)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id");
            }

            var foundDetails = m_userSyncs.GetSync(id, Guid.Parse(syncId));

            if (foundDetails == null)
            {
                return Request.CreateResponse(HttpStatusCode.NoContent);
            }

            //create a new sync instance
            if (syncDetails.Metadata != null)
            {
                foundDetails.Metadata.Name = syncDetails.Metadata.Name ?? foundDetails.Metadata.Name;
                foundDetails.Metadata.Description = syncDetails.Metadata.Description ?? foundDetails.Metadata.Description;
                foundDetails.Metadata.Tags = syncDetails.Metadata.Tags ?? foundDetails.Metadata.Tags;
                foundDetails.Metadata.SystemTags = syncDetails.Metadata.SystemTags ?? foundDetails.Metadata.SystemTags;
            }

            if (syncDetails.Uri != null)
            {
                foundDetails.Uri = syncDetails.Uri;
            }

            if (syncDetails.Interval.HasValue)
            {
                foundDetails.Interval = syncDetails.Interval;
            }

            foundDetails.Updated = DateTime.UtcNow;

            try
            {
                m_userSyncs.SaveSync(galleryId, foundDetails);
                return Request.CreateResponse(HttpStatusCode.OK, foundDetails);
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
        [Route("Galleries/{id}/Sync")]
        [AuthorizeGallery]
        [TimeTrace("id")]
        public PageResult<dynamic> GetSyncs(string id, ODataQueryOptions<SyncDetails> options)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id"));
            }

            try
            {
                // Get contents of the folder
                var result = m_userSyncs.GetAllSync(galleryId).AsQueryable();
                
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
        /// Get the servers linked to a gallery
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="options">Pagination options</param>
        /// <returns>Status code: 200 - operation succeeded</returns>
        /// <response code="400">Invalid gallery id</response>
        /// <response code="401">Invalid gallery</response>
        /// <response code="500">Unable to get servers</response>
        [HttpGet]
        [Route("Galleries/{id}/Sync/{syncId}")]
        [AuthorizeGallery]
        [TimeTrace("id,syncId")]
        public HttpResponseMessage GetSync(string id, string syncId)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id"));
            }

            var foundDetails = m_userSyncs.GetSync(id, Guid.Parse(syncId));

            if (foundDetails == null)
            {
                return Request.CreateResponse(HttpStatusCode.NoContent);
            }

            return Request.CreateResponse(HttpStatusCode.OK, foundDetails);
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
        [Route("Galleries/{id}/Sync/{syncId}")]
        [AuthorizeGallery]
        [TimeTrace("id,snycId")]
        public HttpResponseMessage DeleteServer(string id, Guid snycId)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id"));
            }

            try
            {
                // Get contents of the folder
                var syncDetails = m_userSyncs.GetSync(galleryId, snycId);

                if (syncDetails != null)
                {
                    if (m_userSyncs.DeleteSync(galleryId, syncDetails))
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
        private readonly UserSyncStorage m_userSyncs = new UserSyncStorage();
    }
}
