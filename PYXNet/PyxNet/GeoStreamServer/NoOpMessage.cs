/******************************************************************************
NoOpMessage.cs

begin      : November 11, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.GeoStreamServer
{
    public class NoOpMessage : GeoStreamServerMessage
    {
        public const string MessageID = "GSno";

        public override string GetMsgID()
        {
            return MessageID;
        }

        public override string GetMsgName()
        {
            return "GeoStream-NoOp";
        }

        public string Note
        {
            get;
            set;
        }

        #region Convert to/from message format

        /// <summary>
        /// Append the UsageReportMessage to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.(will be modified)</param>
        /// <returns></returns>
        public override void ToMessage(Message message)
        {
            base.ToMessage(message);

            message.Append(Note);
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a UsageReportMessage.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public override void FromMessageReader(MessageReader reader)
        {
            base.FromMessageReader(reader);

            Note = reader.ExtractUTF8();
        }

        #endregion
    }
}
