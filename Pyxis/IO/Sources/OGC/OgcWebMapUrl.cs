using Pyxis.Utilities;

namespace Pyxis.IO.Sources.OGC
{
    /// <summary>
    /// Helper class for handling OGC WMS URLs
    /// </summary>
    public class OgcWebMapUrl : OgcUrl
    {
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="url">Address of an OGC resource on a OGC WMS server</param>
        public OgcWebMapUrl(string url) :
            base(url)
        {
            var query = new UriQueryBuilder(ServerUrl);

            // Read attributes
            Name = query.Parameters.Get("layers");
            BBox = query.Parameters.Get("bbox");
            Scalehint = query.Parameters.Get("scalehint");
            Format = query.Parameters.Get("format");
            Srs = query.Parameters.Get("srs");
            Styles = query.Parameters.Get("styles");

            // Remove attributes from the server URL
            query.RemoveParameter("width");
            query.RemoveParameter("height");
            query.RemoveParameter("layers");
            query.RemoveParameter("bbox");
            query.RemoveParameter("scalehint");
            query.RemoveParameter("format");
            query.RemoveParameter("srs");
            query.RemoveParameter("styles");

            // Save the server URL and reset the service type
            ServerUrl = query.ToString();
            Service = "WMS";
        }

        public string Name
        {
            get { return GetAttribute("layers"); }
            set { SetAttribute("layers", value); }
        }

        public string BBox
        {
            get { return GetAttribute("bbox"); }
            set { SetAttribute("bbox", value); }
        }

        public string Scalehint
        {
            get { return GetAttribute("scalehint"); }
            set { SetAttribute("scalehint", value); }
        }

        public string Format
        {
            get { return GetAttribute("format"); }
            set { SetAttribute("format", value); }
        }

        public string Srs
        {
            get { return GetAttribute("srs"); }
            set { SetAttribute("srs", value); }
        }

        public string Styles
        {
            get { return GetAttribute("styles"); }
            set { SetAttribute("styles", value); }
        }
    }
}