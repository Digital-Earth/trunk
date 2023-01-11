using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Threading.Tasks;
using System.IO;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Contract;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.IO.DataDiscovery;
using Pyxis.Utilities;

namespace Pyxis.IO.Sources.Microsoft
{

    public class BingDiscoveryService : IDiscoveryService
    {
        private const string BingSearchAPI = "https://api.cognitive.microsoft.com/bing/v7.0/search";
        
        //TODO: convert this into IPermit and store it on the Root object
        //get this from our Azure Bing Search
        private const string AccessKey1 = "94f0bae7da7c40888fc562ae93d157c0";

        private const int MaximumResultsToCrawl = 2000;

        //number of response search entries. This is the max value provided by Bing. 
        private const int ResultsPerPage = 50;

        public string ServiceIdentifier
        {
            get { return "BingWebSearch"; }
        }

        public bool IsUriSupported(string uri)
        {
            return uri.StartsWith(BingSearchAPI);
        }

        public async Task<DiscoveryResult> DiscoverAsync(IDiscoveryContext context)
        {
            if (!IsUriSupported(context.Request.Uri))
            {
                return null;
            }

            long totalSearchItems = 0;

            var query = new UriQueryBuilder(context.Request.Uri).Parameters;

            long offset = long.Parse(query["offset"] ?? "0");
            
            long count = long.Parse(query["count"] ?? ResultsPerPage.ToString());

            var results = await GetBingSearchResults(context, offset, count);
            totalSearchItems = Math.Min(MaximumResultsToCrawl, results.webPages.totalEstimatedMatches);

            var result = new DiscoveryResult
            {
                Uri = context.Request.Uri,
                LastUpdated = DateTime.Now,
                ServiceIdentifier = ServiceIdentifier,
                AdditionalRoots = CollectUrlsFromSearchResult(results.webPages).Select(url=>new DiscoveryRequest() { Uri = url }).ToList(),
            };

            //add next lead if we have more results to crawl
            if (totalSearchItems > offset + count)
            {
                result.Leads = new List<DiscoveryRequest>
                {
                    new DiscoveryRequest() {
                        Uri = CreateBingSearchRequestUrl(context, offset + count, count),
                        Permit = context.Request.Permit,
                        ServiceIdentifier = ServiceIdentifier
                    }
                };
            }

            return result;
        }

        public IDataSetImportService GetDataSetImportService(DataSet dataSet, IPermit permit = null)
        {
            throw new NotSupportedException();
        }


        private async Task<BingWebSearchResult> GetBingSearchResults(IDiscoveryContext context, long offset, long count)
        {
            var response = (DiscoveryHttpResult)await context.SendRequestAsync(CreateBingSearchRequest(context, offset, count));

            if (!response.Headers.Get("BingAPIs-TraceId").HasContent())
            {
                return null;
            }
            else
            {
                return JsonConvert.DeserializeObject<BingWebSearchResult>(response.Content);
            }
        }

        private string CreateBingSearchRequestUrl(IDiscoveryContext context, long offset, long count)
        {
            UriQueryBuilder uriQueryBuilder = new UriQueryBuilder(context.Request.Uri);

            uriQueryBuilder.OverwriteParameter("responseFilter", "WebPages");
            uriQueryBuilder.OverwriteParameter("count", count.ToString());
            uriQueryBuilder.OverwriteParameter("offset", offset.ToString());

            return uriQueryBuilder.ToString();
        }

        private DiscoveryHttpRequest CreateBingSearchRequest(IDiscoveryContext context, long offset, long count)
        {
            
            var headers = new WebHeaderCollection();
            headers["Ocp-Apim-Subscription-Key"] = AccessKey1;

            return new DiscoveryHttpRequest
            {
                Uri = CreateBingSearchRequestUrl(context,offset,count).ToString(),
                Permit = context.Request.Permit,
                Method = WebRequestMethods.Http.Get,
                Headers = headers,
                Timeout = TimeSpan.FromMinutes(1)
            };
        }

        private string ParseArcGISUrl(string url)
        {
            return url.Substring(0, url.ToLower().IndexOf(@"rest/services") + @"rest/services".Length) + "/";
        }

        private string ParseUrl(string url)
        {
            if (url.ToLower().Contains("rest/services"))    //ArcGIS
            {
                return ParseArcGISUrl(url);
            }
            else
            {
                return null;
            }
        }

        private List<string> CollectUrlsFromSearchResult(BingWebSearchWebPagesResults webPagesResults)
        {
            var urlsFromBing = new List<string>();
            foreach (var pageResult in webPagesResults.value)
            {
                var url = ParseUrl(pageResult.url);
                if (!String.IsNullOrEmpty(url) && !urlsFromBing.Contains(url))
                {
                    urlsFromBing.Add(url);
                    Console.WriteLine(url);
                }
            }

            return urlsFromBing;
        }
    }

    #region BingWebSearchResults models

    internal class BigWebSearchPageResult
    {
        public string url { get; set; }
    }

    internal class BingWebSearchWebPagesResults
    {
        public string webSearchUrl { get; set; }
        public long totalEstimatedMatches { get; set; }
        public List<BigWebSearchPageResult> value { get; set; }
    }

    internal class BingWebSearchResult
    {
        public BingWebSearchWebPagesResults webPages { get; set; }
    }

    #endregion
}