using System;
using System.Xml.Linq;

namespace worldView.gallery.Services
{
    /// <summary>
    /// create a sitemap.xml index file. A sitemap file is used by Google crawler to index a website. 
    /// A sitemap file contains list of known sitemap.xml files.
    /// this allow the crawler to fetch smaller sitemap.xml for bigger / dynamic sites.
    /// 
    /// For more details see: http://www.sitemaps.org/protocol.html
    /// </summary>
    public class SitemapIndexFile
    {
        private readonly XElement m_root;
        private readonly XNamespace m_xmlns;

        public SitemapIndexFile()
        {
            m_xmlns = XNamespace.Get("http://www.sitemaps.org/schemas/sitemap/0.9");
            m_root = new XElement(m_xmlns + "sitemapindex");
        }

        /// <summary>
        /// Adds a sitemap.xml url to the sitemap index file.
        /// </summary>
        /// <param name="url">A sitemap url string.</param>
        public void AddSitemap(string url)
        {
            m_root.Add(new XElement(m_xmlns + "sitemap", new XElement(m_xmlns + "loc", Uri.EscapeUriString(url))));
        }

        /// <summary>
        /// Generate xml content for the sitemap index file.
        /// </summary>
        /// <returns>Xml string</returns>
        public override string ToString()
        {
            return m_root.ToString();
        }
    }
}