using System;

namespace PyxNet
{
    /// <summary>
    /// The message sent to let the other side know what kind of connection this is.
    /// </summary>
    public class StackConnectionRequest : ITransmissible
    {
        /// <summary>
        /// The Message ID.
        /// </summary>
        public const string MessageID = "HiHo";

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

        #region To Node GUID

        /// <summary>
        /// Storage for the unique ID which identifies the "to" node.
        /// Note that this might be the empty guid, if connecting by address.
        /// </summary>
        private Guid m_toNodeGUID;

        /// <summary>
        /// The unique ID which identifies the "to" node.
        /// Note that this might be the empty guid, if connecting by address.
        /// </summary>
        public Guid ToNodeGUID
        {
            get { return m_toNodeGUID; }
            set { m_toNodeGUID = value; }
        }

        public bool IsToNodeGUID(Guid guid)
        {
            return m_toNodeGUID == Guid.Empty || m_toNodeGUID == guid;
        }

        #endregion ToNodeGUID

        #region Constructors

        public StackConnectionRequest(bool isPersistent, NodeInfo fromNodeInfo, 
            KnownHubList fromKnownHubList)
        {
            m_isPersistent = isPersistent;
            m_fromNodeInfo = fromNodeInfo;
            m_fromKnownHubList = fromKnownHubList;
        }

        public StackConnectionRequest(bool isPersistent, NodeInfo fromNodeInfo, 
            KnownHubList fromKnownHubList, Guid toNode)
            : this(isPersistent, fromNodeInfo, fromKnownHubList)
        {
            m_toNodeGUID = toNode;
        }

        /// <summary>
        /// Construct from a message.
        /// </summary>
        public StackConnectionRequest(Message message)
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
            message.Append(m_toNodeGUID);
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
            m_toNodeGUID = reader.ExtractGuid();
        }

        #endregion Convert to/from message format
    }
}