using System;
using ApplicationUtility;
using Pyxis.Contract;
using Pyxis.Contract.DataDiscovery;

namespace Pyxis.IO.Sources.OGC
{
    /// <summary>
    /// Implements IWebDataDiscoveryService for OGC WFS data
    /// </summary>
    public class OgcWebFeatureService : OgcWebServiceBase
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public OgcWebFeatureService()
            : base("WFS - Web Feature Service", "wfs")
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
                var ogcUrl = new OgcWebFeatureUrl(uri);

                // Try to open a server with the server URL
                var server = OgcWebFeatureServer.Create(ogcUrl.ServerUrl, RetrieveCredentials(permit), this);

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
            if (!IsUriSupported(dataSet.Uri))
            {
                return null;
            }

            var url = new OgcWebFeatureUrl(dataSet.Uri);

            // Name attribute of the data set is mandatory
            if (string.IsNullOrEmpty(url.Name))
            {
                throw new Exception("No data set name provided in URI" + url);
            }

            // Create the WS process.
            var procName = dataSet.Metadata != null && dataSet.Metadata.Name.HasContent() ? dataSet.Metadata.Name : (url.Name ?? "");
            var spProc = PYXCOMFactory.CreateProcess(
                new PYXCOMProcessCreateInfo(pyxlib.strToGuid("{AA47A7D3-6749-4bd4-99B4-9B4B6CF3EF9C}"))
                    .AddAttribute("layer_name", url.Name)
                    .AddAttribute("server", url.ServerUrl)
                    .SetName(procName)
            );
            if (spProc.isNull())
            {
                throw new Exception("Failed to create a WS process from GUID {AA47A7D3-6749-4bd4-99B4-9B4B6CF3EF9C}");
            }

            // TODO: this is only needed when the logic handles exchanging credentials with the application level
            // this is going to be covered by an upcoming task
            //
            //add credentials provider to notify the WFS to ask for credentials for that url
            //var credentialsProvider =
            //    PYXCOMFactory.CreateProcess(PYXCOMFactory.WellKnownProcesses.UserCredentialsProvider);
            //
            //spProc.getParameter(1).addValue(credentialsProvider);

            // Create and return the features summary
            return PYXCOMFactory.CreateProcess(
                new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.FeaturesSummary)
                    .AddInput(0, spProc)
                    .SetName(procName)
                    .SetDescription(dataSet.Metadata != null && dataSet.Metadata.Description.HasContent() ? dataSet.Metadata.Description : "Web Features Service data source")
            );
        }
    }
}