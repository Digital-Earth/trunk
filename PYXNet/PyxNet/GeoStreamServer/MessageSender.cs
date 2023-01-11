/******************************************************************************
SendMessage.cs

begin      : November 11, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.GeoStreamServer
{
    /// <summary>
    /// Class that encapsulates the PyxNet message sending mechanism.
    /// </summary>
    public class MessageSender : PyxNet.MessageSender
    {
        private string SessionID
        {
            get;
            set;
        }

        public MessageSender(PyxNet.Stack inStack, GeoStreamServerMessage inMsg, NodeId inDstNode) 
            : base(inStack,inMsg,inDstNode)
        {
            SessionID = inMsg.SessionID;
        }


        public GeoStreamServerMessage SendAndWait(int waitTime)
        {
            var timer = SessionSynchronization.CreateEvent(SessionID, waitTime);

            Send();

            timer.Wait();
            timer.Release();

            return timer.Msg;
        }
    }
}
