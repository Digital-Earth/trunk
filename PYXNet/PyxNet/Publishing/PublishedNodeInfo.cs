using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using Pyxis.Utilities;

namespace PyxNet.Publishing
{
    /// <summary>
    /// Publishes a NodeInfo (using Xml).
    /// </summary>
    public class PublishedNodeInfo: PublishedXmlObject, Publisher.IDynamicPublishedItem 
    {
        private NodeInfo m_nodeInfo;

        /// <summary>
        /// Gets the node info.
        /// </summary>
        /// <value>The node info.</value>
        public NodeInfo NodeInfo
        {
            get { return m_nodeInfo; }
            set
            {
                m_nodeInfo = value;
                XmlObject = value;
                KeywordsChanged.SafeInvoke(this);
            }
        }

        public event EventHandler KeywordsChanged;

        /// <summary>
        /// Initializes a new instance of the <see cref="PublishedNodeInfo"/> class.
        /// </summary>
        /// <param name="nodeInfo">The node info.</param>
        public PublishedNodeInfo(NodeInfo nodeInfo): base(nodeInfo)
        {
            m_nodeInfo = nodeInfo;
        }

        private static XmlDocument s_document = new XmlDocument();

        /// <summary>
        /// Gets the XMLDocumentFragment that corresponds to this object.
        /// </summary>
        /// <value>The XML document fragment.</value>
        protected override XmlDocumentFragment XmlDocumentFragment
        {
            get
            {
                XmlDocumentFragment result = s_document.CreateDocumentFragment();
                string xmlText = String.Format( "<NodeInfo id=\"{0}\"></NodeInfo>", 
                    NodeInfo.NodeGUID.ToString());
                result.InnerXml = xmlText;
                return result;
            }
        }

        /// <summary>
        /// Gets the keywords for this published item.  This is the
        /// set of terms that the item will be indexed on in the
        /// query hash table.
        /// </summary>
        /// <value>The keywords.</value>
        public override IEnumerable<string> Keywords
        {
            get {
                yield return NodeInfo.NodeGUID.ToString();
                yield return NodeInfo.FriendlyName;
                yield return String.Format("<NodeInfo id=\"{0}\"></NodeInfo>",
                    NodeInfo.NodeGUID.ToString());
            }
        }

        public override QueryResult Matches(Query query, Stack stack)
        {
            if (query.Contents.Equals(NodeInfo.FriendlyName,StringComparison.OrdinalIgnoreCase))
            {
                return Publisher.CreateQueryResult(stack, query, NodeInfo.ToMessage());
            }

            return base.Matches(query, stack);
        }
    }
}
