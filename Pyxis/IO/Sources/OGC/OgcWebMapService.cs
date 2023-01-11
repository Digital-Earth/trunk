using System;
using System.Linq;
using System.Xml;
using ApplicationUtility;
using Pyxis.Contract;
using Pyxis.Contract.DataDiscovery;

namespace Pyxis.IO.Sources.OGC
{
    /// <summary>
    /// Implements IOgcWebService for OGC WMS data
    /// </summary>
    public class OgcWebMapService : OgcWebServiceBase
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public OgcWebMapService()
            : base("WMS - Web Map Service", "wms")
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
                var ogcUrl = new OgcWebMapUrl(uri);

                // Try to open a server with the server URL
                var server = OgcWebMapServer.Create(ogcUrl.ServerUrl, RetrieveCredentials(permit), this);

                // Build the catalog
                return server.Catalog;
            }
            catch(Exception e)
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
                var url = new OgcWebMapUrl(dataSet.Uri);

                //if bbox is not present in given url, perform a discover and extract the full information on the dataset
                if (url.BBox == null && url.Name.HasContent())
                {
                    var catalog = BuildCatalog(dataSet.Uri, permit);

                    if (catalog == null || catalog.DataSets == null)
                    {
                        throw new Exception("Failed to perform GetCapabilites on WMS server");
                    }

                    var matchingDataSet = catalog.DataSets.FirstOrDefault(d => d.Layer == url.Name);

                    if (matchingDataSet == null)
                    {
                        throw new Exception("failed to find dataset on given WMS server");
                    }

                    //used matching dataset to build the dataset
                    return BuildPipeline(matchingDataSet, permit);
                }


                var credentials = RetrieveCredentials(permit);

                // normalize attributes that are not mandatory
                url.Scalehint = url.Scalehint ?? "";
                url.Srs = url.Srs ?? "";

                // Create the WS process.
                var wmsProcessGuid = pyxlib.strToGuid("{F5E595F7-B58B-4d23-AC2B-865829306E10}");
                var spProc = PYXCOMFactory.CreateProcess(wmsProcessGuid);
                if (spProc.isNull())
                {
                    throw new Exception("Failed to create a WS process from GUID " + wmsProcessGuid);
                }

                // Set the Process 'Metadata'
                var procName = dataSet.Metadata != null && dataSet.Metadata.Name.HasContent() ? dataSet.Metadata.Name : (url.Name ?? "");
                var procDescription = dataSet.Metadata != null && dataSet.Metadata.Description.HasContent()  ? dataSet.Metadata.Description : "";
                spProc.setProcName(procName);
                spProc.setProcDescription(procDescription);

                // TODO: this is only needed when the logic handles exchanging credentials with the application level
                // this is going to be covered by an upcoming task
                //
                //add credentials provider to notify the WMS to ask for credentials for that url
                //var credentialsProvider =
                //    PYXCOMFactory.CreateProcess(PYXCOMFactory.WellKnownProcesses.UserCredentialsProvider);
                //spProc.getParameter(0).addValue(credentialsProvider);

                // Get min and max X and Y
                double[] bbox = RetrieveBoundingBox(url.Version, url.BBox);
                // Get our min and max resolutions for this data source.

                // Make attribute map
                var map = new Attribute_Map();
                map.set("server", url.ServerUrl);
                map.set("layer", url.Name);
                map.set("format", url.Format ?? "");
                map.set("srs", url.Srs ?? "");
                // Only set the styles if available, to avoid an error in the process
                if (url.Styles != null)
                {
                    //pick first style
                    //TODO: we might want to give the user the ability to chose different styles after import a WMS layer
                    map.set("styles", url.Styles.Split(',')[0]);
                }
                // get our mins and maxs x's and y's
                map.set("minLat", bbox[0].ToString());
                map.set("minLon", bbox[1].ToString());
                map.set("maxLat", bbox[2].ToString());
                map.set("maxLon", bbox[3].ToString());
                // Set the process attributes
                spProc.setAttributes(map);

                // Initialize the process
                if (spProc.initProc() != IProcess.eInitStatus.knInitialized)
                {
                    throw new Exception("Failed to initialize WMS server with error: " + spProc.getInitError().getError());
                }

                // Create and initialize the nearest neighbour sampler.
                var nnSamplerGuid = pyxlib.strToGuid("{D612004E-FC51-449a-B0D6-1860E59F3B0D}");
                var spSampler = PYXCOMFactory.CreateProcess(
                    new PYXCOMProcessCreateInfo(nnSamplerGuid)
                        .SetName("Nearest Neighbour Sampler - " + procName)
                        .SetDescription(procDescription)
                        .AddInput(0, spProc)
                );
                if (spSampler.isNull())
                {
                    throw new Exception("Failed to create a sampler process (GUID " + nnSamplerGuid + ")");
                }
                spSampler.initProc();

                // Create the coverage cache and return it (without initialization)
                return PYXCOMFactory.CreateProcess(
                    new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.CoverageCache)
                        .SetName(procName)
                        .SetDescription(procDescription)
                        .AddInput(0, spSampler)
                );
            }

            return null;
        }

        /// <summary>
        /// Retrieves (minX, minY, maxX, maxY) from outer text of 
        /// either a "BoundingBox", or "LatLonBoundingBox" or "EX_GeographicBoundingBox" element
        /// </summary>
        /// <param name="version">WMS version</param>
        /// <param name="bboxXml">outer XML</param>
        /// <returns>Array of the values</returns>
        private double[] RetrieveBoundingBox(String version, String bboxXml)
        {
            // get our mins and maxs x's and y's
            double[] bboxRow;
            if (version.StartsWith("1.3."))
            {
                // We don't know at this point whether this is a "BoundingBox", or "LatLonBoundingBox"
                // or "EX_GeographicBoundingBox", however there is a simple way to find out
                try
                {
                    bboxRow = ParseGeographicBoundingBoxElement(bboxXml);
                }
                catch (Exception)
                {
                    bboxRow = ParseBoundingBoxElement(bboxXml);
                }
            }
            else
            {
                bboxRow = ParseBoundingBoxElement(bboxXml);
            }

            return bboxRow;
        }

        /// <summary>
        /// Retrieves (minX, minY, maxX, maxY) from outer text an "EX_GeographicBoundingBox" element
        /// </summary>
        /// <param name="version">WMS version</param>
        /// <param name="bboxXml">outer XML</param>
        /// <returns>Array of the values</returns>
        private double[] ParseGeographicBoundingBoxElement(string bboxXml)
        {
            XmlDocument boundingBox = new XmlDocument();
            boundingBox.LoadXml(bboxXml);

            var minX = Double.Parse(boundingBox.SelectSingleNode("//*[local-name()='southBoundLatitude']").InnerText);
            var minY = Double.Parse(boundingBox.SelectSingleNode("//*[local-name()='westBoundLongitude']").InnerText);
            var maxX = Double.Parse(boundingBox.SelectSingleNode("//*[local-name()='northBoundLatitude']").InnerText);
            var maxY = Double.Parse(boundingBox.SelectSingleNode("//*[local-name()='eastBoundLongitude']").InnerText);

            return new double[] { minX, minY, maxX, maxY };
        }

        /// <summary>
        /// Retrieves (minX, minY, maxX, maxY) from outer text of 
        /// either a "BoundingBox", or "LatLonBoundingBox" element
        /// </summary>
        /// <param name="version">WMS version</param>
        /// <param name="bboxXml">outer XML</param>
        /// <returns>Array of the values</returns>
        private double[] ParseBoundingBoxElement(string bboxXml)
        {
            string[] latLonTokens = bboxXml.Split('"');

            var minX = Double.Parse(GetTokenValue(latLonTokens, "minx"));
            var minY = Double.Parse(GetTokenValue(latLonTokens, "miny"));
            var maxX = Double.Parse(GetTokenValue(latLonTokens, "maxx"));
            var maxY = Double.Parse(GetTokenValue(latLonTokens, "maxy"));

            return new double[] { minX, minY, maxX, maxY };
        }

        /// <summary>
        /// Retrieves the value for a key in an XML string.
        /// Used to get values for lat, lons and scales.
        /// ex: pass in the tokenized XML for the lat lon WMS
        /// data, and 'minx' to get the min lat.
        /// </summary>
        /// <param name="tokens">Tokenized XML</param>
        /// <param name="tokenKey"></param>
        /// <returns></returns>
        string GetTokenValue(string[] tokens, string tokenKey)
        {
            // TODO: remove tokens as input, and just pass XML and create tokens in this method
            var tokenLocation = -1;

            for (var index = 0; index < tokens.Length; index++)
            {
                if (tokens[index].ToLower().Contains(tokenKey.ToLower()))
                {
                    tokenLocation = index;
                    break;
                }
            }

            if (tokenLocation == -1)
            {
                return "";
            }

            var tokenValue = tokens[tokenLocation + 1].Replace("\\", "");
            return tokenValue;
        }
    }
}