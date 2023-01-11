using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web.Http;
using System.Web.Http.OData;
using System.Web.Http.OData.Extensions;
using System.Web.Http.OData.Query;
using GeoWebCore.WebConfig;
using Pyxis.Contract.DataDiscovery;
using Pyxis.IO.Import;
using GeoWebCore.Services;
using GeoWebCore.Services.Cache;
using GeoWebCore.Services.Storage;

namespace GeoWebCore.Controllers
{

    /// <summary>
    /// GalleryController enables the user to upload files to a private storage and manage its file system structure.
    /// Also, the files then can be used for performing import operations.
    /// </summary>
    [RoutePrefix("api/v1")]
    public class GalleryCatalogsController : ApiController
    {
        /// <summary>
        /// Settings to control OData pagination.
        /// </summary>
        private static readonly ODataQuerySettings s_ODataQuerySettings = new ODataQuerySettings() { PageSize = 50, EnsureStableOrdering = false };

        /// <summary>
        /// Get the catalogs in the specified url (non-recursive)
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="options">Pagination options</param>
        /// <param name="path">Optional url inside the gallery</param>
        /// <returns>Status code: 200 - operation succeeded</returns>
        /// <response code="400">Invalid gallery id</response>
        /// <response code="401">Invalid gallery</response>
        /// <response code="404">Gallery, file or directory not found</response>
        /// <response code="500">Unable to get catalogs</response>
        [HttpGet]
        [Route("Galleries/{id}/Catalogs")]
        [AuthorizeGallery]
        [TimeTrace("id,path")]
        public PageResult<dynamic> GetCatalogs(string id, ODataQueryOptions<DataSetCatalog> options, string path = null)
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

            if (GalleryController.IsValidUri(path))
            {
                try
                {
                    var result = InMemoryServersCatalogsCache.GetCatalogs(path).AsQueryable();

                    // Send the catalogs in JSON format
                    var results = options.ApplyTo(result, s_ODataQuerySettings);
                    return new PageResult<dynamic>(results as IEnumerable<dynamic>, Request.ODataProperties().NextLink, Request.ODataProperties().TotalCount);
                }
                catch (Exception e)
                {
                    throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.InternalServerError, e.Message));
                }
            }
            else
            {                
                // Check if gallery/url exists as a directory or a file
                if (!m_storage.DirectoryExists(galleryId, path) && !m_storage.FileExists(galleryId, path))
                {
                    throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.NotFound, "Path does not exist"));
                }

                try
                {
                    // Get the catalogs from this url
                    var result = BuildCatalogs(galleryId, path).AsQueryable();

                    // Strip the gallery prefix from the catalog uri
                    var galleryPath = m_storage.GetPath(galleryId);
                    foreach (var catalog in result)
                    {
                        if (catalog.Uri.StartsWith(galleryPath) && catalog.Uri.Length > galleryPath.Length)
                        {
                            catalog.Uri = catalog.Uri.Remove(0, galleryPath.Length + 1);
                        }
                        else
                        {
                            throw new Exception("Invalid catalog url: " + catalog.Uri);
                        }
                    }

                    // Send the catalogs in JSON format
                    var results = options.ApplyTo(result, s_ODataQuerySettings);
                    return new PageResult<dynamic>(results as IEnumerable<dynamic>, Request.ODataProperties().NextLink, Request.ODataProperties().TotalCount);
                }
                catch (Exception e)
                {
                    throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.InternalServerError, e.Message));
                }
            }
        }

        /// <summary>
        /// Get the data sets in the specified url
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="options">Pagination options</param>
        /// <param name="path">Optional url inside the gallery</param>
        /// <returns>Status code: 200 - operation succeeded</returns>
        /// <response code="400">Invalid gallery id</response>
        /// <response code="401">Invalid gallery</response>
        /// <response code="404">Gallery, directory or file does not exist</response>
        /// <response code="500">Unable to get data sets</response>
        [HttpGet]
        [Route("Galleries/{id}/DataSets")]
        [AuthorizeGallery]
        [TimeTrace("id,path")]
        public PageResult<dynamic> GetDataSets(string id, ODataQueryOptions<DataSet> options, string path = null)
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

            if (GalleryController.IsValidUri(path))
            {
                try
                {
                    var result = InMemoryServersCatalogsCache.GetDatasets(path).AsQueryable();

                    // Send the catalogs in JSON format
                    var results = options.ApplyTo(result, s_ODataQuerySettings);
                    return new PageResult<dynamic>(results as IEnumerable<dynamic>, Request.ODataProperties().NextLink, Request.ODataProperties().TotalCount);
                }
                catch (Exception e)
                {
                    throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.InternalServerError, e.Message));
                }
            }
            else
            {

                // Check if url exists as a directory or a file
                if (!m_storage.DirectoryExists(galleryId, path) && !m_storage.FileExists(galleryId, path))
                {
                    throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.NotFound, "Path does not exist"));
                }

                try
                {
                    // Get the data sets from this url
                    var result = BuildDatasets(galleryId, path).AsQueryable();

                    // strip the gallery url from the data set uri
                    var galleryPath = m_storage.GetPath(galleryId);
                    foreach (var dataSet in result)
                    {
                        if (dataSet.Uri.StartsWith(galleryPath) && dataSet.Uri.Length > galleryPath.Length)
                        {
                            dataSet.Uri = dataSet.Uri.Remove(0, galleryPath.Length + 1);
                        }
                    }

                    // Send the catalogs in JSON format
                    var results = options.ApplyTo(result, s_ODataQuerySettings);
                    return new PageResult<dynamic>(results as IEnumerable<dynamic>, Request.ODataProperties().NextLink, Request.ODataProperties().TotalCount);
                }
                catch (Exception e)
                {
                    throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.InternalServerError, e.Message));
                }
            }
        }

        internal List<DataSet> BuildDatasets(string galleryId, string path)
        {
            //check if we have gallery files
            if (!m_storage.DirectoryExists(galleryId))
            {
                return new List<DataSet>();
            }

            var fullPath = m_storage.GetPath(galleryId, path);

            if (string.IsNullOrEmpty(path))
            {
                var result = new List<DataSet>();

                //iterator over all results directories
                foreach (var session in Directory.EnumerateDirectories(fullPath))
                {
                    result.AddRange(GeoSourceInitializer.Engine.GetDataSets(session));
                }
                return result;
            }
            else
            {
                var result = GeoSourceInitializer.Engine.GetDataSets(fullPath);
                return result;
            }
        }

        internal List<DataSetCatalog> BuildCatalogs(string galleryId, string path)
        {
            //check if we have gallery files
            if (!m_storage.DirectoryExists(galleryId))
            {
                return new List<DataSetCatalog>();
            }

            var fullPath = m_storage.GetPath(galleryId, path);

            if (string.IsNullOrEmpty(path))
            {
                var result = new List<DataSetCatalog>();

                //iterator over all results directories
                foreach (var session in Directory.EnumerateDirectories(fullPath))
                {
                    result.AddRange(GeoSourceInitializer.Engine.GetCatalogs(session));
                }
                return result;
            }
            else
            {
                var result = GeoSourceInitializer.Engine.GetCatalogs(fullPath);
                return result;
            }
        }
        
        /// <summary>
        /// Manages storage of user files
        /// </summary>
        private readonly UserFilesStorage m_storage = new UserFilesStorage();
    }
}
