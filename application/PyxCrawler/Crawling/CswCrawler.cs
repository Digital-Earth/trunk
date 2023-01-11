using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Text.RegularExpressions;
using System.Web;
using System.Xml.Linq;
using PyxCrawler.Models;
using PyxCrawler.Utilities;
using Pyxis.Core;
using Pyxis.IO.Import;
using Pyxis.IO.Sources.OGC;
using Pyxis.Utilities;

namespace PyxCrawler.Crawling
{
    public class CswCrawler : ICrawler
    {
        public void Crawl(CrawlingTask task, OnlineGeospatialEndpoint endpoint)
        {
            var foundService = endpoint.Services.Where(x => x.Protocol == "CSW" && x.Version == "2.0.2").FirstOrDefault();
            var noVersionService = endpoint.Services.Where(x => x.Protocol == "CSW" && String.IsNullOrEmpty(x.Version)).FirstOrDefault();

            var service = new OnlineGeospatialService(){Protocol = "CSW", Version = "2.0.2"};

            if (foundService != null)
            {
                service = foundService;
            }

            var unrecoverable = false;
            var startRecord = 1;
            var nextRecord = 1;
            var totalRecords = int.MaxValue;
            var page = 100;
            var urls = new HashSet<Uri>();

            var cswService = new OgcCatalogService(EngineConfig.Engine);
            var credentialEncodedUri = new CredentialEncodedUri(endpoint.Uri);
            var catalogUri = RequestHelper.CreateCapabilitiesRequest(credentialEncodedUri.Uri, service).AbsoluteUri;
            var catalog = cswService.BuildCatalog(catalogUri, credentialEncodedUri.Permit);
            if (catalog == null || catalog.SubCatalogs == null || !catalog.SubCatalogs.Any())
            {
                unrecoverable = true;
            }
            else
            {
                var query = new UriQueryBuilder(catalog.SubCatalogs[0].Uri);
                startRecord = Convert.ToInt32(query.Parameters.Get("startPosition"));
                page = Convert.ToInt32(query.Parameters.Get("maxRecords"));
                service.Status = OnlineGeospatialServiceStatus.Accessible;
                catalogUri = catalog.SubCatalogs[0].Uri;
            }

            while (!unrecoverable && startRecord < totalRecords)
            {
                try
                {
                    catalog = cswService.BuildCatalog(catalogUri, credentialEncodedUri.Permit);
                    var mergedUrls = catalog.SubCatalogs.Select(sc => sc.Uri).Distinct().ToList();
                    mergedUrls.AddRange(catalog.DataSets.Select(ds => ds.Uri).Distinct().ToList());
                    var newUrls = FilterUrls(endpoint, mergedUrls);

                    if (startRecord == 1)
                    {
                        totalRecords = page;
                        service.Status = OnlineGeospatialServiceStatus.Working;
                    }
                    if (newUrls.Count > 0)
                    {
                        OnlineGeospatialEndpointsDb.Save();
                    }

                    urls.UnionWith(newUrls);

                    if (catalog.NextPage != null)
                    {
                        var query = new UriQueryBuilder(catalog.NextPage.Uri);
                        page = Convert.ToInt32(query.Parameters.Get("maxRecords"));
                        totalRecords = catalog.NextPage.TotalResults.Value;
                        nextRecord = Convert.ToInt32(query.Parameters.Get("startPosition"));
                        catalogUri = catalog.NextPage.Uri;
                    }
                    else
                    {
                        nextRecord = totalRecords;
                    }

                    task.Progress = 100.0 * nextRecord / totalRecords;
                    task.Status = "Crawled " + (nextRecord - 1) + " from " + totalRecords + " records - found " + urls.Count + " endpoints";

                    startRecord = nextRecord;
                }
                catch (WebException ex)
                {
                    unrecoverable = true;
                    if (ex.Status == WebExceptionStatus.ConnectFailure)
                    {
                        service.Status = OnlineGeospatialServiceStatus.Offline;
                    }
                    else if (ex.Status == WebExceptionStatus.KeepAliveFailure)
                    {
                        unrecoverable = false;
                        startRecord += 1;
                    }
                    else
                    {
                        var we = ex.Response as HttpWebResponse;
                        if (we != null && we.StatusCode == HttpStatusCode.NotFound)
                        {
                            service.Status = OnlineGeospatialServiceStatus.Offline;
                        }
                        else
                        {
                            service.Status = OnlineGeospatialServiceStatus.Broken;
                        }
                    }
                }
                catch (Exception exception)
                {
                    unrecoverable = true;
                    service.Status = OnlineGeospatialServiceStatus.Broken;
                }
            }

            if (!unrecoverable && urls.Count == 0)
            {
                service.Status = OnlineGeospatialServiceStatus.WorkingButNotUsable;
            }

            if ((foundService == null && service.Status == OnlineGeospatialServiceStatus.Working) || 
                noVersionService != null)
            {
                endpoint.GetService(service.Protocol, service.Version).Status = service.Status;
            }
        }

