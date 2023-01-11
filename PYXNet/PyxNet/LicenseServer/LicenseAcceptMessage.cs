/******************************************************************************
LicenseAcceptMessage.cs

begin      : November 11, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.LicenseServer
{
    public class LicenseAcceptMessage : ITransmissible
    {
        public const string MessageID = "LSae";


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


        public LicenseAcceptMessage()
        {
            TransactionId = "";
            Note = "";
        }

        public LicenseAcceptMessage(Message message)
        {
            FromMessage(message);
        }

        public void FromMessage(Message message)
        {
            if (message == null)
            {
                throw new System.ArgumentException("\"License Server license accept\" message is null.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            reader.AssertAtEnd( "Extra data in a \"License Server license accept\" message.");
        }

        /// <summary>
        /// Build a PyxNet message that contains the license accept message.
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
