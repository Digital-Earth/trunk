using System;
using System.Linq;
using ApplicationUtility;
using Pyxis.Contract;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Utilities;

namespace Pyxis.IO.Sources.OGC
{
    /// <summary>
    /// Implements IOgcWebService for OGC WCS data
    /// </summary>
    public class OgcWebCoverageService : OgcWebServiceBase
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public OgcWebCoverageService()
            : base("WCS - Web Coverage Service", "wcs")
        {
        }

        public override DataSetCatalog BuildCatalog(string uri, IPermit permit)
        {
            if (!IsUriSupported(uri))
            {
                return null;
            }
            try
            {
                // Clean the server url from the data set attributes
                var ogcUrl = new OgcWebCoverageUrl(uri);

                var serverUrl = ogcUrl.ServerUrl;

                // bypass the describeEOCoverageSet
                if (ogcUrl.Request == "DescribeEOCoverageSet" || ogcUrl.Request == "DescribeCoverage")
                {
                    serverUrl = ogcUrl.ToString();
                }

                // Try to open a server with the server URL
                var server = OgcWebCoverageServer.Create(serverUrl, RetrieveCredentials(permit), this);

                // Build the catalog
                return server.Catalog;
            }
            catch (Exception e)
            {
                Trace.error(e.Message);
                return null;
            }
        }

        /// <summary>
        /// Create a pipeline for importing a GeoSource from a data set
        /// </summary>
        /// <param name="dataSet">The data set to be imported.</param>
        /// <param name="permit">Provides credentials to access the data set, if required</param>
        /// <returns>IProcess_SPtr object</returns>
        public override IProcess_SPtr BuildPipeline(DataSet dataSet, IPermit permit)
        {
            if (IsUriSupported(dataSet.Uri))
            {
                var url = new OgcWebCoverageUrl(dataSet.Uri);

                // Create the WS process.
                var spProc = PYXCOMFactory.CreateProcess(pyxlib.strToGuid("{3C11ACF7-AD4B-4bf5-A7FA-98ADCD454FDC}"));
                if (spProc.isNull())
                {
                    throw new Exception("Failed to create a WS process from GUID {3C11ACF7-AD4B-4bf5-A7FA-98ADCD454FDC}");
                }

                var map = new Attribute_Map();
                map.set("wcs_version", url.Version ?? "");
                map.set("layer", url.Name);
                map.set("server", GetGdalUrlFromServerUrl(url.ServerUrl));
                map.set("over_sampling", "Nearest Neighbor");

                if (url.RangeSubset.HasContent())
                {
                    map.set("list_of_bands", url.RangeSubset);
                }

                if (url.EncodedSubset.HasContent())
                {
                    map.set("subset", url.EncodedSubset);
                }

                if (url.Format.HasContent())
                {
                    map.set("format", url.Format);
                }

                spProc.setAttributes(map);

                if (spProc.initProc() != IProcess.eInitStatus.knInitialized)
                {
                    // Failed to init.  this is bad.
                    throw new Exception(spProc.getInitError().getError());
                }

                // Create the coverage cache and initialize it
                var spCache = PYXCOMFactory.CreateProcess(PYXCOMFactory.WellKnownProcesses.CoverageCache);
                if (spCache.isNull())
                {
                    throw new Exception("Failed to create a coverage cache process");
                }
                spCache.getParameter(0).addValue(spProc);
                spCache.setProcName(dataSet.Metadata != null && dataSet.Metadata.Name.HasContent() ? dataSet.Metadata.Name : (url.Name ?? ""));
                spCache.setProcDescription(dataSet.Metadata != null && dataSet.Metadata.Description.HasContent() ? dataSet.Metadata.Description : "");
                spCache.initProc();

                //check if this is a single field coverage. and give it default styling
                var coverage = pyxlib.QueryInterface_ICoverage(spCache.getOutput());
                if (coverage.isNotNull())
                {
                    var definition = coverage.getCoverageDefinition();

                    if (definition.isNotNull() && definition.getFieldCount() == 1 && definition.getFieldDefinition(0).getCount() == 1)
                    {
                        // use simple greyscale values, same as gdal_pipe_builder
                        const string palette = "2  0 0 0 0 255  255 255 255 255 255";

                        bool isElevation = dataSet.Metadata != null && dataSet.Metadata.Name != null &&
                                           (dataSet.Metadata.Name.IndexOf("elevation", StringComparison.CurrentCultureIgnoreCase) >= 0 ||
                                            dataSet.Metadata.Name.IndexOf("lidar", StringComparison.CurrentCultureIgnoreCase) >= 0);

                        var process = PYXCOMFactory.CreateProcess(PYXCOMFactory.WellKnownProcesses.StyledCoverage);
                        if (process.isNull())
                        {
                            throw new Exception("Failed to create a styled coverage process");
                        }

                        map.clear();
                        map.set("palette", palette);
                        map.set("show_as_elevation", isElevation ? "1" : "0");

                        process.setAttributes(map);
                        process.getParameter(0).addValue(spCache);
                        process.setProcName(spCache.getProcName());
                        process.setProcDescription(spCache.getProcDescription());

                        return process;
                    }
                }

                return spCache;
            }

            return null;
        }

        /// <summary>
        /// Create an url suitable for GDAL from a server url.
        /// </summary>
        /// <param name="serverUrl">The server url</param>
        /// <returns>The gdal url</returns>
        private string GetGdalUrlFromServerUrl(string serverUrl)
        {
            var query = new UriQueryBuilder(serverUrl);

            // GDAL driver adds service and version parameters to url
            query.RemoveParameter("service");
            query.RemoveParameter("version");

            // GDAL requires the url to be ready for appending parameters
            var gdalUrl = query.ToString();
            if (!gdalUrl.Contains('?'))
            {
                // no ? present in url, add one
                gdalUrl += '?';
            }
            else if (!gdalUrl.EndsWith("?"))
            {
                // ? is present but not at the end of the url, make sure the url ends in &
                if (!gdalUrl.EndsWith("&"))
                {
                    gdalUrl += '&';
                }
            }

            return gdalUrl;
        }
    }
}