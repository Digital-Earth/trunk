using System.Net;

namespace Pyxis.IO.Sources.OGC
{
    /// <summary>
    /// Implements IDataSetCatalog for OGC WMS data
    /// </summary>
    public class OgcWebMapServer : OgcWebServerBase
    {
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="url">Address of a resource on the OGC WMS server</param>
        /// <param name="credentials">User credentials (if authentication required)</param>
        /// <param name="service">The corresponding OGC WMS service object</param>
        protected OgcWebMapServer(string url, NetworkCredential credentials, OgcWebMapService service)
            : base(url, credentials, service)
        {
            Catalog.Metadata.Tags.Add("WMS");
        }

        public static OgcWebMapServer Create(string url, NetworkCredential credentials, OgcWebMapService service)
        {
            var server = new OgcWebMapServer(url, credentials, service);
            server.GetCapabilities();
            server.ParseCapabilites();
            return server;
        }

        protected sealed override void ParseCapabilites()
        {
            var parser = new OgcWebMapCapabilitiesParser(m_url, Catalog);
            parser.Parse(m_capabilities);            
        }
    }
}