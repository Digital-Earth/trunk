using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Xml;
using System.Xml.Xsl;

namespace ApplicationUtility
{
    public static class XmlUtils
    {
        public static string SafeInnerText(this XmlNode node)
        {
            if (node != null) return node.InnerText;
            return "";
        }

        public static string SafeAttributeValue(this XmlAttribute attribute)
        {
            if (attribute != null) return attribute.Value;
            return null;
        }

        public static XmlNode FindSingleNodeWithLocalName(XmlNode root, string localName)
        {
            return root.SelectSingleNode(".//*[local-name()=\"" + localName + "\"]");
        }

        public static XmlNodeList FindNodesWithLocalName(XmlNode root, string localName)
        {
            return root.SelectNodes(".//*[local-name()=\"" + localName + "\"]");
        }

        public static XmlAttribute FindAttributeWithLocalName(this XmlNode node, string localName)
        {
            foreach (XmlAttribute attr in node.Attributes)
            {
                if (attr.LocalName == localName)
                {
                    return attr;
                }
            }
            return null;
        }

        public static string ApplyXslt(string xmlString,string xsltString)
        {
            // do the XSL transform
            StringReader strReader = new StringReader(xsltString);
            XmlTextReader xsltReader = new XmlTextReader(strReader);

            strReader = new StringReader(xmlString);
            XmlTextReader xmlReader = new XmlTextReader(strReader);

            StringBuilder strHtmlOutput = new StringBuilder();
            XmlWriterSettings settings = new XmlWriterSettings();

            // makes the html easier to read for debugging purposes
            settings.Indent = true;

            // to ensure the html is XHTML 1.0 Strict
            settings.OmitXmlDeclaration = true;

            XmlWriter htmlWriter = XmlWriter.Create(strHtmlOutput, settings);

            XslCompiledTransform xslt = new XslCompiledTransform();

            // compile the style sheet
            xslt.Load(xsltReader);

            // perform the transformation
            xslt.Transform(xmlReader, htmlWriter);

            string htmlOutput = strHtmlOutput.ToString();

            return htmlOutput;
        }
    }
}
