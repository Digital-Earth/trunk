using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Logging
{
    /// <summary>
    /// The logging client class manages the background logging of messages.  If the 
    /// logging service cannot be reached, then the messages are stored locally, and
    /// we try to send them the next time we connect.
    /// </summary>
    public class LoggingClient
    {
        private static LoggingClient s_defaultClient = null;

        /// <summary>
        /// Gets the default client.
        /// </summary>
        /// <value>The default client.</value>
        public static LoggingClient DefaultClient
        {
            get
            {
                if (s_defaultClient == null)
                {
                    s_defaultClient = new LoggingClient(StackSingleton.Stack);
                }
                return s_defaultClient;
            }
        }

        private Stack m_stack;

        /// <summary>
        /// Initializes a new instance of the <see cref="LoggingClient"/> class.
        /// </summary>
        /// <param name="stack">The stack.</param>
        public LoggingClient(Stack stack)
        {
            m_stack = stack;
            Initialize();
        }

        /// <summary>
        /// Initializes this instance.
        /// </summary>
        private void Initialize()
        {
            m_loggingServerNodeInfo = null;
            m_stack.NetworkAddressChanged += 
                new EventHandler<Stack.NetworkAddressChangedEventArgs>(StackAddressChanged);
        }

        /// <summary>
        /// Handle the stack changing - it might have mapped a port.  
        /// </summary>
        /// <remarks>
        /// We don't disconnect this, so this object will live on for as long as the 
        /// associated stack does.  This would be bad if we made a lot of loggers.
        /// TODO: Correct this behaviour by implementing Dispose.
        /// </remarks>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="PyxNet.Stack.NetworkAddressChangedEventArgs"/> instance containing the event data.</param>
        void StackAddressChanged(object sender, Stack.NetworkAddressChangedEventArgs e)
        {
            m_loggingServerNodeInfo = null;
        }

        private NodeInfo m_loggingServerNodeInfo = null;
        private object m_loggingServerNodeInfoLock = new object();

        public NodeInfo LoggingServerNodeInfo
        {
            get 
            {
                lock (m_loggingServerNodeInfoLock)
                {
                    if (m_loggingServerNodeInfo == null)
                    {
                        Service.ServiceFinder finder = new PyxNet.Service.ServiceFinder(m_stack);
                        PyxNet.Service.ServiceInstance loggingService =
                            finder.FindService(LoggingService.LoggingServiceId, TimeSpan.FromSeconds(5), out m_loggingServerNodeInfo);
                    }
                }
                return m_loggingServerNodeInfo; 
            }
            set { m_loggingServerNodeInfo = value; }
        }

        /// <summary>
        /// Sends the message to logging service.
        /// </summary>
        /// <param name="topic">The topic.</param>
        /// <param name="messageText">The message text.</param>
        /// <returns></returns>
        public bool Send( string topic, string messageText)
        {
            Message message = new LoggedEventMessage(topic, messageText).ToMessage();

            NodeInfo loggingService = this.LoggingServerNodeInfo; 
            if (loggingService != null)
            {
                StackConnection connection = m_stack.GetConnection(loggingService, false, TimeSpan.Zero);
                if (connection != null)
                {
                    m_stack.Tracer.WriteLine("Sending message to remote log: {0}: {1}",
                        topic, messageText);
                    m_stack.SendSignedMessage(connection, message);
                    return true;
                }
            }

            // TODO: Save the outgoing message to a local file, and send it when we next get online.
            m_stack.Tracer.WriteLine("Error sending message to remote log: {0}: {1}",
                topic, messageText);

            return false;
        }
    }
}
