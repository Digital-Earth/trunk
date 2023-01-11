using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Logging
{
    /// <summary>
    /// Implementation of a LoggingService.
    /// </summary>
    public class LoggingService : Service.ServiceBase
    {
        #region ServiceIds

        /// <summary>
        /// The globally known identity of a logging service.
        /// </summary>
        public static readonly Service.ServiceId LoggingServiceId =
            new PyxNet.Service.ServiceId(
                new Guid("{BE765BEE-7E7B-4671-98E8-C87D92A5567A}"));

        #endregion ServiceIds

        #region Properties


        #endregion /* Properties */

        #region Construction

        /// <summary>
        /// Constructor.  Creates a server attached to the given stack.
        /// This can block for a while.
        /// </summary>
        /// <param name="stack"></param>
        public LoggingService(Stack stack) :
            base(stack, LoggingServiceId)
        {
            if (null == stack)
            {
                throw new ArgumentNullException("stack");
            }

            Tracer.DebugWriteLine("Constructing on stack {0}.", stack.ToString());

            // Register the message handler.
            this.Stack.SignedStreamRegisterHandler(LoggedEventMessage.MessageID,
                HandleLoggedMessage);

            // Check to see if there is a valid certificate in the stack's 
            //  certificate repository.  If there is, then we are authorized,
            //  and the stack will automatically publish that certificate (so
            //  other nodes can find us.)
            Tracer.DebugWriteLine("Checking validity of certificate.");
            if (!Certificate.Valid)
            {
                InvalidOperationException exception = new InvalidOperationException(
                    "This node is not authorized to act as a logging server.");
                Tracer.WriteLine("The certificate is invalid.  Throwing exception: {0}.", exception.ToString());
                throw exception;
            }
            Tracer.DebugWriteLine("The certificate is valid.  Construction complete.");
        }

        #endregion Construction

        #region LoggedEventReceived Event

        /// <summary> EventArgs for a LoggedEventReceived event. </summary>    
        public class LoggedEventReceivedEventArgs : EventArgs
        {
            private readonly NodeId m_sender;

            public NodeId Sender
            {
                get { return m_sender; }
            }

            private readonly LoggedEventMessage m_loggedEvent;

            public LoggedEventMessage LoggedEvent
            {
                get { return m_loggedEvent; }
            }

            internal LoggedEventReceivedEventArgs(NodeId sender, LoggedEventMessage loggedEvent)
            {
                m_sender = sender;
                m_loggedEvent = loggedEvent;
            }
        }

        /// <summary> Event handler for CertificateRequestReceived. </summary>
        public event EventHandler<LoggedEventReceivedEventArgs> LoggedEventReceived
        {
            add
            {
                m_LoggedEventReceived.Add(value);
            }
            remove
            {
                m_LoggedEventReceived.Remove(value);
            }
        }
        private Pyxis.Utilities.EventHelper<LoggedEventReceivedEventArgs> m_LoggedEventReceived = new Pyxis.Utilities.EventHelper<LoggedEventReceivedEventArgs>();

        /// <summary>
        /// Raises the LoggedEventReceived event.
        /// </summary>
        /// <param name="loggedEvent">The logged event.</param>
        public void OnLoggedEventReceived(NodeId sender, LoggedEventMessage loggedEvent)
        {
            m_LoggedEventReceived.Invoke(this, new LoggedEventReceivedEventArgs(sender, loggedEvent));
        }

        #endregion LoggedEventReceived Event

        private class PublishedServiceId : Publishing.Publisher.IPublishedItemInfo
        {
            private readonly PyxNet.Service.ServiceId m_serviceId;

            /// <summary>
            /// Gets the service id.
            /// </summary>
            /// <value>The service id.</value>
            public PyxNet.Service.ServiceId ServiceId
            {
                get { return m_serviceId; }
            }

            /// <summary>
            /// Initializes a new instance of the <see cref="PublishedServiceId"/> class.
            /// </summary>
            /// <param name="serviceId">The service id.</param>
            public PublishedServiceId(PyxNet.Service.ServiceId serviceId)
            {
                m_serviceId = serviceId;
            }

            #region IPublishedItemInfo Members

            /// <summary>
            /// Gets the keywords for this published item.  This is the
            /// set of terms that the item will be indexed on in the
            /// query hash table.
            /// </summary>
            /// <value>The keywords.</value>
            public IEnumerable<string> Keywords
            {
                get
                {
                    yield return ServiceId.ToSearchString();
                }
            }

            /// <summary>
            /// Does this match the specified query?
            /// </summary>
            /// <param name="query">The query.</param>
            /// <param name="stack">The stack.</param>
            /// <returns>
            /// True iff this matches the specified query.
            /// </returns>
            public QueryResult Matches(Query query, Stack stack)
            {
                foreach (string keyword in Keywords)
                {
                    if (query.Contents.Contains(keyword))
                    {
                        Message result = new Message("SIdX");
                        ServiceId.ToMessage(result);
                        return Publishing.Publisher.CreateQueryResult(stack, query, result);
                    }
                }
                return null;
            }

            #endregion

            #region Helper Functions (Statics)
            /// <summary>
            /// Sends the message to logging service.
            /// </summary>
            /// <param name="stack">The stack.</param>
            /// <param name="topic">The topic.</param>
            /// <param name="message">The message.</param>
            /// <returns></returns>
            public static bool SendMessageToLoggingService(PyxNet.Stack stack, string topic, string message)
            {
                Service.ServiceFinder finder = new PyxNet.Service.ServiceFinder(stack);
                NodeInfo nodeInfo;
                PyxNet.Service.ServiceInstance loggingService =
                    finder.FindService(LoggingServiceId, TimeSpan.FromSeconds(5), out nodeInfo);
                if (loggingService != null)
                {
                    StackConnection connection = stack.GetConnection(nodeInfo, false, TimeSpan.Zero);
                    if (connection != null)
                    {
                        return connection.SendMessage(new LoggedEventMessage(topic, message).ToMessage());
                    }
                }
                // TODO: Save the outgoing message to a local file, and send it when we next get online.
                return false;
            }

            #endregion Helper Functions (Statics)
        }


        /// <summary>
        /// Processes a publish request, by first validating it, then passing 
        /// it on to a virtual function for actual processing.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void HandleLoggedMessage(object sender, MessageHandlerCollection.MessageReceivedEventArgs args)
        {
            // TODO: We don't handle forwarded log messages yet.
            LoggedEventMessage message = LoggedEventMessage.FromMessage(args.Message);
            OnLoggedEventReceived(args.Context.Sender.RemoteNodeInfo.NodeId, message);
        }
    }
}