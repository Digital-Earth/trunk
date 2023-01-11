/******************************************************************************
StartPublishingMessage.cs

begin      : February 8, 2010
copyright  : (c) 2010 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.LicenseServer
{
    /// <summary>
    /// Message sent from worldview to license server to start publication process.
    /// Sent after all the licensing and hosting issues have been resolved.
    /// From worldview's perspective, this completes the publication process.
    /// </summary>
    public class StartPublishingMessage : ITransmissible
    {
        public const string MessageID = "LSsp";

        public string TransactionId
        {
            get;
            set;
        }

        public string Note
        {
            get;
            set;
        }

        public StartPublishingMessage()
        {
            TransactionId = "";
            Note = "";
        }

        public StartPublishingMessage(Message message)
        {
            FromMessage(message);
        }

        public void FromMessage(Message message)
        {
            if (message == null)
            {
                throw new System.ArgumentException("\"License Server start publishing\" message is null.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            reader.AssertAtEnd( "Extra data in a \"License Server start publishing\" message.");
        }

        /// <summary>
        /// Build a PyxNet message that contains the start publishing message.
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(MessageID);
            ToMessage(message);
            return message;
        }

        #region Convert to/from message format

        /// <summary>
        /// </summary>
        /// <param name="message">The message to append to.(will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            message.Append(TransactionId);
            message.Append(Note);
        }

        /// <summary>
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            TransactionId = reader.ExtractUTF8();
            Note = reader.ExtractUTF8();
        }

        #endregion
    }
}
