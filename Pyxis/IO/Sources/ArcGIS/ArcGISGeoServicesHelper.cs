using System;
using System.Collections.Generic;
using System.Net;
using Newtonsoft.Json;
using Pyxis.Contract;
using Pyxis.Contract.Publishing;
using Pyxis.IO.Sources.Remote;

namespace Pyxis.IO.Sources.ArcGIS
{
    internal static class ArcGISGeoServicesHelper
    {
        /// <summary>
        /// Checks if a URI represents a GeoServices URI
        /// </summary>
        /// <param name="uri">URI of the resource</param>
        /// <returns>True if the url represents a resource of this kind, otherwise false.</returns>
        public static bool IsUriSupported(string uri)
        {
            try
            {
                // GeoService URL's have the form http://<host>/<site>/rest/services/<folder>/<serviceName>/<serviceType>
                Uri result;
                if (Uri.TryCreate(uri, UriKind.Absolute, out result) &&
                    ((uri.StartsWith("http:") || uri.StartsWith("https:")) &&
                     uri.Contains("/rest/services")))
                {
                    return true;
                }
            }
            catch (Exception e)
            {
                Trace.error(e.Message);
            }

            return false;
        }

        /// <summary>
        /// Retrieves network credential information.
        /// </summary>
        /// <param name="permit">Permission to access the resource.</param>
        /// <returns>NetworkCredential object</returns>
        public static NetworkCredential RetrieveCredentials(IPermit permit)
        {
            var networkPermit = permit as INetworkPermit;
            return networkPermit != null ? networkPermit.Credentials : null;
        }

        public static bool NoPaginationSupport(ArcGISGeoServicesHelper.ArcGISLayerDescription layerDescription)
        {
            var field = layerDescription.advancedQueryCapabilities;
            return field == null
                   || field.supportsPagination == null
                   || (field.supportsPagination is string && (field.supportsPagination as string).ToUpper() == "FALSE")
                   || (field.supportsPagination is bool && (bool)field.supportsPagination == false);
        }

        public static bool NoDynamicLayersSupports(ArcGISGeoServicesHelper.ArcGISServiceDescription serviceDescription)
        {
            var field = serviceDescription.supportsDynamicLayers;
            return field == null
                   || (field is string && (field as string).ToUpper() == "FALSE")
                   || (field is bool && (bool)field == false);
        }

        /// <summary>
        /// Convert a GeoServices URL to a JSON request
        /// </summary>
        /// <param name="url">The URL</param>
        /// <returns>The new URL</returns>
        public static string AsJsonRequest(string url)
        {
            return url + "?f=json";
        }

        /// <summary>
        /// Convert a GeoServices URL to a JSON feature request usable by GDAL.
        /// </summary>
        /// <param name="url">The URL</param>
        /// <returns>The new URL</returns>
        public static string AsJsonFeatureRequest(string url,string objectIdFieldName = "objectid")
        {
            if (!url.EndsWith("/"))
            {
                url = url + "/";
            }

            return url + "query?where="+objectIdFieldName+"+%3D+"+objectIdFieldName+"&outfields=*&f=json";
        }


        /// <summary>
        /// Convert a GeoServices URL to a JSON feature count request
        /// </summary>
        /// <param name="url">The URL</param>
        /// <returns>The new URL</returns>
        public static string AsFeatureCountRequest(string url, string objectIdFieldName = "objectid")
        {
            if (!url.EndsWith("/"))
            {
                url = url + "/";
            }

            return url + "query?where=" + objectIdFieldName + "+%3D+" + objectIdFieldName + "&returnCountOnly=true&f=json";
        }

        /// <summary>
        /// Get the spatial reference system from an ArcGISExtent.
        /// </summary>
        /// <param name="extent">The ArcGISExtent.</param>
        /// <returns>Spatial reference system as a string</returns>
        public static string GetSrs(ArcGISGeoServicesHelper.ArcGISExtent extent)
        {
            string srs;
            if (!String.IsNullOrEmpty(extent.spatialReference.latestWkid))
            {
                srs = extent.spatialReference.latestWkid;
            }
            else if (!String.IsNullOrEmpty(extent.spatialReference.wkid))
            {
                srs = extent.spatialReference.wkid;
            }
            else if (!String.IsNullOrEmpty(extent.spatialReference.wkt))
            {
                srs = extent.spatialReference.wkt;
            }
            else
            {
                // default srs for ArcGIS MapServer
                srs = "3857";
            }
            return srs;
        }

