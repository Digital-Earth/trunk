using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Net;
using Pyxis.Contract;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.IO.DataDiscovery;
using Pyxis.IO.Sources.Remote;

namespace Pyxis.IO.Sources.ArcGIS
{
    /// <summary>
    /// Implements IWebDataSetDiscoveryService for ArcGIS Feature Server data
    /// See: https://services.arcgis.com/help/index.html?overview.html
    /// </summary>
    public class ArcGISGeoServices : IWebDataSetDiscoveryService
    { 
        /// <summary>
        /// The user's network credentials
        /// </summary>
        protected NetworkCredential m_credentials;

        /// <summary>
        /// Gets the title.  This is the (human readable) text that identifies this service.
        /// </summary>
        /// <value>The title.</value>
        public string Title { get { return "ArcGIS Services"; } }

        /// <summary>
        /// Gets the service identifier.  This is the (machine readable) text that identifies this service.
        /// </summary>
        /// <value>The service identifier.</value>
        public string ServiceIdentifier { get { return "ags"; } }

        /// <summary>
        /// The category of the discovery service (corresponds to "DataType" in Catalog)
        /// </summary>
        public string Category { get { return DataSetType.ArcGIS.ToString(); } }

        private string m_version = "";

        /// <summary>
        /// The version of the Service, if known.
        /// </summary>
        public string Version
        {
            get { return m_version; }
            private set { m_version = value; }
        }

        /// <summary>
        /// Checks if a URI represents a GeoServices URI
        /// </summary>
        /// <param name="uri">URI of the resource</param>
        /// <returns>True if the url represents a resource of this kind, otherwise false.</returns>
        public bool IsUriSupported(string uri)
        {
            return ArcGISGeoServicesHelper.IsUriSupported(uri);
        }

        /// <summary>
        /// Settings to control the behaviour of BuildCatalog(). Temporary until BuildCatalog() is removed.
        /// </summary>
        [Flags]
        private enum ParseMethod
        {
            None = 0,           // placeholder, not used
            Catalogs = 0x01,    // parse catalogs only
            DataSets = 0x02,    // parse data sets only, no recursion
            Recurse = 0x04      // recurse into subcatalogs, must be bitwise anded with Catalogs and/or DataSets
        }

        /// <summary>
        /// Get the catalogs accessible from this uri (non-recursive)
        /// </summary>
        /// <param name="uri">The uri.</param>
        /// <param name="permit">The permit.</param>
        /// <returns>The catalogs</returns>
        public List<DataSetCatalog> GetCatalogs(string uri, IPermit permit)
        {
            var catalog = BuildCatalog(uri, permit, ParseMethod.Catalogs);
            if (catalog != null)
            {
                return catalog.SubCatalogs;
            }

            return null;
        }

        /// <summary>
        /// Get the data sets accessible from this uri.
        /// </summary>
        /// <param name="uri">The uri.</param>
        /// <param name="permit">The permit.</param>
        /// <returns>The data sets</returns>
        public List<DataSet> GetDataSets(string uri, IPermit permit)
        {
            var catalog = BuildCatalog(uri, permit, ParseMethod.DataSets);
            if (catalog != null)
            {
                return catalog.DataSets;
            }

            return null;
        }

        /// <summary>
        /// Build the catalog of data sets that can be accessed on this server.
        /// </summary>
        /// <param name="uri">A URI containing the server.</param>
        /// <param name="permit">Contains user credentials.</param>
        /// <returns>The catalog or null if no data sets.</returns>
        [Obsolete("This method is obsolete. Use GetCatalogs or GetDataSets instead.", true)]
        public DataSetCatalog BuildCatalog(string uri, IPermit permit)
        {
            return BuildCatalog(uri, permit, ParseMethod.Catalogs | ParseMethod.DataSets );
        }

