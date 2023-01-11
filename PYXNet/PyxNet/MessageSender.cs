/******************************************************************************
MessageSender.cs  

begin      : November 11, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet
{
    /// <summary>
    /// Class that encapsulates the PyxNet message sending mechanism.
    /// </summary>
    public class MessageSender
    {
        #region Private Properties

        protected ITransmissible Message
        { get; set; }

        protected PyxNet.Stack Stack
        { get; set; }

        protected NodeId Destination
        { get; set; }

        protected PyxNet.NodeInfo DestinationNodeInfo
        { get; set; }

        #endregion

        /// <summary>
        /// Gets or sets a value indicating whether this message requires 
        /// a direct connection.  If true, then the message will only be passed
        /// directly to the target node.  If false, then MessageSender.Send
        /// is allowed to forward the message.
        /// </summary>
        /// <value>
        /// 	<c>true</c> if [requires direct connection]; otherwise, <c>false</c>.
        /// </value>
        public bool RequiresDirectConnection { get; set; }

        /// <summary>
        /// Initializes a new instance of the <see cref="SendMessage"/> class.
        /// </summary>
        /// <param name="inStack">Communication stack.</param>
        /// <param name="inMsg">message.</param>
        /// <param name="inDstNode">Node Guid for destination.</param>
        public MessageSender(PyxNet.Stack inStack, ITransmissible inMsg, NodeId inDstNode)
        {
            Message = inMsg;
            Stack = inStack;
            Destination = inDstNode;
            DestinationNodeInfo = null;
            RequiresDirectConnection = false;
        }

        public MessageSender(PyxNet.Stack inStack, ITransmissible inMsg, PyxNet.NodeInfo inDstNodeInfo )
			: this(inStack,inMsg,inDstNodeInfo.NodeId)
        {
            DestinationNodeInfo = inDstNodeInfo;
        }

        /// <summary>
        /// Mechanism to send message.
        /// </summary>
        public void Send()
        {
#if DEBUG
            // When sending messages, the client may have constructed a faulty 
            // (or incomplete) message.  Test this here before sending the 
            // operation on to the background thread.
            try
            {
                Message m = this.Message.ToMessage();
            }
            catch (Exception ex)
            {
                Stack.Tracer.ForcedWriteLine("Error in message.  Send will fail.  {0}", ex.ToString());
                //throw;
            }
#endif
            System.Threading.ThreadPool.QueueUserWorkItem(delegate
            {
                try
                {
                    if (DestinationNodeInfo == null)
                    {
                        DestinationNodeInfo = PyxNet.NodeInfo.Find(Stack, Destination, TimeSpan.FromSeconds(30));
                        if (DestinationNodeInfo == null)
                        {
                            Stack.Tracer.ForcedWriteLine("Failed to find node {0}.  MessageSender failed.",
                                Destination.ToString());
                            return;
                        }
                    }

                    PyxNet.StackConnection connection = Stack.FindConnection(DestinationNodeInfo, false);
                    if ((connection == null) && !this.RequiresDirectConnection)
                    {
                        // TODO: We should pass incomingConnection on to the relayer - this 
                        // is a good candidate for the relay.
                        Stack.Tracer.WriteLine("Forwarding message to {0}.", this.DestinationNodeInfo.ToString());
                        Stack.RelayMessage(this.DestinationNodeInfo, this.Message.ToMessage());
                        return;
                    }

                    if (connection == null)
                    {
                        connection = Stack.GetConnection(DestinationNodeInfo, false, TimeSpan.FromSeconds(15));
                    }

                    if (connection != null)
                    {
                        bool result = connection.SendMessage(Message.ToMessage());
                        Stack.Tracer.WriteLine("PyxNet.MessageSender: result={0}", result);
                    }
                    else
                    {
                        Stack.Tracer.WriteLine("PyxNet.MessageSender: No Connection, message not sent.");
                    }
                }
                catch (Exception ex)
                {
                    Stack.Tracer.WriteLine("PyxNet.MessageSender: Exception: {0}", ex.Message);
                }
            });
        }
    }
}
