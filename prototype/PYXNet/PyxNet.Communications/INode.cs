using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Communications
{
    /// <summary>
    /// An interface for a generic communications "node".  Nodes can receive
    /// messages.
    /// </summary>
    /// <remarks>Consider adding OnConnect, OnClose, etc.</remarks>
    public interface INode
    {
        /// <summary>
        /// This node has received a message from another node.
        /// </summary>
        event MessageDelegate OnMessage;
        void RaiseMessage(IMessage message);
    }
}
