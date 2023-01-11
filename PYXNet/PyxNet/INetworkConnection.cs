namespace PyxNet
{
    /// <summary>
    /// Delegate type for handling new messages being received.
    /// </summary>
    /// <param name="sender">The sender of the message.</param>
    /// <param name="message">The message the came in.</param>
    public delegate void MessageHandler(object sender, Message message);

    /// <summary>
    /// Delegate type for handling a closed connection.
    /// </summary>
    /// <param name="sender">The sender of the event, usually the connection that is being closed.</param>
    /// <param name="connection">The connection that just closed.</param>
    public delegate void ClosedConnectionHandler(object sender, INetworkConnection connection);

    /// <summary>
    /// This is just a dumb network connection that receives messages without 
    /// knowing what they mean.
    /// </summary>
    public interface INetworkConnection
    {
        Pyxis.Utilities.NumberedTraceTool<INetworkConnection> Tracer
        {
            get;
        }

        #region Remote Node Info

        /// <summary>
        /// The network address of the connected machine.
        /// </summary>
        NetworkAddress Address
        {
            get;
        }

        #endregion

        #region Message

        /// <summary>
        /// Send a message to the connected machine.
        /// </summary>
        /// <param name="message">The message to be sent.</param>
        bool SendMessage(Message message);

        /// <summary>
        /// Callback that fires when a new message is received.
        /// </summary>
        event MessageHandler OnMessage;

        #endregion

        #region Close

        /// <summary>
        /// Close this connection.
        /// </summary>
        void Close();

        /// <summary>
        /// Event that fires when this connection has been closed.
        /// </summary>
        event ClosedConnectionHandler OnClosed;

        /// <summary>
        /// Is the connection closed?
        /// </summary>
        bool IsClosed
        {
            get;
        }

        #endregion
    }

    /// <summary>
    /// A dummy implementation of the INetworkConnection that is used in tests for 
    /// constructing StackConnections that are not actually live.
    /// </summary>
    public class DummyConnection : INetworkConnection
    {
        #region INetworkConnection Members

        bool m_closed; // false by default

        Pyxis.Utilities.NumberedTraceTool<INetworkConnection> m_tracer 
            = new Pyxis.Utilities.NumberedTraceTool<INetworkConnection>(Pyxis.Utilities.TraceTool.GlobalTraceLogEnabled);

        NetworkAddress m_address = new NetworkAddress();

        public Pyxis.Utilities.NumberedTraceTool<INetworkConnection> Tracer
        {
            get { return m_tracer; }
        }

        public NetworkAddress Address
        {
            get { return m_address; }
        }

        public bool SendMessage(Message message)
        {
            return true;
        }

        private event MessageHandler m_onMessage
        {
            // dummy implementation to remove unused warning
            add {}
            remove {}
        }

        private object m_messageEventLock = new object();
        public event MessageHandler OnMessage
        {
            add
            {
                lock (m_messageEventLock)
                {
                    m_onMessage += value;
                }
            }

            remove
            {
                lock (m_messageEventLock)
                {
                    m_onMessage -= value;
                }
            }
        }

        public void Close()
        {
            m_closed = true;
        }

        private event ClosedConnectionHandler m_onClosed
        {
            // dummy implementation to remove unused warning
            add {}
            remove {}
        }

        private object m_closedEventLock = new object();
        public event ClosedConnectionHandler OnClosed
        {
            add
            {
                lock (m_closedEventLock)
                {
                    m_onClosed += value;
                }
            }

            remove
            {
                lock (m_closedEventLock)
                {
                    m_onClosed -= value;
                }
            }
        }

        public bool IsClosed
        {
            get { return m_closed; }
        }

        #endregion
    }

}
