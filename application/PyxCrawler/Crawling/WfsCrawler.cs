using System.Globalization;
using PyxCrawler.Models;
using PyxCrawler.Utilities;
using Pyxis.Contract.DataDiscovery;
using Pyxis.IO.Sources.OGC;

namespace PyxCrawler.Crawling
{
    public class WfsCrawler : OwsCrawler, ICrawler
    {
        private static readonly string[] s_supportedVersions = { "2.0.0", "1.1.1", "1.1.0", "1.0.0" };
        private static readonly OwsDefinition s_definition = new OwsDefinition
        {
            Type = OwsType.WFS,
            SupportedVersions = s_supportedVersions,
            OwsService = new OgcWebFeatureService()
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

            return DataSetHelper.GetWgs84BoundingBox(dataSet);
        }
    }
}