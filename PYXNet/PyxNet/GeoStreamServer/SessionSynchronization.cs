/******************************************************************************
SessionSynchronization.cs

begin      : November 11, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Web;
using System.Data;

namespace PyxNet.GeoStreamServer
{
    public class SessionSynchronization
    {
        public class EventNode : Pyxis.Utilities.SynchronizationEvent
        {
            public EventNode(string sessionId, int timeout)
                : base(TimeSpan.FromSeconds(timeout))
            {
                SessionId = sessionId;
                Msg = null;
            }

            public void Release()
            {
                eventList.Remove(this);
            }

            public string SessionId
            {
                get;
                set;
            }

            public GeoStreamServerMessage Msg
            {
                get;
                set;
            }
        }

        static List<EventNode> eventList = new List<EventNode>();

        static public EventNode CreateEvent(string sessionId, int timeout)
        {
            var node = new EventNode(sessionId, timeout);
            eventList.Add(node);
            return node;
        }

        static public void MessageReceived(GeoStreamServerMessage Msg)
        {
            foreach (EventNode item in eventList)
            {
                if (item.SessionId == Msg.SessionID)
                {
                    item.Msg = Msg;
                    item.Pulse();
                }
            }
        }
    }
}