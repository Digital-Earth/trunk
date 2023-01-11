using System;
using System.Collections.Generic;

namespace PyxNet
{
    /// <summary>
    /// Encapsulates a PYXNet query.  
    /// </summary>
    public class Query : ITransmissible
    {
        #region Fields and properties

        #region Tracer

        /// <summary>
        /// This is the trace tool that one should use for all things to do with this object.
        /// </summary>
        private Pyxis.Utilities.NumberedTraceTool<Query> m_tracer
            = new Pyxis.Utilities.NumberedTraceTool<Query>(Pyxis.Utilities.TraceTool.GlobalTraceLogEnabled);

        public Pyxis.Utilities.NumberedTraceTool<Query> Tracer
        {
            get
            {
                return m_tracer;
            }
        }

        #endregion

        #region Origin Node

        /// <summary>
        /// The info about the origin node.
        /// </summary>
        private NodeInfo m_originNode;

        /// <summary>
        /// The info about the origin node.
        /// </summary>
        public NodeInfo OriginNode
        {
            get
            {
                return m_originNode;
            }
        }

        #endregion

        #region Guid

        /// <summary>
        /// The GUID of the query.
        /// </summary>
        private System.Guid m_guid;

        /// <summary>
        /// The GUID of the query.
        /// </summary>
        public System.Guid Guid
        {
            get
            {
                return m_guid;
            }
        }

        #endregion

        #region Hop count

        /// <summary>
        /// The number of times that the query has been forwarded.
        /// </summary>
        private ushort m_hopCount = 0;

        /// <summary>
        /// The number of times that the query has been forwarded.
        /// </summary>
        public ushort HopCount
        {
            get
            {
                return m_hopCount;
            }
        }

        #endregion

        #region Contents

        /// <summary>
        /// The string contents of the query.
        /// </summary>
        private string m_contents;

        /// <summary>
        /// The string contents of the query.
        /// </summary>
        public string Contents
        {
            get
            {
                return m_contents;
            }
        }

        /// <summary>
        /// Storage for QueryQualifiers
        /// </summary>
        private List<Message> m_queryQualifiers = new List<Message>();

        /// <summary>
        /// A list of messages that contain extra infomation to query on.
        /// Publishers are responsible for recognizing the types of messages
        /// that they can handle.  Matching a query based on QueryQualifiers
        /// wil depend on the specific qualifier.  For instance, a file search
        /// will add a qualifier that has a file length and hash code.  This will
        /// add matches for files that have the same length and hash code even though
        /// they do NOT satisfy the string contents.  However, a geo query that
        /// adds a geometry qualifier would need to match both the geometry and
        /// the string contents to return a positive query result.
        /// 
        /// If a publisher sees a query qualifier that it doesn't understand,
        /// then it should treat that like it matches.
        /// 
        /// As new query qualifier message types are added to the set of messages
        /// embedded in a query consideration should be given to how the PASS/FAIL
        /// filter works in the query distribution.  The default behaviour for the
        /// distribution of queries through the system will be: if there is a 
        /// QueryQualifier that the query distribution system does not understand,
        /// then the query will be passed on to all nodes.
        /// 
        /// TODO: make this list an object that contains the message data and information
        /// about how to treat this qualifier.  For instance, when searching for a coverage
        /// you should pass the query on only if it passes the QHT.  Whereas, the file length
        /// and hash code search needs to opposite behaviour.  If we build this information into
        /// the QueryQualifiers list, then the distribution mechanism would be able to handle
        /// future messages without code changes.
        /// </summary>
        public List<Message> QueryQualifiers
        {
            get { return m_queryQualifiers; }
        }

        #endregion

        #endregion

        #region Constructors

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <remarks>
        /// TODO: The originNode could be set by the Querier when the query gets sent.
        /// </remarks>
        /// <param name="originNode">The info about the origin node.</param>
        /// <param name="contents">The string contents of the query.</param>
        public Query(NodeInfo originNode, string contents)
        {
        	if (null == originNode)
        	{
                throw new System.ArgumentNullException("originNode");
        	}
            if (null == contents)
            {
                throw new System.ArgumentNullException("contents");
            }

            m_originNode = originNode;
            m_guid = Guid.NewGuid();
            m_contents = contents;
        }

        /// <summary>
        /// Construct from a message.
        /// </summary>
        /// <param name="message">The message to construct from.</param>
        public Query(Message message)
        {
            FromMessage(message);
        }

        /// <summary>
        /// Copy constructor.
        /// </summary>
        /// <param name="query">The query to copy.</param>
        public Query(Query query)
        {
            m_originNode = query.OriginNode;
            m_guid = query.Guid;
            m_hopCount = query.HopCount;
            m_contents = query.Contents;
            m_queryQualifiers = new List<Message>(query.QueryQualifiers);
        }

        #endregion

        #region Convert to/from message

        /// <summary>
        /// Build a message that contains the query.
        /// </summary>
        /// <returns>The resulting message.</returns>
        public Message ToMessage()
        {
            Message message = new Message(StackConnection.QueryMessageID);
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

            m_originNode.ToMessage(message);
            message.Append(m_guid);
            message.Append(m_hopCount);
            message.Append(m_contents);

            message.Append(QueryQualifiers.Count);
            for (int index = 0; index < QueryQualifiers.Count; ++index)
            {
                QueryQualifiers[index].ToMessage(message);
            }
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
            if (!message.StartsWith(StackConnection.QueryMessageID))
            {
                throw new System.ArgumentException("Message is not a query message.");
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

            m_originNode = new NodeInfo(reader);
            m_guid = reader.ExtractGuid();
            m_hopCount = reader.ExtractUInt16();
            m_contents = reader.ExtractUTF8();

            int qualifierCount = reader.ExtractInt();
            for (int count = 0; count < qualifierCount; ++count)
            {
                QueryQualifiers.Add(new Message(reader));
            }
        }

        #endregion

        #region Methods

        /// <summary>
        /// Return a new query with an incremented hop count.
        /// </summary>
        /// <returns>A new query with an incremented hop count.</returns>
        public Query GetHoppedQuery()
        {
            // Create a new query.
            Query hoppedQuery = new Query(this);

            // Increment the hop count.
            ++hoppedQuery.m_hopCount;

            // Return the new query.
            return hoppedQuery;
        }

        #endregion
    }
}