        /// <summary>
        /// Get BoundingBox from an ArcGISExtent.
        /// </summary>
        /// <param name="extent">The ArcGISExtent.</param>
        /// <returns>The created BoundingBox, or null if generation failed.</returns>
        public static BoundingBox GetBoundingBox(ArcGISGeoServicesHelper.ArcGISExtent extent)
        {
            BoundingBox boundingBox = null;

            //broken ArcGIS Extent has NaN
            if (double.IsNaN(extent.xmin) || double.IsNaN(extent.xmax) || 
                double.IsNaN(extent.ymin) || double.IsNaN(extent.ymax))
            {
                return null;
            }

            try
            {
                var srs = ArcGISGeoServicesHelper.GetSrs(extent);
                boundingBox = new BoundingBox
                {
                    LowerLeft = new BoundingBoxCorner
                    {
                        X = extent.xmin,
                        Y = extent.ymin
                    },
                    UpperRight = new BoundingBoxCorner
                    {
                        X = extent.xmax,
                        Y = extent.ymax
                    },
                    Srs = srs
                };
            }
            catch
            {
            }
            return boundingBox;
        }
        
        /// <summary>
        /// Error details from ArcGIS server
        /// </summary>
        public class ArcGISErrorDetails
        {
            public int code { get; set; }
            public string message { get; set; }
            //public List<string> details { get; set; }
        }

        /// <summary>
        /// Error details that can be returned as valid response from ArcGIS servers
        /// </summary>
        public class ArcGISError
        {
            [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
            public ArcGISErrorDetails error { get; set; }
        }

        // The following classes correspond to JSON responses from a ArcGIS GeoServices server.
        // The commented out fields are available, but unused in the our current implementation.
        // They can be accessed by uncommenting them.
        // See http://resources.arcgis.com/en/help/arcgis-rest-api/

        public class ArcGISDataSetLayer : ArcGISError
        {
            public string id { get; set; }
            public string name { get; set; }
            //            public bool defaultVisibility { get; set; }
            //            public string parentLayerId { get; set; }
            //            public IList<string> subLayerIds { get; set; }
        }

        //        public class ArcGISDataSetTable
        //        {
        //            public string id { get; set; }
        //            public string name { get; set; }
        //        }

        public class ArcGISDataSetTileLOD
        {
            //            public string level { get; set; }
            public double resolution { get; set; }
            //            public string scale { get; set; }
        }

        public class ArcGISDataSetTileInfo
        {
            //            public int rows { get; set; }
            //            public int cols { get; set; }
            //            public int dpi { get; set; }
            //            public string format { get; set; }
            //            public string compressionQuality { get; set; }
            //            public string origin { get; set; }
            //            public string spatialReference { get; set; }
            public IList<ArcGISDataSetTileLOD> lods { get; set; }
        }

        //        public class ArcGISDataSetTimeReference
        //        {
        //            public string timeZone { get; set; }
        //            public bool respectsDaylighSaving { get; set; }
        //        }

        //        public class ArcGISDataSetTimeInfo
        //        {
        //            public IList<string> timeExtent { get; set; }
        //            public ArcGISDataSetTimeReference timeReference { get; set; }
        //        }

        public class ArcGISSpatialReference
        {
            public string wkid { get; set; }
            public string latestWkid { get; set; }
            public string wkt { get; set; }
        }

        public class ArcGISExtent
        {
            public double xmin { get; set; }
            public double ymin { get; set; }
            public double xmax { get; set; }
            public double ymax { get; set; }
            public ArcGISSpatialReference spatialReference { get; set; }
        }

        public class ArcGISServiceDescription : ArcGISError
        {
            public string serviceDescription { get; set; }
            //            public string mapName { get; set; }
            //            public string description { get; set; }
            //            public string copyrightText { get; set; }
            public object supportsDynamicLayers { get; set; }
            public IList<ArcGISDataSetLayer> layers { get; set; }
            //            public ArcGISSpatialReference spatialReference { get; set; }
            //            public bool singleFusedMapCache { get; set; }
            public ArcGISDataSetTileInfo tileInfo { get; set; }
            public ArcGISExtent initialExtent { get; set; }
            //            public ArcGISExtent fullExtent { get; set; }
            //            public ArcGISDataSetTimeInfo timeInfo { get; set; }
            //            public string units { get; set; }
            //            public string supportedImageFormatTypes { get; set; }
            //            public string documentInfo { get; set; }
            //            public string capabilities { get; set; }
        }

        public class ArcGISService
        {
            public string name { get; set; }
            public string type { get; set; }
        }

        public class ArcGISCatalog
        {
            //            public string specVersion { get; set; }
            public string currentVersion { get; set; }
            public IList<string> folders { get; set; }
            public IList<ArcGISService> services { get; set; }
        }

        public class ArcGISRenderer
        {
            public string type { get; set; }
        }

        public class ArcGISDrawingInfo
        {
            public ArcGISRenderer renderer { get; set; }
            //            public int transparency { get; set; }
            //            public string labelingInfo { get; set; }
        }

