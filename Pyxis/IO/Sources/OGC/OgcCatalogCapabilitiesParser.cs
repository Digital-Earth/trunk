using System;
using System.Collections.Generic;
using System.Linq;
using System.Xml;
using ApplicationUtility;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.IO.Sources.ArcGIS;
using Pyxis.Utilities;

namespace Pyxis.IO.Sources.OGC
{
    /// <summary>
    /// Class that create a valid IOgcUrl and Catalog from an input GetCapabilities document
    /// </summary>
    public class OgcCatalogCapabilitiesParser
    {
        private readonly IOgcUrl m_url;
        private readonly DataSetCatalog m_catalog;
        private readonly Func<string, bool> m_uriFilter;

        /// <summary>
        /// Provide stats about parsing issues, errors or fixes been done on the xml document
        /// </summary>
        public Dictionary<string,int> Stats { get; set; }

        public OgcCatalogCapabilitiesParser(IOgcUrl url, DataSetCatalog catalog, Func<string,bool> uriFilter = null )
        {
            m_url = url;
            m_catalog = catalog;
            m_uriFilter = uriFilter ?? (x => true);

            Stats = new Dictionary<string, int>();
        }

        public void Parse(XmlDocument capabilities)
        {
            if (capabilities == null)
            {
                throw new Exception("The server capabilities are not available");
            }

            // Retrieve the server's OGC version
            RetrieveVersion(capabilities);

            var parser = new XmlNodeExtractor()
                .On("ServiceIdentification", new OgcMetadataExtractor(m_catalog.Metadata))
                .On("SearchResults", ExtractSearchResult);
            
            parser.Parse(capabilities.DocumentElement);

            if (m_url.Request != "GetRecords")
            {
                var getRecordsUrl = new OgcCatalogUrl(m_url.ToString())
                {
                    Request = "GetRecords",
                    StartPosition = "1",
                    MaxRecords = "100",
                    RequestId = Guid.NewGuid().ToString()
                };

                m_catalog.SubCatalogs.Add(new DataSetCatalog()
                {
                    DataType = DataSetType.OGC.ToString(),
                    Metadata = new SimpleMetadata()
                    {
                        Name = getRecordsUrl.ServerUrl,
                        Description = "search results"
                    },
                    Uri = getRecordsUrl.ToString()
                });
            }
        }

        protected void ExtractSearchResult(XmlNode searchResults)
        {
            var foundEndpoints = new HashSet<string>();

            var nextRecord = searchResults.FindAttributeWithLocalName("nextRecord").SafeAttributeValue();
            var numberOfRecordsMatched = searchResults.FindAttributeWithLocalName("numberOfRecordsMatched").SafeAttributeValue();

            if (nextRecord.HasContent() || numberOfRecordsMatched.HasContent())
            {
                //add next search result
                var nextSearchUrl = new OgcCatalogUrl(m_url.ToString());

                int next = int.Parse(nextRecord ?? "0");
                int total = int.Parse(numberOfRecordsMatched ?? "0");
                int start = int.Parse(nextSearchUrl.StartPosition ?? "0");
                int pageSize = int.Parse(nextSearchUrl.MaxRecords ?? "100");

                if (next == 0)
                {
                    next = start + pageSize;                    
                }

                if (next < total)
                {
                    nextSearchUrl.StartPosition = next.ToString();
                    m_catalog.NextPage = new NextPageResult
                    {
                        Uri = nextSearchUrl.ToString(),
                        TotalResults = total
                    };
                }
            }

            var recordsParser = new XmlNodeExtractor("MD_Metadata", (node) => ExtractRecord(node, foundEndpoints));
            recordsParser.Parse(searchResults);
        }

