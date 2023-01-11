namespace PyxNet
{
    using System;

    /// <summary>
    /// This class creates a persistent connection that reconnects when closed.
    /// </summary>
    public class PersistentConnection
    {
        #region Fields

        /// <summary>
        /// The stack that the persistent connection will be created from.
        /// </summary>
        private readonly Stack m_stack;

        /// <summary>
        /// The host name to connect to.
        /// </summary>
        private readonly String m_hostName = null;

        /// <summary>
        /// The port to connect to the host name on.
        /// </summary>
        private readonly int m_port;

        /// <summary>
        /// The network address to connect on if the hostname and port cannot be resolved to an address.
        /// </summary>
        private readonly NetworkAddress m_address = null;

        /// <summary>
        /// A pointer to the resulting connection.
        /// </summary>
        private StackConnection m_connection = null;

        /// <summary>
        /// A lock for accessing the connection.
        /// </summary>
        private readonly Object m_connectionLock = new Object();

        /// <summary>
        /// The thread that attempts connection.
        /// </summary>
        private System.Threading.Thread m_connectionThread = null;

        /// <summary>
        /// A lock for ensuring that multiple connection threads are not created.
        /// </summary>
        private readonly Object m_connectionThreadLock = new Object();

        /// <summary>
        /// Whether to reconnect if a connection closed event is received.
        /// </summary>
        private bool m_reconnect = true;

        #endregion

        #region Constructors

        /// <summary>
        /// A private utility constructor.
        /// </summary>
        /// <param name="stack">The stack to create the connection from.</param>
        private PersistentConnection(Stack stack)
        {
            if (null == stack)
            {
                throw new ArgumentNullException("stack");
            }
            m_stack = stack;
        }

        /// <summary>
        /// A constructor that creates a persistent connection by host name and port.
        /// </summary>
        /// <param name="stack">The stack to create the connection from.</param>
        /// <param name="hostName">The host name to connect to.</param>
        /// <param name="port">The port to connect on.</param>
        /// <param name="timeBetweenRetries">The amount of time to wait between retries.</param>
        public PersistentConnection(Stack stack, String hostName, int port, TimeSpan timeBetweenRetries)
            : this(stack)
        {
            if (null == hostName)
            {
                throw new ArgumentNullException("hostName");
            }
            m_hostName = hostName;

            m_port = port;

            m_timeBetweenRetries = timeBetweenRetries;

            StartConnectionThread();
        }

        /// <summary>
        /// A constructor that creates a persistent connection by NetworkAddress.
        /// </summary>
        /// <param name="stack">The stack to create the connection from.</param>
        /// <param name="address">The network address to connect to.</param>
        /// <param name="timeBetweenRetries">The amount of time to wait between retries.</param>
        public PersistentConnection(Stack stack, NetworkAddress address, TimeSpan timeBetweenRetries)
            : this(stack)
        {
            if (null == address)
            {
                throw new ArgumentNullException("address");
            }
            m_address = address;

            m_timeBetweenRetries = timeBetweenRetries;

            StartConnectionThread();
        }

        /// <summary>
        /// A constructor that creates a persistent connection by host name and port.
        /// </summary>
        /// <param name="stack">The stack to create the connection from.</param>
        /// <param name="hostName">The host name to connect to.</param>
        /// <param name="port">The port to connect on.</param>
        public PersistentConnection(Stack stack, String hostName, int port)
            : this(stack, hostName, port, DefaultTimeBetweenRetries)
        {
        }

        /// <summary>
        /// A constructor that creates a persistent connection by NetworkAddress.
        /// </summary>
        /// <param name="stack">The stack to create the connection from.</param>
        /// <param name="address">The network address to connect to.</param>
        public PersistentConnection(Stack stack, NetworkAddress address)
            : this(stack, address, DefaultTimeBetweenRetries)
        {
        }

        #endregion

        #region Properties

        /// <summary>
        /// The default amount of time between reconnection retries.
        /// </summary>
        public static TimeSpan DefaultTimeBetweenRetries
        {
            get
            {
                return m_defaultTimeBetweenRetries;
            }
        }
       
        private static readonly TimeSpan m_defaultTimeBetweenRetries =
            TimeSpan.FromMilliseconds(Context.ConnectionRetryWaitMilliseconds);

        /// <summary>
        /// The amount of time between reconnection retries.
        /// </summary>
        public TimeSpan TimeBetweenRetries
        {
            get
            {
                return m_timeBetweenRetries;
            }
            set
            {
                m_timeBetweenRetries = value;
            }
        }
        private TimeSpan m_timeBetweenRetries = DefaultTimeBetweenRetries;

        /// <summary>
        /// The address to connect to.  Lazily evaluated from host name, if specified.
        /// </summary>
        private NetworkAddress Address
        {
            get
            {
                // Try the host name first, and fall through to the address.
                try
                {
                    return new NetworkAddress(m_hostName, m_port);
                }
                catch
                {
                }
                return m_address;
            }
        }

        /// <summary>
        /// Whether or not to attempt reconnection.
        /// Changing the reconnection state will either stop or start the connection thread.
        /// </summary>
        public bool Reconnect
        {
            get
            {
                return m_reconnect;
            }
            set
            {
                if (value != m_reconnect)
                {
                    m_reconnect = value;
                    if (value)
                    {
                        StartConnectionThread();
                    }
                    else
                    {
                        StopConnectionThread();
                    }
                }
            }
        }

        #endregion

        #region Connection

        /// <summary>
        /// Clear the connection and reconnect if Reconnect is true.
        /// </summary>
        public void Clear()
        {
            lock (m_connectionLock)
            {
                if (m_connection != null)
                {
                    m_connection.Close();
                    m_connection = null;
                }
            }

            if (Reconnect)
            {
                StartConnectionThread();
            }
        }

        internal void Close()
        {
            lock (m_connectionLock)
            {
                if (m_connection != null)
                {
                    m_connection.Close();
                }
            }
        }

        /// <summary>
        /// The body of the connection thread.
        /// </summary>
        private void ConnectionThreadBody()
        {
            for (; ; System.Threading.Thread.Sleep(TimeBetweenRetries))
            {
                // Stop if the thread has been flagged to stop from the outside.
                lock (m_connectionThreadLock)
                {
                    if (null == m_connectionThread)
                    {
                        break;
                    }
                }

                // Attempt connection, and stop looping if successful.
                lock (m_connectionLock)
                {
                    // If we already have a connection, break.
                    if (null != m_connection)
                    {
                        break;
                    }

                    // Lazily resolve the address.
                    NetworkAddress address = this.Address;
                    if (null == address)
                    {
                        m_stack.Tracer.WriteLine("Could not connect; no address could be obtained.");
                    }
                    else
                    {
                        // Create connection.
                        m_stack.Tracer.DebugWriteLine("Connecting to address {0}.", address);
                        try
                        {
                            m_connection = m_stack.CreateConnection(address, true);
                        }
                        catch (Stack.ConnectionException exception)
                        {
                            m_stack.Tracer.WriteLine("Exception when creating connection to {0}: {1}",
                                address, exception.ErrorType.ToString());
                        }
                        if (null == m_connection)
                        {
                            m_stack.Tracer.WriteLine("Could not create connection to {0}.",
                                address);
                        }
                        else
                        {
                            m_stack.Tracer.DebugWriteLine("Created persistent connection to {0}.", address);

                            // Attach "disconnect" handler, which will attempt reconnection after a wait period.
                            m_connection.OnClosed += HandleConnectionClosed;

                            // If it was closed before the handler was hooked up, clear it.
                            if (m_connection.IsClosed)
                            {
                                m_connection = null;
                            }
                            else if (null != m_connection)
                            {
                                // Stop if a connection has been obtained.
                                break;
                            }
                        }
                    }
                }
            }

            // Flag that the thread is stopped.
            lock (m_connectionThreadLock)
            {
                m_connectionThread = null;
            }
        }

        /// <summary>
        /// Starts the connection thread.
        /// </summary>
        private void StartConnectionThread()
        {
            lock (m_connectionThreadLock)
            {
                if (null == m_connectionThread)
                {
                    // Start thread that tries to connect.
                    m_connectionThread = new System.Threading.Thread(ConnectionThreadBody);
                    m_connectionThread.Name = String.Format("Persistent Connection thread for {0}", 
                        (this.Address == null) ? 
                        this.m_hostName :
                        this.Address.ToString());
                    m_connectionThread.IsBackground = true;
                    m_connectionThread.Start();
                }
            }
        }

        /// <summary>
        /// Tells the connection thread to stop at the beginning of its next iteration.
        /// </summary>
        private void StopConnectionThread()
        {
            lock (m_connectionThreadLock)
            {
                m_connectionThread = null;
            }
        }

        /// <summary>
        /// Handle a closed connection.
        /// </summary>
        /// <param name="connection">The connection that was closed.</param>
        private void HandleConnectionClosed(StackConnection connection)
        {
            Clear();
        }

        #endregion
    }
}
