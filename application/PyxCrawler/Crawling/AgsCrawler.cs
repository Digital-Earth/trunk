using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Web;
using System.Xml.Linq;
using Newtonsoft.Json;
using PyxCrawler.Models;
using PyxCrawler.Utilities;
using Pyxis.Contract.DataDiscovery;
using Pyxis.IO.Sources.ArcGIS;

namespace PyxCrawler.Crawling 
{
    public class Folder
    {
        public string Uri { get; set; }
        public bool RootLevel { get; set; }
    }

    public class AgsCrawler : ICrawler
    {
        private ArcGISGeoServices m_agsGeoServices = new ArcGISGeoServices();
        public void Crawl(CrawlingTask task, OnlineGeospatialEndpoint endpoint)
        {
            var agsService = endpoint.Services.FirstOrDefault(s => s.Protocol == "AGS");
            var hasService = agsService != null;

            var service = new OnlineGeospatialService() { Protocol = "AGS", Version = "" };

            if (hasService)
            {
                service = agsService;
            }

            var credentialEncodedUri = new CredentialEncodedUri(endpoint.Uri);
            var catalogUri = RequestHelper.CreateCapabilitiesRequest(credentialEncodedUri.Uri, service).AbsoluteUri;
            var firstCatalog = true;
            var dataSetsCount = 0;
            var folderQueue = new Queue<Folder>(new []
            {
                new Folder{
                    Uri = catalogUri,
                    RootLevel = false
                }
            });
            var rootFolders = 1;
            var rootFolderTally = 1;

            try
            {
                while (folderQueue.Any())
                {
                    var folder = folderQueue.Dequeue();
                    if (folder.RootLevel)
                    {
                        rootFolderTally++;
                    }

                    catalogUri = folder.Uri;

                    var subCatalogs = m_agsGeoServices.GetCatalogs(catalogUri, credentialEncodedUri.Permit);

                    if (firstCatalog && subCatalogs == null)
                    {
                        service.Status = OnlineGeospatialServiceStatus.Broken;
                        return;
                    }

                    var dataSets = m_agsGeoServices.GetDataSets(catalogUri, credentialEncodedUri.Permit);

                    service.Status = OnlineGeospatialServiceStatus.Accessible;

                    if (firstCatalog && (dataSets.Count > 0 || subCatalogs.Count > 0))
                    {
                        service.Status = OnlineGeospatialServiceStatus.Working;
                        service.Version = m_agsGeoServices.Version;
                        OnlineGeospatialEndpointsDb.Save();
                        rootFolders = subCatalogs.Count;
                    }

                    EnqueueSubCatalogs(subCatalogs, folderQueue);

                    dataSetsCount += AddDataSets(endpoint, dataSets);

                    firstCatalog = false;

                    task.Progress = 100.0 * rootFolderTally / rootFolders;
                    task.Status = "Crawled " + rootFolderTally + " folder from " + rootFolders + " root folders - found " + dataSetsCount + " data sets";
                }
            }
            catch (WebException ex)
            {
                if (ex.Status == WebExceptionStatus.ConnectFailure)
                {
                    service.Status = OnlineGeospatialServiceStatus.Offline;
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
            catch (Exception)
            {
                service.Status = OnlineGeospatialServiceStatus.Broken;
            }

            if (dataSetsCount == 0)
            {
                service.Status = OnlineGeospatialServiceStatus.WorkingButNotUsable;
            }

            if (!hasService && service.Status == OnlineGeospatialServiceStatus.Working)
            {
                endpoint.GetService(service.Protocol, service.Version).Status = service.Status;
            }
        }

        private static void EnqueueSubCatalogs(List<DataSetCatalog> subCatalogs, Queue<Folder> folderQueue)
        {
            foreach (var subCatalog in subCatalogs)
            {
                folderQueue.Enqueue(new Folder
                {
                    Uri = subCatalog.Uri,
                    RootLevel = false
                });
            }
        }

        private int AddDataSets(OnlineGeospatialEndpoint endpoint, List<DataSet> dataSets)
        {
            var dataSetCount = 0;
            foreach (var dataSet in dataSets)
            {
                var bBox = DataSetHelper.GetWgs84BoundingBox(dataSet);

                var protocol = CultureInfo.InvariantCulture.CompareInfo.IndexOf(dataSet.Uri, "MapServer", CompareOptions.IgnoreCase) > 0 ? "AGSM"
                    : CultureInfo.InvariantCulture.CompareInfo.IndexOf(dataSet.Uri, "FeatureServer", CompareOptions.IgnoreCase) > 0 ? "AGSF" : null;

                var service = endpoint.GetService(protocol, m_agsGeoServices.Version);
                DataSetHelper.UpsertDataSet(endpoint, service, dataSet, bBox);
                dataSetCount++;
            }
            return dataSetCount;
        }
    }
}