using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using System.Web;
using System.Xml.Linq;
using PyxCrawler.Models;
using PyxCrawler.Utilities;
using Pyxis.Contract.DataDiscovery;
using Pyxis.IO.Sources.OGC;

namespace PyxCrawler.Crawling
{
    public enum OwsType
    {
        WMS,
        WFS,
        WCS
    }

    public class OwsDefinition
    {
        public OwsType Type { get; set; }
        public string[] SupportedVersions { get; set; }
        public OgcWebServiceBase OwsService { get; set; }
    }

    public abstract class OwsCrawler
    {
        private static readonly Regex s_srsIdRegex = new Regex(@"[CS]RS=""EPSG:(\d+)""", RegexOptions.IgnoreCase);

        public void Crawl(CrawlingTask task, OnlineGeospatialEndpoint endpoint, OwsDefinition definition)
        {
            var noVersionService = endpoint.Services.FirstOrDefault(x => x.Protocol == definition.Type.ToString() && x.Version == "");

            var services = endpoint.Services.Where(x => x.Protocol == definition.Type.ToString() && x.Version != "").ToList();

            var serviceVersionsDetected = services.Count > 0;

            if (!serviceVersionsDetected)
            {
                services = definition.SupportedVersions
                    .Select(version => new OnlineGeospatialService() { Protocol = definition.Type.ToString(), Version = version, Status = OnlineGeospatialServiceStatus.Unknown })
                    .ToList();
            }

            int i = 0;
            foreach (var service in services)
            {
                CrawlSpecificVersion(task, endpoint, service, definition.OwsService);
                i++;
                task.Progress = 100.0*i/services.Count;
            }

            if (!serviceVersionsDetected)
            {
                foreach (var service in services.Where(x => x.Status == OnlineGeospatialServiceStatus.Working))
                {
                    endpoint.GetService(service.Protocol, service.Version).Status = service.Status;
                }
            }

            if (noVersionService != null)
            {
                if (services.All(x => x.Status == OnlineGeospatialServiceStatus.Offline))
                {
                    noVersionService.Status = OnlineGeospatialServiceStatus.Offline;
                }
                else if (services.All(x => x.Status == OnlineGeospatialServiceStatus.Working))
                {
                    noVersionService.Status = OnlineGeospatialServiceStatus.Working;
                }
                else if (services.All(x => x.Status == OnlineGeospatialServiceStatus.Broken))
                {
                    noVersionService.Status = OnlineGeospatialServiceStatus.Broken;
                }
            }

            task.Status = "Completed " + definition.Type + " crawling";
        }

        private void CrawlSpecificVersion(CrawlingTask task, OnlineGeospatialEndpoint endpoint, OnlineGeospatialService service, OgcWebServiceBase owsService)
        {
            try
            {
                task.Status = "request capabilities for version " + service.Version;

                var credentialEncodedUri = new CredentialEncodedUri(endpoint.Uri);
                var catalogUri = RequestHelper.CreateCapabilitiesRequest(credentialEncodedUri.Uri, service).AbsoluteUri;
                var catalog = owsService.BuildCatalog(catalogUri, credentialEncodedUri.Permit);
                if (catalog == null)
                {
                    service.Status = OnlineGeospatialServiceStatus.Broken;
                    return;
                }

                if (!String.IsNullOrWhiteSpace(catalog.Metadata.Name) && !catalog.Metadata.Name.StartsWith(endpoint.Uri.ToString().Substring(0, 6)))
                {
                    endpoint.Name = catalog.Metadata.Name;
                }
                service.Status = OnlineGeospatialServiceStatus.Accessible;
                if (DetectLayers(task, catalog, endpoint, service))
                {
                    service.Status = OnlineGeospatialServiceStatus.Working;
                }
                else
                {
                    service.Status = OnlineGeospatialServiceStatus.WorkingButNotUsable;
                }
            }
            catch (Exception)
            {
                service.Status = OnlineGeospatialServiceStatus.Broken;
            }
        }

        protected bool DetectLayers(CrawlingTask task, DataSetCatalog catalog, OnlineGeospatialEndpoint endpoint, OnlineGeospatialService service)
        {
            bool foundLayer = false;
            foreach (var dataSet in catalog.DataSets)
            {
                var bbox = ExtractBBox(dataSet);
                DataSetHelper.UpsertDataSet(endpoint, service, dataSet, bbox);
                foundLayer = true;
            }
            return foundLayer;
        }

        protected abstract Wgs84BoundingBox ExtractBBox(DataSet dataSet);
        
        protected static void CorrectProjection(string bBox, Wgs84BoundingBox bbox)
        {
            var wkidMatch = s_srsIdRegex.Match(bBox);
            if (wkidMatch.Success)
            {
                var wkid = Convert.ToInt32(wkidMatch.Groups[1].Value);
                if (wkid != 4326)
                {
                    ProjectionHelper.ProjectToWgs84(bbox, wkid);
                }
            }
        }

        protected static Wgs84BoundingBox ExtractXYBBox(XElement xBBox)
        {
            return new Wgs84BoundingBox()
            {
                South = Double.Parse(xBBox.Attribute("miny").Value),
                North = Double.Parse(xBBox.Attribute("maxy").Value),
                West = Double.Parse(xBBox.Attribute("minx").Value),
                East = Double.Parse(xBBox.Attribute("maxx").Value),
            };
        }

        protected static Wgs84BoundingBox ExtractGeographicBBox(XElement xBBox)
        {
            return new Wgs84BoundingBox
            {
                South = Double.Parse(xBBox.ElementByLocalName("southBoundLatitude").Value),
                North = Double.Parse(xBBox.ElementByLocalName("northBoundLatitude").Value),
                West = Double.Parse(xBBox.ElementByLocalName("westBoundLongitude").Value),
                East = Double.Parse(xBBox.ElementByLocalName("eastBoundLongitude").Value)
            };
        }
    }
}