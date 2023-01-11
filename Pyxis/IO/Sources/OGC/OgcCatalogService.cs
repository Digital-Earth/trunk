using System;
using Pyxis.Contract;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Core;
using Pyxis.IO.Import;

namespace Pyxis.IO.Sources.OGC
{
    /// <summary>
    /// Implements IOgcWebService for OGC CSW server
    /// </summary>
    public class OgcCatalogService : OgcWebServiceBase
    {
        private Engine m_engine = null;

        /// <summary>
        /// Constructor
        /// </summary>
        public OgcCatalogService()
            : base("CSW - Catalog Service", "csw")
        {
        }

        public OgcCatalogService(Engine engine)
            : this()
        {
            m_engine = engine;
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
                var ogcUrl = new OgcCatalogUrl(uri);

                var requestUrl = ogcUrl.ServerUrl;
                if (ogcUrl.Request == "GetRecords")
                {
                    requestUrl = ogcUrl.ToString();
                }

                // Try to open a server with the server URL
                var server = OgcCatalogServer.Create(requestUrl, RetrieveCredentials(permit), this);

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
            //we don't generate any services
            return null;
        }
    }
}