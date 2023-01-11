using System;

namespace PyxNet
{
    public class MessageRelay : ITransmissible
    {
        /// <summary>
        /// The Message ID.
        /// </summary>
        public const string MessageID = "RLay";

        #region Guid

        /// <summary>
        /// The GUID of the relay.
        /// </summary>
        private System.Guid m_guid;

        /// <summary>
        /// The GUID of the relay.
        /// </summary>
        public System.Guid Guid
        {
            get
            {
                return m_guid;
            }
        }

        #endregion

        #region Relayed Message

        private Message m_relayedMessage;

        public Message RelayedMessage
        {
            get
            {
                return m_relayedMessage;
            }
        }

        #endregion

        #region To Node Guid

        /// <summary>
        /// Storage for the unique ID which identifies the "to" node.
        /// </summary>
        private Guid m_toNodeGuid;

        public Guid ToNodeGuid
        {
            get
            {
                return m_toNodeGuid;
            }
        }

        #endregion

        #region Constructors

        /// <summary>
        /// Construct a message relay message.
        /// </summary>
        /// <param name="relayedMessage">The message to relay.</param>
        /// <param name="toNodeGuid">The guid of the node to relay to.</param>
        public MessageRelay(Message relayedMessage, Guid toNodeGuid)
        {
            if (null == relayedMessage)
            {
                throw new ArgumentNullException("relayedMessage");
            }
            if (toNodeGuid.Equals(Guid.Empty))
            {
                throw new ArgumentOutOfRangeException("The destination guid cannot be empty.");
            }

            m_guid = Guid.NewGuid();
            m_relayedMessage = relayedMessage;
            m_toNodeGuid = toNodeGuid;
        }

        /// <summary>
        /// Construct from a message.
        /// </summary>
        public MessageRelay(Message message)
        {
            FromMessage(message);
        }

        #endregion Constructors

        #region Convert to/from message format

        /// <summary>
        /// Build a PyxNet message that represents the object.
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(MessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Serialize the object to an existing message.  
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

            message.Append(m_guid);
            m_relayedMessage.ToMessage(message);
            message.Append(m_toNodeGuid);
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
                    String.Format("The message type {0} is incorrect.", message.Identifier));
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            reader.AssertAtEnd( "Extra data in message.");
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the beginning of the object data.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        internal void FromMessageReader(MessageReader reader)
        {
            if (reader == null)
            {
                throw new System.ArgumentException("Null message reader.");
            }

            m_guid = reader.ExtractGuid();
            m_relayedMessage = new Message(reader);
            m_toNodeGuid = reader.ExtractGuid();
        }

        #endregion Convert to/from message format
    }
}