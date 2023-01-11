using System;
namespace PyxNet.Communications
{
    /// <summary>
    /// An ICommunicationsChannel defines a channel between two nodes.
    /// </summary>
    interface ICommunicationsChannel
    {
        /// <summary>
        /// A message has been received on the channel.
        /// </summary>
        event MessageDelegate OnMessage;

        /// <summary>
        /// Sends a message asynchronously. 
        /// </summary>
        /// <param name="message"></param>
        void Send(IMessage message);

        /// <summary>
        /// Send a message and wait for a response.  Synchronous.
        /// </summary>
        /// <remarks>We might want to add a "timeout".</remarks>
        /// <param name="message"></param>
        /// <returns>The response message.</returns>
        IMessage SendAndReceive(IMessage message);
    }
}