        public IDataSetImportService GetDataSetImportService(DataSet dataSet, IPermit permit = null)
        {
            return new ArcGISDataSetImportService(dataSet, permit);
        }
        
        
        /// <summary>
        /// Build the catalog of data sets that can be accessed on this server.
        /// </summary>
        /// <param name="uri">A URI containing the server.</param>
        /// <param name="permit">Contains user credentials.</param>
        /// <param name="method">Controls how the catalogs and data sets are parsed.</param>
        /// <returns>The catalog or null if no data sets.</returns>
        private DataSetCatalog BuildCatalog(string uri, IPermit permit, ParseMethod method)
        {
            if (!IsUriSupported(uri))
            {
                return null;
            }

            try
            {
                m_credentials = ArcGISGeoServicesHelper.RetrieveCredentials(permit);

                // find the server portion
                const string pattern = "/rest/services";
                var idx = uri.LastIndexOf(pattern);
                if (idx > 0)
                {
                    idx += pattern.Length;
                    return BuildCatalog(method, uri.Substring(0, idx), uri.Substring(idx));
                }
            }
            catch (Exception e)
            {
                Trace.error(e.Message);
            }

            return null;
        }

        /// <summary>
        /// Retrieve the data sets on this server.
        /// </summary>
        /// <param name="method">Controls how the catalogs and data sets are parsed.</param>
        /// <param name="url">The url to the catalog service.</param>
        /// <param name="folder">The path to the resource (includes leading /).</param>
        /// <returns>The catalog if successful, otherwise null</returns>
        private DataSetCatalog BuildCatalog(ParseMethod method, string url, string folder = "")
        {
            var catalogUrl = url + folder;
            var catalog = CreateEmptyCatalog(catalogUrl);

            var response =
                WebRequestHelper.GetJsonWithAuthorization<ArcGISGeoServicesHelper.ArcGISCatalog>(ArcGISGeoServicesHelper.AsJsonRequest(catalogUrl),
                    m_credentials);
            if (response != null)
            {
                if (response.currentVersion != null)
                {
                    Version = response.currentVersion;
                }

                // create a data set for each of the services
                if (response.services != null && ((method & ParseMethod.DataSets) == ParseMethod.DataSets))
                {
                    foreach (var service in response.services)
                    {
                        // we only support FeatureServers and MapServers for now
                        if (service.type == "FeatureServer" || service.type == "MapServer" || service.type == "ImageServer")
                        {
                            var dataSetUrl = url + "/" + service.name + "/" + service.type;
                            var serviceDescription =
                                WebRequestHelper.GetJsonWithAuthorization<ArcGISGeoServicesHelper.ArcGISServiceDescription>(ArcGISGeoServicesHelper.AsJsonRequest(dataSetUrl), m_credentials);

                            if (service.type == "ImageServer" )
                            {
                                // the MapServer does not support dynamic layers, we have no control over the layers that are displayed
                                var dataSet = new DataSet
                                {
                                    Uri = dataSetUrl,
                                    Layer = "",
                                    Metadata =
                                    {
                                        // <layer name> : <folder> / <service name>
                                        Name = service.name,
                                        Description = serviceDescription.serviceDescription
                                    },
                                    BBox = new List<BoundingBox>() { ArcGISGeoServicesHelper.GetBoundingBox(serviceDescription.initialExtent) }
                                };

                                catalog.DataSets.Add(dataSet);
                            }

                            if (service.type == "MapServer" &&
                                (ArcGISGeoServicesHelper.NoDynamicLayersSupports(serviceDescription)))
                            {
                                // the MapServer does not support dynamic layers, we have no control over the layers that are displayed
                                var dataSet = new DataSet
                                {
                                    Uri = dataSetUrl,
                                    Layer = "",
                                    Metadata =
                                    {
                                        // <layer name> : <folder> / <service name>
                                        Name = service.name,
                                        Description = serviceDescription.serviceDescription
                                    },
                                    BBox = new List<BoundingBox>() { ArcGISGeoServicesHelper.GetBoundingBox(serviceDescription.initialExtent) }
                                };

                                catalog.DataSets.Add(dataSet);
                            }
    
                            if (serviceDescription.layers != null)
                            {
                                foreach (var layer in serviceDescription.layers)
                                {
                                    var layerUrl = dataSetUrl + "/" + layer.id;
                                    var layerDescription =
                                        WebRequestHelper.GetJsonWithAuthorization<ArcGISGeoServicesHelper.ArcGISLayerDescription>(ArcGISGeoServicesHelper.AsJsonRequest(layerUrl), m_credentials);

                                    // exclude non-spatial layers
                                    if (layerDescription.type != null && layerDescription.type.ToUpper() == "TABLE")
                                    {
                                        continue;
                                    }

                                    // exclude layers that do not support pagination (required by GDAL GeoJSON driver)
                                    if (ArcGISGeoServicesHelper.NoPaginationSupport(layerDescription))
                                    {
                                        continue;
                                    }

                                    // exclude non-queryable layers from non-dynamic MapServers
                                    if (service.type == "MapServer" &&
                                        (ArcGISGeoServicesHelper.NoDynamicLayersSupports(serviceDescription) ||
                                        layerDescription.capabilities == null ||
                                        !layerDescription.capabilities.ToUpper().Contains("QUERY")))      
                                    {
                                        continue;
                                    }

                                    var dataSet = new DataSet
                                    {
                                        Layer = layer.name,
                                        Metadata =
                                        {
                                            // <layer name> : <folder> / <service name>
                                            Name = layer.name + ":" + service.name,
                                            Description = !String.IsNullOrEmpty(layerDescription.description)
                                                ? layerDescription.description
                                                : serviceDescription.serviceDescription
                                        },
                                        Fields = layerDescription.fields.Select(x=>x.name).ToList(),
                                        BBox = new List<BoundingBox>() { ArcGISGeoServicesHelper.GetBoundingBox(layerDescription.extent) ?? ArcGISGeoServicesHelper.GetBoundingBox(serviceDescription.initialExtent) }
                                    };

                                    if (service.type == "FeatureServer" ||
                                        (layerDescription.type != null &&
                                        layerDescription.type.ToUpper() == "FEATURE LAYER" &&
                                        layerDescription.capabilities != null &&
                                        layerDescription.capabilities.ToUpper().Contains("QUERY")))
                                    {
                                        // a FeatureServer layer or a MapServer feature layer with query capabilities
                                        dataSet.Uri = ArcGISGeoServicesHelper.AsJsonFeatureRequest(dataSetUrl + "/" + layer.id + "/");
                                        dataSet.Layer = layer.name;
                                    }
                                    else if (service.type == "MapServer")
                                    {
                                        dataSet.Uri = dataSetUrl;
                                        dataSet.Layer = layer.id;
                                    }

                                    catalog.DataSets.Add(dataSet);
                                }
                            }
                        }
                    }
                }

                // create a subcatalog for each of the subfolders
                if (response.folders != null && ((method & ParseMethod.Catalogs) == ParseMethod.Catalogs))
                {
                    foreach (var subfolder in response.folders)
                    {
                        DataSetCatalog subcatalog = null;
                        if ((method & ParseMethod.Recurse) == ParseMethod.Recurse)
                        {
                            subcatalog = BuildCatalog(method, url, folder + "/" + subfolder);
                            if (subcatalog != null && (subcatalog.DataSets.Any() || subcatalog.SubCatalogs.Any()))
                            {
                                // The downloadable user interface is unable to handle nested catalogs, so flatten the catalog
                                //catalog.SubCatalogs.Add(subcatalog);
                                catalog.DataSets.AddRange(subcatalog.DataSets);
                            }
                        }
                        else
                        {
                            subcatalog = CreateEmptyCatalog(url + folder + "/" + subfolder);
                            catalog.SubCatalogs.Add(subcatalog);
                        }
                    }
                }
            }

            return catalog;
        }

        /// <summary>
        /// Create an empty ArcGISGeoServices catalog for the given url
        /// </summary>
        /// <param name="url">The url representing this catalog</param>
        /// <returns></returns>
        private DataSetCatalog CreateEmptyCatalog(string url)
        {
            return new DataSetCatalog()
            {
                DataType = DataSetType.ArcGIS.ToString(),
                Uri = url,
                Metadata = new SimpleMetadata()
                {
                    // Set the default Metadata values
                    Name = "ArcGIS GeoServices",
                    Description = "",
                    Tags = new List<string> { "ArcGIS", "GeoServices" }
                },

                SubCatalogs = new List<DataSetCatalog>(),
                DataSets = new List<DataSet>()
            };
        }
    }

    public class RasterValueTransformTable
    {
        public class ColorToValue
        {
            public Color Color { get; set; }
            public int Value { get; set; }
            public string Label { get; set; }
        }

        public string FieldName { get; set; }
        public List<ColorToValue> Transform { get; set; }
    }
}