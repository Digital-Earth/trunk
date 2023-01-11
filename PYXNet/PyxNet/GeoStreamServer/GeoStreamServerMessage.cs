/******************************************************************************
GeoStreamServerMessage.cs

begin      : November 1, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.GeoStreamServer
{
    public abstract class GeoStreamServerMessage : ITransmissible
    {
        public abstract string GetMsgID();
        public abstract string GetMsgName();

        /// <summary>
        /// Construct an empty RequestConfigMessage.
        /// </summary>
        public GeoStreamServerMessage()
        {
        }

        /// <summary>
        /// Construct a UsageReporstMessage from a message.   
        /// The message must be a PyxNet UsageReportsMessage message.
        /// </summary>
        /// <param name="message"></param>
        public GeoStreamServerMessage(Message message)
        {
            FromMessage(message);
        }

        /// <summary>
        /// Gets or sets the DST node.
        /// Node where report is being sent.
        /// </summary>
        /// <value>The DST node.</value>
        public NodeId DstNode
        { 
            get; 
            set; 
        }

        /// <summary>
        /// Gets or sets the SRC node.
        /// Node where report originated.
        /// </summary>
        /// <value>The SRC node.</value>
        public NodeId SrcNode
        { 
            get; 
            set; 
        }

        public string SessionID
        {
            get;
            set;
        }

        public virtual Message ToMessage()
        {
            Message message = new Message(GetMsgID());
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.(will be modified)</param>
        /// <returns></returns>

        public virtual void ToMessage(Message message)
        {
            DstNode.ToMessage(message);
            SrcNode.ToMessage(message);
            message.Append(SessionID);
        }

        /// <summary>
        /// Initialize the members from a PyxNet message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        public void FromMessage(Message message)
        {
            if (message == null )
            {
                throw new System.ArgumentException("GeoStreamServer message is null.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            reader.AssertAtEnd( "Extra data in a GeoStreamServer message.");
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a UsageReportMessage.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public virtual void FromMessageReader(MessageReader reader)
        {
            DstNode = new NodeId(reader);
            SrcNode = new NodeId(reader);
            SessionID = reader.ExtractUTF8();
        }

        /// <summary>
        /// Prepares message for return trip.
        /// Flip source and destination nodes.
        /// </summary>
        public void PrepareForReturnTrip()
        {
            //--
            //-- flip src/dst to complete round trip
            //--
            NodeId tmpNode = DstNode;
            DstNode = SrcNode;
            SrcNode = tmpNode;
        }

        /// <summary>
        /// Send a message back.
        /// </summary>
        /// <param name="stack">Communication stack.</param>
        public void Send(PyxNet.Stack stack)
        {
            new PyxNet.GeoStreamServer.MessageSender(stack, this, this.DstNode).Send();
        }

        public GeoStreamServerMessage SendAndWait(PyxNet.Stack stack, int timeout)
        {
            return new PyxNet.GeoStreamServer.MessageSender(stack, this, this.DstNode).SendAndWait(timeout);
        }    
    
    }

}
