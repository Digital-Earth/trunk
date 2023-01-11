using System;
using System.Collections.Generic;
using System.Net;
using System.Xml;
using ApplicationUtility;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.IO.Sources.Remote;

namespace Pyxis.IO.Sources.OGC
{
    /// <summary>
    /// Base implementation for an OGC web server
    /// </summary>
    public abstract class OgcWebServerBase
    {
        /// <summary>
        /// The corresponding OGC service object
        /// </summary>
        public IWebDataSetDiscoveryService Service { get; private set; }

        /// <summary>
        /// The catalog of data sets on this server.
        /// </summary>
        public DataSetCatalog Catalog { get; private set; }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="url">URL to use to connect to the server</param>
        /// <param name="credentials">User credentials (if authentication required)</param>
        /// <param name="ogcWebService">The corresponding OGC service object</param>
        protected OgcWebServerBase(string url, NetworkCredential credentials, OgcWebServiceBase ogcWebService)
        {
            m_url = new OgcInvariantUrl(url);
            m_credentials = credentials;
            Service = ogcWebService;

            Catalog = new DataSetCatalog()
            {
                DataType = DataSetType.OGC.ToString(),
                Uri = m_url.ToString(),
                Metadata = new SimpleMetadata()
                {
                    // Set the default Metadata values
                    Name = m_url.ServerUrl,
                    Description = Service.Title,
                    Tags = new List<string> {"OGC"}
                },

                SubCatalogs = new List<DataSetCatalog>(),
                DataSets = new List<DataSet>()
            };
        }

        /// <summary>
        /// Performs a GetCapabilities request to the server and receives an XML document as a result
        /// </summary>
        protected virtual void GetCapabilities()
        {
            try
            {
                if (!m_url.Request.HasContent())
                {
                    m_url.Request = "GetCapabilities";
                }
                // Use the credentials if provided
                m_capabilities = m_credentials != null ?
                    WebRequestHelper.GetXmlDocumentWithAuthorization(m_url.ToString(), m_credentials)
                    : WebRequestHelper.GetXmlDocument(m_url.ToString());
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

        /// <summary>
        /// Creates a data set description text from information retrieved from the GetCapabilities response
        /// <param name="dataSetTitle">Title of the dataset (mandatory)</param>
        /// <param name="dataSetAbstract">The abstract (optional)</param>
        /// </summary>
        protected string DataSetDescription(string dataSetTitle, string dataSetAbstract = null)
        {
            return string.IsNullOrEmpty(dataSetAbstract) ? dataSetTitle : dataSetTitle + " - " + dataSetAbstract;
        }

        /// <summary>
        /// Parses information received with a GetCapabilities request to the server
        /// </summary>
        /// <exception cref="System.Exception">When the capabilities are not available></exception>
        protected abstract void ParseCapabilites();

        protected IOgcUrl m_url;
        protected NetworkCredential m_credentials;
        protected XmlDocument m_capabilities;
    }
}