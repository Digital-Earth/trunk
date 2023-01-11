using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Communications
{
    /// <summary>
    /// A DirectCommunicationsChannel defines a channel between a client and 
    /// server where both nodes are running in the same process space.  
    /// </summary>
    /// <remarks>
    /// Intended for testing.
    /// </remarks>
    public class DirectCommunicationsChannel : PyxNet.Communications.ICommunicationsChannel
    {
        public event MessageDelegate OnMessage;
        public void RaiseMessage(IMessage message)
        {
            OnMessage(m_BackChannel, message);
        }

        private INode m_Recipient;
        private DirectCommunicationsChannel m_BackChannel;

        /// <summary>
        /// Sends a message asynchronously. 
        /// </summary>
        /// <param name="message"></param>
        public void Send(IMessage message)
        {
            m_Recipient.RaiseMessage( message);
        }

        /// <summary>
        /// Send a message and wait for a response.  Synchronous.
        /// </summary>
        /// <remarks>We might want to add a "timeout".</remarks>
        /// <param name="message"></param>
        /// <returns>The response message.</returns>
        public IMessage SendAndReceive(IMessage message)
        {
            lock (this)
            {
                IMessage response = null;
                MessageDelegate captureResponse = 
                    delegate( object sender, IMessage msg) {
                        response = msg;};
                m_BackChannel.OnMessage += captureResponse;
                m_Recipient.RaiseMessage( message);
                // Wait for response.... forever.
                while (response == null)
                {
                    ;
                }
                m_BackChannel.OnMessage -= captureResponse;
                return response;
            }
        }

        private DirectCommunicationsChannel( 
            INode recipient, INode self, DirectCommunicationsChannel backChannel)
        {
            m_Recipient = recipient;
            m_BackChannel = backChannel;
            OnMessage += delegate( object sender, IMessage message) {
                self.RaiseMessage( message);};
        }

        public DirectCommunicationsChannel(INode client, INode server)
        {
            m_Recipient = server;
            m_BackChannel = new DirectCommunicationsChannel(client, server, this);            
        }
    }
}
