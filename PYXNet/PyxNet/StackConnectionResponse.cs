namespace PyxNet
{
    /// <summary>
    /// The connection request acknowledgement.
    /// </summary>
    public class StackConnectionResponse : ITransmissible
    {
        /// <summary>
        /// The Message ID.
        /// </summary>
        public const string MessageID = "HiGo";

        #region Is Persistent

        private bool m_isPersistent;

        public bool IsPersistent
        {
            get
            {
                return m_isPersistent;
            }
        }

        #endregion

        #region From Node Info

        private NodeInfo m_fromNodeInfo;

        public NodeInfo FromNodeInfo
        {
            get
            {
                return m_fromNodeInfo;
            }
        }

        #endregion

        #region From Known Hub List

        private KnownHubList m_fromKnownHubList;

        public KnownHubList FromKnownHubList
        {
            get
            {
                return m_fromKnownHubList;
            }
        }

        #endregion

        #region Error

        public enum ErrorType
        {
            None = 0,
            IncorrectNode,
            NodeNotPending,
            TimedOut,
            RequestNotSent,
            SameNode
        }

        private ErrorType m_error = ErrorType.None;

        public ErrorType Error
        {
            get
            {
                return m_error;
            }
            set
            {
                m_error = value;
            }
        }

        #endregion

        #region Constructors

        public StackConnectionResponse(bool isPersistent, NodeInfo fromNodeInfo, KnownHubList fromKnownHubList)
        {
            m_isPersistent = isPersistent;
            m_fromNodeInfo = fromNodeInfo;
            m_fromKnownHubList = fromKnownHubList;
        }

        /// <summary>
        /// Construct from a message.
        /// </summary>
        public StackConnectionResponse(Message message)
        {
            FromMessage(message);
        }

        #endregion Constructors

        #region Convert to/from message format

        /// <summary>
        /// Build a PyxNet message that contains the connection info.
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(MessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append the connection info to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns>The </returns>
        public void ToMessage(Message message)
        {
            if (message == null)
            {
                throw new System.ArgumentNullException("message");
            }

            message.Append(m_isPersistent);
            m_fromNodeInfo.ToMessage(message);
            m_fromKnownHubList.ToMessage(message);
            message.Append((int)m_error);
        }

        /// <summary>
        /// Initialize the members from a PyxNet message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        internal void FromMessage(Message message)
        {
            if (message == null || message.Identifier != MessageID)
            {
                throw new System.ArgumentException(
                    "Message is not a Connection Info message.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            reader.AssertAtEnd( "Extra data in message.");
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the mode for the Connection Info.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        internal void FromMessageReader(MessageReader reader)
        {
            if (reader == null)
            {
                throw new System.ArgumentException(
                    "Null message reader.");
            }

            m_isPersistent = reader.ExtractBool();
            m_fromNodeInfo = new NodeInfo(reader);
            m_fromKnownHubList = new KnownHubList(reader);
            m_error = (ErrorType)reader.ExtractInt();
        }

        #endregion Convert to/from message format
    }
}
