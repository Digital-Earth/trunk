using System.Net;

namespace Pyxis.IO.Sources.OGC
{
    /// <summary>
    /// Implements IDataSetCatalog for OGC WCS data
    /// </summary>
    public class OgcWebCoverageServer : OgcWebServerBase
    {
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="url">Address of a resource on the OGC WCS server</param>
        /// <param name="credentials">User credentials (if authentication required)</param>
        /// <param name="service">The corresponding OGC WCS service object</param>
        protected OgcWebCoverageServer(string url, NetworkCredential credentials, OgcWebServiceBase service)
            : base(url, credentials, service)
        {
            Catalog.Metadata.Tags.Add("WCS");
        }

        public static OgcWebCoverageServer Create(string url, NetworkCredential credentials, OgcWebServiceBase service)
        {
            var server = new OgcWebCoverageServer(url,credentials,service);
            server.GetCapabilities();
            server.ParseCapabilites();            
            return server;
        }

        protected sealed override void ParseCapabilites()
        {
           var parser = new OgcWebCoverageCapabilitiesParser(m_url,Catalog);
            parser.Parse(m_capabilities);
        }

    }
}