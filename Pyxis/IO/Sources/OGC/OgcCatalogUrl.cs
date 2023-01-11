using Pyxis.Utilities;

namespace Pyxis.IO.Sources.OGC
{
    /// <summary>
    /// Helper class for handling OGC CSW URLs
    /// </summary>
    public class OgcCatalogUrl : OgcUrl
    {
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="url">Address of an OGC resource on a OGC CSW server</param>
        public OgcCatalogUrl(string url) :
            base(url)
        {
            var query = new UriQueryBuilder(ServerUrl);

            // Read attributes
            RequestId = query.Parameters.Get("requestId");
            StartPosition = query.Parameters.Get("startPosition");
            MaxRecords = query.Parameters.Get("maxRecords");
            
            // Remove attributes from the server URL
            query.RemoveParameter("requestId");
            query.RemoveParameter("startPoisition");
            query.RemoveParameter("maxRecords");
            
            // Save the server URL and reset the service type
            ServerUrl = query.ToString();
            Service = "CSW";
        }

        public string RequestId
        {
            get { return GetAttribute("requestId"); }
            set { SetAttribute("requestId", value); }
        }

        public string StartPosition
        {
            get { return GetAttribute("startPosition"); }
            set { SetAttribute("startPosition", value); }
        }

        public string MaxRecords
        {
            get { return GetAttribute("maxRecords"); }
            set { SetAttribute("maxRecords", value); }
        }
    }
}