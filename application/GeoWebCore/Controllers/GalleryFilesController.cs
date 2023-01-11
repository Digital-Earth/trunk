using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Threading.Tasks;
using System.Web.Http;
using System.Web.Http.OData;
using System.Web.Http.OData.Extensions;
using System.Web.Http.OData.Query;
using ApplicationUtility;
using GeoWebCore.Models;
using GeoWebCore.Utilities;
using GeoWebCore.WebConfig;
using GeoWebCore.Services.Storage;

namespace GeoWebCore.Controllers
{

    /// <summary>
    /// GalleryController enables the user to upload files to a private storage and manage its file system structure.
    /// Also, the files then can be used for performing import operations.
    /// </summary>
    [RoutePrefix("api/v1")]
    public class GalleryFilesController : ApiController
    {
        /// <summary>
        /// Settings to control OData pagination.
        /// </summary>
        private static readonly ODataQuerySettings s_ODataQuerySettings = new ODataQuerySettings() { PageSize = 50, EnsureStableOrdering = false };

        /// <summary>
        /// Get the gallery usage information
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="path">Path inside gallery</param>
        /// <returns>Status code: 200 - operation succeeded</returns>
        /// <response code="400">Invalid gallery id</response>
        /// <response code="401">Invalid gallery</response>
        /// <response code="500">Unable to get directories</response>
        [HttpGet]
        [Route("Galleries/{id}/Usage")]
        [AuthorizeGallery]
        [TimeTrace("id,path")]
        public GalleryUsageResponse GetUsage(string id, string path = null)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id"));
            }

            //ensure we have a valid file system directory
            if (String.IsNullOrEmpty(path) && !m_storage.DirectoryExists(galleryId))
            {
                m_storage.CreateDirectory(galleryId);
            }

            var root = m_storage.GetPath(galleryId, path);

