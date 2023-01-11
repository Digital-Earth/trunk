/******************************************************************************
StatusMessage.cs

begin      : March 9, 2010
copyright  : (c) 2010 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
using System;

namespace PyxNet
{
    /// <summary>
    /// Encapsulates the information that we send and recieve in a 
    /// status message in PYXNet.  
    /// </summary>
    [Serializable]
    public class StatusMessage : ITransmissible
    {
        #region Properties

        public const string MessageId = "Stat";

        private string m_text = "";
        public string Text
        {
            get
            {
                return m_text;
            }
            set
            {
                m_text = value;
            }
        }

        #endregion

        #region Constructors

        /// <summary>
        /// Default constructor.
        /// </summary>
        public StatusMessage()
        {
        }

        /// <summary>
        /// Convenience constructor.
        /// </summary>
        /// <param name="text">The text.</param>
        public StatusMessage(String text)
        {
            Text = text;
        }

        /// <summary>
        /// Construct from a message.
        /// </summary>
        public StatusMessage(Message message)
        {
            this.FromMessage(message);
        }

        /// <summary>
        /// Construct from a message reader.
        /// </summary>
        public StatusMessage(MessageReader reader)
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

            message.Append(Text);
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
            FromMessageReader(reader);
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

            Text = reader.ExtractUTF8();
        }

        #endregion


        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString()
        {
            return Text;
        }
    }
}