/******************************************************************************
Stack.cs

begin      : April 16, 2007
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#define MULTITHREAD

using System;
using System.Linq;
using System.Collections.Generic;
using Pyxis.Utilities;
using PyxNet.Publishing.Files;

namespace PyxNet
{
    /// <summary>
    /// Delegate type for handling messages that arrive on a stack connection.
    /// </summary>
    /// <param name="stack">The stack that received the message.</param>
    /// <param name="connection">
    /// The connection that was involved in this message.  Will be null when not applicable.
    /// </param>
    /// <param name="message">The message that came in.</param>
    public delegate void StackMessageHandler(Stack stack, StackConnection connection, Message message);

    /// <summary>
    /// Manages a list of StackConnection objects, and handles PYXNet messages which
    /// are pan connection.
    /// </summary>
    public partial class Stack : MessageHandlerCollection, IDisposable
    {
        #region Tracer

        private readonly Pyxis.Utilities.NumberedTraceTool<Stack> m_tracer
            = new Pyxis.Utilities.NumberedTraceTool<Stack>(Pyxis.Utilities.TraceTool.GlobalTraceLogEnabled);

        new public Pyxis.Utilities.NumberedTraceTool<Stack> Tracer
        {
            get
            {
                return m_tracer;
            }
        }

        #endregion

        #region Constants

        /// <summary>
        /// The default ping timer interval, in milliseconds.
        /// </summary>
        public const long DefaultPingTimerInterval = 90000;

        /// <summary>
        /// The default LNI timer interval, in milliseconds.
        /// </summary>
        const long DefaultLNITimerInterval = 90000;

        /// <summary>
        /// The default LNI maximum send time, in milliseconds.
        /// </summary>
        const long DefaultLNIMaxTime = 360000;

        /// <summary>
        /// The default KHL timer interval, in milliseconds.
        /// </summary>
        const long DefaultKHLTimerInterval = 180000;

        /// <summary>
        /// The default QHaT timer interval, in milliseconds.
        /// </summary>
        const long DefaultQHaTTimerInterval = 120000;

        #endregion

        #region Locks

        /// <summary>
        /// The manipulation of all timers is controlled by this lock.
        /// </summary>
        private object m_timerLock = new object();

        #endregion

        #region Constructors

        /// <summary>
        /// Constructor specifying all allowable parameters.
        /// </summary>
        /// <param name="address">The address(es) to listen for connections on.</param>
        /// <remarks>All constructors end up here.</remarks>
        private Stack(
            PyxNet.DLM.PrivateKey privateKey,
            Guid nodeID,
            long pingTimerInterval,
            long lniTimerInterval,
            long lniMaxTime,
            long khlTimerInterval,
            long qhtTimerInterval)
        {
            if (null == privateKey)
            {
                throw new ArgumentNullException("privateKey");
            }

            Status = new StatusMessage("PyxNet stack initializing.");

            ConnectionManager = new ConnectionManagerHelper(this);

            // Create known hub list.
            m_knownHubList = new KnownHubList(this);

            // Set timer intervals.
            m_pingTimerInterval = pingTimerInterval;
            m_lniTimerInterval = lniTimerInterval;
            m_khlTimerInterval = khlTimerInterval;
            m_lniMaxTime = lniMaxTime;
            m_qhtTimerInterval = qhtTimerInterval;

            if (nodeID != Guid.Empty)
            {
                NodeInfo.NodeGUID = nodeID;
                Tracer.DebugWriteLine("Setting stack {0} node id to {1}.", ToString(), nodeID);
            }

            // Store the private key.
            m_privateKey = privateKey;
            NodeInfo.PublicKey = m_privateKey.PublicKey;

            // set up the timed event for Local Node Info propagation.
            m_sendLocalNodeInfoTimer.Interval = m_lniTimerInterval;
            m_sendLocalNodeInfoTimer.Elapsed += SendLocalNodeInfoTimerElapsed;

            // set up the event for Pings to keep the network alive.
            m_sendPingTimer.Interval = m_pingTimerInterval;
            m_sendPingTimer.Elapsed += SendPingTimerElapsed;

            m_sendKnownHubListTimer.Elapsed += SendKnownHubListTimerElapsed;
            m_sendKnownHubListTimer.AutoReset = false;

            m_sendHashTablesTimer.Elapsed += SendHashTablesTimerElapsed;
            m_sendHashTablesTimer.AutoReset = false;

            // monitor our local query Hash Table so that changes can be propagated.
            m_localQueryHashTable.OnChange += HandleLocalQueryHashTableChange;

            // Handle connection creation messages from any connection.
            RegisterHandler(StackConnector.MessageID, HandleConnectorMessage);

            RegisterHandler(StatusMessageRequest.MessageId, HandleStatusMessageRequest);

            // Handle query results from any connection.
            RegisterHandler(QueryResultMessageID, HandleQueryResultMessage);

            // Handle message relay from any connection.
            RegisterHandler(MessageRelay.MessageID, HandleMessageRelay);

            // Handle encrypted messages...
            RegisterHandler(EncryptedMessageHelper.MessageID, HandleEncryptedMessage);
            // Handle signed messages...
            RegisterHandler(SignedMessageHelper.MessageID, HandleSignedMessage);
            // Handle signed, encrypted messages...
            EncryptedStreamRegisterHandler(SignedMessageHelper.MessageID, HandleSecureMessage);

            m_publisher = new PyxNet.Publishing.Publisher(this);

            this.NodeInfo.ModeChanged += ModeChanged;

            m_filePublisher = new FilePublisher(this);

            CreatePerformanceEvents();

            Status.Text = "PyxNet stack initialized.";

            CertificateProvider = new NullCertificateProvider();
            CertificateValidator = new PermissiveCertificateValidator();
        }

        /// <summary>
        /// Construct using all the times but assign a default Node ID.
        /// </summary>
        /// <param name="privateKey"></param>
        /// <param name="pingTimerInterval"></param>
        /// <param name="lniTimerInterval"></param>
        /// <param name="lniMaxTime"></param>
        /// <param name="khlTimerInterval"></param>
        /// <param name="qhtTimerInterval"></param>
        public Stack(
            PyxNet.DLM.PrivateKey privateKey,
            long pingTimerInterval,
            long lniTimerInterval,
            long lniMaxTime,
            long khlTimerInterval,
            long qhtTimerInterval)
            : this(privateKey, new Guid(),
                pingTimerInterval, lniTimerInterval, lniMaxTime,
                khlTimerInterval, qhtTimerInterval)
        {
        }

        /// <summary>
        /// Construct the stack using default constant values for timers.
        /// </summary>
        /// <param name="address"></param>
        /// <param name="privateKey"></param>
        public Stack(PyxNet.DLM.PrivateKey privateKey)
            : this(privateKey, new Guid(),
                DefaultPingTimerInterval, DefaultLNITimerInterval, DefaultLNIMaxTime,
                DefaultKHLTimerInterval, DefaultQHaTTimerInterval)
        {
        }

        /// <summary>
        /// Construct the stack using default constant values for timers.
        /// </summary>
        /// <param name="privateKey"></param>
        public Stack(PyxNet.DLM.PrivateKey privateKey, Guid nodeID)
            : this(privateKey, nodeID,
                DefaultPingTimerInterval, DefaultLNITimerInterval, DefaultLNIMaxTime,
                DefaultKHLTimerInterval, DefaultQHaTTimerInterval)
        {
        }

        #endregion

        #region ToString

        /// <summary>
        /// Get a friendly name for the stack.
        /// </summary>
        /// <returns>A friendly name.</returns>
        public override string ToString()
        {
            NodeInfo nodeInfo = this.NodeInfo;
            if (nodeInfo != null)
            {
                return nodeInfo.ToString();
            }
            return "";
        }

        /// <summary>
        /// Get detailed information about the state of the stack.
        /// </summary>
        /// <returns>A human readable string with the state of the stack described.</returns>
        public string ToStringVerbose()
        {
            System.Text.StringBuilder output = new System.Text.StringBuilder();
            output.AppendLine(CurrentConnectionStatus);

            NodeInfo nodeInfo = this.NodeInfo;
            if (nodeInfo == null)
            {
                output.AppendLine("We have no node info!");
            }
            else
            {
                output.AppendLine(nodeInfo.ToStringVerbose());
            }

            // TODO: dump out more information about this stack.
            return output.ToString();
        }

        #endregion


        #region IDisposable

        /// <summary>
        /// Disposes the object.
        /// </summary>
        /// <remarks>
        /// Do not make this method virtual.
        /// A derived class should not be able to override this method. 
        /// Follows the Dispose pattern:
        /// http://msdn.microsoft.com/en-us/library/b1yfkh5e.aspx
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
        /// <remarks>
        /// Follows the Dispose pattern:
        /// http://msdn.microsoft.com/en-us/library/b1yfkh5e.aspx
        /// </remarks>
        protected virtual void Dispose(bool disposing)
        {
            if (disposing) // Dispose managed resources.
            {
                // Stop handling messages.
                UnregisterHandlers();

                // Remove handler for local query hash table change.
                LocalQueryHashTable.OnChange -= HandleLocalQueryHashTableChange;

                // Stop accepting new connections.
                Stop();

                Publisher.ClearItems();

                // Stop sending timer-based messages.
                lock (m_timerLock)
                {
                    m_sendLocalNodeInfoTimer.Elapsed -= SendLocalNodeInfoTimerElapsed;
                    m_sendLocalNodeInfoTimer.Enabled = false;

                    {
                        System.Timers.Timer sendKnownHubListTimer = m_sendKnownHubListTimer;
                        m_sendKnownHubListTimer = null;
                        if (null != sendKnownHubListTimer)
                        {
                            sendKnownHubListTimer.Elapsed -= SendKnownHubListTimerElapsed;
                            sendKnownHubListTimer.Enabled = false;
                        }
                    }

                    m_sendPingTimer.Elapsed -= SendPingTimerElapsed;
                    m_sendPingTimer.Enabled = false;

                    {
                        System.Timers.Timer sendHashTablesTimer = m_sendHashTablesTimer;
                        m_sendHashTablesTimer = null;
                        if (null != sendHashTablesTimer)
                        {
                            sendHashTablesTimer.Elapsed -= SendHashTablesTimerElapsed;
                            sendHashTablesTimer.Enabled = false;
                        }
                    }
                }

                ConnectionManager.Dispose(true);

                foreach (var mapping in m_portMappings)
                {
                    mapping.Dispose();
                }

                m_tcpNetwork.Dispose();
            }
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
        /// Follows the Dispose pattern:
        /// http://msdn.microsoft.com/en-us/library/b1yfkh5e.aspx
        /// </remarks>
        ~Stack()
        {
            // Do not re-create Dispose clean-up code here.
            // Calling Dispose(false) is optimal in terms of
            // readability and maintainability.
            Dispose(false);
        }

        #endregion

        #region Port Mapping

        private DynamicList<NatTraversal.PortMapping> m_portMappings = new DynamicList<NatTraversal.PortMapping>();

        /// <summary>
        /// A blocking function that gets an external port mapping, creating if necessary.
        /// </summary>
        /// <param name="internalIPEndPoint">The internal IP endpoint to map.</param>
        /// <returns>The external IP endpoint mapped.</returns>
        private System.Net.IPEndPoint GetExternalPortMapping(
            System.Net.IPEndPoint internalIPEndPoint)
        {
            //there is not need to get external ip for a loopback address
            if (System.Net.IPAddress.IsLoopback(internalIPEndPoint.Address))
            {
                return null;
            }

            try
            {
                NatTraversal.PortMapping newMapping = NatTraversal.PortMapping.Create(internalIPEndPoint, NodeInfo.NodeGUID.ToString());
                m_portMappings.Add(newMapping);
                return newMapping.ExternalIPEndPoint;
            }
            catch (Exception ex)
            {
                // creating an outside port mapping failed.
                StackSingleton.Stack.Tracer.DebugWriteLine("Exception in port mapping: {0}", ex.ToString());
            }
            return null;
        }

        /// <summary>
        /// Get external port mappings to internal IP end points, creating if necessary.
        /// </summary>
        public void GetExternalPortMappings()
        {
            NetworkAddress address = NodeInfo.Address;

            Action<System.Net.IPEndPoint> mapToExternal = delegate(System.Net.IPEndPoint internalIPEndPoint)
            {
#if MULTITHREAD
                System.Threading.Thread thread = new System.Threading.Thread(delegate()
                {
#endif
                    // Get an external port mapping, create one if necessary.
                    System.Net.IPEndPoint externalIPEndPoint = GetExternalPortMapping(internalIPEndPoint);
                    if (null != externalIPEndPoint)
                    {
                        System.Diagnostics.Trace.WriteLine(String.Format(
                            "PYXNATTraversal: Obtained a mapping from external port {1} to internal port {0}",
                            internalIPEndPoint.ToString(), externalIPEndPoint.ToString()));

                        lock (address.ExternalIPEndPoints)
                        {
                            if (!address.ExternalIPEndPoints.Contains(externalIPEndPoint))
                            {
                                address.ExternalIPEndPoints.Add(externalIPEndPoint);
                                m_localNodeInfoChanged = true;
                                OnNetworkAddressChanged();
                                Logging.Categories.Stack.Log("UPNP", String.Format(
                                        "external port: {1} to internal port: {0}",
                                        internalIPEndPoint.ToString(), externalIPEndPoint.ToString()));
                            }
                        }
                    }
                    else
                    {
                        System.Diagnostics.Trace.WriteLine(
                            "PYXNATTraversal: Failed to map an external port.");

                        Logging.Categories.Stack.Log("UPNP", "failed");
                    }
#if MULTITHREAD
                });
                thread.IsBackground = true;
                thread.Start();
#endif
            };

            System.Diagnostics.Trace.WriteLine(
              "PYXNATTraversal: Attempting to map external port.");

            lock (address.InternalIPEndPoints)
            {
                address.InternalIPEndPoints.ForEach(mapToExternal);
            }
        }

        #region NetworkAddressChanged Event
        /// <summary> EventArgs for a NetworkAddressChanged event. </summary>    
        public class NetworkAddressChangedEventArgs : EventArgs
        {
            private Stack m_Stack;

            /// <summary>The Stack.</summary>
            public Stack Stack
            {
                get { return m_Stack; }
                set { m_Stack = value; }
            }

            internal NetworkAddressChangedEventArgs(Stack theStack)
            {
                m_Stack = theStack;
            }
        }

        /// <summary> Event handler for NetworkAddressChanged. </summary>
        public event EventHandler<NetworkAddressChangedEventArgs> NetworkAddressChanged
        {
            add
            {
                m_NetworkAddressChanged.Add(value);
            }
            remove
            {
                m_NetworkAddressChanged.Remove(value);
            }
        }
        private Pyxis.Utilities.EventHelper<NetworkAddressChangedEventArgs> m_NetworkAddressChanged = new Pyxis.Utilities.EventHelper<NetworkAddressChangedEventArgs>();

        /// <summary>
        /// Raises the NetworkAddressChanged event.
        /// </summary>
        private void OnNetworkAddressChanged()
        {
            m_NetworkAddressChanged.Invoke(this, new NetworkAddressChangedEventArgs(this));
        }
        #endregion NetworkAddressChanged Event

        #endregion

        #region Network

        public ConnectionManagerHelper ConnectionManager { get; private set; }

        /// <summary>
        /// The TCP/IP network object that we use for listening for connections,
        /// and for making out going connections.
        /// </summary>
        private readonly TcpNetwork m_tcpNetwork = new TcpNetwork();

        /// <summary>
        /// Handler to receive incoming connections from the network.
        /// </summary>
        /// <param name="sender">The network that sent to connection.</param>
        /// <param name="newConnection">The connection that was just established.</param>
        void OnConnectNetwork(object sender, ConnectionEventArgs args)
        {
            StackConnection connection = new StackConnection(args.Connection);

            // Until we receive a connection request, we don't
            // know whether the other side is temporary or persistent.
            // That means this is a pending connection.
            connection.Tracer.DebugWriteLine("{0} adding incoming connection.", NodeInfo.FriendlyName);
            ConnectionManager.AddConnection(connection);
        }

        private readonly Object m_startStopLock = new Object();
        private bool m_isStarted = false;

        public bool IsStarted
        {
            get
            {
                lock (m_startStopLock)
                {
                    return m_isStarted;
                }
            }
        }

        /// <summary>
        /// Start listening.
        /// This can throw a SocketException.
        /// </summary>
        /// <returns>
        /// True if successful; false if unsucessful
        /// (i.e. couldn't start or already started).
        /// </returns>
        public bool Start(NetworkAddress address)
        {
            lock (m_startStopLock)
            {
                if (m_isStarted)
                {
                    return false;
                }

                try
                {
                    // Set the address.
                    NodeInfo.Address = address;

                    lock (m_timerLock)
                    {
                        // Start the timed event for Local Node Info propagation.
                        m_sendLocalNodeInfoTimer.Enabled = true;

                        // Start the timed event for Pings to keep the network alive.
                        m_sendPingTimer.Enabled = true;
                    }

                    // set up the network to enable incoming connections
                    m_tcpNetwork.ConnectionOpened += OnConnectNetwork;
                    m_tcpNetwork.SetListeners(address);

                    PublishNodeInfo(this.NodeInfo);

                    m_isStarted = true;
                }
                catch
                {
                    Stop();
                    throw;
                }
            }

            return true;
        }

        /// <summary>
        /// Stop listening.
        /// </summary>
        /// <returns>
        /// True if successful; false if unsucessful
        /// (i.e. couldn't stop or already stopped).
        /// </returns>
        public bool Stop()
        {
            lock (m_startStopLock)
            {
                if (!m_isStarted)
                {
                    return false;
                }

                lock (m_timerLock)
                {
                    // Stop the timed event for Local Node Info propagation.
                    m_sendLocalNodeInfoTimer.Enabled = false;

                    // Stop the timed event for Pings that keep the network alive.
                    m_sendPingTimer.Enabled = false;
                }

                m_tcpNetwork.ResetListeners();
                m_tcpNetwork.ConnectionOpened -= OnConnectNetwork;

                m_publisher.Stop();

                m_isStarted = false;
            }

            return true;
        }
        #endregion Connections

        #region Any Message

        #region To Connection

        /// <summary>
        /// Called when a message is sent to a connection; fires the "OnSendingMessageToConnection" event.
        /// </summary>
        /// <param name="connection">The connection receiving the message.</param>
        /// <param name="message">The message being sent.</param>
        void HandleSendingMessage(StackConnection connection, Message message)
        {
            // Fire "on sending message to connection" event.
            StackMessageHandler handler = m_onSendingMessageToConnection;
            if (null != handler)
            {
                handler(this, connection, message);
            }
        }

        /// <summary>
        /// Stores the event handler for OnSendingConnectionMessage event.
        /// </summary>
        private event StackMessageHandler m_onSendingMessageToConnection;
        private object m_sendingMessageToConnectionEventLock = new object();

        /// <summary>
        /// Fired just before any message is sent through the stack to an individual 
        /// connection.
        /// </summary>
        public event StackMessageHandler OnSendingMessageToConnection
        {
            add
            {
                lock (m_sendingMessageToConnectionEventLock)
                {
                    m_onSendingMessageToConnection += value;
                }
            }

            remove
            {
                lock (m_sendingMessageToConnectionEventLock)
                {
                    m_onSendingMessageToConnection -= value;
                }
            }
        }

        #region AnyMessage Event

        /// <summary>
        /// Class which will be passed as the second argument to a AnyMessageHandler which 
        /// wraps a Stack object.
        /// </summary>
        public class AnyMessageEventArgs : EventArgs
        {
            private Stack m_Stack;

            public Stack Stack
            {
                get { return m_Stack; }
                set { m_Stack = value; }
            }

            private StackConnection m_connection;

            public StackConnection Connection
            {
                get { return m_connection; }
                set { m_connection = value; }
            }

            private Message m_message;

            public Message Message
            {
                get { return m_message; }
                set { m_message = value; }
            }

            internal AnyMessageEventArgs(Stack theStack, StackConnection theConnection, Message theMessage)
            {
                m_Stack = theStack;
                m_connection = theConnection;
                m_message = theMessage;
            }
        }

        /// <summary>
        /// Event which is fired when any message comes into the Stack.
        /// </summary>
        public event EventHandler<AnyMessageEventArgs> OnAnyMessage
        {
            add
            {
                m_OnAnyMessage.Add(value);
            }
            remove
            {
                m_OnAnyMessage.Remove(value);
            }
        }
        private Pyxis.Utilities.EventHelper<AnyMessageEventArgs> m_OnAnyMessage = new Pyxis.Utilities.EventHelper<AnyMessageEventArgs>();

        /// <summary>
        /// Method to safely raise event OnAnyMessage.
        /// </summary>
        protected void OnAnyMessageRaise(StackConnection theConnection, Message theMessage)
        {
            m_OnAnyMessage.Invoke(this, new AnyMessageEventArgs(this, theConnection, theMessage));
        }

        void HandleAnyMessage(StackConnection connection, Message message)
        {
            OnAnyMessageRaise(connection, message);
        }

        #endregion AnyMessage Event

        void HandleUnknownStackConnectionMessage(StackConnection connection, Message message)
        {
            HandleMessage(this, new MessageReceivedEventArgs(message, new MessageContext(connection)));
        }

        #endregion

        #region To All

        /// <summary>
        /// Use to send the same message to all current connections.
        /// Will fire the OnSendingMessageToAll event before sending.
        /// </summary>
        /// <param name="message">The message to be sent.</param>
        public void SendMessage(Message message)
        {
            StackMessageHandler allHandler = m_onSendingMessageToAll;
            if (allHandler != null)
            {
                allHandler(this, null, message);
            }

            ConnectionManager.SendMessage(message);
        }

        /// <summary>
        /// Stores the event handler for OnSendingToAllMessage event.
        /// </summary>
        private event StackMessageHandler m_onSendingMessageToAll;
        private object m_sendingMessageToAllEventLock = new object();

        /// <summary>
        /// Fired just before any message is sent through the stack to all connection.
        /// </summary>
        public event StackMessageHandler OnSendingMessageToAll
        {
            add
            {
                lock (m_sendingMessageToAllEventLock)
                {
                    m_onSendingMessageToAll += value;
                }
            }

            remove
            {
                lock (m_sendingMessageToAllEventLock)
                {
                    m_onSendingMessageToAll -= value;
                }
            }
        }

        #endregion

        #endregion

        #region Connection Creator

        public void HandleConnectorMessage(object sender, MessageReceivedEventArgs args)
        {
            // Reconstitute the object from the message.
            StackConnector creator = StackConnector.FromMessage(args.Message);

            Tracer.WriteLine("Handling Conn message from {0} id {2} at address {1}.",
                creator.ToNode.ToString(), creator.ToNode.Address.ToString(),
                creator.ToNode.NodeGUID);

            // Get a connection to the node specified.
            // Always make this a persistent connection, because a temporary connection will go out of scope
            // by the end of this function.
            StackConnection connection = GetConnection(creator.ToNode, true, TimeSpan.Zero);
            if (connection == null)
            {
                Tracer.WriteLine("Could not get connection from {0} to {1}.", this.ToString(), creator.ToNode.ToString());
            }
        }

        #endregion

        #region Status Message

        /// <summary>
        /// Gets or sets the status.  This status will be sent to anyone who requests it.
        /// </summary>
        /// <value>The status.</value>
        public StatusMessage Status { get; set; }

        /// <summary>
        /// Handles the status message request by sending the status immediately.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="args">The <see cref="PyxNet.MessageHandlerCollection.MessageReceivedEventArgs"/> instance containing the event data.</param>
        public void HandleStatusMessageRequest(object sender, MessageReceivedEventArgs args)
        {
            args.Context.Sender.SendMessage(Status.ToMessage());
        }

        #endregion /* Status Message */


        #region Ping

        /// <summary>
        /// The time between sending Ping messages
        /// in 1000ths of a second.
        /// </summary>
        private long m_pingTimerInterval;

        /// <summary>
        /// Timer used to propagate Ping messages through the stack.
        /// </summary>
        private readonly System.Timers.Timer m_sendPingTimer = new System.Timers.Timer();

        /// <summary>
        /// Periodic event to propagate the Ping message from the stack.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void SendPingTimerElapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
