using System;
using System.Net;
using System.Xml;
using ApplicationUtility;
using Pyxis.IO.Sources.Remote;

namespace Pyxis.IO.Sources.OGC
{
    /// <summary>
    /// Implements IDataSetCatalog for OGC CSW server
    /// </summary>
    public class OgcCatalogServer : OgcWebServerBase
    {
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="url">Address of a resource on the OGC CSW server</param>
        /// <param name="credentials">User credentials (if authentication required)</param>
        /// <param name="service">The corresponding OGC CSW service object</param>
        protected OgcCatalogServer(string url, NetworkCredential credentials, OgcCatalogService service)
            : base(url, credentials, service)
        {
            Catalog.Metadata.Tags.Add("CSW");
        }

        public static OgcCatalogServer Create(string url, NetworkCredential credentials, OgcCatalogService service)
        {
            var server = new OgcCatalogServer(url, credentials, service);
            server.GetCapabilities();
            server.ParseCapabilites();
            return server;
        }

        protected override void GetCapabilities()
        {
            if (m_url.Request == "GetRecords")
            {
                var cswUrl = new OgcCatalogUrl(m_url.ToString());
                m_capabilities = GetRecords(cswUrl);
                m_url = cswUrl;
            }
            else
            {
                base.GetCapabilities();
            }
        }

        protected XmlDocument GetRecords(OgcCatalogUrl cswUrl)
        {
            if (!cswUrl.RequestId.HasContent())
            {
                cswUrl.RequestId = Guid.NewGuid().ToString();
            }

            try
            {
                var body = "<?xml version=\"1.0\"?>" +
                   "<csw:GetRecords service=\"CSW\" version=\"2.0.2\" resultType=\"results\" maxRecords=\"" +
                   cswUrl.MaxRecords + "\" startPosition=\"" + cswUrl.StartPosition + "\" " + " requestId=\"" + cswUrl.RequestId + "\" " +
                   "xmlns:csw=\"http://www.opengis.net/cat/csw/2.0.2\" " +
                   "xmlns:gmd=\"http://www.isotc211.org/2005/gmd\" " +
                   "xmlns:gml=\"http://www.opengis.net/gml\" " +
                   "outputSchema=\"http://www.isotc211.org/2005/gmd\" >" +
                    //"outputSchema=\"http://www.opengis.net/cat/csw/2.0.2\" >" +
                   "<csw:Query typeNames=\"gmd:MD_Metadata\">" +
                   "<csw:ElementSetName typeNames=\"gmd:MD_Metadata\">full</csw:ElementSetName>" +
                   "</csw:Query>" +
                   "</csw:GetRecords>";

                // Use the credentials if provided
                return m_credentials != null ?
                    WebRequestHelper.GetXmlDocumentWithAuthorization(m_url.ToString(), m_credentials, body)
                    : WebRequestHelper.GetXmlDocument(m_url.ToString(), body);
            }
            catch (Exception ex)
            {
                Trace.error("Error accessing an OGC server at " + m_url.ToString());
                throw new ApplicationException(
                    "Error accessing an OGC server at " + m_url.ToString(),
                    ex
                    );
            }
        }

        protected sealed override void ParseCapabilites()
        {
            var parser = new OgcCatalogCapabilitiesParser(m_url, Catalog, (Service as OgcCatalogService).IsUriSupported);
            parser.Parse(m_capabilities);            
        }
    }
}