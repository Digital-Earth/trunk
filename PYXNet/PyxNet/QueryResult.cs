/******************************************************************************
QueryResult.cs

begin      : 07/02/2007 6:11:35 AM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet
{
    public class QueryResult : ITransmissible
    {
        #region Fields and properties

        #region Query Guid

        /// <summary>
        /// The GUID of the query.
        /// </summary>
        private System.Guid m_queryGuid;

        /// <summary>
        /// The GUID of the query.
        /// </summary>
        public System.Guid QueryGuid
        {
            get
            {
                return m_queryGuid;
            }
        }

        #endregion Query Guid

        #region Query Origin Node

        /// <summary>
        /// The node that originated the query, and to which the query results message will be sent.
        /// </summary>
        public NodeInfo m_queryOriginNode;

        /// <summary>
        /// The node that originated the query, and to which the query results message will be sent.
        /// </summary>
        public NodeInfo QueryOriginNode
        {
            get
            {
                return m_queryOriginNode;
            }
        }

        #endregion Query Origin Node

        #region Result Node

        /// <summary>
        /// Storage for the node info of the node that matched the result.
        /// </summary>
        private NodeInfo m_resultNode;

        /// <summary>
        /// The node info of the node that matched the result.
        /// </summary>
        public NodeInfo ResultNode
        {
            get
            {
                return m_resultNode;
            }
        }

        #endregion Result Node

        #region Connected Node

        /// <summary>
        /// Storage for the node info of a hub that is directly connected to the node that matched the result.
        /// </summary>
        private NodeInfo m_connectedNode;

        /// <summary>
        /// The node info of a hub that is directly connected to the node that matched the result.
        /// </summary>
        public NodeInfo ConnectedNode
        {
            get
            {
                return m_connectedNode;
            }
        }

        #endregion Connected Node

        #region Data Size

        /// <summary>
        /// Storage for the size of the data set that matched the query.
        /// </summary>
        private long m_dataSize;

        /// <summary>
        /// The size of the data set that matched the query.
        /// </summary>
        public long DataSize
        {
            get { return m_dataSize; }
            set { m_dataSize = value; }
        }

        #endregion Data Size

        #region Hash Code Type

        /// <summary>
        /// Storage for the hash code type.
        /// </summary>
        private byte m_hashCodeType;

        // TODO:  Refactor the hash code type and data into its own class.

        /// <summary>
        /// The hash code type.
        /// </summary>
        public byte HashCodeType
        {
            get { return m_hashCodeType; }
            set { m_hashCodeType = value; }
        }

        #endregion Hash Code Type

        #region Hash Code

        /// <summary>
        /// Storage for the hash code of the data that matched the query.
        /// </summary>
        private byte[] m_hashCode;

        /// <summary>
        /// The hash code of the data that matched the query.
        /// </summary>
        public byte[] HashCode
        {
            get { return m_hashCode; }
            set { m_hashCode = value; }
        }

        #endregion Hash Code

        #region Matching Data Set ID

        /// <summary>
        /// Storage for the identity of the data set that this chunk belongs to.
        /// </summary>
        private DataHandling.DataGuid m_MatchingDataSetID = new DataHandling.DataGuid();

        /// <summary>
        /// The identity of the data set that this chunk belongs to.
        /// </summary>
        public DataHandling.DataGuid MatchingDataSetID
        {
            get { return m_MatchingDataSetID; }
            set { m_MatchingDataSetID = value; }
        }

        #endregion

        #region Matching Contents

        /// <summary>
        /// Storage for the string that matched the query string.
        /// </summary>
        private string m_matchingContents;

        /// <summary>
        /// The string that matched the query string.
        /// </summary>
        public string MatchingContents
        {
            get
            {
                return m_matchingContents;
            }
            set
            {
                m_matchingContents = value;
            }
        }

        #endregion Matching Contents

        #region Matching Description

        /// <summary>
        /// Storage for the matching description string property.
        /// </summary>
        private string m_matchingDescription;

        /// <summary>
        /// A description of what matched the query that should be useful to display on the querying
        /// node to help them decide if this is what they are looking for.
        /// </summary>
        public string MatchingDescription
        {
            get
            {
                return m_matchingDescription;
            }
            set
            {
                m_matchingDescription = value;
            }
        }

        #endregion Matching Description

        #region Extra Info

        /// <summary>
        /// Storage for the data extra info
        /// </summary>
        private Message m_extraInfo = new Message();

        /// <summary>
        /// Extended information about what the query matched.
        /// One use case would be the version and resolution of the 
        /// coverage that is being published.
        /// </summary>
        public Message ExtraInfo
        {
            get { return m_extraInfo; }
            set { m_extraInfo = value; }
        }

        #endregion Extra Info

        /// <summary>
        /// Gets or sets a value indicating whether this query result requires 
        /// a direct connection.  If true, then the message will only be passed
        /// directly to the target node.  If false, then Stack.SendQueryResult
        /// is allowed to forward the message.
        /// </summary>
        /// <value>
        /// 	<c>true</c> if [requires direct connection]; otherwise, <c>false</c>.
        /// </value>
        /// <remarks>
        /// This is used at the "server" node only, and is not transmitted.
        /// </remarks>
        public bool RequiresDirectConnection { get; set; }

        #endregion Fields and properties

        #region Conversion

        /// <summary>
        /// Display as the friendly name.
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return MatchingContents + ": " + MatchingDescription;
        }

        #endregion

        #region Constructors

        /// <summary>
        /// Constructor.
        /// </summary>
        public QueryResult(Guid queryGuid, NodeInfo queryOriginNode, 
            NodeInfo resultNode, NodeInfo connectedNode)
        {
            if (null == queryOriginNode)
            {
                throw new ArgumentNullException("queryOriginNode");
            }

            m_queryGuid = queryGuid;
            m_queryOriginNode = queryOriginNode;
            m_resultNode = resultNode;
            m_connectedNode = connectedNode;

            RequiresDirectConnection = true;
        }

        /// <summary>
        /// Construct from a message.
        /// </summary>
        /// <param name="message">The message to construct from.</param>
        public QueryResult(Message message)
        {
            FromMessage(message);
        }

        #endregion Constructors

        #region Convert to/from message

        /// <summary>
        /// Build a message that contains the query.
        /// </summary>
        /// <returns>The resulting message.</returns>
        public Message ToMessage()
        {
            Message message = new Message(Stack.QueryResultMessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append the query to an existing message.
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to (will be modified).</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            if (null == message)
            {
                throw new System.ArgumentNullException("message");
            }

            message.Append(m_queryGuid);
            m_queryOriginNode.ToMessage(message);
            message.Append(m_dataSize);
            message.Append(m_hashCodeType);

            if (null == m_hashCode)
            {
                message.Append(0);
            }
            else
            {
                message.Append(m_hashCode.Length);
                message.Append(m_hashCode);
            }

            m_resultNode.ToMessage(message);
#if INCLUDE_CONNECTED_NODE_IN_MESSAGE
            m_connectedNode.ToMessage(message);
#endif
            message.Append(m_matchingContents);
            message.Append(m_matchingDescription);
            MatchingDataSetID.ToMessage(message);
            ExtraInfo.ToMessage(message);
        }

        /// <summary>
        /// Initialize the members from a message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        public void FromMessage(Message message)
        {
            if (null == message)
            {
                throw new System.ArgumentNullException("message");
            }
            if (!message.StartsWith(Stack.QueryResultMessageID))
            {
                throw new System.ArgumentException("Message is not a query result message.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a query.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            if (null == reader)
            {
                throw new System.ArgumentNullException("reader");
            }

            m_queryGuid = reader.ExtractGuid();
            m_queryOriginNode = new NodeInfo(reader);
            m_dataSize = reader.ExtractInt64();
            m_hashCodeType = reader.ExtractByte();
            int hashCodeLength = reader.ExtractInt();
            m_hashCode = reader.ExtractBytes(hashCodeLength);

            m_resultNode = new NodeInfo(reader);
#if INCLUDE_CONNECTED_NODE_IN_MESSAGE
            m_connectedNode = new NodeInfo(reader);
#endif
            m_matchingContents = reader.ExtractUTF8();
            m_matchingDescription = reader.ExtractUTF8();
            MatchingDataSetID.FromMessageReader(reader);
            ExtraInfo = new Message(reader);
        }

        #endregion Convert to/from message

        /// <summary>
        /// Utility method to return if the MD5 from a file found by the querier
        /// matches the MD5 of the file we are searching for (in String format).
        /// </summary>
        /// <param name="md5">The MD5 of the file we are searching for.</param>
        /// <returns></returns>
        public bool MatchesMD5String(String md5)
        {
            String foundFileMD5 = FileHash.FileHashGenerator.MD5ToHex(HashCode);
            return foundFileMD5.Equals(md5);
        }

        /// <summary>
        /// Determines whether the result is a match for the provided criteria.
        /// </summary>
        /// <param name="md5ChecksumToMatch">The file checksum that must match in order for the result to be a match.</param>
        /// <returns>True if the result is a match for the criteria passed in.</returns>
        public bool MatchesMD5Checksum(byte[] md5ChecksumToMatch)
        {
            // A publisher can only be a result with a matching checksum.
            if (HashCodeType == 1)
            {
                if (HashCode.Length != md5ChecksumToMatch.Length)
                {
                    return false;
                }

                for (int index = HashCode.Length - 1; index >= 0; --index)
                {
                    if (HashCode[index] != md5ChecksumToMatch[index])
                    {
                        return false;
                    }
                }

                return true;
            }

            return false;
        }

        /// <summary>
        /// The lower-case matchable text string for queries, based on 
        /// MatchingDescription and MatchingContents.
		/// Reconstructs every time; this is likely more efficient than caching considering usage.
        /// </summary>
        private String LowerCaseMatchableText
        {
            get
            {
                StringBuilder matchableTextBuilder = new StringBuilder();
                if (MatchingDescription != null)
                {
                    matchableTextBuilder.Append(MatchingDescription);
                }
                if (MatchingContents != null)
                {
                    matchableTextBuilder.Append(MatchingContents);
                }

                return matchableTextBuilder.ToString().ToLower();
            }
        }

        /// <summary>
        /// Does the query match all of the given search tokens.
        /// </summary>
        /// <param name="m_searchTokens">The search tokens.</param>
        /// <returns></returns>
        public bool DoesQueryMatch(IList<String> m_searchTokens)
        {
            String lowerCaseMatchableText = LowerCaseMatchableText;
            foreach (String searchToken in m_searchTokens)
            {
                if (!lowerCaseMatchableText.Contains(searchToken.ToLower()))
                {
                    return false;
                }
            }

            return true;
        }
    }
}
