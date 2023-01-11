using System;
using System.Collections.Generic;
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
using Pyxis.Contract.Publishing;
using GeoWebCore.Services;
using GeoWebCore.Services.Cluster;
using GeoWebCore.Services.Storage;
using Newtonsoft.Json;
using Pyxis.Contract.DataDiscovery;
using Pyxis.IO.DataDiscovery;
using Pyxis.IO.Import;
using Pyxis.IO.Search;
using Pyxis.Utilities;
using UrlDiscoveryStatus = GeoWebCore.Services.Storage.UrlDiscoveryStatus;

namespace GeoWebCore.Controllers
{
    /// <summary>
    /// GalleryUrlsController enables the user to connect to remote urls and make them searchable and to trigger import if required
    /// Please note, this also should be integrated with a global search
    /// </summary>
    [RoutePrefix("api/v1")]
    public class GalleryUrlsController : ApiController
    {
        /// <summary>
        /// Settings to control OData pagination.
        /// </summary>
        private static readonly ODataQuerySettings s_ODataQuerySettings = new ODataQuerySettings()
        {
            PageSize = 50,
            EnsureStableOrdering = false
        };

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
        [Route("Galleries/{id}/Urls")]
        [AuthorizeGallery]
        [TimeTrace("id,uri")]
        public HttpResponseMessage PostUrl(string id, string uri)
        {
            var galleryId = (string) ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
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
                var discoverResult = Program.Engine.Discover(uri).Result;

                if (!discoverResult.ServiceIdentifier.HasContent())
                {
                    return Request.CreateErrorResponse(HttpStatusCode.BadRequest, "uri is not supported");
                }

                var discoveryStatus = UrlDiscoveryStatus.FromUri(uri);

                discoveryStatus.Status = "Discovering";

                m_userUrls.SaveUrl(galleryId, discoveryStatus, new List<DataSet>());

                //start discovery in the background - no await been called
                DiscoverUrl(galleryId, discoveryStatus);

                return Request.CreateResponse(HttpStatusCode.OK, discoveryStatus);
            }
            catch (Exception e)
            {
                return Request.CreateErrorResponse(HttpStatusCode.InternalServerError, e.Message);
            }
        }

