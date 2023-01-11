using System.Linq;
using Pyxis.Utilities;

namespace Pyxis.IO.Sources.OGC
{
    /// <summary>
    /// Generalized implementation of IOgcUrl that contains no logic specific for any kind of an OGC service.
    /// </summary>
    public class OgcInvariantUrl : OgcUrl
    {
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="url">The input url string that gets parsed</param>
        public OgcInvariantUrl(string url) :
            base(url)
        {
            // Retrieve all attributes
            var query = new UriQueryBuilder(url);
            foreach (var key in query.Parameters.Keys.Cast<object>()
                .Select(obj => obj.ToString().ToLower()).Where(
                    key => key != "service" && key != "version" && key != "request"
                ))
            {
                // Memorize the attribute
                SetAttribute(key, query.Parameters.Get(key));
            }
        }

        /// <summary>
        /// Gets an attribute of the URL. Allow to read the public fields too.
        /// </summary>
        /// <param name="key">Attribute name</param>
        /// <returns>The attribute value</returns>
        public new string GetAttribute(string key)
        {
            switch (key.ToLower())
            {
                case "service":
                    return Service;
                case "version":
                    return Version;
                case "request":
                    return Request;
                default:
                    return base.GetAttribute(key);
            }
        }

        /// <summary>
        /// Sets an attribute of the URL. Allows setting the public fields via this interface.
        /// </summary>
        /// <param name="key">Attribute name</param>
        /// <param name="value">Atribute value to set</param>
        public new void SetAttribute(string key, string value)
        {
            switch (key.ToLower())
            {
                case "service":
                    Service = value;
                    break;
                case "version":
                    Version = value;
                    break;
                case "request":
                    Request = value;
                    break;
                default:
                    base.SetAttribute(key, value);
                    break;
            }
        }

        /// <summary>
        /// Deletes all attributes.
        /// </summary>
        public void ClearAttributes()
        {
            foreach (var attr in Attributes)
            {
                SetAttribute(attr.Name, null);
            }
        }

        /// <summary>
        /// Additionally, allow to change the service type.
        /// </summary>
        public new string Service
        {
            get { return base.Service; }
            set { base.Service = value; }
        }
    }
}