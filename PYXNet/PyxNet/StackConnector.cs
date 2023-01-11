using System;
using System.Collections.Generic;

namespace PyxNet
{
    /// <summary>
    /// A message to request a connectin be made to a specific node.  This
    /// message is used when trying to make a connection to a node and 
    /// you don't have the ability to connect to it.  This condition is
    /// most likely because of firewalls between you and the node you 
    /// want to connect to.  So, you can ask the other node to connect to
    /// you to try and get around the firewall setup.
    /// </summary>
    public class StackConnector : ITransmissible
    {
        /// <summary>
        /// The Message ID.
        /// </summary>
        public const string MessageID = "Conn";

        #region Fields and properties

        /// <summary>
        /// Storage for the node to connect to.
        /// </summary>
        private readonly NodeInfo m_toNode;

        /// <summary>
        /// The node to connect to.
        /// </summary>
        public NodeInfo ToNode
        {
            get
            {
                return m_toNode;
            }
        }

        /// <summary>
        /// Whether or not it is persistent.
        /// </summary>
        private readonly bool m_isPersistent;

        /// <summary>
        /// Whether or not it is persistent.
        /// </summary>
        public bool IsPersistent
        {
            get
            {
                return m_isPersistent;
            }
        }

        #endregion

        #region Constructors

        /// <summary>
        /// TODO: Comment
        /// </summary>
        /// <param name="toNode"></param>
        /// <param name="isPersistent"></param>
        public StackConnector(NodeInfo toNode, bool isPersistent)
        {
            if (null == toNode)
            {
                throw new ArgumentNullException("toNode");
            }

            // Set the destination node.
            m_toNode = toNode;

            // Set the persistence of the connection.
            m_isPersistent = isPersistent;
        }

        /// <summary>
        /// Construct from a message.
        /// </summary>
        /// <param name="reader">
        /// The message reader to construct from.
        /// This doesn't include the message header.
        /// </param>
        public StackConnector(MessageReader reader)
        {
            m_toNode = new NodeInfo(reader);
            m_isPersistent = reader.ExtractBool();
        }

        #endregion

        #region Convert to/from message

        /// <summary>
        /// Build a message that contains the acknowledgement.
        /// </summary>
        /// <returns>The resulting message.</returns>
        public Message ToMessage()
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
        public void ToMessage(Message message)
        {
            if (null == message)
            {
                throw new System.ArgumentNullException("message");
            }

            m_toNode.ToMessage(message);
            message.Append(m_isPersistent);
        }

        /// <summary>
        /// Initialize the members from a message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        public static StackConnector FromMessage(Message message)
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
            StackConnector result = new StackConnector(reader);
            reader.AssertAtEnd( "Extra data in message.");
            return result;
        }

        #endregion
    }
}