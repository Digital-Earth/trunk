using System;

namespace PyxNet
{
    /// <summary>
    /// Encapsulates the information that we send and recieve in a 
    /// status message request in PYXNet.  This is a lightweight 
    /// message.
    /// </summary>
    [Serializable]
    public class StatusMessageRequest : ITransmissible
    {
        #region Properties

        public const string MessageId = "Sta?";

        #endregion

        #region Constructors

        /// <summary>
        /// Default constructor.
        /// </summary>
        public StatusMessageRequest()
        {
        }

        /// <summary>
        /// Construct from a message.
        /// </summary>
        public StatusMessageRequest(Message message)
        {
            this.FromMessage(message);
        }

        /// <summary>
        /// Construct from a message reader.
        /// </summary>
        public StatusMessageRequest(MessageReader reader)
        {
            this.FromMessageReader(reader);
        }

        #endregion

        #region Convert to/from message format

        /// <summary>
        /// Build a PyxNet message that contains the Node Info.
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(MessageId);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append the Node Info to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        public void ToMessage(Message message)
        {
            if (message == null)
            {
                throw new System.ArgumentNullException("message");
            }

            // This is a no-op.  There is no data!
        }

        /// <summary>
        /// Initialize the members from a PyxNet message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        public void FromMessage(Message message)
        {
            if (message == null || !message.StartsWith(MessageId))
            {
                throw new System.ArgumentException(
                    "Message is not a Node Info message.");
            }

            MessageReader reader = new MessageReader(message);

            // This is a no-op.  There is no data!

            reader.AssertAtEnd( "Extra data in a NodeInfo Message.");
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the mode for the Node Info.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            if (reader == null)
            {
                throw new System.ArgumentException(
                    "Null message reader.");
            }

            // This is a no-op.  There is no data!
        }

        #endregion
    }
}