        protected void ExtractRecord(XmlNode record, HashSet<string> foundEndpoints)
        {
            var title = "";
            var datasetId = "";
            var urls = new List<string>();

            var parser = new XmlNodeExtractor()
                .On(new[] {"identificationInfo", "MD_DataIdentification", "citation", "CI_Citation"},
                    new XmlNodeExtractor()
                        .On("title", node => title = node.InnerText.Trim())
                        .On("identifier", node => datasetId =
                                //first method: title attribute include the identifier of the dataset
                                node.FindAttributeWithLocalName("title").SafeAttributeValue() ??
                                //second method - use inner text - seem to irrelevant
                                //node.InnerText.Trim())
                                "")
                )
                .On("distributionInfo", node => urls.AddRange(ExtractEndpoint(node, datasetId)))
                .On("containsOperations", node => urls.AddRange(ExtractEndpoint(node, datasetId)));

            parser.Parse(record);

            foreach (var url in urls.Distinct())
            {
                if (!m_uriFilter(url))
                {
                    continue;
                }

                if (foundEndpoints.Contains(url))
                {
                    continue;
                }

                foundEndpoints.Add(url);

                if (UrlLooksLikeAServer(url))
                {
                    var catalog = new DataSetCatalog()
                    {
                        Uri = url,
                        Metadata = new Metadata()
                        {
                            Name = title
                        }
                    };
                    m_catalog.SubCatalogs.Add(catalog);
                }
                else
                {
                    var dataSet = new DataSet
                    {
                        Layer = datasetId,
                        Uri = url,
                        Metadata = new Metadata()
                        {
                            Name = title
                        }
                    };
                    m_catalog.DataSets.Add(dataSet);
                }
            }
        }

        private bool UrlLooksLikeAServer(string url)
        {
            var queryBuilder = new UriQueryBuilder(url);

            var service = queryBuilder.Parameters["service"] ?? "";

            switch (service.ToLower())
            {
                case "wms":
                    return !(new OgcWebMapUrl(url).Name.HasContent());
                case "wfs":
                    return !(new OgcWebFeatureUrl(url).Name.HasContent());
                case "wcs":
                    return !(new OgcWebCoverageUrl(url).Name.HasContent());
                case "csw":
                    return true;
            }

            if (new ArcGISGeoServices().IsUriSupported(url))
            {
                return true;
            }
            return false;
        }

        private IEnumerable<string> ExtractEndpoint(XmlNode node, string datasetId)
        {
            string url;
            string protocol;

            var parser = new XmlNodeExtractor()
                .On(new[] {"linkage", "url"}, x => url = x.InnerText.Trim())
                .On("protocol", x => protocol = x.InnerText.Trim());

            foreach (XmlNode resource in XmlUtils.FindNodesWithLocalName(node, "CI_OnlineResource"))
            {
                url = "";
                protocol = "";
                parser.Parse(resource);

                if (url.HasContent() && Uri.IsWellFormedUriString(url,UriKind.Absolute))
                {
                    var queryBuilder = new UriQueryBuilder(url);
                    if (!queryBuilder.Parameters["service"].HasContent())
                    {
                        foreach (var supportedProtocol in new[] {"WMS", "WFS", "WCS", "CSW"})
                        {
                            //inject service if needed based on protocol information
                            if (protocol.IndexOf(supportedProtocol, StringComparison.InvariantCultureIgnoreCase) != -1)
                            {
                                queryBuilder.OverwriteParameter("service",supportedProtocol);
                            }
                            url = queryBuilder.ToString();

                            var service = queryBuilder.Parameters["service"];
                            //inject dataset id into the url if found.
                            if (datasetId.HasContent() && service.HasContent())
                            {
                                switch (service.ToUpper())
                                {
                                    case "WMS":
                                        url = new OgcWebMapUrl(url) { Name = datasetId }.ToString();
                                        break;
                                    case "WFS":
                                        url = new OgcWebFeatureUrl(url) {Name = datasetId}.ToString();
                                        break;
                                    case "WCS":
                                        url = new OgcWebCoverageUrl(url) { Name = datasetId }.ToString();
                                        break;
                                }
                            }
                        }
                        
                    }

                    yield return url;
                }
            }
        }

        protected void RetrieveVersion(XmlDocument capabilities)
        {
            try
            {
                if (capabilities.DocumentElement != null)
                {
                    m_url.Version = capabilities.DocumentElement.Attributes["version"].Value;
                }
            }
            // NullReferenceException is possible
            catch (Exception)
            {
                try
                {
                    var capabilitiesUrl = new OgcWebMapUrl(capabilities.BaseURI);
                    m_url.Version = capabilitiesUrl.Version;
                }
                catch (Exception)
                {
                    // Just continue
                }
            }
            if (string.IsNullOrEmpty(m_url.Version))
            {
                m_url.Version = "2.0.2";
            }
        }
    }
}