/******************************************************************************
RequestConfigMessage.cs

begin      : November 11, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.GeoStreamServer
{
    public class RequestConfigMessage : GeoStreamServerMessage
    {
        public const string MessageID = "GSrc";

        public override string GetMsgID()
        {
            return MessageID;
        }

        public override string GetMsgName()
        {
            return "GeoStream-RequestConfig";
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
        }
     
        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a UsageReportMessage.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public override void FromMessageReader(MessageReader reader)
        {
            base.FromMessageReader(reader);
        }

        #endregion
    }
}
