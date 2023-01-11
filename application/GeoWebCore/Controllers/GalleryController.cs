using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Web.Http;
using System.Web.Http.OData;
using System.Web.Http.OData.Extensions;
using System.Web.Http.OData.Query;
using GeoWebCore.Models;
using GeoWebCore.Utilities;
using GeoWebCore.WebConfig;
using Pyxis.Contract.Publishing;
using Pyxis.Publishing;
using Pyxis.Publishing.Permits;
using User = Pyxis.Publishing.User;
using Newtonsoft.Json;
using GeoWebCore.Services;
using GeoWebCore.Services.Cache;
using GeoWebCore.Services.Storage;
using Pyxis.Contract.DataDiscovery;

namespace GeoWebCore.Controllers
{
    /// <summary>
    /// GalleryController enables the user to upload files to a private storage and manage its file system structure.
    /// Also, the files then can be used for performing import operations.
    /// </summary>
    [RoutePrefix("api/v1")]
    public class GalleryController : ApiController
    {
        /// <summary>
        /// Settings to control OData pagination.
        /// </summary>
        private static readonly ODataQuerySettings s_ODataQuerySettings = new ODataQuerySettings() { PageSize = 50, EnsureStableOrdering = false };

        /// <summary>
        /// Get information about all galleries that a user can access
        /// </summary>
        /// <param name="options">Pagination options</param>
        /// <returns>Status code: 200 - operation succeeded</returns>
        /// <response code="401">Invalid user</response>
        /// <response code="500">Unable to get gallery information</response>
        [HttpGet]
        [Route("Galleries")]
        [LsAuthorizeUser]
        [TimeTrace("UserId")]
        public PageResult<dynamic> GetGalleries(ODataQueryOptions<Gallery> options)
        {
            try
            {
                // Get the galleries for this user
                var authenticatedUser = GetAuthenticatedUser(ActionContext.Request.Headers);
                var result = authenticatedUser.GetGalleries();

                // Send the information in JSON format as an array of objects
                var results = options.ApplyTo(result, s_ODataQuerySettings);
                return new PageResult<dynamic>(results as IEnumerable<dynamic>, Request.ODataProperties().NextLink, Request.ODataProperties().TotalCount);
            }
            catch (Exception e)
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.InternalServerError, e.Message));
            }
        }

        /// <summary>
        /// Get gallery content (root catalogs and datasets), linked servers and imports
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="path">Optional url inside the gallery</param>
        /// <returns>Status code: 200 - operation succeeded</returns>
        /// <response code="401">Invalid user</response>
        /// <response code="500">Unable to create token</response>
        [HttpGet]
        [Route("Galleries/{id}")]
        [AuthorizeGallery]
        [TimeTrace("UserId,id,path")]
        public GalleryContent GetGalleryContent(string id, string path = null)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id"));
            }

            var content = new GalleryContent
            {
                Id = Guid.Parse(id),
                Uri = path,
            };

            if (IsValidUri(path))
            {
                content.Catalogs = InMemoryServersCatalogsCache.GetCatalogs(path);
                content.DataSets = InMemoryServersCatalogsCache.GetDatasets(path);
            }
            else
            {
                content.Catalogs = new GalleryCatalogsController().BuildCatalogs(galleryId, path).Select(catalog => NormalizeCatalog(catalog, galleryId)).ToList();
                content.DataSets = new GalleryCatalogsController().BuildDatasets(galleryId, path).Select(dataset => NormalizeDataSet(dataset, galleryId)).ToList();
            }

            if (string.IsNullOrEmpty(path))
            {
                content.Imports = BuildImports(id).ToList();
                content.Servers = m_servers.GetAllServers(id);
                content.Urls = m_urls.GetAllUrls(id);
            }

            return content;
        }

        /// <summary>
        /// Get an authorization token for accessing a gallery
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <returns>Status code: 200 - operation succeeded</returns>
        /// <response code="401">Invalid user</response>
        /// <response code="500">Unable to create token</response>
        [HttpGet]
        [Route("Galleries/{id}/Token")]
        [LsAuthorizeUser]
        [TimeTrace("UserId,id")]
        public HttpResponseMessage GetGalleryToken(string id)
        {
            var userId = (string)ActionContext.ActionArguments[GlobalActionArgument.UserId];

            var authenticatedUser = GetAuthenticatedUser(ActionContext.Request.Headers);
            try
            {
                var token = GalleryGrantingAuthority.Issue(authenticatedUser, userId, id);

                return Request.CreateResponse(HttpStatusCode.OK, token);
            }
            catch (Exception)
            {
                return Request.CreateErrorResponse(HttpStatusCode.Unauthorized, "Authorization has been denied for this request.");
            }
        }

        /// <summary>
        /// Check if Uri looks like a valid uri. aka, public faceing uri.
        /// </summary>
        /// <param name="url">string that can be a uri</param>
        /// <returns>return true if this a valid uri</returns>
        public static bool IsValidUri(string url)
        {
            if (url == null)
            {
                return false;
            }

            if (url.StartsWith("http://",StringComparison.InvariantCultureIgnoreCase) ||
                url.StartsWith("https://",StringComparison.InvariantCultureIgnoreCase))
            {
                Uri uri;
                if (Uri.TryCreate(url,UriKind.Absolute,out uri))
                {
                    IPAddress address;
                    if (IPAddress.TryParse(uri.Host,out address))
                    {
                        //TODO: this is a security issue, we need beter way to verify Uri users are entering.
                        switch (address.AddressFamily)
                        {
                            case System.Net.Sockets.AddressFamily.InterNetwork:
                                var bytes = address.GetAddressBytes();
                                switch (bytes[0])
                                {
                                    case 10:
                                        return true;
                                    case 172:
                                        return bytes[1] < 32 && bytes[1] >= 16;
                                    case 192:
                                        return bytes[1] == 168;
                                    default:
                                        return false;
                                }
                            case System.Net.Sockets.AddressFamily.InterNetworkV6:
                                if (address.IsIPv6LinkLocal ||
                                    address.IsIPv6LinkLocal ||
                                    address.IsIPv6SiteLocal ||
                                    address.IsIPv6Teredo)
                                {
                                    return false;
                                }
                                break;
                        }                        
                    }
                    return true;
                }
            }

            return false;
        }
        
        /// <summary>
        /// Get import requests Ids for a given gallery
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="options">Pagination options</param>
        /// <returns>Status code: 200 - operation succeeded</returns>
        /// <response code="400">Invalid gallery id</response>
        /// <response code="401">Invalid gallery</response>
        /// <response code="404">Gallery does not exist</response>
        /// <response code="500">Unable to get importRequests Ids list</response>
        [HttpGet]
        [Route("Galleries/{id}/Imports")]
        [AuthorizeGallery]
        [TimeTrace("id")]
        public PageResult<dynamic> GetImports(string id, ODataQueryOptions<ImportDataSetRequestProgress> options)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id"));
            }

            // Check if url exists as a directory or a file
            if (!m_storage.DirectoryExists(galleryId))
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.NotFound, "Gallery does not exist"));
            }

            try
            {
                // Get the data sets from this url
                var result = BuildImports(galleryId).AsQueryable();

                // Send the import keys in JSON format
                var results = options.ApplyTo(result, s_ODataQuerySettings);
                return new PageResult<dynamic>(results as IEnumerable<dynamic>, Request.ODataProperties().NextLink, Request.ODataProperties().TotalCount);
            }
            catch (Exception e)
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.InternalServerError, e.Message));
            }
        }

        /// <summary>
        /// Build list of imports for a given gallery
        /// </summary>
        /// <param name="galleryId">Gallery id</param>
        /// <returns></returns>
        internal IEnumerable<ImportDataSetRequestProgress> BuildImports(string galleryId)
        {
            return ImportRequestsQueue.GetAllImportRequestsProgress(Guid.Parse(galleryId))
                .Concat(m_importses.GetAllImports(galleryId))
                .Select(x=>NormalizeImportRequest(x,galleryId))
                //return latest requests first
                .OrderByDescending(x=>x.RequestedTime);
        }

        /// <summary>
        /// Get import request details and progress
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="requestId">ImportDataSetRequest key</param>
        /// <param name="pull">if specificed, amount of seconds to wait before returning if import in progress or waiting</param>
        /// <returns>Status code: 200 - operation succeeded</returns>
        /// <response code="400">Invalid gallery id</response>
        /// <response code="401">Invalid gallery</response>
        /// <response code="404">Gallery or request does not exist</response>
        /// <response code="500">Unable to importRequest details</response>
        [HttpGet]
        [Route("Galleries/{id}/Imports/{requestId}")]
        [AuthorizeGallery]
        [TimeTrace("id,requestId,pull")]
        public ImportDataSetRequestProgress GetImport(string id, string requestId, int pull = 0)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id"));
            }

            //search active request
            var requestStatus = ImportRequestsQueue.Get(requestId);

            if (requestStatus != null)
            {
                //perform long pull if requested
                if (!requestStatus.IsCompleted && pull > 0)
                {
                    requestStatus.WaitForCompletion(TimeSpan.FromSeconds(Math.Min(pull,30)));
                }
                return NormalizeImportRequest(requestStatus.AsImportDataSetRequestProgress(),galleryId);
            }

            //search stored results
            var request = m_importses.GetImportRequest(id, requestId);

            if (request == null)
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.NotFound, "can't find import request"));
            }

            return NormalizeImportRequest(request,galleryId);
        }

        private DataSetCatalog NormalizeCatalog(DataSetCatalog catalog, string galleryId)
        {
            // Strip the gallery prefix from the catalog uri
            var galleryPath = m_storage.GetPath(galleryId);
        
            if (catalog.Uri.StartsWith(galleryPath) && catalog.Uri.Length > galleryPath.Length)
            {
                catalog.Uri = catalog.Uri.Remove(0, galleryPath.Length + 1);
            }
            else
            {
                throw new Exception("Invalid catalog url: " + catalog.Uri);
            }
            return catalog;
        }

        private DataSet NormalizeDataSet(DataSet dataset, string galleryId)
        {
            // Strip the gallery prefix from the catalog uri
            var galleryPath = m_storage.GetPath(galleryId);

            if (dataset.Uri.StartsWith(galleryPath) && dataset.Uri.Length > galleryPath.Length)
            {
                dataset.Uri = dataset.Uri.Remove(0, galleryPath.Length + 1);
            }
            else
            {
                throw new Exception("Invalid dataset url: " + dataset.Uri);
            }
            return dataset;
        }

        private ImportDataSetRequestProgress NormalizeImportRequest(ImportDataSetRequestProgress request,string galleryId)
        {
            //dirty but safe way to clone the request
            var clone = JsonConvert.DeserializeObject<ImportDataSetRequestProgress>(JsonConvert.SerializeObject(request));

            var galleryPath = m_storage.GetPath(galleryId);
            if (clone.Uri.StartsWith(galleryPath) && clone.Uri.Length > galleryPath.Length)
            {
                clone.Uri = clone.Uri.Remove(0, galleryPath.Length + 1);
            }
            return clone;
        }

        /// <summary>
        /// Get import request status
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="requestId">ImportDataSetRequest key</param>
        /// <returns>Status code: 200 - operation succeeded</returns>
        /// <response code="400">Invalid gallery id</response>
        /// <response code="401">Invalid gallery</response>
        /// <response code="404">Gallery or request does not exist</response>
        /// <response code="500">Unable to importRequest status</response>
        [HttpGet]
        [Route("Galleries/{id}/Imports/{requestId}/Status")]
        [AuthorizeGallery]
        [TimeTrace("id,requestId")]
        public ImportDataSetRequestProgressDetails GetImportStatus(string id, string requestId)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id"));
            }

            var requestStatus = ImportRequestsQueue.Get(requestId);

            if (requestStatus != null)
            {
                return new ImportDataSetRequestProgressDetails
                {
                    Status = requestStatus.Status,
                    Id = requestId,
                    Progress = 0
                };
            }

            var request = m_importses.GetImportRequest(id, requestId);

            if (request == null)
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.NotFound, "Can't find import request"));
            }

            //try to load the geosoure 
            var geoSource = m_importses.GetImportResult(id, request);

            if (geoSource != null)
            {
                return new ImportDataSetRequestProgressDetails
                {
                    Status = ImportDataSetRequestProgress.RequestStatus.Published,
                    Id = requestId,
                    Progress = 100
                };
            }
            else
            {
                //what should we return here?
                return new ImportDataSetRequestProgressDetails
                {
                    Status = ImportDataSetRequestProgress.RequestStatus.Failed,
                    Id = requestId,
                    Progress = 100
                };
            }
        }

        /// <summary>
        /// Get the result of importing a GeoSource 
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="requestId">ImportDataSetRequest key</param>
        /// <returns>Status code: 200 - operation succeeded</returns>
        /// <response code="400">Invalid gallery id</response>
        /// <response code="401">Invalid gallery</response>
        /// <response code="404">Gallery or request does not exist</response>
        /// <response code="500">Unable to get resulting GeoSource</response>
        [HttpGet]
        [Route("Galleries/{id}/Imports/{requestId}/GeoSource")]
        [AuthorizeGallery]
        [TimeTrace("id,requestId")]
        public GeoSource GetImportResult(string id, string requestId)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                throw new ApiException(FailureResponse.ErrorCodes.BadRequest, "Invalid gallery id");
            }

            var requestStatus = ImportRequestsQueue.Get(requestId);

            if (requestStatus != null)
            {
                if (requestStatus.IsCompleted)
                {
                    if (requestStatus.Exception != null)
                    {
                        throw requestStatus.Exception;
                    }

                    return requestStatus.GeoSource;
                }
                else
                {
                    throw new ApiException(FailureResponse.ErrorCodes.BadRequest, "Import request still in progress");
                }
            }

            var request = m_importses.GetImportRequest(id, requestId);

            if (request == null)
            {
                throw new ApiException(FailureResponse.ErrorCodes.NotFound , "Can't find import request");
            }

            //try to load the geosoure 
            var geoSource = m_importses.GetImportResult(id, request);

            if (geoSource != null)
            {
                return geoSource;
            }
            else
            {
                throw new ApiException(FailureResponse.ErrorCodes.NotFound, "Can't find result GeoSource for import request");
            }
        }

        /// <summary>
        /// Update a GeoSource information, including styles
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="styleRequest">Style request to use for update the GeoSource Metadata and Style</param>
        /// <returns>Status code: 200 - operation succeeded</returns>
        /// <returns cref="GeoWebCore.Models.ImportDataSetRequestProgress">The import requests progress if successful</returns>
        /// <response code="400">Invalid gallery id or style request</response>
        /// <response code="401">Invalid gallery</response>
        /// <response code="500">Unable to get data sets</response>
        [HttpPost]
        [Route("Galleries/{id}/GeoSources")]
        [AuthorizeGallery]
        [TimeTrace("id,publish")]
        public HttpResponseMessage UpdateGeoSource(string id, [FromBody] StyleRequest styleRequest)
        {
            //TODO: we realy should move "style" as a field in GeoSource json object and remove this operation
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            var userId = (string)ActionContext.ActionArguments[GlobalActionArgument.UserId];
            var userToken = (string)ActionContext.ActionArguments[GlobalActionArgument.UserToken];

            if (!id.Equals(galleryId))
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id");
            }

            if (styleRequest.GeoSource == null)
            {
                throw new ApiException(new FailureResponse(FailureResponse.ErrorCodes.BadRequest, "Invalid update GeoSource request").AddRequiredInformation("GeoSource", "please specify geo source to update"));                
            }

            if (styleRequest.GeoSource.Metadata.User.Id != Guid.Parse(userId))
            {
                throw new ApiException(new FailureResponse(FailureResponse.ErrorCodes.BadRequest, "Invalid update GeoSoruce request, invalid UserId"));
            }

            var publishedGeoSource = GeoSourceInitializer.GetGeoSource(styleRequest.GeoSource.Id);

            if (publishedGeoSource == null)
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid GeoSource Id");
            }

            //verify the published geoSource is up to date
            if (styleRequest.GeoSource.Metadata.User.Id != publishedGeoSource.Metadata.User.Id)
            {
                throw new ApiException(new FailureResponse(FailureResponse.ErrorCodes.BadRequest, "Invalid update GeoSoruce request, invalid UserId"));
            }

            var defaultStyle = GeoSourceInitializer.GetGeoSourceState(styleRequest.GeoSource.Id).GetStyle().Result;

            if (styleRequest.Style != null)
            {
                if (defaultStyle.Icon == styleRequest.Style.Icon &&
                    defaultStyle.Fill == styleRequest.Style.Fill &&
                    defaultStyle.Line == styleRequest.Style.Line &&
                    defaultStyle.ShowAsElevation.GetValueOrDefault(false) == styleRequest.Style.ShowAsElevation.GetValueOrDefault(false))
                {
                    //no style change
                    styleRequest.Style = null;    
                }
            }

            GeoSource resultGeoSource;
            
            if (styleRequest.Style == null && styleRequest.GeoSource.ProcRef == publishedGeoSource.ProcRef)
            {
                //detect simple metadata changes
                resultGeoSource = PublishGeoSourceHelper.UpdateGeoSourceMetadataForUser(userToken, styleRequest.GeoSource);   
            }
            else
            {
                resultGeoSource = PublishGeoSourceHelper.PublishGeoSourceForUser(userToken, styleRequest.GeoSource, styleRequest.Style);
            }

            //we just updated the GeoSource, broad cast the change 
            GeoSourceInitializer.BoradcastGeoSourceInvalidation(resultGeoSource.Id);

            //return updated GeoSource
            return Request.CreateResponse(resultGeoSource);
        }

        /// <summary>
        /// Import a data set and post it to the license server
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="importRequest">The import request</param>
        /// <param name="publish">enable or disable publishing resulting GeoSource</param>
        /// <returns>Status code: 200 - operation succeeded</returns>
        /// <returns cref="GeoWebCore.Models.ImportDataSetRequestProgress">The import requests progress if successful</returns>
        /// <response code="400">Invalid gallery id or import request</response>
        /// <response code="401">Invalid gallery</response>
        /// <response code="500">Unable to get data sets</response>
        [HttpPost]
        [Route("Galleries/{id}/Imports")]
        [AuthorizeGallery]
        [TimeTrace("id,publish")]
        public HttpResponseMessage Import(string id, [FromBody] ImportDataSetRequest importRequest,bool publish = true)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            var userId = (string)ActionContext.ActionArguments[GlobalActionArgument.UserId];
            var userToken = (string)ActionContext.ActionArguments[GlobalActionArgument.UserToken];
            if (!id.Equals(galleryId))
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id");
            }

            if (String.IsNullOrEmpty(importRequest.Uri))
            {
                throw new ApiException(new FailureResponse(FailureResponse.ErrorCodes.BadRequest, "Invalid import request").AddRequiredInformation("Uri", "DataSet uri"));                
            }

            if (String.IsNullOrEmpty(importRequest.Type))
            {
                throw new ApiException(new FailureResponse(FailureResponse.ErrorCodes.BadRequest, "Invalid import request").AddRequiredInformation("Type", "DataSet Type"));
            }

            //fix import request url
            var galleryPath = m_storage.GetPath(galleryId);

            if (!IsValidUri(importRequest.Uri))
            {
                importRequest.Uri = Path.Combine(galleryPath, importRequest.Uri);
            }

            var importRequestProgress = m_importses.GetImportRequest(galleryId, importRequest);

            if (importRequestProgress != null)
            {
                return Request.CreateResponse(NormalizeImportRequest(importRequestProgress,galleryId));
            }

            //publish is enabled only on production
            if (Program.RunInformation.Environment != GeoWebCoreRunInformation.RunEnvironment.Production)
            {
                publish = false;
            }

            var publishRequest = new PublishRequest
            {
                UserId = Guid.Parse(userId),
                GalleryId = Guid.Parse(galleryId),
                Token = userToken,
                Visibility = VisibilityType.Private,
                //please note, if publish = false, the new geosource will not be published to LS
                Enabled = publish
            };

            //start to get existing progress request
            var queuedImportRequest = ImportRequestsQueue.StartOrGetInProgress(
                importRequest,
                publishRequest,
                (request) =>
                {
                    if (request.Status == ImportDataSetRequestProgress.RequestStatus.Published)
                    {
                        m_importses.SaveImport(id, request.AsImportDataSetRequestProgress(), request.GeoSource);
                    }
                    if (request.Status == ImportDataSetRequestProgress.RequestStatus.Imported)
                    {
                        GeoSourceInitializer.InitializeLocalGeoSource(request.GeoSource);
                    }
                });

            if (queuedImportRequest != null)
            {
                return Request.CreateResponse(NormalizeImportRequest(queuedImportRequest.AsImportDataSetRequestProgress(),galleryId));
            }
            else
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest);
            }
        }

        [HttpPost]
        [Route("Galleries/{id}/PublishExpression")]
        [AuthorizeGallery]
        [TimeTrace("id")]
        public HttpResponseMessage PublishExpression(string id,[FromBody] PublishExpressionRequest request, int pull = 30)
        {
            var galleryId = (string)ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            var userId = (string)ActionContext.ActionArguments[GlobalActionArgument.UserId];
            var userToken = (string)ActionContext.ActionArguments[GlobalActionArgument.UserToken];
            if (!id.Equals(galleryId))
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "Invalid gallery id");
            }

            var publishRequest = new PublishRequest()
            {
                UserId = Guid.Parse(userId),
                GalleryId = Guid.Parse(galleryId),
                Token = userToken,
                Visibility = VisibilityType.Private,
                //please note, if publish = false, the new geosource will not be published to LS
                Enabled = true
            };

            var geoSourceTask = ExpressionPublisher.PublishExpressionAsync(request, publishRequest);

            if (geoSourceTask.IsFaulted)
            {
                //forget it until next time
                ExpressionPublisher.ForgetTask(geoSourceTask);
            }

            if (!geoSourceTask.IsCompleted && pull > 0)
            {
                geoSourceTask.Wait(TimeSpan.FromSeconds(Math.Min(pull,30)));
            }

            if (geoSourceTask.IsCompleted)
            {
                return Request.CreateResponse(geoSourceTask.Result);
            }
            else
            {
                return Request.CreateResponse(HttpStatusCode.RequestTimeout, "Calculation takes longer then expected");    
            }            
        }

       

        /// <summary>
        /// Create an authenticated user from the http request headers
        /// </summary>
        /// <param name="headers">The http request headers</param>
        /// <returns>An authenticated user</returns>
        private User GetAuthenticatedUser(HttpRequestHeaders headers)
        {
            // make authenticated channel
            var channel = new Channel(ApiUrl.ProductionLicenseServerRestAPI);
            var tokenDetails = new AccessToken.TokenDetails
            {
                Token = headers.Authorization.Parameter
            };
            var authenticatedChannel = channel.Authenticate(tokenDetails);

            // get a user for the channel
            return authenticatedChannel.AsUser();
        }

        /// <summary>
        /// Manages storage of user files
        /// </summary>
        private readonly UserFilesStorage m_storage = new UserFilesStorage();
        private readonly UserImportsStorage m_importses = new UserImportsStorage();
        private readonly UserServersStorage m_servers = new UserServersStorage();
        private readonly UserUrlsStorage m_urls = new UserUrlsStorage();
    }
}