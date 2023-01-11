using System;
using System.Collections.Generic;
using System.Xml;
using ApplicationUtility;

namespace Pyxis.IO.Sources.OGC
{
    /// <summary>
    /// XmlNodeExtractor helps to define Xml document parsing by creating a list of callback to be called per node local name
    /// 
    /// Example:
    /// var book = new Book();
    /// var parser = new XMLNodeExtractor()
    ///     .On("title", (node) => book.Title = node.InnerText)
    ///     .On("pages", (node) => book.PagesCount = int.Parse(node.InnerText));
    /// parser.pase(xmlDocument.DocumentElemenet);
    /// </summary>
    internal class XmlNodeExtractor
    {
        private readonly Dictionary<string, Action<XmlNode>> m_handlers = new Dictionary<string, Action<XmlNode>>();

        /// <summary>
        /// Static function to allow passsing a given node with a dictionary of handlers per node local name
        /// </summary>
        /// <param name="node">XmlNode to parse</param>
        /// <param name="callbacks">list of callbacks per nodename. Please note, nodenames have be lowercase</param>
        public static void ParseXmlNode(XmlNode node, Dictionary<string, Action<XmlNode>> callbacks)
        {
            foreach (XmlNode child in node.ChildNodes)
            {
                var name = child.LocalName.ToLower();

                if (callbacks.ContainsKey(name))
                {
                    callbacks[name](child);
                }
            }
        }

        /// <summary>
        /// Default constructor
        /// </summary>
        public XmlNodeExtractor()
        {    
        }

        /// <summary>
        /// Create a simple XmlNoeExctractor for parsing a single node
        /// </summary>
        /// <param name="name">name of the node</param>
        /// <param name="callback">callback to handle that node</param>
        public XmlNodeExtractor(string name, Action<XmlNode> callback)
        {
            On(name, callback);
        }

        /// <summary>
        /// Create a simple XmlNoeExctractor for parsing a single node
        /// </summary>
        /// <param name="name">name of the node</param>
        /// <param name="extractor">extractor to call on given node</param>
        public XmlNodeExtractor(string name, XmlNodeExtractor extractor)
        {
            On(name, extractor);
        }

        /// <summary>
        /// Add another node parsing function
        /// </summary>
        /// <param name="name">name of the node</param>
        /// <param name="callback">callback to handle that node</param>
        public XmlNodeExtractor On(string name, Action<XmlNode> callback)
        {
            var normalizedName = name.ToLower().Trim();
            m_handlers[normalizedName] = callback;    
            return this;
        }

        /// <summary>
        /// Add another node parsing function
        /// </summary>
        /// <param name="name">name of the node</param>
        /// <param name="extractor">extractor to call on given node</param>
        public XmlNodeExtractor On(string name, XmlNodeExtractor extractor)
        {
            return On(name, extractor.Parse);
        }


        /// <summary>
        /// Add another node parsing function.
        /// This is good when you need only one item in a deep xml path.
        /// 
        /// parer.On(new []{"shelf","book","author","name"}, (node) => name = node.InnerText);
        /// </summary>
        /// <param name="path">path of nodes to look for</param>
        /// <param name="extractor">extractor to call on given node</param>
        public XmlNodeExtractor On(string[] path, XmlNodeExtractor extractor)
        {
            if (path.Length == 0 || !path[0].HasContent())
            {
                throw new ArgumentException("Empty node name in path.", "path");
            }

            //simple path
            if (path.Length == 1)
            {
                return On(path[0], extractor);
            }

            for (var i = path.Length - 1; i > 0; i--)
            {
                extractor = new XmlNodeExtractor(path[i], extractor.Parse);
            }

            return On(path[0], extractor.Parse);
        }

        /// <summary>
        /// Add another node parsing function.
        /// This is good when you need only one item in a deep xml path.
        /// 
        /// parer.On(new []{"shelf","book","author","name"}, (node) => name = node.InnerText);
        /// </summary>
        /// <param name="path">path of nodes to look for</param>
        /// <param name="callback">callback to handle that node</param>
        public XmlNodeExtractor On(string[] path, Action<XmlNode> callback)
        {
            if (path.Length == 0 || ! path[0].HasContent())
            {
                throw new ArgumentException("Empty node name in path.", "path");
            }

            //simple path
            if (path.Length == 1)
            {
                return On(path[0], callback);
            }

            //build path from back to front
            var extractor = new XmlNodeExtractor(path[path.Length - 1], callback);

            for (var i = path.Length - 2; i > 0; i--)
            {
                extractor = new XmlNodeExtractor(path[i],extractor.Parse);
            }

            return On(path[0], extractor.Parse);
        }

        /// <summary>
        /// Perform a parsing of the given node
        /// </summary>
        /// <param name="node">XmlNode to parse</param>
        public virtual void Parse(XmlNode node)
        {
            ParseXmlNode(node, m_handlers);
        }
    }
}