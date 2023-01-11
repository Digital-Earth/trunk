/******************************************************************************
StackConnection.cs

begin      : 27/03/2007 12:00:00 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
using System;

namespace PyxNet
{
    /// <summary>
    /// Delegate type for handling messages that arrive on a stack connection.
    /// </summary>
    /// <param name="connection">The stack connection that received the message.</param>
    /// <param name="message">The message that came in.</param>
    public delegate void StackConnectionMessageHandler(StackConnection connection, Message message);

    /// <summary>
    /// Delegate type for handling a closed connection.
    /// </summary>
    /// <param name="connection">The connection that just closed.</param>
    public delegate void ClosedStackConnectionHandler(StackConnection connection);

    /// <summary>
    /// A wrapper class that is used as part of the system to determine if there are any 
    /// current users of a connection.  Consumers of a connection should only hang onto 
    /// the Connection handle, and make calls into the Connection that it contains without
    /// keeping a copy of the connection that is wrapped up.
    /// </summary>
    public class StackConnectionHandle : IDisposable
    {
        /// <summary>
        /// Storage for the wrapped connection.
        /// </summary>
        private readonly StackConnection m_connection;

        /// <summary>
        /// The wrapped connection.
        /// </summary>
        public StackConnection Connection
        {
            get { return m_connection; }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="StackConnectionHandle"/> class.
        /// </summary>
        /// <param name="connection">The connection.</param>
        public StackConnectionHandle(StackConnection connection)
        {
            m_connection = connection;
        }

        #region Disposing

        /// <summary>
        /// Track whether Dispose has been called.
        /// </summary>
        private bool m_disposed = false;

        /// <summary>
        /// Implement IDisposable.
        /// </summary>
        /// <remarks>
        /// Do not make this method virtual.
        /// A derived class should not be able to override this method. 
        /// </remarks>
        public void Dispose()
        {
            Dispose(true);

            // This object will be cleaned up by the Dispose method.
            // Therefore, you should call GC.SupressFinalize to
            // take this object off the finalization queue 
            // and prevent finalization code for this object
            // from executing a second time.
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Dispose(bool disposing) executes in two distinct scenarios, 
        /// indicated by the "disposing" argument.
        /// </summary>
        /// <param name="disposing">
        /// If disposing equals true, the method has been called directly
        /// or indirectly by a user's code. Managed and unmanaged resources
        /// can be disposed.
        /// If disposing equals false, the method has been called by the 
        /// runtime from inside the finalizer and you should not reference 
        /// other objects. Only unmanaged resources can be disposed.
        /// </param>
        private void Dispose(bool disposing)
        {
            // Check to see if Dispose has already been called.
            if (!m_disposed)
            {
                // We do this logic if we are disposing or if we are in the destructor.
                // This is out of the ordinary for this kind pattern, but it is really 
                // what we want in this case.  We perform the operation inside a try
                // because the object that we are calling into may be in a bad state if
                // this is really happening in the destructor.
                try
                {
                    m_connection.HandleSCHDispose();
                }
                catch
                {
                }

                // Call the appropriate methods to clean up 
                // unmanaged resources here.
                // If disposing is false, 
                // only the following code is executed.
            }
            m_disposed = true;
        }

        /// <summary>
        /// The finalization code.
        /// </summary>
        /// <remarks>
        /// Use C# destructor syntax for finalization code.
        /// This destructor will run only if the Dispose method 
        /// does not get called.
        /// It gives your base class the opportunity to finalize.
        /// Do not provide destructors in types derived from this class.
        /// </remarks>
        ~StackConnectionHandle()
        {
            // Do not re-create Dispose clean-up code here.
            // Calling Dispose(false) is optimal in terms of
            // readability and maintainability.
            Dispose(false);
        }

        #endregion
    }


    public class StackConnectionStatistics
    {
        public StackConnectionStatistics(StackConnection connection)
        {
            m_connection = connection;
            connection.OnAnyMessage += OnMessageReceivied;
            connection.OnClosed += OnConnectionClosed;
            connection.OnSendingMessage += OnSendingMessage;

            m_lastSampleTime = DateTime.Now;
            m_timer.AutoReset = true;
            m_timer.Elapsed += new System.Timers.ElapsedEventHandler(UpdateByteRates);
            m_timer.Start();
        }        

        #region Statistics

        private Pyxis.Utilities.ThreadSafeDictionary<string, int> m_messagesReceived = new Pyxis.Utilities.ThreadSafeDictionary<string,int>();
        private int m_totalBytesReceived = 0;
        private int m_lastTotalBytesReceived = 0;
        private double m_bytesReceivedRate = 0;

        private Pyxis.Utilities.ThreadSafeDictionary<string, int> m_messagesSent = new Pyxis.Utilities.ThreadSafeDictionary<string,int>();
        private int m_totalBytesSent = 0;
        private int m_lastTotalBytesSent = 0;
        private double m_bytesSentRate = 0;

        DateTime m_lastSampleTime;
        System.Timers.Timer m_timer = new System.Timers.Timer(1000);

        private StackConnection m_connection; 

        #endregion

        #region Statistics properties

        public Pyxis.Utilities.ThreadSafeDictionary<string, int> MessagesReceived
        {
            get
            {
                return m_messagesReceived;
            }
        }

        public int TotalBytesReceived
        {
            get
            {
                return m_totalBytesReceived;
            }
        }

        public Pyxis.Utilities.ThreadSafeDictionary<string, int> MessagesSent
        {
            get
            {
                return m_messagesSent;
            }
        }

        public int TotalBytesSent
        {
            get
            {
                return m_totalBytesSent;
            }
        }

        public double BytesSentRate
        {
            get
            {
                return m_bytesSentRate;
            }
        }

        public double BytesReceivedRate
        {
            get
            {
                return m_bytesReceivedRate;
            }
        }

        public StackConnection Connection
        {
            get
            {
                return m_connection;
            }
        }

        #endregion
        
        #region Handlers

        void OnSendingMessage(StackConnection connection, Message message)
        {
            if (!m_messagesSent.ContainsKey(message.Identifier))
            {
                m_messagesSent[message.Identifier] = 0;
            }
            m_messagesSent[message.Identifier] = m_messagesSent[message.Identifier] + 1;
            m_totalBytesSent += message.Length;
        }        

        void OnMessageReceivied(StackConnection connection, Message message)
        {
            if (!m_messagesReceived.ContainsKey(message.Identifier))
            {
                m_messagesReceived[message.Identifier] = 0;
            }
            m_messagesReceived[message.Identifier] = m_messagesReceived[message.Identifier] + 1;
            m_totalBytesReceived += message.Length;
        }

        void OnConnectionClosed(StackConnection connection)
        {
            m_timer.Stop();
        }

        void UpdateByteRates(object sender, System.Timers.ElapsedEventArgs e)
        {
            DateTime now = DateTime.Now;

            double seconds = (now - m_lastSampleTime).TotalSeconds;

            m_lastSampleTime = now;

            m_bytesReceivedRate = (m_totalBytesReceived - m_lastTotalBytesReceived) / seconds;
            m_lastTotalBytesReceived = m_totalBytesReceived;

            m_bytesSentRate = (m_totalBytesSent - m_lastTotalBytesSent) / seconds;
            m_lastTotalBytesSent = m_totalBytesSent;
        }

        #endregion
    }

    /// <summary>
    /// Wraps an INetworkConnection and makes sense of its messages.
    /// Also knows about the remote INode it's connected to via INodeID.
    /// </summary>
    public class StackConnection 
    {
        /// <summary>
        /// This is the trace tool that one should use for all things to do with this stack connection.
        /// </summary>
        private Pyxis.Utilities.NumberedTraceTool<StackConnection> m_tracer
            = new Pyxis.Utilities.NumberedTraceTool<StackConnection>(Pyxis.Utilities.TraceTool.GlobalTraceLogEnabled);

        public Pyxis.Utilities.NumberedTraceTool<StackConnection> Tracer
        {
            get
            {
                return m_tracer;
            }
        }

        /// <summary>
        /// This class encapsulates a weak reference to a StackConnection, and 
        /// attaches it to a Stack's events.  This allows the SC to respond to 
        /// the stack's events without extending the life of the SC.
        /// </summary>
        private class WeakReferencedEventHandler
        {
            private readonly Pyxis.Utilities.TypedWeakReference<StackConnection> m_weakReference;

            internal WeakReferencedEventHandler(StackConnection target)
            {
                m_weakReference = target;
            }

            /// <summary>
            /// Handles an incoming message on the stack.  
            /// </summary>
            /// <param name="sender">The sender.</param>
            /// <param name="message">The message.</param>
            internal void HandleMessage(object sender, Message message)
            {
                StackConnection stackConnection = m_weakReference;
                if (stackConnection != null)
                {
                    stackConnection.HandleMessage(message);
                }
            }

            /// <summary>
            /// Handles a "closed connection" message from the stack.
            /// </summary>
            /// <param name="sender">The sender.</param>
            /// <param name="connection">The connection.</param>
            internal void HandleClosedConnection(object sender, INetworkConnection connection)
            {
                StackConnection stackConnection = m_weakReference;
                if (stackConnection != null)
                {
                    stackConnection.HandleClosedConnection(connection);
                }
            }
        }

        private readonly WeakReferencedEventHandler m_handler;

        #region Constructors

        /// <summary>
        /// Constructs a non-temporary connection.
        /// </summary>
        /// <param name="connection">A live connection with a matching Stack Connection at the other end.</param>
        public StackConnection(INetworkConnection connection)
        {
            if (null == connection)
            {
                throw new ArgumentNullException("connection");
            }

            m_tracer.DebugWriteLine("Constructing connection.");

            m_connection = connection;

            m_handler = new WeakReferencedEventHandler(this);
        }

        #endregion

        #region Conversion

        /// <summary>
        /// Display as the friendly name.
        /// </summary>
        /// <returns>The connected node's friendly name, or "unknown node".</returns>
        public override string ToString()
        {
            return this.FriendlyName;
        }

        /// <summary>
        /// Display verbose information about this connection.
        /// </summary>
        /// <returns>A list of address:port in clear text format.</returns>
        public string ToStringVerbose()
        {
            System.Text.StringBuilder output = new System.Text.StringBuilder();
            output.AppendFormat("StackConnection: connection to {0} is {1}.\n",
                m_connection.Address, (m_connection.IsClosed ? "Closed" : "Open"));

            output.AppendFormat("    remote node id {2} is {0} with address {1}.\n",
                ((RemoteNodeInfo == null) ? "Unknown" : RemoteNodeInfo.FriendlyName),
                ((RemoteNodeInfo == null) ? "Unknown" : RemoteNodeInfo.Address.ToString()),
                ((RemoteNodeInfo == null) ? "Unknown" : RemoteNodeInfo.NodeId.ToString()));

            output.AppendFormat("    Pings received {0}. Pongs received {1}.\n",
                m_numPingsReceived, m_numPongsReceived);

            return output.ToString();
        }

        #endregion

        #region Connection State
        /// <summary>
        /// The various states that a connection can be in.
        /// </summary>
        public enum State { 
            Constructed,        // this StackConnection is constucted or is being constructed. 
            Opening,            // the connection is in the process of being opened.
            Active,             // live and ready to send and recieve messages
            CloseRequested,     // the other end has asked us to close.
            Closing,            // this end has requested that we close.
            Closed              // we are closed.
        };

        private State m_state = State.Constructed;

        public State ConnectionState
        {
            get
            {
                return m_state;
            }
        }

        internal void SetConnectionState(State newState)
        {
            m_state = newState;
        }

        #endregion

        #region Handle

        private Pyxis.Utilities.TypedWeakReference<StackConnectionHandle> m_connectionHandle = null;

        private Object m_handleLock = new object();

        public StackConnectionHandle Handle
        {
            get 
            {
                lock (m_handleLock)
                {
                    if (m_connectionHandle == null)
                    {
                        // we need to make sure that we are allowed to send messages.
                        if (m_state == State.Active)
                        {
                            m_connectionHandle = 
                                new Pyxis.Utilities.TypedWeakReference<StackConnectionHandle>(
                                    new StackConnectionHandle(this));

                            // We may want to kill the deadman timer here.
                        }
                    }
                }
                return m_connectionHandle; 
            }
        }

        #endregion

        #region Close Messages

        /// <summary>
        /// The message ID for a Close message.
        /// </summary>
        public const string CloseMessageID = "CLOS";

        /// <summary>
        /// The static definition of a Close that is shared across all Stack Connections.
        /// </summary>
        private static readonly Message CloseMessage = new Message(CloseMessageID);

        #endregion

        #region Connection

        /// <summary>
        /// the network connection to talk across.
        /// </summary>
        private readonly INetworkConnection m_connection;

        /// <summary>
        /// Start handling messages.
        /// </summary>
        public void Start()
        {
            // Attach the event handlers.
            m_connection.OnMessage += m_handler.HandleMessage;
            m_connection.OnClosed += m_handler.HandleClosedConnection;
        }

        /// <summary>
        /// Stop handling messages.
        /// </summary>
        public void Stop()
        {
            // Detach the event handlers.
            m_connection.OnMessage -= m_handler.HandleMessage;
            m_connection.OnClosed -= m_handler.HandleClosedConnection;
        }

        /// <summary>
        /// Handle when a connection has been closed.
        /// </summary>
        /// <param name="connection"></param>
        internal void HandleClosedConnection(INetworkConnection connection)
        {
            // Tell the underlying TCP connection
            // to stop handling events.
            Stop();

            // Fire closed event for stack connection.
            if (null != m_onClosed)
            {
                m_onClosed(this);
            }            
        }

        #endregion

        #region Messages

        #region Any Message

        /// <summary>
        /// Single point for sending all messages in a Stack Connection.
        /// All methods within the stack connection and all clients of the
        /// stack connection should send messages using this method.
        /// </summary>
        /// <param name="message">The message to send.</param>
        public bool SendMessage(Message message)
        {
            StackConnectionMessageHandler handler = m_onSendingMessage;
            if (null != handler)
            {
                handler(this, message);
            }
            return m_connection.SendMessage(message);
        }

        /// <summary>
        /// Stores the event handler for OnSendingMessage event.
        /// </summary>
        private event StackConnectionMessageHandler m_onSendingMessage;
        private object m_sendingMessageEventLock = new object();

        /// <summary>
        /// Fired just before any message is sent through the stack connection.
        /// </summary>
        public event StackConnectionMessageHandler OnSendingMessage
        {
            add
            {
                lock (m_sendingMessageEventLock)
                {
                    m_onSendingMessage += value;
                }
            }

            remove
            {
                lock (m_sendingMessageEventLock)
                {
                    m_onSendingMessage -= value;
                }
            }
        }

        /// <summary>
        /// Handler that is connected the Network Connections OnMessage.
        /// </summary>
        /// <param name="message"></param>
        internal void HandleMessage(Message message)
        {
            // fire this event for all messages that come through the system.
            StackConnectionMessageHandler any = m_onAnyMessage;
            if (any != null)
            {
                any(this, message);
            }

            // A Ping Message
            if (message.StartsWith(PingMessageID))
            {
                System.Diagnostics.Debug.Assert(message.Length == 4);
                // count a received Ping
                ++m_numPingsReceived;
                // every good Ping deserves a Pong.
                SendPong();
                StackConnectionMessageHandler ping = m_onPing;
                if (ping != null)
                {
                    ping(this, message);
                }
                return;
            }

            // A Pong Message
            if (message.StartsWith(PongMessageID))
            {
                System.Diagnostics.Debug.Assert(message.Length == 4);
                // count a received Pong
                ++m_numPongsReceived;
                m_lastPongReceivedTime = DateTime.Now;
                StackConnectionMessageHandler pong = m_onPong;
                if (pong != null)
                {
                    pong(this, message);
                }
                return;
            }

            // A Local Node Info Message
            if (message.StartsWith(LocalNodeInfoMessageID))
            {
                // store away the latest info about the node.
                m_remoteNodeInfo = new NodeInfo(message);
                StackConnectionMessageHandler local = m_onLocalNodeInfo;
                if (local != null)
                {
                    local(this, message);
                }
                return;
            }

            // A Known Hub List Message            
            if (message.StartsWith(KnownHubListMessageID))
            {
                // store away the latest info about the node.
                m_remoteKnownHubList = new KnownHubList(message);
                StackConnectionMessageHandler known = m_onKnownHubList;
                if (known != null)
                {
                    known(this, message);
                }
                return;
            }

            // A Query Message
            if (message.StartsWith(QueryMessageID))
            {
                StackConnectionMessageHandler query = m_onQuery;
                if (query != null)
                {
                    query(this, message);
                }
                return;
            }

            // A Query Acknowledge Message
            if (message.StartsWith(QueryAcknowledgementMessageID))
            {
                StackConnectionMessageHandler queryAck = m_onQueryAcknowledgement;
                if (queryAck != null)
                {
                    queryAck(this, message);
                }
                return;
            }

            // A Query Hash Table Message
            if (message.StartsWith(QueryHashTableMessageID))
            {
                // store away the latest info about the node.
                m_remoteQueryHashTable = new QueryHashTable(message);

                StackConnectionMessageHandler hash = m_onQueryHashTable;
                if (hash != null)
                {
                    hash(this, message);
                }
                return;
            }

            // A Close Message
            if (message.StartsWith(CloseMessageID))
            {
                System.Diagnostics.Debug.Assert(message.Length == 4);
                // TODO: close message recieved logic goes here...
                return;
            }

            // if we have reached this point, then we don't know what the message was
            StackConnectionMessageHandler onUnknownMessage = m_onUnknownMessage;
            if (onUnknownMessage != null)
            {
                // Ensure that we only log each message type once, for each connection.
                LogMissedMessage(message);

                onUnknownMessage(this, message);
            }
        }

        private Pyxis.Utilities.DynamicList<string> m_missedMessageTypes =
            new Pyxis.Utilities.DynamicList<string>();

        /// <summary>
        /// Logs the missed message.  Messages are logged at most once.
        /// </summary>
        /// <param name="message">The message.</param>
        [System.Diagnostics.Conditional("DEBUG")]
        private void LogMissedMessage(Message message)
        {
            if (!m_missedMessageTypes.Contains( message.Identifier))
            {
                m_missedMessageTypes.Add( message.Identifier);
                m_tracer.DebugWriteLine(
                    "Message {0} is unknown to stack connection.  Propagating.",
                    message.Identifier);
            }
        }

        /// <summary>
        /// Stores the event handler for OnAnyMessage event.
        /// </summary>
        private event StackConnectionMessageHandler m_onAnyMessage;
        private object m_anyMessageEventLock = new object();

        /// <summary>
        /// Fires when any message is received.
        /// </summary>
        public event StackConnectionMessageHandler OnAnyMessage
        {
            add
            {
                lock (m_anyMessageEventLock)
                {
                    m_onAnyMessage += value;
                }
            }

            remove
            {
                lock (m_anyMessageEventLock)
                {
                    m_onAnyMessage -= value;
                }
            }
        }

        #endregion

        #region Ping Message

        /// <summary>
        /// The message ID for a ping message.
        /// </summary>
        public const string PingMessageID = "PING";

        /// <summary>
        /// The static definition of a Ping that is shared across all Stack Connections.
        /// </summary>
        private static readonly Message PingMessage = new Message(PingMessageID);

        /// <summary>
        /// The time that we last sent a Ping message.
        /// </summary>
        private DateTime m_lastPingSentTime;

        /// <summary>
        /// Storage for the number of pings that we have received.
        /// </summary>
        private int m_numPingsReceived = 0;

        /// <summary>
        /// The number of pings that we have received.
        /// </summary>
        public int NumPingsReceived
        {
            get { return m_numPingsReceived; }
        }

        /// <summary>
        /// Send a Ping over to the connected node.
        /// </summary>
        public void SendPing()
        {
            if (SendMessage(PingMessage))
            {
                m_lastPingSentTime = DateTime.Now;
            }
        }

        /// <summary>
        /// Stores the event handler for OnPing event. 
        /// </summary>
        private event StackConnectionMessageHandler m_onPing;
        private object m_pingEventLock = new object();

        /// <summary>
        /// Event called when a Ping message is received.
        /// A Pong message will already have been dispatched before this method is called.
        /// </summary>
        public event StackConnectionMessageHandler OnPing
        {
            add
            {
                lock (m_pingEventLock)
                {
                    m_onPing += value;
                }
            }

            remove
            {
                lock (m_pingEventLock)
                {
                    m_onPing -= value;
                }
            }
        }

        #endregion

        #region Pong Message

        /// <summary>
        /// The message ID for a pong message.
        /// </summary>
        public const string PongMessageID = "PONG";

        /// <summary>
        /// A static PONG Message used by all to send a Pong
        /// </summary>
        private static readonly Message PongMessage = new Message(PongMessageID);

        /// <summary>
        /// The time that we last received a Pong message.
        /// </summary>
        private DateTime m_lastPongReceivedTime;

        /// <summary>
        /// Storage for the number of pongs that we have received.
        /// </summary>
        private int m_numPongsReceived = 0;

        /// <summary>
        /// The number of pongs that we have received.
        /// </summary>
        public int NumPongsReceived
        {
            get { return m_numPongsReceived; }
        }

        /// <summary>
        /// Send a Pong message to the connected node.
        /// </summary>
        public void SendPong()
        {
            SendMessage(PongMessage);
        }

        /// <summary>
        /// Stores the event handler for OnPong event.
        /// </summary>
        private event StackConnectionMessageHandler m_onPong;
        private object m_pongEventLock = new object();

        /// <summary>
        /// Fired when a Pong message is received.
        /// </summary>
        public event StackConnectionMessageHandler OnPong
        {
            add
            {
                lock (m_pongEventLock)
                {
                    m_onPong += value;
                }
            }

            remove
            {
                lock (m_pongEventLock)
                {
                    m_onPong -= value;
                }
            }
        }

        #endregion

        #region Node Info Message

        /// <summary>
        /// The Message ID for a Local Node Info Message.
        /// </summary>
        public const string LocalNodeInfoMessageID = "LNIn";

        /// <summary>
        /// We keep the last know information about the remote node stored here.
        /// </summary>
        private NodeInfo m_remoteNodeInfo = null;

        /// <summary>
        /// Information about the node that is on the other end of this connection.
        /// This will return a null value if we have never recieved any local node
        /// information from the other end of the pipe.
        /// </summary>
        public NodeInfo RemoteNodeInfo
        {
            get
            {
                return m_remoteNodeInfo;
            }
            set
            {
                if (null == value)
                {
                    throw new ArgumentNullException("RemoteNodeInfo");
                }

                // we need to give a hash table event because we would have been unable 
                // to handle the hash table until we had info on the remote node.
                bool fireHashTable = (m_remoteNodeInfo == null && m_remoteQueryHashTable != null);

                // store away the latest info about the node.
                m_remoteNodeInfo = value;

                if (m_onQueryHashTable != null && fireHashTable)
                {
                    m_tracer.DebugWriteLine(
                        "Refiring OnQueryHashTable for connection {0} after setting the Remote Node Info.",
                        m_remoteNodeInfo.FriendlyName);

                    // if we need it we could recontruct the QHT message from the current QHT
                    m_onQueryHashTable(this, null);
                }
            }
        }

        /// <summary>
        /// Gets the friendly name for the node on the other end of this connection.
        /// </summary>
        public String FriendlyName
        {
            get
            {
                if (RemoteNodeInfo != null)
                {
                    return RemoteNodeInfo.FriendlyName;
                }
                // TODO: can we get the address info here?
                // possibly, we can fill in some address info when the connection comes in to 
                // the listener.
                return "Unidentified node.";
            }
        }

        /// <summary>
        /// Stores the event handler for OnLocalNodeInfo event.
        /// </summary>
        private event StackConnectionMessageHandler m_onLocalNodeInfo;
        private object m_localNodeInfoEventLock = new object();

        /// <summary>
        /// Fires when a Local Node Info message has been received.
        /// The message will have already been decoded and the RemoteNodeInfo
        /// for this connection will have been updated to reflect the latest
        /// information in this message.
        /// </summary>
        public event StackConnectionMessageHandler OnLocalNodeInfo
        {
            add
            {
                lock (m_localNodeInfoEventLock)
                {
                    m_onLocalNodeInfo += value;
                }
            }

            remove
            {
                lock (m_localNodeInfoEventLock)
                {
                    m_onLocalNodeInfo -= value;
                }
            }
        }

        #endregion

        #region Known Hub List Message

        /// <summary>
        /// The Message ID for a Known Hub List Message.
        /// </summary>
        public const string KnownHubListMessageID = "KHLi";

        /// <summary>
        /// Storage for the remote known hub list.
        /// </summary>
        private KnownHubList m_remoteKnownHubList = null;

        /// <summary>
        /// The remote known hub list.
        /// </summary>
        public KnownHubList RemoteKnownHubList
        {
            get
            {
                return m_remoteKnownHubList;
            }
            set
            {
                m_remoteKnownHubList = value;
                m_tracer.DebugWriteLine(
                    String.Format("{0} setting known hub list to {1}", 
                    this.ToString(), m_remoteKnownHubList.ToString()));
            }
        }

        /// <summary>
        /// Stores the event handler for OnKnownHubList event.
        /// </summary>
        private event StackConnectionMessageHandler m_onKnownHubList;
        private object m_knownHubListEventLock = new object();

        /// <summary>
        /// Fires when a Known Hub List message has been recieved.
        /// </summary>
        public event StackConnectionMessageHandler OnKnownHubList
        {
            add
            {
                lock (m_knownHubListEventLock)
                {
                    m_onKnownHubList += value;
                }
            }

            remove
            {
                lock (m_knownHubListEventLock)
                {
                    m_onKnownHubList -= value;
                }
            }
        }

        #endregion

        #region Query Message

        public const string QueryMessageID = "Qery";

        /// <summary>
        /// Stores the event handler for OnQuery event.
        /// </summary>
        private event StackConnectionMessageHandler m_onQuery;
        private object m_queryEventLock = new object();

        /// <summary>
        /// Fired when a Query message is recieved.
        /// </summary>
        public event StackConnectionMessageHandler OnQuery
        {
            add
            {
                lock (m_queryEventLock)
                {
                    m_onQuery += value;
                }
            }

            remove
            {
                lock (m_queryEventLock)
                {
                    m_onQuery -= value;
                }
            }
        }

        #endregion

        #region Query Acknowledgement Message

        public const string QueryAcknowledgementMessageID = "QuAk";

        /// <summary>
        /// Stores the event handler for OnQueryAcknowledgement event.
        /// </summary>
        private event StackConnectionMessageHandler m_onQueryAcknowledgement;
        private object m_queryAcknowledgementEventLock = new object();

        /// <summary>
        /// Fired when a Query message is recieved.
        /// </summary>
        public event StackConnectionMessageHandler OnQueryAcknowledgement
        {
            add
            {
                lock (m_queryAcknowledgementEventLock)
                {
                    m_onQueryAcknowledgement += value;
                }
            }

            remove
            {
                lock (m_queryAcknowledgementEventLock)
                {
                    m_onQueryAcknowledgement -= value;
                }
            }
        }

        #endregion

        #region Query Hash Table Message

        public const string QueryHashTableMessageID = "QHaT";

        /// <summary>
        /// Stores the event handler for OnQueryHashTable event.
        /// </summary>
        private event StackConnectionMessageHandler m_onQueryHashTable;
        private object m_queryHashTableEventLock = new object();

        /// <summary>
        /// Fired when a QueryHashTable message is recieved.
        /// </summary>
        public event StackConnectionMessageHandler OnQueryHashTable
        {
            add
            {
                lock (m_queryHashTableEventLock)
                {
                    m_onQueryHashTable += value;
                }
            }

            remove
            {
                lock (m_queryHashTableEventLock)
                {
                    m_onQueryHashTable -= value;
                }
            }
        }

        /// <summary>
        /// Storage for the remote node's amalgamated query hash table
        /// (which is the same as the local hash table if it's a leaf).
        /// </summary>
        private QueryHashTable m_remoteQueryHashTable = null;

        /// <summary>
        /// The remote node's amalgamated query hash table
        /// (which is the same as the local hash table if it's a leaf).
        /// </summary>
        public QueryHashTable RemoteQueryHashTable
        {
            get
            {
                return m_remoteQueryHashTable;
            }
        }

        #endregion

        #region Unknown Message

        /// <summary>
        /// Stores the event handler for OnUnknownMessage event.
        /// </summary>
        private event StackConnectionMessageHandler m_onUnknownMessage;
        private object m_unknownMessageEventLock = new object();

        /// <summary>
        /// Fired when a message is received but not recognized.
        /// </summary>
        public event StackConnectionMessageHandler OnUnknownMessage
        {
            add
            {
                lock (m_unknownMessageEventLock)
                {
                    m_onUnknownMessage += value;
                }
            }

            remove
            {
                lock (m_unknownMessageEventLock)
                {
                    m_onUnknownMessage -= value;
                }
            }
        }

        #endregion

        #endregion

        #region Statistics

        private StackConnectionStatistics m_statistics;

        private Object m_statisticsLock = new object();

        public StackConnectionStatistics Statistics
        {
            get
            {
                lock (m_statisticsLock)
                {
                    if (m_statistics == null)
                    {
                        m_statistics = new StackConnectionStatistics(this);
                    }
                }

                return m_statistics;
            }
        }

        #endregion

        #region Close

        /// <summary>
        /// Close down this stack connection.
        /// It may be considered impolite to call this, because others 
        /// may be using the connection.
        /// Instead, prefer letting the stack connection simply go out of scope,
        /// allowing the finalizer to call this when garbage collected.
        /// </summary>
        public void Close()
        {
            if (!IsClosed)
            {
                m_tracer.DebugWriteLine("Closing stack connection to {0}.", ToString());

                m_connection.Close();
            }
        }

        /// <summary>
        /// Is true if the stack connection is closed.
        /// </summary>
        public bool IsClosed
        {
            get
            {
                if (m_connection == null)
                {
                    return true;
                }

                // Is true if the underlying network connection is closed.
                return m_connection.IsClosed;
            }
        }

        /// <summary>
        /// This event is triggered when the connection has been closed.
        /// </summary>
        private event ClosedStackConnectionHandler m_onClosed;
        private object m_closedEventLock = new object();
        public event ClosedStackConnectionHandler OnClosed
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

        #endregion

        internal void HandleSCHDispose()
        {
            throw new Exception("The method or operation is not implemented.");
        }
    }
}

