using Pyxis.Utilities;

namespace Pyxis.IO.Sources.OGC
{
    /// <summary>
    /// Helper class for handling OGC WFS URLs
    /// </summary>
    public class OgcWebFeatureUrl : OgcUrl
    {
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="url">Address of an OGC resource on a OGC WFS server</param>
        public OgcWebFeatureUrl(string url) :
            base(url)
        {
            var query = new UriQueryBuilder(ServerUrl);

            // Read attributes
            Name = (query.Parameters.Get("typename")
                    ?? query.Parameters.Get("typenames"))
                   ?? query.Parameters.Get("name");

            // Remove attributes from the server URL
            query.RemoveParameter("outputformat");
            query.RemoveParameter("filter");
            query.RemoveParameter("featureID");
            query.RemoveParameter("bbox");
            query.RemoveParameter("typename");
            query.RemoveParameter("typenames");
            query.RemoveParameter("name");

            // Save the server URL and reset the service type
            ServerUrl = query.ToString();
            Service = "WFS";
        }

        public string Name
        {
            get { return GetAttribute("typename"); }
            set { SetAttribute("typename", value); }
        }
    }
}