        private async Task DiscoverUrl(string galleryId, UrlDiscoveryStatus discoveryStatus)
        {
            var urls = new Stack<string>();
            var dataSets = new List<DataSet>();

            urls.Push(discoveryStatus.Uri);
            var counter = 0;

            try
            {
                while (urls.Count > 0)
                {
                    var url = urls.Pop();
                    counter++;

                    try
                    {
                        var discoverResult = await Program.Engine.Discover(url);

                        if (discoverResult != null)
                        {
                            if (discoverResult.DataSet != null)
                            {
                                ConvertBBoxToWgs84(discoverResult.DataSet);
                                dataSets.Add(discoverResult.DataSet);

                                discoveryStatus.DataSetCount = dataSets.Count;
                            }

                            if (discoverResult.Leads != null)
                            {
                                foreach (var lead in discoverResult.Leads)
                                {
                                    urls.Push(lead.Uri);

                                }
                            }

                            discoveryStatus.Status = "Discovering (" + urls.Count + " leads)";

                            if (counter%10 == 0)
                            {
                                //update progress
                                m_userUrls.SaveUrl(galleryId, discoveryStatus, null);
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine("Error while discovering {0} : {1}", url, ex.Message);
                    }
                }

                discoveryStatus.Status = "Cataloged";
            }
            catch (Exception)
            {
                discoveryStatus.Status = "Error";
            }

            discoveryStatus.DataSetCount = dataSets.Count;
            m_userUrls.SaveUrl(galleryId, discoveryStatus, dataSets);
            InvalidateGazetteerFor(galleryId);
        }

        private void ConvertBBoxToWgs84(DataSet dataSet)
        {

            if (dataSet.BBox == null)
            {
                return;
            }

            try
            {
                dataSet.BBox = dataSet.BBox.ConvertBBoxToWgs84();
            }
            catch (Exception ex)
            {
                //bboxs are broken
                dataSet.BBox = null;
                dataSet.AddSystemTag("BrokenBBox");
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
        [Route("Galleries/{id}/Urls")]
        [AuthorizeGallery]
        [TimeTrace("id")]
        public PageResult<dynamic> GetUrls(string id, ODataQueryOptions<UrlDiscoveryStatus> options)
        {
            var galleryId = (string) ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.BadRequest,
                    "Invalid gallery id"));
            }

            try
            {
                // Get contents of the folder
                var result = m_userUrls.GetAllUrls(galleryId).AsQueryable();

                // Send the information in JSON format as an array of directories
                var results = options.ApplyTo(result, s_ODataQuerySettings);
                return new PageResult<dynamic>(results as IEnumerable<dynamic>, Request.ODataProperties().NextLink,
                    Request.ODataProperties().TotalCount);
            }
            catch (Exception e)
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.InternalServerError,
                    e.Message));
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
        [Route("Galleries/{id}/Urls")]
        [AuthorizeGallery]
        [TimeTrace("id")]
        public HttpResponseMessage DeleteUrl(string id, string uri)
        {
            var galleryId = (string) ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.BadRequest,
                    "Invalid gallery id"));
            }

            try
            {
                // Get contents of the folder
                var foundServer = m_userUrls.GetAllUrls(galleryId).FirstOrDefault(server => server.Uri == uri);

                if (foundServer != null)
                {
                    if (m_userUrls.DeleteUrl(galleryId, foundServer))
                    {
                        InvalidateGazetteerFor(galleryId);
                        return Request.CreateResponse(HttpStatusCode.Accepted);
                    }
                }

                return Request.CreateResponse(HttpStatusCode.NoContent);
            }
            catch (Exception e)
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.InternalServerError,
                    e.Message));
            }
        }

        /// <summary>
        /// unlink a server from a gallery
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="uri">server uri</param>
        /// <param name="limitDataSets">Amount of datasets to verify</param>
        /// <param name="limitFeatures">Maximum number of features to verify. verification will skip large datasets</param>
        /// <returns>Status code: 202 - operation succeeded</returns>
        /// <response code="204">no server found to delete</response>
        /// <response code="400">Invalid gallery id</response>
        /// <response code="500">Unable to get servers</response>
        [HttpPost]
        [Route("Galleries/{id}/Urls/Verify")]
        [AuthorizeGallery]
        [TimeTrace("id")]
        public HttpResponseMessage VerifyUrl(string id, string uri, int limitDataSets = 100, long limitFeatures = 10000)
        {
            var galleryId = (string) ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.BadRequest,
                    "Invalid gallery id"));
            }

            try
            {
                // Get contents of the folder
                var foundServer = m_userUrls.GetAllUrls(galleryId).FirstOrDefault(server => server.Uri == uri);

                if (foundServer != null)
                {
                    var dataSets = m_userUrls.GetDataSets(galleryId, foundServer);

                    //ensure all dataSets have discovery reports
                    dataSets.ForEach((dataSet) =>
                    {
                        if (dataSet.DiscoveryReport == null)
                        {
                            dataSet.DiscoveryReport = new DataSetDiscoveryReport();
                        }
                    });

                    var datasetsToVerify = dataSets.Where(
                            x =>
                                x.DiscoveryReport.Status == DataSetDiscoveryStatus.Unknown &&
                                (x.DiscoveryReport.Issues == null || x.DiscoveryReport.Issues.Count == 0) &&
                                x.DiscoveryReport.FeaturesCount < limitFeatures)
                        .OrderBy(x => x.DiscoveryReport.FeaturesCount)
                        .Take(limitDataSets).ToList();


                    var userId = (string) ActionContext.ActionArguments[GlobalActionArgument.UserId];
                    var userToken = (string) ActionContext.ActionArguments[GlobalActionArgument.UserToken];

                    var publishRequest = new PublishRequest
                    {
                        UserId = Guid.Parse(userId),
                        GalleryId = Guid.Parse(galleryId),
                        Token = userToken,
                        Visibility = VisibilityType.Private,
                        Enabled = false
                    };

                    foreach (var dataSet in datasetsToVerify)
                    {
                        Console.WriteLine("Verifiying: {0}", dataSet.Uri);

                        var start = DateTime.UtcNow;

                        try
                        {

                            dataSet.DiscoveryReport.Issues = null;
                            dataSet.DiscoveryReport.DiscoveredTime = DateTime.UtcNow;

                            var progress = ImportRequestsQueue.StartOrGetInProgress(new ImportDataSetRequest(dataSet),
                                publishRequest);
                            progress.WaitForCompletion();

                            if (progress.Status == ImportDataSetRequestProgress.RequestStatus.Imported)
                            {
                                dataSet.DiscoveryReport.Status = DataSetDiscoveryStatus.Successful;
                                dataSet.DiscoveryReport.ImportTime = DateTime.UtcNow - start;

                                if (progress.GeoSource.DataSize.HasValue)
                                {
                                    dataSet.DiscoveryReport.DataSize = progress.GeoSource.DataSize.Value;
                                }

                                if (dataSet.DiscoveryReport.FeaturesCount == 0 &&
                                    progress.GeoSource.Specification.OutputType ==
                                    PipelineSpecification.PipelineOutputType.Feature)
                                {
                                    var process = GeoSourceInitializer.Initialize(progress.GeoSource);
                                    var featureGroup = pyxlib.QueryInterface_IFeatureGroup(process.getOutput());
                                    dataSet.DiscoveryReport.FeaturesCount = featureGroup.getFeaturesCount().max;
                                }

                                Console.WriteLine("Verified: {0} @ {1}", dataSet.Uri, dataSet.DiscoveryReport.ImportTime);
                            }
                            else
                            {
                                dataSet.DiscoveryReport.AddError(progress.Exception);
                                dataSet.DiscoveryReport.Status = DataSetDiscoveryStatus.Failed;

                                Console.WriteLine("Verification failed: {0}", dataSet.Uri);
                                foreach (var error in dataSet.DiscoveryReport.Issues)
                                {
                                    Console.WriteLine("  {0}", error);
                                }
                            }
                        }
                        catch (Exception ex)
                        {
                            dataSet.DiscoveryReport.AddError(ex);
                            dataSet.DiscoveryReport.Status = DataSetDiscoveryStatus.Failed;

                            Console.WriteLine("Verification failed: {0}", dataSet.Uri);
                            foreach (var error in dataSet.DiscoveryReport.Issues)
                            {
                                Console.WriteLine("  {0}", error);
                            }
                        }
                    }

                    m_userUrls.SaveUrl(galleryId, foundServer, dataSets);

                    return Request.CreateResponse(datasetsToVerify);
                }
                else
                {
                    throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.BadRequest,
                        "Server not found"));
                }
            }
            catch (Exception e)
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.InternalServerError,
                    e.Message));
            }
        }

        /// <summary>
        /// Search for relevant datasets in a gallery
        /// </summary>
        /// <param name="id">The gallery id</param>
        /// <param name="search">search string</param>
        /// <param name="center">center (lon,lat,radius) for near search and filtering</param>
        /// <returns>Search Results with aggregations sugguestions to allow UI for search Results</returns>
        /// <response code="400">Invalid gallery id</response>
        /// <response code="401">Invalid gallery</response>
        /// <response code="500">Unable to get servers</response>
        [HttpGet]
        [Route("Galleries/{id}/Urls/Search")]
        [AuthorizeGallery]
        [TimeTrace("id,search,center")]
        public SearchResults SearchGallery(string id, string search, string center = null, string bbox = null)
        {
            var options = PyxisOData.FromRequest(Request);

            var galleryId = (string) ActionContext.ActionArguments[GlobalActionArgument.GalleryId];
            if (!id.Equals(galleryId))
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.BadRequest,
                    "Invalid gallery id"));
            }

            try
            {
                return PerformSearch(GetGazetteer(galleryId), search ?? "", center, bbox, options);
            }
            catch (Exception e)
            {
                throw new HttpResponseException(Request.CreateErrorResponse(HttpStatusCode.InternalServerError, e.Message));
            }
        }

        [HttpGet]
        [Route("Galleries/{id}/Urls/PublicSearch")]
        public SearchResults PerformPublicSearch(string id, string search, string center = null, string bbox = null)
        {
            var options = PyxisOData.FromRequest(Request);
            options.Top = Math.Min(options.Top, 50);

            var me = Request.RequestUri.GetLeftPart(UriPartial.Authority);

            var searchRing = Program.RunInformation.Cluster.SearchRing;

            if (searchRing.Count == 0 || searchRing.ContainsEndpoint(me))
            {
                return PerformSearch(GetPublicGazetteer(), search ?? "", center, bbox, options);
            }
            else
            {
                List<Task<SearchResults>> requests = new List<Task<SearchResults>>();
                foreach (var endpoint in searchRing.Endpoints)
                {
                    var requestUri = Request.RequestUri.ToString().Replace(me, endpoint);
                    UriQueryBuilder builder = new UriQueryBuilder(requestUri);
                    builder.RemoveParameter("$skip");
                    builder.OverwriteParameter("$top", (options.Skip + options.Top).ToString());
                    if (search.HasContent())
                    {
                        builder.OverwriteParameter("search", search);
                    }
                    requestUri = builder.ToString();

                    var client = new WebClient();

                    var task = client.DownloadStringTaskAsync(requestUri).ContinueWith((t) =>
                    {
                        var result = JsonConvert.DeserializeObject<SearchResults>(t.Result);
                        client.Dispose();
                        return result;
                    });

                    requests.Add(task);
                }


                var searchResult = new SearchResults();

                foreach (var task in requests)
                {
                    searchResult.Add(task.Result);
                }

                searchResult.Results = searchResult.Results.Skip(options.Skip).Take(options.Top).ToList();
                return searchResult;
            }
        }


        private SearchResults PerformSearch(IGazetteer gazetteer, string search, string center, string bbox, PyxisOData options)
        {
            options.Top = Math.Min(options.Top, 50);

            WeightedScorer filter;
            WeightedScorer scorer;

            WeightedScorer(search, center, bbox, out filter, out scorer);

            var searchRequest = new SearchRequest()
            {
                Filter = filter,
                FilterThershold = 0.05f,
                Scorer = scorer,
                Aggregations = new Dictionary<string, IGazetteerAggregator>()
                {
                    {"types", new GazetteerAggregator((entry) => entry.DataSet.Specification.OutputType.ToString())},
                    //{"words", new GazetteerAggregator((entry) => new Tokenizer().Tokenize(entry.DataSet.Metadata.Name))},
                    {"words", new GazetteerAggregator((entry) => entry.Words)},
                    {"hosts", new GazetteerAggregator((entry) => entry.Host,1000)}
                }
            };

            var searchResults = gazetteer.Search(searchRequest);

            searchResults.Results = searchResults.Results.Skip(options.Skip).Take(options.Top).ToList();

            return searchResults;
        }

        private static void WeightedScorer(string search, string center, string bbox,
            out WeightedScorer filter, out WeightedScorer scorer)
        {
            var hosts = new List<string>();
            var words = new List<string>();
            var types = new List<string>();

            if (search.HasContent())
            {
                foreach (var word in search.Split(new[] {' '}, StringSplitOptions.RemoveEmptyEntries))
                {
                    if (word.StartsWith("@"))
                    {
                        hosts.Add(word.Substring(1));
                    }
                    else if (word.StartsWith("#"))
                    {
                        types.Add(word.Substring(1));
                    }
                    else
                    {
                        words.Add(word);
                    }
                }
            }


            //var datasetImports = GetImportsForGallery(galleryId);

            filter = new WeightedScorer {Mode = Pyxis.IO.Search.WeightedScorer.AggregatedMode.Multiply};
            scorer = new WeightedScorer();

            if (words.Count > 0)
            {
                filter.SearchAll(words.ToArray());
                scorer.Search(words.ToArray());
            }
            if (hosts.Count > 0)
            {
                filter.Where((entry) => hosts.Contains(entry.Host));
            }
            if (types.Count > 0)
            {
                filter.Where((entry) => types.Contains(entry.DataSet.Specification.OutputType.ToString()));
            }

            if (bbox.HasContent())
            {
                var numbers = bbox.Split(',').Select(x => double.Parse(x)).ToArray();

                filter.Intersects(new BoundingBox()
                {
                    LowerLeft = new BoundingBoxCorner()
                    {
                        X = numbers[0],
                        Y = numbers[1]
                    },
                    UpperRight = new BoundingBoxCorner()
                    {
                        X = numbers[2],
                        Y = numbers[3]
                    },
                    Srs = "4326"
                });
            }

            if (center.HasContent())
            {
                var numbers = center.Split(',').Select(x => double.Parse(x)).ToArray();

                if (numbers.Length == 3)
                {
                    if (!bbox.HasContent())
                    {
                        filter.Intersects(numbers[1], numbers[0], numbers[2]);
                    }
                    scorer.Near(numbers[1], numbers[0], numbers[2]);
                }
            }

            if (search.HasContent() &&
                (search.Contains("image") || search.Contains("raster") || search.Contains("coverage")))
            {
                scorer.Add(
                    new FloatScorer(
                        entry =>
                            entry.DataSet.Specification.OutputType == PipelineSpecification.PipelineOutputType.Coverage
                                ? 1
                                : 0), 0.5f);
            }
            else
            {
                scorer.Add(
                    new FloatScorer(
                        entry => entry.DataSet.DiscoveryReport != null ? entry.DataSet.DiscoveryReport.FeaturesCount : 0, 1000,
                        1000), 0.5f);
                scorer.Add(
                    new FloatScorer(
                        entry => entry.DataSet.Specification != null ? entry.DataSet.Specification.Fields.Count : 0,
                        20, 20), 0.5f);
            }
        }

        /// <summary>
        /// Manages storage of user files
        /// </summary>
        private readonly UserUrlsStorage m_userUrls = new UserUrlsStorage();

        private static IGazetteer s_publicGazetteer;
        private static readonly LimitedSizeDictionary<string, IGazetteer> s_gazetteers = new LimitedSizeDictionary<string, IGazetteer>(20);
        private static readonly object s_lock = new object();

        internal static void InitPublicGazetteer()
        {
            lock (s_lock)
            {
                if (s_publicGazetteer == null)
                {
                    s_publicGazetteer = new UserUrlsStorage().CreatePublicGazetteer();
                }
            }
        }

        private IGazetteer GetPublicGazetteer()
        {
            InitPublicGazetteer();
            return s_publicGazetteer;
        }

        private IGazetteer GetGazetteer(string galleryId)
        {
            IGazetteer gazetteer;
            lock (s_lock)
            {
                if (!s_gazetteers.TryGetValue(galleryId,out gazetteer))
                {
                    gazetteer = m_userUrls.CreateUserGazetteer(galleryId);

                    
                    if (gazetteer.Count > 0)
                    {
                        //merge both gazetters
                        gazetteer =
                            new MultiSourceGazetteer().AddSource("public", GetPublicGazetteer())
                                .AddSource("private", gazetteer);
                    }
                    else
                    {
                        //just use the public one
                        gazetteer = GetPublicGazetteer();
                    }

                    s_gazetteers[galleryId] = gazetteer;
                }
            }
            return gazetteer;
        }

        private void InvalidateGazetteerFor(string galleryId)
        {
            lock (s_lock)
            {
                s_gazetteers.Remove(galleryId);
            }
        }

        private Dictionary<string,ImportDataSetRequestProgress> GetImportsForGallery(string galleryId)
        {
            var imports = new GalleryController().BuildImports(galleryId)
                .ToDictionary(import => import.ToString(), import => import);

            return imports;
        }
    }
}