#if TRACE_TIMED_PING
            Tracer.DebugWriteLine("Sending timed ping.");
#endif
            ConnectionManager.SendPing();
        }

        #endregion

        #region Performance Counters

        /// <summary>
        /// The performance counters for the Stack.
        /// </summary>
        private volatile Pyxis.Utilities.PyxStackPerformanceCounters m_performanceCounters;

        /// <summary>
        /// Gets the performance counters.
        /// </summary>
        /// <value>The performance counters.</value>
        public Pyxis.Utilities.PyxStackPerformanceCounters PerformanceCounters
        {
            get
            {
                CreatePerformanceCounters();
                return m_performanceCounters;
            }
        }

        /// <summary>
        /// A object to lock which controls creation of the Performance Counters.
        /// </summary>
        private object m_performanceCountersCreationLock = new object();

        /// <summary>
        /// We remember the name that we used to create the counters, so that
        /// if the name changes we can create new counters to reflect that change.
        /// </summary>
        private string m_counterFriendlyName = "";

        /// <summary>
        /// Creates the performance events.
        /// </summary>
        private void CreatePerformanceEvents()
        {
            ConnectionManager.CreatePerformanceEvents();
        }

        /// <summary>
        /// Creates the performance counters.
        /// </summary>
        private void CreatePerformanceCounters()
        {
            if (m_performanceCounters == null || NodeInfo.FriendlyName != m_counterFriendlyName)
            {
                lock (m_performanceCountersCreationLock)
                {
                    if (m_performanceCounters == null || NodeInfo.FriendlyName != m_counterFriendlyName)
                    {
                        string name = NodeInfo.FriendlyName;
                        m_counterFriendlyName = name;
                        if (name == "")
                        {
                            name = "Unknown";
                        }
                        name = "Stack: " + name;

                        m_performanceCounters = new PyxStackPerformanceCounters(name);

                        // initialize the correct values
                        ConnectionManager.InitializePerformanceCounters();
                    }
                }
            }
        }
        #endregion

        #region Publishing Support

        private readonly Publishing.Publisher m_publisher;

        /// <summary>
        /// A simple publisher attached to this Stack.
        /// </summary>
        public Publishing.Publisher Publisher
        {
            get { return m_publisher; }
        }

        private readonly FilePublisher m_filePublisher;

        /// <summary>
        /// Gets the file publisher.
        /// TODO: Refactor this to use the Publisher?
        /// </summary>
        /// <value>The file publisher.</value>
        public FilePublisher FilePublisher
        {
            get { return m_filePublisher; }
        }
        #endregion

        #region Node Info
        /// <summary>
        /// Storage for our local node info.
        /// </summary>
        private NodeInfo m_localNodeInfo = new NodeInfo();

        /// <summary>
        /// True if our local node info has changed since we last sent it
        /// out across the PYXNet.
        /// </summary>
        private bool m_localNodeInfoChanged = false;

        /// <summary>
        /// Timer used to propagate Local Node Info messages through the stack.
        /// </summary>
        private readonly System.Timers.Timer m_sendLocalNodeInfoTimer = new System.Timers.Timer();

        /// <summary>
        /// The minimum time between sending Local Node Info messages
        /// in 1000ths of a second.
        /// </summary>
        private long m_lniTimerInterval;

        /// <summary>
        /// The maximum time between sending Local Node Info messages 
        /// in 1000ths of a second.
        /// </summary>
        private long m_lniMaxTime;

        /// <summary>
        /// The last time that a LocalNodeInfo message was sent.
        /// </summary>
        private DateTime m_timeLNISent;

        /// <summary>
        /// The Local Node Info that we broadcast to our connected nodes.
        /// </summary>
        public NodeInfo NodeInfo
        {
            get { return m_localNodeInfo; }
            set
            {
                m_localNodeInfo = value;
                m_localNodeInfoChanged = true;
                PublishNodeInfo(m_localNodeInfo);

                Pyxis.Utilities.Logging.LogRepository.SetLocalNodeInfo(m_localNodeInfo.FriendlyName, m_localNodeInfo.NodeGUID);
                Logging.Categories.Stack.Log("NodeInfo", NodeInfo.ToStringVerbose());
            }
        }

        /// <summary>
        /// Periodic event to enable the propagation of Local Node Info
        /// messages through the stack.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void SendLocalNodeInfoTimerElapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            SendLocalNodeInfoMessage();
        }

        /// <summary>
        /// Send the Local Node Info on to all Connected nodes if it is time to do it.
        /// </summary>
        private void SendLocalNodeInfoMessage()
        {
            TimeSpan elapsedTime = DateTime.Now - m_timeLNISent;
            bool timeToSendAnyway = (elapsedTime.TotalMilliseconds >= m_lniMaxTime);
            if (m_localNodeInfoChanged || timeToSendAnyway)
            {
#if TRACE_TIMED_LNI
                Tracer.DebugWriteLine("Sending timed LNI message.");
#endif
                m_localNodeInfoChanged = false;
                SendMessage(NodeInfo.ToMessage());
                m_timeLNISent = DateTime.Now;
            }
        }

        /// <summary>
        /// This is the handler for a "local node info" message.
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="message"></param>
        void HandleLocalNodeInfoMessage(StackConnection connection, Message message)
        {
            if ((connection == null) || (connection.RemoteNodeInfo == null))
            {
                Tracer.WriteLine("A remote node has sent a node-info message, " +
                    "but that message doesn't contain a node info.");
                return;
            }

            if (NodeInfo.Equals(connection.RemoteNodeInfo))
            {
                // It is a connection to this stack.
                Tracer.WriteLine("The stack appears to have a connection to itself.");
                return;
            }

            lock (m_lockKnownHubList)
            {
                if (m_knownHubList.Add(connection.RemoteNodeInfo))
                {
                    // Because the hub list has changed, send "update known hub list" message.
                    SendKnownHubListMessage();

                    SendQHTToConnection(connection);
                }
            }
        }

        /// <summary>
        /// Handler for "mode changed" event.
        /// </summary>
        /// <param name="sender">The event sender.</param>
        /// <param name="e">The event arguments.</param>
        private void ModeChanged(object sender, NodeInfo.ModeChangedEventArgs e)
        {
            switch (e.OperatingMode)
            {
                case NodeInfo.OperatingMode.Unknown:
                    break;
                case NodeInfo.OperatingMode.Hub:
                    Tracer.DebugWriteLine("Node has been promoted to hub with {0} connections.",
                        ConnectionManager.PersistentConnections.Count);
                    SendQueryHashTable();
                    break;
                case NodeInfo.OperatingMode.Leaf:
                    // Clear away the old amalgamated hash table.
                    m_amalgamatedQueryHashTable = null;
                    SendQueryHashTable();
                    break;
                default:
                    break;
            }
        }
        /// <summary>
        /// Publish the node's info
        /// </summary>
        /// <param name="nodeInfo"></param>
        private void PublishNodeInfo(NodeInfo nodeInfo)
        {
            var publishedNodeInfo = m_publisher.FindPublishedItemsByType<Publishing.PublishedNodeInfo>()
                .FirstOrDefault() as Publishing.PublishedNodeInfo;
            if (publishedNodeInfo == null)
            {
                m_publisher.PublishItem(new Publishing.PublishedNodeInfo(nodeInfo));
            }
            else
            {
                publishedNodeInfo.NodeInfo = this.NodeInfo;
            }
        }
        #endregion

        #region Private Key

        private readonly PyxNet.DLM.PrivateKey m_privateKey;

        /// <summary>
        /// Each node in the network has a private key.
        /// </summary>
        public PyxNet.DLM.PrivateKey PrivateKey
        {
            get
            {
                return m_privateKey;
            }
        }

        #region EncryptedMessageStream

        /// <summary>
        /// Note that we don't want to allow "just anyone" to place a message 
        /// into the encrypted message stream, so we make it private.  Only
        /// HandleEncryptedMessage should add new events to this stream.
        /// </summary>
        private MessageHandlerCollection EncryptedMessageStream = new MessageHandlerCollection();

        /// <summary>
        /// An encrypted message has been received.  Decrypt it using this node's private
        /// key, and pass it along on the (protected) EncryptedMessageStream.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void HandleEncryptedMessage(object sender, MessageReceivedEventArgs args)
        {
            Tracer.DebugWriteLine("Handling encrypted message {0}.", args.Message.Identifier);
            try
            {
                // Decrypt the message (will throw if decryption fails.)
                Message decryptedMessage = EncryptedMessageHelper.DecryptMessage(args.Message, PrivateKey);

                /// Then pass the decrypted message on.  Note that sender and 
                /// context are unchanged.
                Tracer.DebugWriteLine(
                    "Handling decrypted message {0} (from encrypted {1}).",
                    decryptedMessage.Identifier, args.Message.Identifier);
                EncryptedMessageStream.HandleMessage(sender,
                    new MessageReceivedEventArgs(decryptedMessage, args.Context));
            }
            catch (System.Security.Cryptography.CryptographicException c)
            {
                System.Diagnostics.Trace.WriteLine(
                    String.Format("Error decrypting message from {0}: {1}",
                    args.Context.Sender, c.Message));
                //throw;
            }
        }

        public void EncryptedStreamRegisterHandler(string messageIdentifier, EventHandler<MessageReceivedEventArgs> handler)
        {
            EncryptedMessageStream.RegisterHandler(messageIdentifier, handler);
        }

        public void EncryptedStreamUnregisterHandler(string messageIdentifier, EventHandler<MessageReceivedEventArgs> handler)
        {
            EncryptedMessageStream.UnregisterHandler(messageIdentifier, handler);
        }

        /// <summary>
        /// Sends a message to a specific node, encrypted with that node's public key.
        /// </summary>
        /// <param name="recipient">The stack-connection to send the message to.</param>
        /// <param name="message">The message to be sent.</param>
        public bool SendEncryptedMessage(StackConnection recipient, Message message)
        {
            if (null == recipient)
            {
                return false;
            }

            Message encryptedMessage =
                EncryptedMessageHelper.EncryptMessage(message, recipient.RemoteNodeInfo);
            return recipient.SendMessage(encryptedMessage);
        }

        /// <summary>
        /// Sends a message to a specific node, encrypted with that node's public key.
        /// Uses RelayMessage to do the actual sending.
        /// </summary>
        /// <param name="recipient">The node to send the message to.</param>
        /// <param name="message">The message to be sent.</param>
        public bool SendEncryptedMessage(NodeInfo recipient, Message message)
        {
            if (null == recipient)
            {
                return false;
            }

            Message encryptedMessage =
                EncryptedMessageHelper.EncryptMessage(message, recipient);
            RelayMessage(recipient, encryptedMessage);
            return true;
        }

        #endregion /* EncryptedMessageStream */

        #region SignedMessageStream

        /// <summary>
        /// Note that we don't want to allow "just anyone" to place a message 
        /// into the signed message stream, so we make it private.  Only
        /// HandleSignedMessage should add new events to this stream.
        /// </summary>
        private MessageHandlerCollection SignedMessageStream = new MessageHandlerCollection();

        /// <summary>
        /// An encrypted message has been received.  Decrypt it using this node's private
        /// key, and pass it along on the (protected) SignedMessageStream.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void HandleSignedMessage(object sender, MessageReceivedEventArgs args)
        {
            try
            {
                // Verify the signature (will throw if verification fails.)
                Message decryptedMessage = SignedMessageHelper.VerifySignedMessage(args.Message,
                    args.Context.Sender.RemoteNodeInfo);

                /// Then pass the embedded message on.  Note that sender and 
                /// context are unchanged.
                SignedMessageStream.HandleMessage(sender,
                    new MessageReceivedEventArgs(decryptedMessage, args.Context));
            }
            catch (System.Security.Cryptography.CryptographicException c)
            {
                System.Diagnostics.Trace.WriteLine(
                    String.Format("Error decrypting message from {0}: {1}",
                    args.Context, c.Message));
                //throw;
            }
        }

        public void SignedStreamRegisterHandler(string messageIdentifier, EventHandler<MessageReceivedEventArgs> handler)
        {
            SignedMessageStream.RegisterHandler(messageIdentifier, handler);
        }

        public void SignedStreamUnregisterHandler(string messageIdentifier, EventHandler<MessageReceivedEventArgs> handler)
        {
            SignedMessageStream.UnregisterHandler(messageIdentifier, handler);
        }

        /// <summary>
        /// Signs a message with this node's credentials, and sends it on to a specific recipient.
        /// </summary>
        /// <param name="recipient">The stack-connection to send the message to.</param>
        /// <param name="message">The message to be sent.</param>
        public void SendSignedMessage(StackConnection recipient, Message message)
        {
            Message signedMessage =
                SignedMessageHelper.SignMessage(message, this.NodeInfo, this.PrivateKey);
            recipient.SendMessage(signedMessage);
        }

        #endregion /* SignedMessageStream */

        #region SecureMessageStream

        /// <summary>
        /// Note that we don't want to allow "just anyone" to place a message 
        /// into the encrypted message stream, so we make it private.  Only
        /// HandleSecureMessage should add new events to this stream.
        /// </summary>
        private MessageHandlerCollection SecureMessageStream = new MessageHandlerCollection();

        /// <summary>
        /// An encrypted message has been received.  Decrypt it using this node's private
        /// key, and pass it along on the (protected) SecureMessageStream.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void HandleSecureMessage(object sender, MessageReceivedEventArgs args)
        {
            Tracer.DebugWriteLine("Handling secure message.");

            try
            {
                // Verify the signature (will throw if verification fails.)
                Message decryptedMessage = SignedMessageHelper.VerifySignedMessage(args.Message,
                    args.Context.Sender.RemoteNodeInfo);

                /// Then pass the embedded message on.  Note that sender and 
                /// context are unchanged.
                SecureMessageStream.HandleMessage(sender,
                    new MessageReceivedEventArgs(decryptedMessage, args.Context));
            }
            catch (ArgumentException c)
            {
                System.Diagnostics.Trace.WriteLine(
                    String.Format("Error verifying signed message from {0}: {1}",
                    args.Context.Sender, c.ToString()));
                //throw;
            }
            catch (IndexOutOfRangeException c)
            {
                System.Diagnostics.Trace.WriteLine(
                    String.Format("Error verifying signed message from {0}: {1}",
                    args.Context.Sender, c.ToString()));
                //throw;
            }
        }

        public void SecureStreamRegisterHandler(string messageIdentifier, EventHandler<MessageReceivedEventArgs> handler)
        {
            SecureMessageStream.RegisterHandler(messageIdentifier, handler);
        }

        public void SecureStreamUnregisterHandler(string messageIdentifier, EventHandler<MessageReceivedEventArgs> handler)
        {
            SecureMessageStream.UnregisterHandler(messageIdentifier, handler);
        }

        /// <summary>
        /// Signs a message with this node's credentials, encrypts it with the 
        /// recipient's public key, and sends it on to that specific recipient.
        /// </summary>
        /// <param name="recipient">The stack-connection to send the message to.</param>
        /// <param name="message">The message to be sent.</param>
        public bool SendSecureMessage(StackConnection recipient, Message message)
        {
            Message signedMessage =
                SignedMessageHelper.SignMessage(message, this.NodeInfo, this.PrivateKey);
            return SendEncryptedMessage(recipient, signedMessage);
        }

        /// <summary>
        /// Signs a message with this node's credentials, encrypts it with the 
        /// recipient's public key, and sends it (using ForwardMessage) to that 
        /// specific recipient.
        /// </summary>
        /// <param name="recipient">The node to send the message to.</param>
        /// <param name="message">The message to be sent.</param>
        public bool SendSecureMessage(NodeInfo recipient, Message message)
        {
            Message signedMessage =
                SignedMessageHelper.SignMessage(message, this.NodeInfo, this.PrivateKey);
            return SendEncryptedMessage(recipient, signedMessage);
        }

        #endregion /* SecureMessageStream */

        #endregion

        #region All Known Nodes
        private ThreadSafeDictionary<NodeId, NodeInfo> m_allKnownNodes =
            new ThreadSafeDictionary<NodeId, NodeInfo>();

        /// <summary>
        /// Gets all known nodes.  This maps NodeId`s to the last NodeInfo that 
        /// matched it.
        /// </summary>
        /// <value>All known nodes.</value>
        public ThreadSafeDictionary<NodeId, NodeInfo> AllKnownNodes
        {
            get { return m_allKnownNodes; }
        }

        #endregion All Known Nodes

        #region Known Hub List

        /// <summary>
        /// Storage for our known hub list.
        /// </summary>
        private readonly KnownHubList m_knownHubList;

        public KnownHubList KnownHubList
        {
            get { return m_knownHubList; }
        }

        private readonly Object m_lockKnownHubList = new Object();

        /// <summary>
        /// Timer used to propagate Known Hub List messages through the stack.
        /// </summary>
        private System.Timers.Timer m_sendKnownHubListTimer = new System.Timers.Timer();

        /// <summary>
        /// The minimum time between sending Known Hub List messages
        /// in 1000ths of a second.
        /// </summary>
        private long m_khlTimerInterval;

        /// <summary>
        /// The last time that a KnownHubList message was sent.
        /// </summary>
        private DateTime m_timeKHLSent;

        /// <summary>
        /// Periodic event to enable the propagation of Known Hub List
        /// messages through the stack.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void SendKnownHubListTimerElapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            Tracer.DebugWriteLine("KHL timer elapsed.");
            SendKnownHubListMessage();
        }

        /// <summary>
        /// Send the Known Hub List on to all Connected nodes if it is time to do it.
        /// </summary>
        private void SendKnownHubListMessage()
        {
            bool sendMessage = false;

            lock (m_timerLock)
            {
                System.Timers.Timer sendKnownHubListTimer = m_sendKnownHubListTimer;
                if (null != sendKnownHubListTimer)
                {
                    TimeSpan elapsedTime = DateTime.Now - m_timeKHLSent;
                    if (elapsedTime.TotalMilliseconds < m_khlTimerInterval)
                    {
                        // If the timer is already enabled, do nothing.
                        // Otherwise, set the timer so that it will go off at the approriate time.
                        if (!sendKnownHubListTimer.Enabled)
                        {
                            // Set up the timed event for Known Hub List propagation.
                            sendKnownHubListTimer.Interval = (m_khlTimerInterval - elapsedTime.TotalMilliseconds);
                            sendKnownHubListTimer.Enabled = true;
                            Tracer.DebugWriteLine("Setting a timer for propagating KHL changes.");
                        }
                    }
                    else // Enough time has elapsed.  Send it.
                    {
                        // Turn off the timer.
                        // This will get turned on if we try to send another KHL 
                        // message before the minimum time between updates.
                        sendKnownHubListTimer.Enabled = false;

                        // Send the message.
                        sendMessage = true;
                    }
                }
            }

            if (sendMessage)
            {
                // Set the time sent, and send the message.
                // The time is set first to prevent multiple threads from sending a message at roughly the same time.
                Tracer.DebugWriteLine("Propagating KHL changes.");
                m_timeKHLSent = DateTime.Now;
                lock (m_lockKnownHubList)
                {
                    SendMessage(m_knownHubList.ToMessage());
                }
            }
        }

        void HandleKnownHubListMessage(StackConnection connection, Message message)
        {
            KnownHubList remoteKHL = connection.RemoteKnownHubList;

            System.Diagnostics.Debug.Assert(null != remoteKHL, "Null remote KHL in a KHL handler.");

            Tracer.DebugWriteLine("Received KHL message from {0}.", connection.ToString());

            if (null != remoteKHL)
            {
                lock (m_lockKnownHubList)
                {
                    if (m_knownHubList.Add(remoteKHL))
                    {
                        SendKnownHubListMessage();

                        // TODO: Remove this hack!  We need to update our QHT 
                        // whenever we connect to a hub.  This forces more
                        // updates.  (We should really do this when the connectedhubs 
                        // changes.)
                        SendQueryHashTable();
                    }
                }
            }
        }

        #endregion

        #region Query Hash Table

        /// <summary>
        /// Storage for the amalgamated query hash table.  
        /// This is only a valid if this stack is operating in hub mode. 
        /// It is basically a list of all of the query strings that 
        /// any leaf, or the hub itself, responds to.
        /// If a string is not in this list, then the string is not found for all the leaves of 
        /// this hub or this hub.
        /// </summary>
        private volatile QueryHashTable m_amalgamatedQueryHashTable = null;
        private object m_updatedQHT = new object();

        /// <summary>
        /// The amalgamated query hash table.
        /// This includes all strings that this node or its connected leaves, if any, 
        /// respond to.
        /// </summary>
        public QueryHashTable AmalgamatedQueryHashTable
        {
            get
            {
                if (NodeInfo.IsLeaf)
                {
                    return m_localQueryHashTable;
                }
                else
                {
                    if (m_amalgamatedQueryHashTable == null)
                    {
                        lock (m_updatedQHT)
                        {
                            // Did somebody else lock us out and update it?
                            if (m_amalgamatedQueryHashTable == null)
                            {
                                Tracer.DebugWriteLine("Updating amalgamatedQHT for '{0}'.", this.ToString());

                                // build the Amalgamated QHT
                                QueryHashTable amalgamatedQHT = new QueryHashTable();

                                amalgamatedQHT.Add(LocalQueryHashTable);

                                // Only amalgamate the persistently-connected leaves.
                                ConnectionManager.PersistentConnections.ForEach(delegate(StackConnection connection)
                                {
                                    if ((connection.RemoteNodeInfo != null) &&
                                        (connection.RemoteNodeInfo.IsLeaf))
                                    {
                                        amalgamatedQHT.Add(connection.RemoteQueryHashTable);
                                    }
                                });

                                // save the table we just generated for testing Queries against.
                                m_amalgamatedQueryHashTable = amalgamatedQHT;
                            }
                        }
                    }
                    return m_amalgamatedQueryHashTable;
                }
            }
        }

        /// <summary>
        /// Storage for the local query hash table for this node of PyxNet.
        /// It is basically a list of all of the query strings that 
        /// this node responds to, whether it's a hub or a node.
        /// Every node has one.
        /// </summary>
        private readonly QueryHashTable m_localQueryHashTable = new QueryHashTable();

        /// <summary>
        /// The Query Hash Table for this node of PyxNet.
        /// If a string does not hit this query table, then this node is
        /// not publishing data with this string.
        /// </summary>
        public QueryHashTable LocalQueryHashTable
        {
            get
            {
                return m_localQueryHashTable;
            }
        }

        /// <summary>
        /// Stores the event handler for OnQueryHashTable event.
        /// </summary>
        private event StackMessageHandler m_onQueryHashTable;
        private object m_queryHashTableEventLock = new object();

        /// <summary>
        /// Fired when a QueryHashTable message is recieved.
        /// </summary>
        public event StackMessageHandler OnQueryHashTable
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
        /// Event handler for when a Query Hash Table Message has arrived from one of
        /// our connections.
        /// </summary>
        /// <param name="connection">The connection that fired the event.</param>
        /// <param name="message">The message that came in.</param>
        private void HandleQueryHashTableMessage(StackConnection connection, Message message)
        {
            try
            {
                // if someone is hooked to the event, then fire it.
                StackMessageHandler handler = m_onQueryHashTable;
                if (handler != null)
                {
                    handler(this, connection, message);
                }
            }
            catch (System.ArgumentOutOfRangeException)
            {
                // TODO: the connection was not in our list of connections
                Tracer.DebugWriteLine(
                    "'{0}' received a Query Hash Table from a node that was not in it's connection list.",
                    NodeInfo.FriendlyName);
            }

            if (connection.RemoteNodeInfo == null)
            {
                Tracer.DebugWriteLine(
                    "'{0}' received a Query Hash Table before it had a remote node info from that connection.",
                    NodeInfo.FriendlyName);
            }

            // if we are a hub and this QHT comes from a leaf then propagate 
            // the changes that a node has sent us.
            if ((connection.RemoteNodeInfo != null) &&
               (connection.RemoteNodeInfo.IsLeaf) &&
               (NodeInfo.IsHub))
            {
                SendQueryHashTable();
            }
        }

        /// <summary>
        /// The query hash table for our node has changed, and we need to propagate changes.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void HandleLocalQueryHashTableChange(object sender, QueryHashTable.ChangeEventArgs args)
        {
            SendQueryHashTable();
        }

        /// <summary>
        /// The minimum time between sending Query Hash Table messages
        /// in 1000ths of a second.
        /// </summary>
        private long m_qhtTimerInterval;

        /// <summary>
        /// The minimum time between sending Query Hash Table messages
        /// in 1000ths of a second.
        /// </summary>
        public long QhtTimerInterval
        {
            get { return m_qhtTimerInterval; }
        }

        /// <summary>
        /// The last time that a Query Hash Table message was sent.
        /// </summary>
        private DateTime m_timeQHTSent;

        /// <summary>
        /// Timer used to propagate Ping messages through the stack.
        /// </summary>
        private System.Timers.Timer m_sendHashTablesTimer = new System.Timers.Timer();

        /// <summary>
        /// Decide how long we have to wait until we send out an updated
        /// Query Hash Table and create the timer to do it.  We always do 
        /// the hash table sending in a timer so that it will have it's own thread
        /// to operate in.  This is because rebuidling the amalgamated table is 
        /// non-trivial, and so we don't want to hold up the main thread, or the 
        /// communications thread.
        /// </summary>
        private void SendQueryHashTable()
        {
            lock (m_timerLock)
            {
                System.Timers.Timer sendHashTablesTimer = m_sendHashTablesTimer;
                if (null != sendHashTablesTimer && !sendHashTablesTimer.Enabled)
                {
                    long timeToWaitUntilSend = 5;
                    TimeSpan elapsedTime = DateTime.Now - m_timeQHTSent;
                    if (elapsedTime.TotalMilliseconds < m_qhtTimerInterval)
                    {
                        timeToWaitUntilSend = m_qhtTimerInterval - (long)elapsedTime.TotalMilliseconds;
                    }
                    sendHashTablesTimer.Interval = timeToWaitUntilSend;
                    sendHashTablesTimer.Enabled = true;
                }
            }
        }

        /// <summary>
        /// Gets a value indicating whether this instance's query hash table is dirty.
        /// </summary>
        /// <value>
        /// 	<c>true</c> if this instance's query hash table is dirty; otherwise, <c>false</c>.
        /// </value>
        public bool IsQueryHashTableDirty
        {
            get
            {
                System.Timers.Timer sendHashTablesTimer = m_sendHashTablesTimer;
                return sendHashTablesTimer != null && sendHashTablesTimer.Enabled;
            }
        }

        /// <summary>
        /// Forces the query hash table update.
        /// </summary>
        public void ForceQueryHashTableUpdate()
        {
            System.Timers.Timer sendHashTablesTimer = m_sendHashTablesTimer;
            if (null != sendHashTablesTimer && sendHashTablesTimer.Enabled)
            {
                SendHashTablesTimerElapsed(this, null);
            }
        }

        /// <summary>
        /// Periodic event to propagate the Ping message from the stack.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void SendHashTablesTimerElapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            Tracer.DebugWriteLine("QHT timer elapsed.");
            lock (m_timerLock)
            {
                System.Timers.Timer sendHashTablesTimer = m_sendHashTablesTimer;
                if (null != sendHashTablesTimer)
                {
                    sendHashTablesTimer.Enabled = false;
                }

                //  This will also be updated at the end of the sending, 
                // but that is OK.  And we should update it here too
                // because someone may be just about to set the timer up again.
                m_timeQHTSent = DateTime.Now;
            }
            DoHashTableSending();
        }

        /// <summary>
        /// Send the appropriate Query Hash Table to the persistently connected hubs.
        /// </summary>
        private void DoHashTableSending()
        {
            Tracer.DebugWriteLine("Propagating QHT changes from '{1}:{0}'.", this.ToString(), NodeInfo.IsLeaf ? "Leaf" : "Hub");

            // Force it to be regenerated.
            m_amalgamatedQueryHashTable = null;

            // stuff it in a message
            Message QHTMessage = AmalgamatedQueryHashTable.ToMessage();

            // Send to all persistently-connected attached hubs.
            ConnectionManager.PersistentConnections.ForEach(delegate(StackConnection connection)
            {
                if ((connection.RemoteNodeInfo != null) &&
                    (connection.RemoteNodeInfo.IsHub))
                {
                    Tracer.DebugWriteLine("Sending QHT to {0}.",
                        connection.RemoteNodeInfo.FriendlyName);
                    connection.SendMessage(QHTMessage);
                }
            });
            m_timeQHTSent = DateTime.Now;
        }

        /// <summary>
        /// Update this connection with our QHT info.
        /// </summary>
        /// <param name="connection"></param>
        private void SendQHTToConnection(StackConnection connection)
        {
            // Only hubs need to get QHTs
            if (connection != null && connection.RemoteNodeInfo != null && connection.RemoteNodeInfo.IsHub)
            {
                connection.SendMessage(AmalgamatedQueryHashTable.ToMessage());
            }
        }

        #endregion

        #region Query

        /// <summary>
        /// A list of guids of queries that have visited here.
        /// </summary>
        private readonly List<Guid> m_recentVisitingQueries = new List<Guid>();

        /// <summary>
        /// Stores the event handler for OnQuery event.
        /// </summary>
        private event StackMessageHandler m_onQuery
        {
            // dummy implementation to remove unused warning
            add {}
            remove {}
        }

        private object m_queryEventLock = new object();

        /// <summary>
        /// Fired when a Query message is recieved.
        /// </summary>
        public event StackMessageHandler OnQuery
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

        /// <summary>
        /// This is called when one of its connections gets a query.
        /// </summary>
        /// <param name="connection">The connection that the query message was received from.</param>
        /// <param name="message">The message.</param>
        void HandleQueryMessage(StackConnection connection, Message message)
        {
            try
            {
                // Reconstitute query.
                Query query = new Query(message);

                // Receive the query.
                QueryAcknowledgement acknowledgement = ProcessQuery(query);

                // Send the acknowledgement.
                if (null != acknowledgement)
                {
                    // Send acknowledgement.
                    connection.SendMessage(acknowledgement.ToMessage());

                    Logging.Categories.Query.Log("Sent.Acknowledgement(" + query.Guid + ")", "IsDeadEnd=" + acknowledgement.IsDeadEnd);
                }
            }
            catch (Exception ex)
            {
                Logging.Categories.Query.Error(ex);
            }
        }

        /// <summary>
        /// Receive the query and deal with it.
        /// </summary>
        /// <param name="query">The query.</param>
        public QueryAcknowledgement ProcessQuery(Query query)
        {
            QueryAcknowledgement acknowledgement = null;

            PerformanceCounters.QueriesReceived.Increment();

            UsageReports.QueryReceived(query.OriginNode.FriendlyName,
                                       query.OriginNode.NodeGUID,
                                       m_localNodeInfo.FriendlyName,
                                       m_localNodeInfo.NodeGUID
                                       );

            Tracer.DebugWriteLine("'{0}' received query '{1}'.",
                NodeInfo.FriendlyName, query.Contents);

            bool wasAlreadyVisited = false;
            lock (m_recentVisitingQueries)
            {
                wasAlreadyVisited = m_recentVisitingQueries.Contains(query.Guid);

                if (wasAlreadyVisited)
                {
                    // Trace that we have seen this one before, and 
                    // fall through so an acknowledgment can be sent if needed.
                    Tracer.DebugWriteLine(
                        "'{0}' has already dealt with query '{1}'.",
                        NodeInfo.FriendlyName, query.Contents);
                }
                else
                {
                    // If the query guid is NOT in m_recentVisitingQueries, add it.
                    m_recentVisitingQueries.Add(query.Guid);
                }
            }

            // If it's a hub...
            if (NodeInfo.IsHub)
            {
                // Get hop count.
                int hopCount = query.HopCount;

                // Increment hop count in query, and serialize query to message for propagation.
                query = query.GetHoppedQuery();
                Message propagatedQueryMessage = query.ToMessage();

                // If the hop count is 0, pass the query on to connected hubs.
                if (hopCount < 1)
                {
                    Tracer.DebugWriteLine(
                        "The hop count is 0. Working on an acknowledgement.");

                    // Generate acknowledgement.
                    {
                        // Construct candidate hubs list from known hubs.
                        List<NodeInfo> candidateList;
                        lock (m_lockKnownHubList)
                        {
                            candidateList = new List<NodeInfo>(m_knownHubList.KnownHubs);
                        }

                        // Ensure this stack isn't in the list.
                        candidateList.Remove(NodeInfo);
                        System.Diagnostics.Debug.Assert(!candidateList.Contains(NodeInfo),
                            "Thread " + System.Threading.Thread.CurrentThread.ManagedThreadId + ": " +
                            "There is still at least one instance of " + NodeInfo.FriendlyName +
                            " in " + NodeInfo.FriendlyName + "'s acknowledgement candidate hubs.");

                        // Iterate through persistent connections and populate "visited hubs" list.
                        List<NodeInfo> visitedList = new List<NodeInfo>();
                        visitedList.Add(NodeInfo);
                        ConnectionManager.PersistentConnections.ForEach(delegate(StackConnection connection)
                        {
                            if (null != connection.RemoteNodeInfo &&
                                connection.RemoteNodeInfo.IsHub)
                            {
                                if (null == connection.RemoteQueryHashTable)
                                {
                                    // If it has no remote hash table, it hasn't been visited.
                                    // It's a candidate.
                                    // Note that we need to ensure that it's not in the visited hub list,
                                    // because the persistent connections are changing underneath us.
                                    if (!candidateList.Contains(connection.RemoteNodeInfo) &&
                                        !visitedList.Contains(connection.RemoteNodeInfo))
                                    {
                                        candidateList.Add(connection.RemoteNodeInfo);
                                    }
                                }
                                else
                                {
                                    // It has a remote hash table, so it has now been officially visited.
                                    visitedList.Add(connection.RemoteNodeInfo);
                                    candidateList.Remove(connection.RemoteNodeInfo);
                                    System.Diagnostics.Debug.Assert(!candidateList.Contains(connection.RemoteNodeInfo),
                                        "Thread " + System.Threading.Thread.CurrentThread.ManagedThreadId + ": " +
                                        "There is still at least one instance of " + connection.RemoteNodeInfo.FriendlyName +
                                        " in " + NodeInfo.FriendlyName + "'s acknowledgement candidate hubs.");
                                }
                            }
                        });

                        acknowledgement = new QueryAcknowledgement(query.Guid, visitedList, candidateList);
                        acknowledgement.IsDeadEnd = !MayHaveQueryResults(query.Contents);
                    }

                    if (wasAlreadyVisited)
                    {
                        System.Diagnostics.Debug.Assert(null != acknowledgement,
                            "The acknowledgement shouldn't be null here.");

                        acknowledgement.IsDeadEnd = true;

                        return acknowledgement;
                    }

#if MULTITHREAD
                    System.Threading.Thread thread = new System.Threading.Thread(delegate()
                    {
#endif
                        // Send query to all persistently-connected hubs.
                        ConnectionManager.PersistentConnections.ForEach(delegate(StackConnection connection)
                        {
                            if (null == connection.RemoteNodeInfo)
                            {
                                Tracer.DebugWriteLine(
                                    "One of the connections in '{0}' has a null remote node info.",
                                    NodeInfo.FriendlyName);
                            }
                            // If it's a hub...
                            else if (connection.RemoteNodeInfo.IsHub)
                            {
                                // If the query string is in the hub's amalgamated query hash table...
                                if (null != connection.RemoteQueryHashTable &&
                                    connection.RemoteQueryHashTable.MayContain(query.Contents))
                                {
                                    Tracer.DebugWriteLine(
                                        "The connection to hub '{0}' in '{1}' has hash table hit for query '{2}'.  Sending...",
                                        connection.RemoteNodeInfo.FriendlyName, NodeInfo.FriendlyName, query.Contents);

                                    // Send to that connected hub.
                                    Logging.Categories.Query.Log("Proprogating.Query(" + query.Guid + ").RemoteNode", connection.RemoteNodeInfo.ToString());
                                    connection.SendMessage(propagatedQueryMessage);
                                }
                            }
                        });
#if MULTITHREAD
                    });
                    thread.IsBackground = true;
                    thread.Start();
#endif
                }

                if (!wasAlreadyVisited)
                {
                    // If the query string is not in the amalgamated local hash table,
                    // there is nothing more to do here.
                    this.Tracer.DebugWriteLine("Checking amalgamated QHT for '{0}'.", query.Contents);
                    if (!AmalgamatedQueryHashTable.MayContain(query.Contents))
                    {
                        this.Tracer.DebugWriteLine("{0} not found in amalgamated QHT.  Checking local QHT.", query.Contents);
                        if (!LocalQueryHashTable.MayContain(query.Contents))
                        {
                            Tracer.DebugWriteLine(
                                "Neither the local or amalgamated hash in '{0}' [yet] contain query '{1}'.",
                                NodeInfo.FriendlyName, query.Contents);

                            return acknowledgement;
                        }
                    }

                    Tracer.DebugWriteLine(
                        "The amalgamated hash in '{0}' matches query '{1}'.",
                        NodeInfo.FriendlyName, query.Contents);

                    // Deal with persistently-connected leaves.
#if MULTITHREAD
                    System.Threading.Thread thread = new System.Threading.Thread(delegate()
                    {
#endif
                        ConnectionManager.PersistentConnections.ForEach(delegate(StackConnection connection)
                        {
                            if (null == connection.RemoteNodeInfo)
                            {
                                Tracer.DebugWriteLine(
                                    "A connection in '{0}' has a null remote node info.",
                                    NodeInfo.FriendlyName);
                            }
                            // If the remote node is a leaf...
                            else if (connection.RemoteNodeInfo.IsLeaf)
                            {
                                if (null == connection.RemoteQueryHashTable)
                                {
                                    var warning = String.Format("A leaf connection to '{0}' in '{1}' has a null remote query hash table.",
                                        connection.RemoteNodeInfo.FriendlyName, NodeInfo.FriendlyName);
                                    Logging.Categories.Query.Warning(warning);
                                    Tracer.DebugWriteLine(warning);
                                }
                                // If the query string is in leaf's local (only) hash table...
                                else if (connection.RemoteQueryHashTable.MayContain(query.Contents))
                                {
                                    var message = String.Format("propagating query (content='{2}',guid='{3}') to '{0}' from '{1}'.",
                                        connection.RemoteNodeInfo.FriendlyName,
                                        NodeInfo.FriendlyName,
                                        query.Contents,
                                        query.Guid);

                                    Tracer.DebugWriteLine(message);
                                    Logging.Categories.Query.Log("Proprogating.Query(" + query.Guid + ").RemoteNode", connection.RemoteNodeInfo.ToString());

                                    // Propagate query message to the leaf.
                                    connection.SendMessage(propagatedQueryMessage);
                                }
                            }
                        });
#if MULTITHREAD
                    });
                    thread.IsBackground = true;
                    thread.Start();
#endif
                }
            }

            if (!wasAlreadyVisited)
            {
                // See if this stack is a hit.
                if (null != m_onQueryHit)
                {
                    Tracer.DebugWriteLine(
                        "About to check if the local query hashtable in '{0}' contains query '{1}'.",
                        NodeInfo.FriendlyName, query.Contents);

                    // If the query string is in the local hash table (for self node)...
                    if (LocalQueryHashTable.MayContain(query.Contents))
                    {
                        Tracer.DebugWriteLine(
                            "{0}'s local hash contains query {1}; firing 'on query hit' event.",
                            NodeInfo.FriendlyName, query.Contents);

                        Logging.Categories.Query.Log("Handling.Query(" + query.Guid + ").Hit", "True");

                        // Fire "search hit this node" event.
                        m_onQueryHit(this, query);
                    }
                    else
                    {
                        Logging.Categories.Query.Log("Handling.Query(" + query.Guid + ").Hit", "False");

                        Tracer.DebugWriteLine(
                            "{0}'s local hash does not contain query {1}.",
                            NodeInfo.FriendlyName, query.Contents);
                    }
                }
            }

            return acknowledgement;
        }

        /// <summary>
        /// Checks to see if there is a chance of query results.
        /// </summary>
        /// <param name="queryString">The query string.</param>
        /// <returns>
        /// This is false if no hope (doesn't hit amalgamated hash, or amalgamated hash of any connected hub).
        /// Don't worry about connections further out;
        /// these are in the candidates list and will be dealt with later.
        /// </returns>
        private bool MayHaveQueryResults(string queryString)
        {
            if (AmalgamatedQueryHashTable.MayContain(queryString))
            {
                return true;
            }

            // It doesn't hit the amalgamated hash.
            // Unless we can rule it out, we have to give it a maybe and return true.
            // To rule it out, all remote query hashtables are non null, and don't contain it.
            // Only check the persistent connections.
            StackConnection possibleConnection = ConnectionManager.PersistentConnections.Find(delegate(StackConnection connection)
            {
                if (null == connection.RemoteNodeInfo)
                {
                    // It might be a hub, which might, in turn, contain a hash table hit.
                    return true;
                }

                if (connection.RemoteNodeInfo.IsHub)
                {
                    if (null == connection.RemoteQueryHashTable)
                    {
                        // We can't be sure that it's ruled out.
                        return true;
                    }

                    // If remote hash table contains the query string, return true.
                    if (connection.RemoteQueryHashTable.MayContain(queryString))
                    {
                        return true;
                    }
                }

                return false;
            });

            return (null != possibleConnection);
        }

        /// <summary>
        /// This is the delegate type for the query hit event handler.
        /// </summary>
        /// <param name="stack">The stack hit.</param>
        /// <param name="query">The query.</param>
        public delegate void QueryHitHandler(Stack stack, Query query);

        /// <summary>
        /// This event is fired when the query hits this node.
        /// </summary>
        private event QueryHitHandler m_onQueryHit;
        private object m_queryHitEventLock = new object();

        /// <summary>
        /// This event is fired when the query hits this node.
        /// </summary>
        public event QueryHitHandler OnQueryHit
        {
            add
            {
                lock (m_queryHitEventLock)
                {
                    if (m_onQueryHit == null || !m_onQueryHit.GetInvocationList().Contains(value))
                    {
                        m_onQueryHit += value;
                    }
                }
            }
            remove
            {
                lock (m_queryHitEventLock)
                {
                    m_onQueryHit -= value;
                }
            }
        }

        #endregion

        #region Query Acknowledgement

        /// <summary>
        /// A handler for a query acknowledgement.
        /// The OnQueryAcknowledgement event takes instances of this delegate type.
        /// </summary>
        /// <param name="connection">The connection that received the query acknowledgement and propagated the event.</param>
        /// <param name="acknowledgement">The query acknowledgement.</param>
        public delegate void QueryAcknowledgementHandler(
            StackConnection connection, QueryAcknowledgement acknowledgement);

        /// <summary>
        /// Stores the event handler for OnQueryAcknowledge event.
        /// </summary>
        private event QueryAcknowledgementHandler m_onQueryAcknowledgement;
        private object m_queryAcknowledgementEventLock = new object();

        /// <summary>
        /// Fired when a Query Acknowledgement message is recieved.
        /// </summary>
        public event QueryAcknowledgementHandler OnQueryAcknowledgement
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

        /// <summary>
        /// Handle a query acknowledgement event triggered in a stack connection.
        /// </summary>
        /// <param name="sender">The stack connection.</param>
        /// <param name="message">The acknowledgement message.</param>
        void HandleQueryAcknowledgementMessage(StackConnection sender, Message message)
        {
            Tracer.DebugWriteLine(
                "Handling query acknowledgement in '{0}'.", NodeInfo.FriendlyName);

            // Reconstitute acknowledgement.
            QueryAcknowledgement acknowledgement = QueryAcknowledgement.FromMessage(message);

            // Add each "candidate hub" in the acknowledgement
            // to the "known hub list" for this node.
            lock (m_lockKnownHubList)
            {
                bool hubsChanged = false;

                acknowledgement.CandidateHubs.ForEach(delegate(NodeInfo candidateHub)
                {
                    if (!candidateHub.Equals(NodeInfo))
                    {
                        // If it's not in the persistent connections, add it to "known hubs".
                        if (m_knownHubList.Add(candidateHub))
                        {
                            hubsChanged = true;
                        }
                    }
                });

                if (hubsChanged)
                {
                    SendKnownHubListMessage();
                }
            }

            // Propagate the event on.
            if (null != m_onQueryAcknowledgement)
            {
                m_onQueryAcknowledgement(sender, acknowledgement);
            }
        }

        #endregion

        #region Query Result

        public const string QueryResultMessageID = "QRes";

        #region Query Result Event

        public class QueryResultEventArgs : EventArgs
        {
            private QueryResult m_queryResult;

            public QueryResult Result
            {
                get { return m_queryResult; }
                set { m_queryResult = value; }
            }

            private StackConnection m_connection;

            public StackConnection Connection
            {
                get { return m_connection; }
                set { m_connection = value; }
            }

            internal QueryResultEventArgs(StackConnection connection, QueryResult queryResult)
            {
                m_connection = connection;
                m_queryResult = queryResult;
            }
        }

        /// <summary>
        /// Event which is fired when a QueryResult message for this stack arrives.
        /// </summary>
        public event EventHandler<QueryResultEventArgs> OnQueryResult
        {
            add
            {
                m_OnQueryResult.Add(value);
            }
            remove
            {
                m_OnQueryResult.Remove(value);
            }
        }
        private Pyxis.Utilities.EventHelper<QueryResultEventArgs> m_OnQueryResult = new Pyxis.Utilities.EventHelper<QueryResultEventArgs>();

        /// <summary>
        /// Method to safely raise event.
        /// </summary>
        private void QueryResultRaise(StackConnection connection, QueryResult queryResult)
        {
            m_OnQueryResult.Invoke(this, new QueryResultEventArgs(connection, queryResult));
        }

        #endregion Query Result Event

        public void HandleQueryResultMessage(object sender, MessageReceivedEventArgs args)
        {
            StackConnection connection = args.Context.Sender;

            Tracer.DebugWriteLine(
                "Handling a query result message in '{0}' from {1}.",
                NodeInfo.FriendlyName,
                ((null == connection.RemoteNodeInfo) ?
                    "some connection" :
                    String.Format("'{0}'", connection.RemoteNodeInfo.FriendlyName)));

            // Create query result.
            QueryResult queryResult = new QueryResult(args.Message);

            // Process it.
            ProcessQueryResult(connection, queryResult);
        }

        /// <summary>
        /// Process the query result.
        /// </summary>
        /// <param name="incomingConnection">The connection it came in on, or null if none.</param>
        /// <param name="queryResult">The query result.</param>
        /// <returns>
        /// True if processed by self; false means it may or may not have been processed remotely.
        /// </returns>
        public bool ProcessQueryResult(StackConnection incomingConnection, QueryResult queryResult)
        {
            PerformanceCounters.QueriesMatched.Increment();

            UsageReports.QueryMatched(queryResult.QueryOriginNode.FriendlyName,
                                      queryResult.QueryOriginNode.NodeGUID,
                                      queryResult.ResultNode.FriendlyName,
                                      queryResult.ResultNode.NodeGUID,
                                      queryResult.MatchingDataSetID.Guid
                                      );

            // If the result is for us then raise the event.
            // Otherwise, send it to the correct person 
            // (either directly, or through the PyxNet in reverse of how the query travelled).
            if (queryResult.QueryOriginNode.Equals(NodeInfo))
            {
                // Raise the event.
                this.Tracer.DebugWriteLine("Sending a query result message to ourself.");
                QueryResultRaise(incomingConnection, queryResult);
                Logging.Categories.Query.Log("Received.QueryResult(" + queryResult.QueryGuid + ").ResultNode", queryResult.ResultNode.ToString());
                return true;
            }

            this.Tracer.DebugWriteLine("Trying to send a query result message from {0} to {1}.",
                NodeInfo.FriendlyName, queryResult.QueryOriginNode.FriendlyName);

            System.Threading.ThreadPool.QueueUserWorkItem(
                delegate(Object unused)
                {
                    // Try to get a "quick" connection.
                    StackConnection connection = FindConnection(queryResult.QueryOriginNode, false);

                    if ((connection == null) && (queryResult.RequiresDirectConnection == false))
                    {
                        Logging.Categories.Query.Log("Relay.QueryResult(" + queryResult.QueryGuid + ").OriginNode", queryResult.QueryOriginNode.ToString());
                        // TODO: We should pass incomingConnection on to the relayer - this 
                        // is a good candidate for the relay.
                        RelayMessage(queryResult.QueryOriginNode, queryResult.ToMessage());
                        return;
                    }

                    // Retry sending on the connection.
                    int tryCount = 1;
                    for (; tryCount <= 3; ++tryCount)
                    {
                        if (null == connection)
                        {
                            // Find a connection to the originating node, or create if necessary.
                            connection = GetConnection(queryResult.QueryOriginNode, false, TimeSpan.FromSeconds(20));
                            if (null == connection)
                            {
                                continue;
                            }
                        }

                        connection.Tracer.Enabled = this.Tracer.Enabled;

                        Logging.Categories.Query.Log("Sending.QueryResult(" + queryResult.QueryGuid + ").OriginNode", queryResult.QueryOriginNode.ToString());

                        if (connection.SendMessage(queryResult.ToMessage()))
                        {
                            PerformanceCounters.QueryResultsSent.Increment();

                            Logging.Categories.Query.Log("Sent.QueryResult(" + queryResult.QueryGuid + ").OriginNode", queryResult.QueryOriginNode.ToString());

                            connection.Tracer.DebugWriteLine("Sent a query result message from {0} to {1} after {2} tries.",
                                NodeInfo.FriendlyName, connection.ToString(), tryCount);

                            // Hold on to this connection for a while
                            ConnectionManager.ConnectionHolder.HoldConnection(connection, TimeSpan.FromSeconds(5));

                            return;
                        }
                        else
                        {
                            // Apparently we found a closed or invalid connection.  Retry.
                            connection = null;
                        }
                    }

                    var message = String.Format("FAILED to send a query result message to node {0} after {1} tries.",
                        queryResult.QueryOriginNode.FriendlyName, tryCount);

                    Logging.Categories.Query.Error(message);

                    Tracer.WriteLine(message);
                });

            return false;
        }

        #endregion

        #region Data Request

        void SendDataRequest(/**/)
        {
        }

        /// <summary>
        /// Stores the event handler for OnDataRequest event.
        /// </summary>
        private event StackMessageHandler m_onDataRequest
        {
            // dummy implementation to remove unused warning
            add {}
            remove {}
        }

        private object m_dataRequestEventLock = new object();

        /// <summary>
        /// Fired when a Data Request message is recieved.
        /// </summary>
        public event StackMessageHandler OnDataRequest
        {
            add
            {
                lock (m_dataRequestEventLock)
                {
                    m_onDataRequest += value;
                }
            }

            remove
            {
                lock (m_dataRequestEventLock)
                {
                    m_onDataRequest -= value;
                }
            }
        }

        #endregion

        #region Message Relay

        /// <summary>
        /// TODO:
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        internal void HandleMessageRelay(object sender, MessageReceivedEventArgs args)
        {
            MessageRelayAcknowledgement acknowledgement = ProcessMessageRelay(sender, new MessageRelay(args.Message));
            args.Context.Sender.SendMessage(acknowledgement.ToMessage());
        }

        /// <summary>
        /// TODO:
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="relay"></param>
        /// <returns></returns>
        internal MessageRelayAcknowledgement ProcessMessageRelay(object sender, MessageRelay relay)
        {
            MessageRelayAcknowledgement acknowledgement = null;

            Tracer.DebugWriteLine("'{0}' processing message relay {1}.",
                NodeInfo.FriendlyName, relay.Guid);

            if (relay.ToNodeGuid.Equals(NodeInfo.NodeGUID))
            {
                // NOTE: this will only get relayed messages handled at the Stack level.
                // TODO: Create a proper message context?
                HandleMessage(sender, new MessageReceivedEventArgs(relay.RelayedMessage, new MessageContext(null)));
                acknowledgement = new MessageRelayAcknowledgement(relay.Guid, relay.ToNodeGuid);
            }
            else
            {
                // Find the connection we're looking for.
                StackConnection connection = FindConnection(relay.ToNodeGuid, false);
                if (null == connection)
                {
                    // Construct candidate hubs list from known hubs.
                    List<NodeInfo> candidateList;
                    lock (m_lockKnownHubList)
                    {
                        candidateList = new List<NodeInfo>(m_knownHubList.KnownHubs);
                    }

                    // Ensure this stack isn't in the list.
                    candidateList.Remove(NodeInfo);
                    System.Diagnostics.Debug.Assert(!candidateList.Contains(NodeInfo),
                        "Thread " + System.Threading.Thread.CurrentThread.ManagedThreadId + ": " +
                        "There is still at least one instance of " + NodeInfo.FriendlyName +
                        " in " + NodeInfo.FriendlyName + "'s acknowledgement candidate hubs.");

                    // Add only this one to the visited list.
                    List<NodeInfo> visitedList = new List<NodeInfo>();
                    visitedList.Add(NodeInfo);

                    // Construct acknowledgement.
                    acknowledgement = new MessageRelayAcknowledgement(relay.Guid, relay.ToNodeGuid, visitedList, candidateList);
                }
                else
                {
                    // Found.
                    connection.SendMessage(relay.RelayedMessage);
                    acknowledgement = new MessageRelayAcknowledgement(relay.Guid, relay.ToNodeGuid);
                }
            }

            return acknowledgement;
        }

        /// <summary>
        /// Relays the message to the specified recipient.
        /// </summary>
        /// <param name="recipient">The recipient.</param>
        /// <param name="m">The message.</param>
        public void RelayMessage(NodeInfo recipient, Message m)
        {
            Tracer.WriteLine("Attempting to relay a {0} message from {1} to {2}.",
                m.Identifier, NodeInfo.FriendlyName, recipient.FriendlyName);
            MessageRelayer relayer = new MessageRelayer(this, new MessageRelay(m.ToMessage(), recipient.NodeGUID),
                (int)System.TimeSpan.FromSeconds(1).TotalMilliseconds);
            relayer.Start();
        }

        #endregion

        #region CertificateRepository

        private Service.CertificateRepository m_certificateRepository;
        private Publishing.CertificatePublisher m_certificatePublisher;
        private readonly Object m_certificateRepositorySetLock = new Object();

        private string DefaultCertificateRepositoryName
        {
            get
            {
                return string.Format("{0}\\PyxNet.{1}.Certificates",
                System.Windows.Forms.Application.UserAppDataPath,
                this.NodeInfo.FriendlyName);
            }
        }

        /// <summary>
        /// Holds all of the certificates that this stack has seen.  Persists
        /// between runs (this is a repository.)
        /// </summary>
        /// <remarks>
        /// In "production" code, this will default to a single well-known
        /// database file.  "Test" users will set the CertificateRepository
        /// to a test-specific file.
        /// </remarks>
        public Service.CertificateRepository CertificateRepository
        {
            get
            {
                if (m_certificateRepository == null)
                {
                    CertificateRepository = new PyxNet.Service.CertificateRepository(
                        DefaultCertificateRepositoryName);
                }
                return m_certificateRepository;
            }
            set
            {
                lock (m_certificateRepositorySetLock)
                {
                    if (m_certificateRepository != null)
                    {
                        this.Tracer.WriteLine(
                            "Invalid state!  Attempt to reset the stack's certificate repository.");
                    }
                    m_certificateRepository = value;
                    m_certificatePublisher = new PyxNet.Publishing.CertificatePublisher(this);
                    m_certificatePublisher.PublishItem(m_certificateRepository);
                }
            }
        }

        #endregion /* CertificateRepository */

        public ICertificateProvider CertificateProvider { get; set; }
        public ICertificateValidator CertificateValidator { get; set; }
    }
}

namespace PyxNet.Test
{
}
