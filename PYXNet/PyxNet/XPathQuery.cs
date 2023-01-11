using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.XPath;

namespace PyxNet
{
    /// <summary>
    /// An "extension" to the Query class to add support for XPath Queries.
    /// </summary>
    public class XPathQuery: ITransmissible
    {
        private string m_xPathExpression;

        /// <summary>
        /// Gets or sets the text of the XPathExpression.
        /// </summary>
        /// <value>The text of the XPathExpression.</value>
        public string XPathExpression
        {
            get { return m_xPathExpression; }
            set { m_xPathExpression = value; }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="XPathQuery"/> class.
        /// </summary>
        /// <param name="node">The node that is sending the query.</param>
        /// <param name="expression">The XPath expression.</param>
        public XPathQuery(NodeInfo node, string expression)
        {
            m_nodeInfo = node;
            m_xPathExpression = expression;
        }

        /// <summary>
        /// Construct from a message.
        /// </summary>
        public XPathQuery(Message message)
        {
            this.FromMessage(message);
        }

        /// <summary>
        /// Construct from a message reader.
        /// </summary>
        public XPathQuery(MessageReader reader)
        {
            this.FromMessageReader(reader);
        }

        #region To/From Message

        /// <summary>
        /// The Message ID/Header.
        /// </summary>
        public const string XPathQueryMessageID = "XPQM";  //"XPathQueryMessage

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a XPathQuery.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            this.m_xPathExpression = reader.ExtractUTF8();
        }

        /// <summary>
        /// Helper function to read a XPathQuery from a Message.
        /// </summary>
        /// <param name="message"></param>
        private void FromMessage(Message message)
        {
            if (message == null || !message.StartsWith(XPathQueryMessageID))
            {
                throw new System.ArgumentException(
                    "Message is not a XPathQuery message.", "message");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            reader.AssertAtEnd( "Extra data in a XPathQuery Message.");
        }

        /// <summary>
        /// Build a PyxNet message that contains the XPathQuery.
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(XPathQueryMessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append the XPathQuery to an existing message.  
        /// This does not include any message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            if (message == null)
            {
                throw new System.ArgumentNullException("message");
            }

            message.Append( this.m_xPathExpression);
        }

        #endregion /* To/From Message */

        private readonly NodeInfo m_nodeInfo = null;

        /// <summary>
        /// Gets the node info for the node that created the query.
        /// </summary>
        /// <value>The node info.</value>
        public NodeInfo NodeInfo
        {
            get { return m_nodeInfo; }
        }

        /// <summary> Regex for quoted strings. </summary>
        private static System.Text.RegularExpressions.Regex s_extractQuotedStrings =
            new System.Text.RegularExpressions.Regex("[\\\'\\\"].*[\\\'\\\"]");

        /// <summary>
        /// Gets the contents of the query as a simple string.  This corresponds to
        /// the Query interface (simply the text that must be matched.)
        /// </summary>
        /// <returns></returns>
        private string GetContents()
        {
            // TODO: We don't handle multiple string expressions.
            foreach (System.Text.RegularExpressions.Match match in 
                s_extractQuotedStrings.Matches(this.XPathExpression))
            {
                // Return the quoted string, minus the quotes.
                return match.Value.Substring( 1, match.Value.Length - 2);
            }
            return "42";
        }

        /// <summary>
        /// Generates a query that wraps up this XPathQuery.
        /// </summary>
        /// <returns>A newly generated query.</returns>
        public Query ToQuery()
        {
            if (this.NodeInfo == null)
            {
                throw new InvalidOperationException(
                    "Unable to generate a new QueryMessage from this XPathQuery, " +
                    "because it was constructed from a message.");
            }
            Query q = new Query(m_nodeInfo, this.GetContents());
            q.QueryQualifiers.Add(this.ToMessage());
            return q;
        }
    }
}
