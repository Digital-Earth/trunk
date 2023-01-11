using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Communications
{
    public delegate void MessageDelegate(object sender, IMessage message);

    public interface IMessage
    {
        int MessageType { get; set;}
        byte[] Contents { get; set;}
    }
}