        private HashSet<Uri> FilterUrls(OnlineGeospatialEndpoint cswEndpoint, IEnumerable<string> strings)
        {            
            var result = new HashSet<Uri>();
            foreach (var s in strings)
            {
                Uri uri = null;

                if (Uri.TryCreate(s, UriKind.Absolute, out uri))
                {
                    if (uri.Host.ToLower() == "localhost" || uri.Host.ToLower() == "127.0.0.1")
                    {
                        continue;
                    }
                    var cleanUri = new UriBuilder(uri);
                    cleanUri.Query = "";
                    
                    var query = HttpUtility.ParseQueryString(uri.Query);
                    var protocol = (query.GetValues("service")??new string[]{}).FirstOrDefault();
                    var version = (query.GetValues("version")??new string[]{}).FirstOrDefault();
                    if (protocol == null)
                    {
                        /* Todo: Skip AGS uris until better support for AGS (too many layers)
                        if (!TryExtractProtocol(ref cleanUri, out protocol))*/
                        {
                            continue;
                        }
                    }
                    if (version == null)
                    {
                        version = "";
                    }

                    result.Add(cleanUri.Uri);

                    var endpoint = OnlineGeospatialEndpointsDb.Get(cleanUri.Uri);
                    var service = endpoint.GetService(protocol, version);

                    if (service.Status == OnlineGeospatialServiceStatus.Unknown)
                    {
                        service.Status = OnlineGeospatialServiceStatus.Crawled;
                    }

                    if (!cswEndpoint.Name.StartsWith("http"))
                    {
                        endpoint.SetTag(cswEndpoint.Name);
                    }
                }
            }
            return result;
        }

        private static bool TryExtractProtocol(ref UriBuilder uriBuilder, out string protocol)
        {
            protocol = TryExtractAgsmProtocol(ref uriBuilder) ? "AGSM" :
                TryExtractAgsfProtocol(ref uriBuilder) ? "AGSF" : null;
            return protocol != null;
        }

        private static readonly Regex s_agsmRegex = new Regex(@"(.*/MapServer)(/.*)?$", RegexOptions.IgnoreCase);
        private static bool TryExtractAgsmProtocol(ref UriBuilder uriBuilder)
        {
            var match = s_agsmRegex.Match(uriBuilder.Uri.AbsoluteUri);
            if (match.Success)
            {
                var uri = match.Groups[1].Value;
                uriBuilder = new UriBuilder(uri);
            }
            return match.Success;
        }

        private static readonly Regex s_agsfRegex = new Regex(@"(.*/FeatureServer)(/.*)?$", RegexOptions.IgnoreCase);
        private static bool TryExtractAgsfProtocol(ref UriBuilder uriBuilder)
        {
            var match = s_agsfRegex.Match(uriBuilder.Uri.AbsoluteUri);
            if (match.Success)
            {
                var uri = match.Groups[1].Value;
                uriBuilder = new UriBuilder(uri);
            }
            return match.Success;
        }
    }
}