            if (!Directory.Exists(root))
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid directory"));
            }

            try
            {
                var result = new GalleryUsageResponse();
                result.Sessions = 0;
                result.StorageUsed = 0;

                foreach (var session in new DirectoryInfo(root).EnumerateDirectories())
                {
                    result.Sessions++;
                    result.StorageUsed += session.EnumerateFiles("*", SearchOption.AllDirectories).Sum(x => x.Length);
                    if (!result.LastSessionUploaded.HasValue || result.LastSessionUploaded < session.CreationTimeUtc)
                    {
                        result.LastSessionUploaded = session.CreationTimeUtc;
                    }
                }

                return result;
            }
            catch (Exception e)
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.InternalServerError, e.Message));
            }
        }

        /// <summary>
        /// Find available directory name.
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="path">Path inside the gallery</param>
        /// <returns>Status code: 200 - operation succeeded</returns>
        /// <response code="400">Gallery id invalid, directory path not provided</response>
        /// <response code="401">Invalid gallery</response>
        /// <response code="500">Unable to create directory</response>
        [HttpPost]
        [Route("Galleries/{id}/AvailableDir")]
        [AuthorizeGallery]
        [TimeTrace("id,path")]
        public HttpResponseMessage AbailableDirectory(string id, string path)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id");
            }

            // Verify the argument
            if (path == null)
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Directory path not provided");
            }

            try
            {
                // Create gallery directory if it doesn't exist
                if (!m_storage.DirectoryExists(galleryId))
                {
                    m_storage.CreateDirectory(galleryId);
                }

                //create random 6 hex digits.
                var postfix = "-" + Guid.NewGuid().ToString().Substring(0,6);

                while (m_storage.DirectoryExists(galleryId, path+postfix))
                {
                    //create random 6 hex digits.
                    postfix = "-" + Guid.NewGuid().ToString().Substring(0, 6);
                }

                // Check if directory already exists
                return Request.CreateResponse(HttpStatusCode.OK, path + postfix);
            }
            catch (Exception e)
            {
                return Request.CreateErrorResponse(HttpStatusCode.InternalServerError, e.Message);
            }
        }

        /// <summary>
        /// Create a directory in a gallery
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="path">Path inside the gallery</param>
        /// <returns>Status code: 200 - operation succeeded</returns>
        /// <response code="400">Gallery id invalid, directory path not provided, or file with path already exists</response>
        /// <response code="401">Invalid gallery</response>
        /// <response code="500">Unable to create directory</response>
        [HttpPost]
        [Route("Galleries/{id}/Dirs")]
        [AuthorizeGallery]
        [TimeTrace("id,path")]
        public HttpResponseMessage CreateDirectory(string id, string path)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id");
            }

            // Verify the argument
            if (path == null)
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Directory path not provided");
            }

            try
            {
                // Create gallery directory if it doesn't exist
                if (!m_storage.DirectoryExists(galleryId))
                {
                    m_storage.CreateDirectory(galleryId);
                }

                // Check if directory already exists
                if (m_storage.DirectoryExists(galleryId, path))
                {
                    return Request.CreateResponse(HttpStatusCode.OK, "Directory already exists");
                }

                // Check if file with same name already exists
                if (m_storage.FileExists(galleryId, path))
                {
                    return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "File with path already exists");
                }

                // Create any missing directories in the path
                m_storage.CreateDirectory(galleryId, path);
                return Request.CreateResponse(HttpStatusCode.OK);
            }
            catch (Exception e)
            {
                return Request.CreateErrorResponse(HttpStatusCode.InternalServerError, e.Message);
            }
        }

        /// <summary>
        /// Create one or more files in a given directory with a multipart request. Existing files are overwritten.
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="path">Path inside the gallery</param>
        /// <returns>Status code: 200 - operation succeeded</returns>
        /// <response code="400">Invalid gallery id, file path not provided</response>
        /// <response code="401">Invalid gallery</response>
        /// <response code="500">Unable to create file</response>
        [HttpPost]
        [Route("Galleries/{id}/Files")]
        [AuthorizeGallery]
        [TimeTrace("id,path")]
        public async Task<HttpResponseMessage> CreateFiles(string id, string path)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id");
            }

            if (!Request.Content.IsMimeMultipartContent())
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid content. Must be multi-part.");
            }

            // Destination must be a directory
            if (!m_storage.DirectoryExists(galleryId, path))
            {
                m_storage.CreateDirectory(galleryId, path);
            }

            var fullPath = m_storage.GetPath(galleryId, path);
            var provider = new FileNamePreservingMultipartFormDataStreamProvider(fullPath);

            try
            {
                // Read the multipart data and create files
                var result = await Request.Content.ReadAsMultipartAsync(provider);

                // Validate filesize in information was provided
                var fileSize = result.FormData["filesize"];
                var filePath = result.FileData[0].LocalFileName;

                var fileInfo = new FileInfo(filePath);

                if (fileSize.HasContent())
                {
                    long exptectedFileSize = 0;
                    if (long.TryParse(fileSize, out exptectedFileSize) && exptectedFileSize != fileInfo.Length)
                    {
                        File.Delete(filePath);
                        throw new ApiException(FailureResponse.ErrorCodes.BadRequest,"file size verification failed. expected size is " + exptectedFileSize + " but sent file size is " + fileInfo.Length );
                    }
                }

                return Request.CreateResponse(HttpStatusCode.OK);
            }
            catch (Exception e)
            {
                return Request.CreateErrorResponse(HttpStatusCode.InternalServerError, e.Message);
            }
        }

        /// <summary>
        /// Get the directories in the specified path
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="options">Pagination options</param>
        /// <param name="path">Path inside gallery</param>
        /// <returns>Status code: 200 - operation succeeded</returns>
        /// <response code="400">Invalid gallery id</response>
        /// <response code="401">Invalid gallery</response>
        /// <response code="500">Unable to get directories</response>
        [HttpGet]
        [Route("Galleries/{id}/Dirs")]
        [AuthorizeGallery]
        [TimeTrace("id,path")]
        public PageResult<dynamic> GetDirectories(string id, ODataQueryOptions<string> options, string path = null)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id"));
            }

            try
            {
                IQueryable<string> result;

                // Verify that the folder exists
                if (!m_storage.DirectoryExists(galleryId, path))
                {
                    // folder does not exist, return empty result
                    result = new List<string>().AsQueryable();
                }
                else
                {
                    // Get contents of the folder
                    result = m_storage.GetDirectoryListing(galleryId, path).Subdirectories.AsQueryable();
                }

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
        /// Get the files in the specified path
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="options">Pagination options</param>
        /// <param name="path">Optional path inside the gallery</param>
        /// <returns>Status code: 200 - operation succeeded</returns>
        /// <response code="400">Invalid gallery id</response>
        /// <response code="401">Invalid user</response>
        /// <response code="500">Unable to get files</response>
        [HttpGet]
        [Route("Galleries/{id}/Files")]
        [AuthorizeGallery]
        [TimeTrace("id,path")]
        public PageResult<dynamic> GetFiles(string id, ODataQueryOptions<string> options, string path = null)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id"));
            }

            try
            {
                IQueryable<string> result;

                // Verify that the folder exists
                if (!m_storage.DirectoryExists(galleryId, path))
                {
                    // folder does not exist, return empty result
                    result = new List<string>().AsQueryable();
                }
                else
                {
                    // Get contents of the folder
                    result = m_storage.GetDirectoryListing(galleryId, path).Files.AsQueryable();
                }

                // Send the information in JSON format as an array of files
                var results = options.ApplyTo(result, s_ODataQuerySettings);
                return new PageResult<dynamic>(results as IEnumerable<dynamic>, Request.ODataProperties().NextLink, Request.ODataProperties().TotalCount);
            }
            catch (Exception e)
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.InternalServerError, e.Message));
            }
        }

        /// <summary>
        /// Manages storage of user files
        /// </summary>
        private readonly UserFilesStorage m_storage = new UserFilesStorage();
    }
}