        public class ArcGISAdvancedQueryCapabilities
        {
            //            public string useStandardizedQueries { get; set; }
            //            public string supportsStatistics { get; set; }
            //            public string supportsOrderBy { get; set; }
            //            public string supportsDistinct { get; set; }
            public object supportsPagination { get; set; }
            //            public string supportsTrueCurve { get; set; }
        }

        public class ArcGISLayerDescription
        {
            public object id { get; set; }
            public string name { get; set; }
            public string type { get; set; }
            public string description { get; set; }
            public string copyrightText { get; set; }
            public ArcGISExtent extent { get; set; }
            public ArcGISDrawingInfo drawingInfo { get; set; }
            public string capabilities { get; set; }
            public ArcGISAdvancedQueryCapabilities advancedQueryCapabilities { get; set; }
            public List<ArcGISField> fields { get; set; }
            public string geometryType { get; set; }
        }

        public enum ArcGISFieldTypes
        {
            esriFieldTypeInteger,
            esriFieldTypeSmallInteger,
            esriFieldTypeDouble,
            esriFieldTypeSingle,
            esriFieldTypeString,
            esriFieldTypeDate,
            esriFieldTypeGeometry,
            esriFieldTypeOID,
            esriFieldTypeBlob,
            esriFieldTypeGlobalID,
            esriFieldTypeRaster,
            esriFieldTypeGUID,
            esriFieldTypeXML
        };

        public enum ArcGISGeometryType
        {
            esriGeometryPoint,
            esriGeometryPolygon,
            esriGeometryPolyline
        }

        public class ArcGISServiceRasterAttributeTable
        {
            public string objectIdFieldName { get; set; }
            public List<ArcGISField> fields { get; set; }
            public List<ArcGISServiceRasterFeatures> features { get; set; }
        }        

        public class ArcGISDomain
        {
            public class CodedValue
            {
                public string name { get; set; }
                public object code { get; set; }
            }

            public string name { get; set; }
            public string type { get; set; }

            //codedValues domain
            public List<CodedValue> codedValues { get; set; }

            //range domain
            public double minValue { get; set; }
            public double maxValue { get; set; }
        }

        public class ArcGISField
        {
            public string name { get; set; }
            public string type { get; set; }
            public string alias { get; set; }
            public int length { get; set; }
            public ArcGISDomain domain { get; set; }
        }

        public class ArcGISServiceRasterFeatures
        {
            public Dictionary<string, object> attributes { get; set; }
        }

        public class ArcGISJsonFeature
        {
            public Dictionary<string, object> attributes { get; set; }
            public Dictionary<string,object> geometry { get; set; }
        }

        public class ArcGISJsonFeatureCollection : ArcGISError
        {
            [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
            public string displayFieldName { get; set; }

            public Dictionary<string,string> fieldAliases { get; set; }
            public object fields { get; set; }
            public string geometryType { get; set; }
            public ArcGISSpatialReference spatialReference { get; set; }
            public List<ArcGISJsonFeature> features { get; set; }
            
            [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
            public bool? exceededTransferLimit { get; set; }
        }

        public class ArcGISCount
        {
            public long count { get; set; }
        }

        public class ArcGISObjectIds : ArcGISError
        {
            public string objectIdFieldName { get; set; }
            public List<int> objectIds { get; set; }
        }

        public static string GetServiceTypeFromUrl(Uri uri)
        {
            var path = uri.AbsolutePath.ToLower();

            if (path.Contains("/mapserver"))
            {
                return "MapServer";
            }
            if (path.Contains("/imageserver"))
            {
                return "ImageServer";
            }
            if (path.Contains("/featureserver"))
            {
                return "FeatureServer";
            }
            return null;
        }

        public static string GetServiceNameFromUrl(Uri uri)
        {
            var trimEnd = "/" + GetServiceTypeFromUrl(uri);
            var trimStart = "/rest/services/";

            var name = uri.AbsolutePath;
            var start = name.IndexOf(trimStart, StringComparison.InvariantCultureIgnoreCase);
            if (start != -1)
            {
                name = name.Substring(start + trimStart.Length);
            }
            var end = name.IndexOf(trimEnd, StringComparison.InvariantCultureIgnoreCase);
            if (end != -1)
            {
                name = name.Substring(0, end);
            }
            return name;
        }

        public static string ExtractServiceUrl(Uri uri)
        {
            var serviceType = GetServiceTypeFromUrl(uri);

            var baseUrl = uri.GetLeftPart(UriPartial.Path);
            var location = baseUrl.LastIndexOf(serviceType, StringComparison.InvariantCultureIgnoreCase);

            if (location != -1)
            {
                return baseUrl.Substring(0, location + serviceType.Length) + "/";
            }
            return baseUrl;
        }
    }
}