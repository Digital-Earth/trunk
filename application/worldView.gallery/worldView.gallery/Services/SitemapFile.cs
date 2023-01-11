using System;
using System.Xml.Linq;

namespace worldView.gallery.Services
{
    /// <summary>
    /// create a sitemap.xml file. A sitemap file is used by Google crawler to index a website. 
    /// A sitemap file contains list of known urls so the crawling effort would be simpler.
    /// Each url page can be decorated with additional information such as change frequency and priority.
    /// 
    /// For more details see: http://www.sitemaps.org/protocol.html
    /// </summary>
    public class SitemapFile
    {
        private readonly XElement m_root;
        private readonly XNamespace m_xmlns;

        public SitemapFile()
        {
            m_xmlns = XNamespace.Get("http://www.sitemaps.org/schemas/sitemap/0.9");
            m_root = new XElement(m_xmlns + "urlset");
        }

        /// <summary>
        /// Add a Url to the site map.
        /// </summary>
        /// <param name="url">A url string.</param>
        /// <param name="refreshRate">An hint for the crawler how frequently this url updates</param>
        /// <param name="priority">priority for that page. default 0.5, range: 0...1</param>
        public void AddUrl(string url, SitemapEntryRefreshRate refreshRate, double priority)
        {
            m_root.Add(
                new XElement(m_xmlns + "url",
                    new XElement(m_xmlns + "loc", Uri.EscapeUriString(url)),
                    new XElement(m_xmlns + "changefreq", refreshRate.ToString().ToLower()),
                    new XElement(m_xmlns + "priority", priority.ToString())
                    ));
        }

        /// <summary>
        /// Generate xml content for the sitemap file.
        /// </summary>
        /// <returns>Xml string</returns>
        public override string ToString()
        {
            return m_root.ToString();
        }
    }
}