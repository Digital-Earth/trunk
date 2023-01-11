using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using ApplicationUtility;
using Pyxis.Contract.DataDiscovery;
using Pyxis.IO.Sources.Remote;

namespace Pyxis.IO.Sources.ArcGIS
{
    class ArcGISFeatureDownloader
    {
        public ArcGISDataSetImportService Service { get; set; }
        
        public string RootUrl { get; set; }
        public NetworkCredential Credentials { get; set; }
        public string ObjectFieldId { get; set; }

        public int LastFeatureIndex { get; set; }
        public int ChunkSize { get; set; }
        public List<int> FeauteIds { get; set; }

        public ArcGISFeatureDownloader(ArcGISDataSetImportService service)
        {
            Service = service;

            RootUrl = new Uri(Service.DataSet.Uri).GetLeftPart(UriPartial.Path);
            if (RootUrl.EndsWith("query"))
            {
                RootUrl = RootUrl.Substring(0, RootUrl.Length - "query".Length);
            }
            if (!RootUrl.EndsWith("/"))
            {
                RootUrl += "/";
            }

            Credentials = ArcGISGeoServicesHelper.RetrieveCredentials(Service.Permit);

            LastFeatureIndex = 0;
            ChunkSize = 10;
        }

        
        public void DiscoverLayerInformation()
        {
            var layerDescription =
                WebRequestHelper.GetJsonWithAuthorization<ArcGISGeoServicesHelper.ArcGISLayerDescription>(
                    ArcGISGeoServicesHelper.AsJsonRequest(RootUrl), Credentials);

            ObjectFieldId = layerDescription.fields.Where(
                                  field =>
                                      field.type ==
                                      ArcGISGeoServicesHelper.ArcGISFieldTypes.esriFieldTypeOID.ToString())
                              .Select(x => x.name)
                              .FirstOrDefault() ?? "OBJECTID";
        }

     
        public void DiscoverFeatureIds()
        {
            if (!ObjectFieldId.HasContent())
            {
                DiscoverLayerInformation();
            }

            var idsQueryUrl = RootUrl + "query?f=json&where=" + ObjectFieldId + "=" + ObjectFieldId + "&returnIdsOnly=true";

            Console.WriteLine("Discovering features: {0}", idsQueryUrl);

            var ids =
                WebRequestHelper.GetJsonWithAuthorization<ArcGISGeoServicesHelper.ArcGISObjectIds>(
                    idsQueryUrl, Credentials);

            FeauteIds = ids.objectIds;
            Console.WriteLine("Discovered {0} Features.", ids.objectIds.Count);
        }

        public ArcGISGeoServicesHelper.ArcGISJsonFeatureCollection GetFeatureChunk(int retry = 0)
        {
            if (FeauteIds == null)
            {
                DiscoverFeatureIds();
            }

            ChunkSize = Math.Min(FeauteIds.Count - LastFeatureIndex, ChunkSize);

            var idsChunk = string.Join(",", FeauteIds.Skip(LastFeatureIndex).Take(ChunkSize));

            Console.WriteLine("Downloading features ({0} features/request)", ChunkSize);

            var queryUrl = RootUrl + "query?f=json&outfields=*&objectIds=" + idsChunk;

            var start = DateTime.Now;
            var result =
            WebRequestHelper.GetJsonWithAuthorization<ArcGISGeoServicesHelper.ArcGISJsonFeatureCollection>(
                queryUrl, Credentials);

            var end = DateTime.Now;

            if (result.error != null)
            {
                var errorMessage = String.Format("Remote server error {0}: {1}", result.error.code, result.error.message);
                if (retry > 0)
                {
                    Console.WriteLine(errorMessage);
                    Console.WriteLine("Reducing chunk size");
                    ChunkSize = Math.Max(1, ChunkSize/2);
                    return GetFeatureChunk(retry - 1);
                }
                else
                {
                    return result;
                }
            }

            LastFeatureIndex += ChunkSize;

            //try to align request that are between 10 sec to 30 sec.
            if ((end - start).TotalSeconds < 10)
            {
                ChunkSize = Math.Min(100, ChunkSize + ChunkSize/2);
            }
            else if ((end - start).TotalSeconds > 30)
            {
                ChunkSize = Math.Max(1, ChunkSize/2);
            }

            return result;
        }

        public bool DownloadCompleted
        {
            get
            {
                if (FeauteIds == null)
                {
                    return false;
                }
                return LastFeatureIndex >= FeauteIds.Count;
            }
        }
    }
}
