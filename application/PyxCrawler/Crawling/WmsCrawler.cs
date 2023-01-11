using System.Globalization;
using System.Xml.Linq;
using PyxCrawler.Models;
using Pyxis.Contract.DataDiscovery;
using Pyxis.IO.Sources.OGC;

namespace PyxCrawler.Crawling
{
    public class WmsCrawler : OwsCrawler, ICrawler
    {
        private static readonly string[] s_supportedVersions = {"1.3.0", "1.1.1", "1.1.0", "1.0.0"};
        private static readonly OwsDefinition s_definition = new OwsDefinition
        {
            Type = OwsType.WMS,
            SupportedVersions = s_supportedVersions,
            OwsService = new OgcWebMapService()
        };

        public void Crawl(CrawlingTask task, OnlineGeospatialEndpoint endpoint)
        {
            base.Crawl(task, endpoint, s_definition);
        }
        
        protected override Wgs84BoundingBox ExtractBBox(DataSet dataSet)
        {
            if (CultureInfo.InvariantCulture.CompareInfo.IndexOf(dataSet.Uri, "service=" + s_definition.Type, CompareOptions.IgnoreCase) < 0)
            {
                return null;
            }

            Wgs84BoundingBox bBox = null;

            var uri = new OgcWebMapUrl(dataSet.Uri);

            if (uri.BBox != null)
            {
                var xBBox = XElement.Parse(uri.BBox);
                try
                {
                    bBox = ExtractGeographicBBox(xBBox);
                }
                catch
                {
                }
                if (bBox == null)
                {
                    try
                    {
                        bBox = ExtractXYBBox(xBBox);
                        CorrectProjection(uri.BBox, bBox);
                    }
                    catch
                    {
                        System.Diagnostics.Trace.WriteLine("Failed to get bbox from " + s_definition.Type + "data set.");
                    }
                }
            }
            return bBox;
        }
    }
}