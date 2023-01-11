using System.Collections.Generic;
using System.Linq;
using ApplicationUtility;
using Pyxis.Utilities;

namespace Pyxis.IO.Sources.OGC
{
    /// <summary>
    /// Base implementation for IOgcUrlAttribute
    /// </summary>
    public class OgcUrlAttribute: IOgcUrlAttribute
    {
        /// <summary>
        /// Name of the attribute (case insensitive)
        /// </summary>
        public string Name { get; protected set; }

        /// <summary>
        /// Value of the attribute
        /// </summary>
        public string Value { get; set; }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="name">Attribute name</param>
        /// <param name="value">Attribute value</param>
        public OgcUrlAttribute(string name, string value = null)
        {
            Name = name;
            Value = value;
        }
    }

    /// <summary>
    /// Base implementation for IOgcUrl
    /// </summary>
    public abstract class OgcUrl: IOgcUrl
    {
        /// <summary>
        /// URL of the server (contains the protocol, domain name and constant attributes)
        /// </summary>
        public string ServerUrl { get; protected set; }

        /// <summary>
        /// Identifier of the related OGC service
        /// </summary>
        public string Service
        {
            get
            {
                var query = new UriQueryBuilder(ServerUrl);
                return query.Parameters.Get("service");
            }
            protected set
            {
                // Replace the value in the server URL
                var query = new UriQueryBuilder(ServerUrl);
                query.RemoveParameter("service");
                if (value != null)
                {
                    query.OverwriteParameter("service", value);
                }
                ServerUrl = query.ToString();
            }
        }

        /// <summary>
        /// OGC version of the server
        /// </summary>
        public string Version
        {
            get
            {
                var query = new UriQueryBuilder(ServerUrl);
                return query.Parameters.Get("version");
            }
            set
            {
                // Replace the value in the server URL
                var query = new UriQueryBuilder(ServerUrl);
                query.RemoveParameter("version");
                if (value != null)
                {
                    query.OverwriteParameter("version", value);
                }
                ServerUrl = query.ToString();
            }
        }

        /// <summary>
        /// Type of a OGC request that this URL represents
        /// </summary>
        public string Request { get; set; }

        /// <summary>
        /// Attributes of the URL that may vary
        /// </summary>
        public List<IOgcUrlAttribute> Attributes
        {
            get
            {
                // Construct an OgcUrlAttribute object from each key-value pair and put in a list
                return m_attributes.Keys.Select(
                    key => new OgcUrlAttribute(key, m_attributes[key]))
                    .Cast<IOgcUrlAttribute>().ToList();
            }
        }

        /// <summary>
        /// Converts the object data to a URL string
        /// </summary>
        public override string ToString()
        {
            var query = new UriQueryBuilder(ServerUrl);
            // Set the request attribute first
            if (Request != null)
            {
                query.OverwriteParameter("request", Request);
            }
            // Set the other attributes
            switch (Request)
            {
                case "GetCapabilities":
                    // GetCapabilities request has no attributes except the object public fields
                    query.OverwriteParameter("request", Request);
                    return query.ToString();

                default:
                    // By default, insert all attributes in the URL string representation
                    foreach (var key in m_attributes.Keys)
                    {
                        if (m_attributes[key] != null)
                        {
                            query.OverwriteParameter(key, m_attributes[key]);
                        }
                        else
                        {
                            // Be safe and remove the attribute from the URL, in case it's there
                            query.RemoveParameter(key);
                        }
                    }
                    return query.ToString();
            }
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="url">The input url string that gets parsed</param>
        protected OgcUrl(string url)
        {
            var query = new UriQueryBuilder(url);
            // In general just extract the request type from the URL
            Request = query.Parameters.Get("request");
            query.RemoveParameter("request");
            // At this level consider everything else as the server URL part
            ServerUrl = query.ToString();
        }

        /// <summary>
        /// Key-value pairs that represent the URL attributes
        /// </summary>
        private readonly Dictionary<string, string> m_attributes = new Dictionary<string, string>();

        /// <summary>
        /// Gets an attribute of the URL.
        /// </summary>
        /// <param name="key">Attribute name</param>
        /// <returns>The attribute value</returns>
        protected string GetAttribute(string key)
        {
            return key == null ? null : m_attributes.ContainsKey(key.ToLower()) ? m_attributes[key.ToLower()] : null;
        }

        /// <summary>
        /// Sets an attribute of the URL.
        /// </summary>
        /// <param name="key">Attribute name</param>
        /// <param name="value">Attribute value to set</param>
        protected void SetAttribute(string key, string value)
        {
            if (key == null)
            {
                return;
            }
            if (value == null)
            {
                m_attributes.Remove(key.ToLower());
            }
            m_attributes[key.ToLower()] = value;
        }

        /// <summary>
        /// Create OgcUrl from a given url string.
        /// </summary>
        /// <param name="url">url</param>
        /// <returns>IOgcUrl instance that is specialized based on request service found in url</returns>
        public static IOgcUrl Create(string url)
        {
            var invariant = new OgcInvariantUrl(url);

            if (invariant.Service.HasContent())
            {
                switch (invariant.Service.ToLower())
                {
                    case "wms":
                        return new OgcWebMapUrl(url);
                    case "wfs":
                        return new OgcWebFeatureUrl(url);
                    case "wcs":
                        return new OgcWebCoverageUrl(url);
                    case "csw":
                        return new OgcCatalogUrl(url);
                }
            }

            return invariant;            
        }
    }
}
