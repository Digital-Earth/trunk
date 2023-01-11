using System;
using System.Collections.Generic;

namespace PyxNet
{
    /// <summary>
    /// Encapsulates a PYXNet message relay acknowledgement.
    /// </summary>
    public class MessageRelayAcknowledgement : ProgressiveBroadcastAcknowledgement
    {
        /// <summary>
        /// The Message ID.
        /// </summary>
        public const string MessageID = "RAck";

        #region Fields and properties

        /// <summary>
        /// The GUID of the relay that is being acknowledged.
        /// </summary>
        private System.Guid m_relayGuid;

        /// <summary>
        /// The GUID of the relay that is being acknowledged.
        /// </summary>
        public System.Guid RelayGuid
        {
            get
            {
                return m_relayGuid;
            }
        }

        /// <summary>
        /// Storage for the unique ID which identifies the "to" node
        /// of the original message relay.
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
        /// This is the constructor to use if the node was found.
        /// </summary>
        /// <param name="toNodeGuid">The guid of the destination node.</param>
        public MessageRelayAcknowledgement(System.Guid relayGuid, System.Guid toNodeGuid) :
            base(new List<NodeInfo>(), new List<NodeInfo>())
        {
            if (relayGuid.Equals(Guid.Empty))
            {
                throw new ArgumentOutOfRangeException("Guid can't be empty.");
            }

            if (toNodeGuid.Equals(Guid.Empty))
            {
                throw new ArgumentOutOfRangeException("Guid can't be empty.");
            }

            // Set the relay guid.
            m_relayGuid = relayGuid;

            // Set the destination guid.
            m_toNodeGuid = toNodeGuid;
        }

        /// <summary>
        /// This is the constructor to use if the node wasn't found.
        /// </summary>
        /// <param name="toNodeGuid">The guid of the destination node.</param>
        /// <param name="candidateHubs">Some hubs that haven't been visited.</param>
        public MessageRelayAcknowledgement(System.Guid relayGuid, System.Guid toNodeGuid,
            List<NodeInfo> visitedHubs, List<NodeInfo> candidateHubs) :
            base(visitedHubs, candidateHubs)
        {
            if (relayGuid.Equals(Guid.Empty))
            {
                throw new ArgumentOutOfRangeException("Guid can't be empty.");
            }

            if (toNodeGuid.Equals(Guid.Empty))
            {
                throw new ArgumentOutOfRangeException("Guid can't be empty.");
            }

            // Set the relay guid.
            m_relayGuid = relayGuid;

            // Set the destination guid.
            m_toNodeGuid = toNodeGuid;

            // The node wasn't found.
            base.IsDeadEnd = true;
        }

        /// <summary>
        /// Construct from a message.
        /// </summary>
        /// <param name="reader">
        /// The message reader to construct from.
        /// This doesn't include the message header.
        /// </param>
        public MessageRelayAcknowledgement(MessageReader reader) :
            base(reader)
        {
            m_relayGuid = reader.ExtractGuid();
            m_toNodeGuid = reader.ExtractGuid();
        }

        #endregion

        #region Convert to/from message

        /// <summary>
        /// Build a message that contains the acknowledgement.
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
            message.Append(m_relayGuid);
            message.Append(m_toNodeGuid);
        }

        /// <summary>
        /// Initialize the members from a message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        public static MessageRelayAcknowledgement FromMessage(Message message)
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
            MessageRelayAcknowledgement result = new MessageRelayAcknowledgement(reader);
            reader.AssertAtEnd( "Extra data in message.");
            return result;
        }

        #endregion
    }
}