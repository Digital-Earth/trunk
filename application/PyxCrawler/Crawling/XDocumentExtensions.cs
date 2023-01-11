using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Xml.Linq;

namespace PyxCrawler.Crawling
{
    public static class XDocumentExtensions
    {
        public static IEnumerable<XElement> DescendantsByLocalName(this XElement root, string localName)
        {
            return root.Descendants().Where(x => x.Name.LocalName.Equals(localName, StringComparison.InvariantCultureIgnoreCase));
        }

        public static IEnumerable<XElement> ElementsByLocalName(this XElement root, string localName)
        {
            return root.Elements().Where(x => x.Name.LocalName.Equals(localName, StringComparison.InvariantCultureIgnoreCase));
        }

        public static XElement ElementByLocalName(this XElement root, string localName)
        {
            if (root == null)
            {
                return null;
            }
            return root.ElementsByLocalName(localName).FirstOrDefault();
        }
    }
}