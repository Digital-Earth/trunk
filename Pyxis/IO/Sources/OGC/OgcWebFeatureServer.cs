using System.Net;

namespace Pyxis.IO.Sources.OGC
{
    /// <summary>
    /// Implements IDataSetCatalog for OGC WFS data
    /// </summary>
    public class OgcWebFeatureServer : OgcWebServerBase
    {
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="url">Address of a resource on the OGC WFS server</param>
        /// <param name="credentials">User credentials (if authentication required)</param>
        /// <param name="service">The corresponding OGC WFS service object</param>
        protected OgcWebFeatureServer(string url, NetworkCredential credentials, OgcWebFeatureService service)
            : base(url, credentials, service)
        {
            Catalog.Metadata.Tags.Add("WFS");
        }

        public static OgcWebFeatureServer Create(string url, NetworkCredential credentials, OgcWebFeatureService service)
        {
            var server = new OgcWebFeatureServer(url, credentials, service);
            server.GetCapabilities();
            server.ParseCapabilites();
            return server;
        }

        protected sealed override void ParseCapabilites()
        {
           var parser = new OgcWebFeatureCapabilitiesParser(m_url,Catalog);
            parser.Parse(m_capabilities);
        }
    }
}