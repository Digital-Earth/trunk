using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Publishing
{
    /// <summary>
    /// This class wraps up publishing an "XML" object (an object that can be
    /// represented as XML.)
    /// </summary>
    public abstract class PublishedXmlObject: Publisher.IPublishedItemInfo
    {
        private ITransmissible m_xmlObject;

        /// <summary>
        /// Gets the XML object.
        /// </summary>
        /// <value>The XML object.</value>
        protected ITransmissible XmlObject
        {
            get { return m_xmlObject; }
            set { m_xmlObject = value; }
        }


        /// <summary>
        /// Initializes a new instance of the <see cref="PublishedXmlObject"/> class.
        /// </summary>
        /// <param name="xmlObject">The XML object.</param>
        public PublishedXmlObject(ITransmissible xmlObject)
        {
            m_xmlObject = xmlObject;
        }

        /// <summary>
        /// Gets the XMLDocumentFragment that corresponds to this object.
        /// </summary>
        /// <value>The XML document fragment.</value>
        protected abstract System.Xml.XmlDocumentFragment XmlDocumentFragment { get;}

        #region IPublishedItemInfo Members

        /// <summary>
        /// Gets the keywords for this published item.  This is the
        /// set of terms that the item will be indexed on in the
        /// query hash table.
        /// </summary>
        /// <value>The keywords.</value>
        public abstract IEnumerable<string> Keywords { get;}


        /// <summary>
        /// Does this match the specified query?
        /// </summary>
        /// <param name="query">The query.</param>
        /// <param name="stack">The stack.</param>
        /// <returns>
        /// True iff this matches the specified query.
        /// </returns>
        public virtual QueryResult Matches(Query query, Stack stack)
        {
            if (query.QueryQualifiers.Count < 1)
                return null;

            try
            {
                XPathQuery actualQuery = new XPathQuery(query.QueryQualifiers[0]);

                if (XmlDocumentFragment.SelectSingleNode(actualQuery.XPathExpression) != null)
                {
                    return Publisher.CreateQueryResult(stack, query, m_xmlObject.ToMessage());
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Trace.WriteLine(
                    String.Format("Unexpected query qualifier. ({0})", ex.Message));
            }
            return null;
        }

        #endregion
    }
}
