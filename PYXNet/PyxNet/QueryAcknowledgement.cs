using System;
using System.Collections.Generic;

namespace PyxNet
{
    /// <summary>
    /// Encapsulates a PYXNet query acknowledgement.
    /// </summary>
    public class QueryAcknowledgement : ProgressiveBroadcastAcknowledgement
    {
        /// <summary>
        /// The Message ID.
        /// </summary>
        public const string MessageID = StackConnection.QueryAcknowledgementMessageID;

        #region Fields and properties

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

        #endregion

        #region Constructors

        /// <summary>
        /// Default constructor.
        /// </summary>
        /// <param name="queryGuid">The guid of the query.</param>
        /// <param name="visitedHubs">The hubs already visited.</param>
        /// <param name="candidateHubs">Some hubs that haven't been visited.</param>
        public QueryAcknowledgement(System.Guid queryGuid,
            List<NodeInfo> visitedHubs, List<NodeInfo> candidateHubs) :
            base(visitedHubs, candidateHubs)
        {
            // Set the query guid.
            m_queryGuid = queryGuid;
        }

        /// <summary>
        /// Construct from a message.
        /// </summary>
        /// <param name="reader">
        /// The message reader to construct from.
        /// This doesn't include the message header.
        /// </param>
        public QueryAcknowledgement(MessageReader reader) : 
            base(reader)
        {
            m_queryGuid = reader.ExtractGuid();
        }

        #endregion

        #region Convert to/from message

        /// <summary>
        /// Build a message that contains the query acknowledgement.
        /// </summary>
        /// <returns>The resulting message.</returns>
        public override Message ToMessage()
        {
            Message message = new Message(MessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append the query acknowledgement to an existing message.
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to (will be modified).</param>
        /// <returns></returns>
        public override void ToMessage(Message message)
        {
            if (null == message)
            {
                throw new System.ArgumentNullException("message");
            }

            base.ToMessage(message);
            message.Append(m_queryGuid);
        }

        /// <summary>
        /// Initialize the members from a message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        public static QueryAcknowledgement FromMessage(Message message)
        {
            if (null == message)
            {
                throw new System.ArgumentNullException("message");
            }
            if (message.Identifier != MessageID)
            {
                throw new System.ArgumentException(
                    String.Format("Incorrect message type: {0} instead of {1}.", message.Identifier, MessageID));
            }

            MessageReader reader = new MessageReader(message);
            QueryAcknowledgement result = new QueryAcknowledgement(reader);
            reader.AssertAtEnd( "Extra data in message.");
            return result;
        }

        #endregion
    }
}
