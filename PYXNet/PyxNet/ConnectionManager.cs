using System;
using System.Collections.Generic;
using System.Text;

using Pyxis.Utilities;

namespace PyxNet
{
    public partial class Stack
    {
        #region Convenience functions (forward to ConnectionManager)
        /// <summary>
        /// Returns true if there is at least one hub connected.
        /// </summary>
        public bool IsHubConnected
        {
            get
            {
                return this.ConnectionManager.IsHubConnected;
            }
        }

        /// <summary>
        /// Add the known hubs from this stack into the list.
        /// </summary>
        /// <param name="candidateHubs">The list to add the known hub info to.</param>
        public void GetHubs(IList<NodeInfo> hubs)
        {
            ConnectionManager.GetHubs(hubs);
        }

        /// <summary>
        /// Find, or create, a connection.
        /// </summary>
        /// <param name="info">The connection node info.</param>
        /// <param name="persistentOnly">True if persistent only.</param>
        /// <param name="timeoutForReverseConnection">
        /// Amount of time to allow for a reverse connection, or TimeSpan.Zero if no reverse connection
        /// is to be attempted.
        /// </param>
        /// <returns>The stack connection</returns>
        public StackConnection GetConnection(NodeInfo info, bool persistentOnly, TimeSpan timeoutForReverseConnection)
        {
            return ConnectionManager.GetConnection(info, persistentOnly, timeoutForReverseConnection);
        }

        /// <summary>
        /// Find an existing stack connection.
        /// </summary>
        /// <param name="info">The remote node info to search for.</param>
        /// <param name="persistentOnly">True if only persistent connections are to be searched.</param>
        /// <returns>A stack connection, or null if not found.</returns>
        public StackConnection FindConnection(NodeInfo info, bool persistentOnly)
        {
            return ConnectionManager.FindConnection(info, persistentOnly);
        }

        /// <summary>
        /// Find an existing stack connection by ID.
        /// </summary>
        /// <param name="nodeID">The remote node ID to search for.</param>
        /// <param name="persistentOnly">True if only persistent connections are to be searched.</param>
        /// <returns>A stack connection, or null if not found.</returns>
        public StackConnection FindConnection(System.Guid nodeID, bool persistentOnly)
        {
            return ConnectionManager.FindConnection(nodeID, persistentOnly);
        }

        /// <summary>
        /// Create a connection to a computer by address.
        /// May throw a ConnectionException if the connection couldn't be established.
        /// </summary>
        /// <param name="address">The address of the computer to connect to.</param>
        /// <param name="isPersistent">True if this is to be a persistent connection.</param>
        /// <returns>The new stack connection.</returns>
        public StackConnection CreateConnection(NetworkAddress address, bool isPersistent)
        {
            return ConnectionManager.CreateConnection( address, isPersistent);
        }

        /// <summary>
        /// Create a connection to a computer by node info.
        /// May throw a ConnectionException if the connection couldn't be established.
        /// </summary>
        /// <param name="info">The node info of the computer to connect to.</param>
        /// <param name="isPersistent">True if this is to be a persistent connection.</param>
        /// <returns>The new stack connection.</returns>
        public StackConnection CreateConnection(NodeInfo info, bool isPersistent)
        {
            return ConnectionManager.CreateConnection(info, isPersistent);
        }

        #endregion

        public class ConnectionException : Exception
        {
            private readonly StackConnectionResponse.ErrorType m_errorType;

            public StackConnectionResponse.ErrorType ErrorType
            {
                get { return m_errorType; }
            }

            public ConnectionException(StackConnectionResponse.ErrorType errorType)
            {
                m_errorType = errorType;
            }
        }


        /// <summary>
        /// Prints the current set of connections for the stack in human readable form.
        /// </summary>
        public string CurrentConnectionStatus
        {
            get
            {
                return ConnectionManager.CurrentConnectionStatus;
            }
        }
                    
        #region Connected Event
        /// <summary> EventArgs for a Connected event. </summary>    
        public class ConnectedEventArgs : EventArgs
        {
            private StackConnection m_StackConnection;

            /// <summary>The StackConnection.</summary>
            public StackConnection StackConnection
            {
                get { return m_StackConnection; }
                set { m_StackConnection = value; }
            }

            internal ConnectedEventArgs(StackConnection theStackConnection)
            {
                m_StackConnection = theStackConnection;
            }
        }

        /// <summary> 
        /// Event handler for Connected. Connected is called whenever a 
        /// connection occurs (including this node connecting to another
        /// node, another hub, or another node connecting to this stack.)
        /// </summary>
        private EventHelper<ConnectedEventArgs> m_connected = new EventHelper<ConnectedEventArgs>();
        public event EventHandler<ConnectedEventArgs> Connected
        {
            add
            {
                m_connected.Add(value);
            }
            remove
            {
                m_connected.Remove(value);
            }
        }

        /// <summary>
        /// Raises the Connected event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="theStackConnection"></param>
        public void OnConnected(object sender, StackConnection theStackConnection)
        {
            Logging.Categories.Connection.Log("State(" + theStackConnection.RemoteNodeInfo + ")", "Connected");
            m_connected.Invoke( sender, new ConnectedEventArgs(theStackConnection));
        }
        #endregion Connected Event

        #region Connected to a Hub Event

        /// <summary> 
        /// Event handler for NetworkConnected. NetworkConnected is called
        /// whenever the stack is permanently connected to a hub in the network
        /// when it was previously permanently connected to no hubs in the network. 
        /// </summary>
        public event EventHandler<EventArgs> HubConnected
        {
            add
            {
                m_hubConnected.Add( value);
                m_knownHubList.HubConnected += value;
            }
            remove
            {
                m_hubConnected.Remove(value);
                m_knownHubList.HubConnected -= value;
            }
        }
        private EventHelper<EventArgs> m_hubConnected = new EventHelper<EventArgs>();

        /// <summary>
        /// Raises the HubConnected event.
        /// </summary>
        /// <param name="sender"></param>
        private void OnHubConnected(object sender)
        {
            m_hubConnected.Invoke( sender, new EventArgs());
        }

        #endregion Connected to a Hub Event

        #region Disconnected from a Hub Event

        /// <summary> 
        /// Event handler for HubDisconnected. HubDisconnected is called
        /// whenever the stack is permanently disconnected from a hub in the network
        /// when it was previously permanently connected to at least one hub
        /// in the network. 
        /// </summary>
        public event EventHandler<EventArgs> HubDisconnected
        {
            add
            {
                m_hubDisconnected.Add( value);
                m_knownHubList.HubDisconnected += value;
            }
            remove
            {
                m_hubDisconnected.Remove( value);
                m_knownHubList.HubDisconnected -= value;
            }
        }
        private EventHelper<EventArgs> m_hubDisconnected = new EventHelper<EventArgs>();

        /// <summary>
        /// Raises the HubDisconnected event.
        /// </summary>
        /// <param name="sender"></param>
        private void OnHubDisconnected(object sender)
        {
            m_hubDisconnected.Invoke(sender, new EventArgs());
        }

        #endregion Disconnected from the Network Event

        /// <summary>
        /// Delegate type for handling a closed connection.
        /// </summary>
        /// <param name="connection">The connection that just closed.</param>
        public delegate void ClosedConnectionHandler(Stack stack, StackConnection connection);

        /// <summary>
        /// This event is triggered when a connection has been closed.
        /// </summary>
        private event ClosedConnectionHandler m_onClosedConnection;
        private object m_closedConnectionEventLock = new object();
        public event ClosedConnectionHandler OnClosedConnection
        {
            add
            {
                lock (m_closedConnectionEventLock)
                {
                    m_onClosedConnection += value;
                }
            }

            remove
            {
                lock (m_closedConnectionEventLock)
                {
                    m_onClosedConnection -= value;
                }
            }
        }
 

        /// <summary>
        /// A ConnectionManager contains all of the connection management logic
        /// that pertains to a particular stack.  Many of the routines found here
        /// were formerly found in Stack, but have been moved here for sanity.
        /// </summary>
        public class ConnectionManagerHelper
        {
            public Stack Stack { get; private set; }

            private TraceTool Tracer { get; set; }

            internal ConnectionManagerHelper(Stack stack)
            {
                Stack = stack;
                Tracer = Stack.Tracer;

                // Hook into persistent connection list's add and remove events.
                // TODO: Remove this logic from here; it should be enough to hook into KnownHubList.ConnectedHubs events,
                // and putting it in both places probably results in 2 events fired each time.
                m_persistentConnections.AddedElement +=
                    delegate(object sender, DynamicList<StackConnection>.ElementEventArgs e)
                    {
                        System.Diagnostics.Debug.Assert(e != null);
                        StackConnection connection = e.Element;
                        System.Diagnostics.Debug.Assert(connection != null);

                        if (1 == e.ElementCount)
                        {
                            NodeInfo info = connection.RemoteNodeInfo;
                            if (info != null && info.IsHub)
                            {
                                Stack.OnHubConnected(this);
                            }
                        }

                        // Update this persistent connection with our QHT.
                        Stack.SendQHTToConnection(connection);
                    };
                m_persistentConnections.RemovedElement +=
                    delegate(object sender, DynamicList<StackConnection>.ElementEventArgs e)
                    {
                        System.Diagnostics.Debug.Assert(e != null);
                        if (0 == e.ElementCount)
                        {
                            StackConnection connection = e.Element;
                            if (connection != null)
                            {
                                NodeInfo info = connection.RemoteNodeInfo;
                                if (info != null && info.IsHub)
                                {
                                    Stack.OnHubDisconnected(this);
                                }
                            }
                        }
                    };

                // Handle handshake messages from any connection.
                Stack.RegisterHandler(StackConnectionRequest.MessageID, HandleConnectionRequestMessage);

                Stack.RegisterHandler(StackConnectionResponse.MessageID, HandleConnectionResponseMessage);
            }

            /// <summary>
            /// Releases unmanaged and - optionally - managed resources.
            /// </summary>
            /// <param name="p">
            /// <c>true</c> to release both managed and unmanaged resources; 
            /// <c>false</c> to release only unmanaged resources.
            /// </param>
            /// <remarks>
            /// ConnectionManager is not actually disposable.  We emulate the
            /// dispose pattern here and call it from Stack.Dispose( bool)
            /// </remarks>
            internal void Dispose(bool p)
            {
                // Close down all our existing connections
                Action<StackConnection> close = delegate(StackConnection connection)
                {
                    if (null != connection)
                    {
                        // close the connection
                        connection.Close();
                    }
                };
                m_pendingConnections.ForEach(close);
                m_persistentConnections.ForEach(close);
                m_temporaryConnections.ForEach(close);
                m_volatileConnections.ForEach(close);

                // Clear the connection lists
                m_pendingConnections.Clear();
                m_persistentConnections.Clear();
                m_temporaryConnections.Clear();
                m_volatileConnections.Clear();
            }

            /// <summary>
            /// Creates the performance events.
            /// </summary>
            internal void CreatePerformanceEvents()
            {
                // Hook into the connection lists changing and use those events to drive the number of connections counters.
                PersistentConnections.CountChanged += delegate(object sender, DynamicList<StackConnection>.CountChangedEventArgs args)
                {
                    Stack.PerformanceCounters.PermanentConnections.RawValue = PersistentConnections.Count;
                };

                TemporaryConnections.CountChanged += delegate(object sender,
                    WeakReferenceList<StackConnection>.CountChangedEventArgs<StackConnection> args)
                {
                    Stack.PerformanceCounters.TemporaryConnections.RawValue = TemporaryConnections.Count;
                };

                VolatileConnections.CountChanged += delegate(object sender, DynamicList<StackConnection>.CountChangedEventArgs args)
                {
                    Stack.PerformanceCounters.VolatileConnections.RawValue = VolatileConnections.Count;
                };
            }

            /// <summary>
            /// Creates the performance counters.
            /// </summary>
            internal void InitializePerformanceCounters()
            {
                // initialize the correct values
                Stack.PerformanceCounters.PermanentConnections.RawValue = PersistentConnections.Count;
                Stack.PerformanceCounters.TemporaryConnections.RawValue = TemporaryConnections.Count;
                Stack.PerformanceCounters.VolatileConnections.RawValue = VolatileConnections.Count;
            }
            #region Status Strings

            /// <summary>
            /// Helper function to add details about lists of stack connections into a string.
            /// </summary>
            /// <param name="ListName">A textual description of the type of connections.</param>
            /// <param name="output">All information will be added ot this.</param>
            /// <param name="List">The list of connections that you want to dump.</param>
            /// <returns></returns>
            private string DumpConnectionList(string ListName, System.Text.StringBuilder output, ICollection<StackConnection> List)
            {
                output.AppendFormat("{0} Connections\n", ListName);
                if (List.Count > 0)
                {
                    foreach (StackConnection connection in List)
                    {
                        output.AppendLine(connection.ToStringVerbose());
                    }
                }
                else
                {
                    output.AppendLine("None");
                }
                return output.ToString();
            }

            /// <summary>
            /// Prints the current set of connections for the stack in human readable form.
            /// </summary>
            public string CurrentConnectionStatus
            {
                get
                {
                    System.Text.StringBuilder output = new System.Text.StringBuilder();
                    DumpConnectionList("Persistent", output, m_persistentConnections);
                    DumpConnectionList("Pending", output, m_pendingConnections);
                    DumpConnectionList("Volitile", output, m_volatileConnections);
                    DumpConnectionList("Temporary", output, m_temporaryConnections);
                    return output.ToString();
                }
            }

            #endregion


            /// <summary>
            /// The number of seconds to wait for a connection response.
            /// </summary>
            // TODO: Revisit this value.
            private readonly TimeSpan m_connectionTimeOut = TimeSpan.FromMinutes(1);

            // Connections that are persistent on both ends.
            #region Persistent Connections

            /// <summary>
            /// The list of stack connections.
            /// </summary>
            private readonly DynamicList<StackConnection> m_persistentConnections = new DynamicList<StackConnection>();

            /// <summary>
            /// A property providing access to the list of stack connections.
            /// </summary>
            public DynamicList<StackConnection> PersistentConnections
            {
                get { return m_persistentConnections; }
            }

            #endregion Persistent Connections

            // Outgoing connections that are temporary on this end.
            #region Temporary Connections

            /// <summary>
            /// The list of temporary stack connections.
            /// </summary>
            private readonly WeakReferenceList<StackConnection> m_temporaryConnections =
                new WeakReferenceList<StackConnection>();

            /// <summary>
            /// Access to the temporary connections.
            /// </summary>
            public WeakReferenceList<StackConnection> TemporaryConnections
            {
                get { return m_temporaryConnections; }
            }

            #endregion Temporary Connections

            // Incoming connections that are temporary on the other end.
            #region Volatile Connections

            /// <summary>
            /// The list of volatile stack connections.
            /// </summary>
            private readonly DynamicList<StackConnection> m_volatileConnections = new DynamicList<StackConnection>();

            /// <summary>
            /// Access to the volatile connections.
            /// </summary>
            public DynamicList<StackConnection> VolatileConnections
            {
                get { return m_volatileConnections; }
            }

            #endregion

            // Connections that haven't yet been confirmed.
            #region Pending Connections

            /// <summary>
            /// The list of pending stack connections.
            /// </summary>
            private readonly DynamicList<StackConnection> m_pendingConnections = new DynamicList<StackConnection>();

            #endregion

            /// <summary>
            /// Storage for the connection holder.
            /// </summary>
            private readonly TimedConnectionHolder m_holder = new TimedConnectionHolder();

            /// <summary>
            /// A connection holder for holding connections in case you are performing an operation
            /// which you think will lead to another connection to the same node.
            /// </summary>
            public TimedConnectionHolder ConnectionHolder
            {
                get { return m_holder; }
            }

            /// <summary>
            /// Create a connection to a computer by address.
            /// May throw a ConnectionException if the connection couldn't be established.
            /// </summary>
            /// <param name="address">The address of the computer to connect to.</param>
            /// <param name="isPersistent">True if this is to be a persistent connection.</param>
            /// <returns>The new stack connection.</returns>
            public StackConnection CreateConnection(NetworkAddress address, bool isPersistent)
            {
                Guid guid = Guid.Empty;
                return CreateConnection(address, isPersistent, guid);
            }

            /// <summary>
            /// Create a connection to a computer by node info.
            /// May throw a ConnectionException if the connection couldn't be established.
            /// </summary>
            /// <param name="info">The node info of the computer to connect to.</param>
            /// <param name="isPersistent">True if this is to be a persistent connection.</param>
            /// <returns>The new stack connection.</returns>
            public StackConnection CreateConnection(NodeInfo info, bool isPersistent)
            {
                Tracer.DebugWriteLine("Attempting connection to {0} (i.e. {1} to {0}).", info.FriendlyName, Stack.NodeInfo.FriendlyName);
                if (null == info)
                {
                    throw new ArgumentNullException("info");
                }

                if (info.Equals(Stack.NodeInfo))
                {
                    throw new InvalidOperationException("Cannot connect to oneself");
                }

                try
                {
                    return CreateConnection(info.Address, isPersistent, info.NodeGUID);
                }
                catch (ConnectionException exception)
                {
                    if (exception.ErrorType == StackConnectionResponse.ErrorType.IncorrectNode)
                    {
                        // It's a bad one.  Remove from known hubs.
                        Stack.m_knownHubList.Remove(info);
                    }

                    throw;
                }
            }

            /// <summary>
            /// Utility method used by both other forms of CreateConnection.
            /// May throw a ConnectionException if the connection couldn't be established.
            /// </summary>
            /// <param name="address">The address of the computer to connect to.</param>
            /// <param name="isPersistent">True if this is to be a persistent connection.</param>
            /// <param name="nodeGUID">The Node ID of the node we are hoping to connect to (for verification); can be null.</param>
            /// <returns>The new stack connection.</returns>
            private StackConnection CreateConnection(NetworkAddress address, bool isPersistent, Guid nodeGUID)
            {
                if (null == address)
                {
                    throw new ArgumentNullException("address");
                }

                // Create the connection by address.
                INetworkConnection networkConnection = null;
                TcpNetwork network = Stack.m_tcpNetwork;
                if (null != network)
                {
                    NetworkAddress fromAddress = Stack.NodeInfo.Address;
                    networkConnection = network.Connect(address, fromAddress);
                }
                if (null == networkConnection)
                {
                    return null;
                }
                StackConnection connection = new StackConnection(networkConnection);

                // Set the remote node info, so that we can check for it when preventing
                // multiple requests for pending connections.
                connection.RemoteNodeInfo = new NodeInfo();
                connection.RemoteNodeInfo.NodeGUID = nodeGUID;

                // Add the connection.
                connection.Tracer.DebugWriteLine("{0} adding new outgoing connection.", Stack.NodeInfo.FriendlyName);
                AddConnection(connection);

                // Establish a connection handshake.  This can throw a ConnectionException.
                if (!EstablishConnectionHandshake(connection,
                    new StackConnectionRequest(isPersistent, Stack.NodeInfo, Stack.KnownHubList, nodeGUID)))
                {
                    Logging.Categories.Connection.Log("State(" + connection.RemoteNodeInfo + ")", "Handshake Failed");
                    RemoveConnection(connection);
                    connection.Close();
                    connection = null;
                }
                else
                {
                    Logging.Categories.Connection.Log("State(" + connection.RemoteNodeInfo + ")", "Created");
                }                

                return connection;
            }


            #region Connection To A Hub

            /// <summary>
            /// Returns true if there is at least one hub connected.
            /// </summary>
            public bool IsHubConnected
            {
                get
                {
                    return 0 < PersistentHubConnectionCount;
                }
            }


            #endregion

            #region Making Connections

            /// <summary>
            /// Tiny little class to hold info shared between EstablishConnectionHandshake 
            /// and HandleConnectionResponseMessage.  This allows the two to synch.
            /// </summary>
            private class ConnectionResponseHandlerInfo
            {
                public StackConnectionResponse.ErrorType ConnectionError { get; set; }
                public SynchronizationEvent HandshakeEstablished { get; set; }
                public StackConnection StackConnection { get; set; }
            }
            private Pyxis.Utilities.DynamicList<ConnectionResponseHandlerInfo> m_connectionResponseHandlers =
                new DynamicList<ConnectionResponseHandlerInfo>();

            /// <summary>
            /// Establishes a connection by sending a handshake then waiting for the response.
            /// </summary>
            /// <param name="connection">The connection.</param>
            /// <param name="connectionRequest">The connection request.</param>
            /// <returns></returns>
            private bool EstablishConnectionHandshake(
                StackConnection connection, StackConnectionRequest connectionRequest)
            {
                System.Diagnostics.Debug.Assert(null != connection,
                    "Connection cannot be null.");
                System.Diagnostics.Debug.Assert(null != connectionRequest,
                    "Connection request cannot be null.");

                ConnectionResponseHandlerInfo connectionResponseHandlerInfo =
                    new ConnectionResponseHandlerInfo
                    {
                        ConnectionError = StackConnectionResponse.ErrorType.None,
                        HandshakeEstablished = new SynchronizationEvent(),
                        StackConnection = connection
                    };
                m_connectionResponseHandlers.Add(connectionResponseHandlerInfo);

                try
                {
                    if (!connection.SendMessage(connectionRequest.ToMessage()))
                    {
                        connectionResponseHandlerInfo.ConnectionError = StackConnectionResponse.ErrorType.RequestNotSent;
                    }
                    else
                    {
                        connectionResponseHandlerInfo.HandshakeEstablished.AddTimedPulse(m_connectionTimeOut);
                        connectionResponseHandlerInfo.HandshakeEstablished.Wait();
                        if (connectionResponseHandlerInfo.HandshakeEstablished.TimedOut)
                        {
                            connectionResponseHandlerInfo.ConnectionError = StackConnectionResponse.ErrorType.TimedOut;
                        }
                    }
                }
                finally
                {
                    m_connectionResponseHandlers.Remove(connectionResponseHandlerInfo);
                }

                // If there is a connection error, throw it.
                if (connectionResponseHandlerInfo.ConnectionError != StackConnectionResponse.ErrorType.None)
                {
                    // Throw the exception that resulted.
                    RemoveConnection(connection);
                    connection.Close();
                    throw new ConnectionException(connectionResponseHandlerInfo.ConnectionError);
                }

                return m_persistentConnections.Contains(connection) ||
                    (!connectionRequest.IsPersistent && m_temporaryConnections.Contains(connection));
            }

            /// <summary>
            /// Handles the connection response message.
            /// </summary>
            /// <param name="sender">The sender.</param>
            /// <param name="args">The <see cref="PyxNet.MessageHandlerCollection.MessageReceivedEventArgs"/> instance containing the event data.</param>
            private void HandleConnectionResponseMessage(object sender, MessageReceivedEventArgs args)
            {
                StackConnection sendingConnection = args.Context.Sender;

                // Get the connection response.
                StackConnectionResponse connectionResponse = new StackConnectionResponse(args.Message);

                // Find the corresponding info.  Note that we use "==" here to do a reference
                // equality, because we want the exact same connection.
                ConnectionResponseHandlerInfo connectionResponseHandlerInfo =
                    m_connectionResponseHandlers.Find(
                        delegate(ConnectionResponseHandlerInfo item)
                        { return item.StackConnection == sendingConnection; });

                if (connectionResponseHandlerInfo == null)
                {
                    // What to do?  This could only happen if a response came from a node that we 
                    // weren't expecting a response from.
                    Tracer.WriteLine("Unexpected ConnectionResponseMessage.  Ignoring.");
                    return;
                }

                try
                {
                    // Remove the connection from the pending connections list.
                    if (!m_pendingConnections.Remove(sendingConnection))
                    {
                        // It must have been closed.
                        return;
                    }

                    // If there is an error, raise it and abort.
                    connectionResponseHandlerInfo.ConnectionError = connectionResponse.Error;
                    if (connectionResponseHandlerInfo.ConnectionError != StackConnectionResponse.ErrorType.None)
                    {
                        Logging.Categories.Connection.Log(sendingConnection.RemoteNodeInfo.ToString(), "Create connection failed with error: " + connectionResponseHandlerInfo.ConnectionError.ToString());
                        sendingConnection.Close();
                        return;
                    }

                    // Update connection's remote node info with FromNodeInfo.
                    // NOTE: this is order dependent code because AddToPersistentConnections needs
                    // to know the node ID to be able to detect connecting to a hub.
                    sendingConnection.RemoteNodeInfo = connectionResponse.FromNodeInfo;

                    // Add the connection to the proper list.
                    if (connectionResponse.IsPersistent)
                    {
                        // Move to persistent.
                        sendingConnection.Tracer.DebugWriteLine("Moving pending connection to {0}'s persistent connections.",
                            Stack.NodeInfo.FriendlyName);
                        m_persistentConnections.Add(sendingConnection);
                    }
                    else
                    {
                        // Move to temporary.
                        sendingConnection.Tracer.DebugWriteLine("Moving pending connection to {0}'s temporary connections.",
                            Stack.NodeInfo.FriendlyName);
                        m_temporaryConnections.Add(sendingConnection);
                    }

                    // Update connection's remote known hub list with FromKnownHubList.
                    sendingConnection.RemoteKnownHubList = connectionResponse.FromKnownHubList;

                    // Add the node info to our known hubs and
                    // update our Known Hub List from this connections Known Hub List
                    lock (Stack.m_lockKnownHubList)
                    {
                        bool addedRemoteNode = Stack.m_knownHubList.Add(sendingConnection.RemoteNodeInfo);
                        if (Stack.m_knownHubList.Add(sendingConnection.RemoteKnownHubList) || addedRemoteNode)
                        {
                            Stack.SendKnownHubListMessage();
                        }
                    }

                    sendingConnection.SetConnectionState(StackConnection.State.Active);
                }
                finally
                {                    
                    connectionResponseHandlerInfo.HandshakeEstablished.Pulse();
                }
            }


            /// <summary>
            /// Add a stack connection.
            /// </summary>
            /// <param name="connection">The connection to add to the stack.</param>
            internal void AddConnection(StackConnection connection)
            {
                if (null == connection)
                {
                    throw new System.ArgumentNullException("connection");
                }

                // Hook into the events.
                // Do so before it gets added to the list.
                connection.OnLocalNodeInfo += Stack.HandleLocalNodeInfoMessage;
                connection.OnKnownHubList += Stack.HandleKnownHubListMessage;
                connection.OnQuery += Stack.HandleQueryMessage;
                connection.OnQueryAcknowledgement += Stack.HandleQueryAcknowledgementMessage;
                connection.OnQueryHashTable += Stack.HandleQueryHashTableMessage;
                connection.OnClosed += HandleClosedStackConnection;
                connection.OnAnyMessage += Stack.HandleAnyMessage;
                connection.OnUnknownMessage += Stack.HandleUnknownStackConnectionMessage;
                connection.OnSendingMessage += Stack.HandleSendingMessage;

                // Add the connection to the pending list.  It will stay here until the connection
                // response is received.
                m_pendingConnections.Add(connection);

                // Tell the connection to connect its own event handlers.
                connection.Start();
            }

            /// <summary>
            /// The count of the number of hubs that are in the persistent connection list.
            /// This property iterates over the list every time and is thus mildly expensive
            /// to use.
            /// </summary>
            public int PersistentHubConnectionCount
            {
                get
                {
                    int hubCount = 0;
                    Action<StackConnection> countHubs = delegate(StackConnection aConnection)
                    {
                        // If the connection is a hub, add it to the count.
                        if (null != aConnection.RemoteNodeInfo &&
                            aConnection.RemoteNodeInfo.IsHub)
                        {
                            hubCount++;
                        }
                    };
                    m_persistentConnections.ForEach(countHubs);
                    return hubCount;
                }
            }

            /// <summary>
            /// Removes a connection from the stack and disconnects all handlers.
            /// </summary>
            /// <param name="connection">the connection to remove</param>
            /// <returns>True if the connection was removed succesfully, otherwise false.</returns>
            private bool RemoveConnection(StackConnection connection)
            {
                if (null == connection)
                {
                    return false;
                }

                bool wasConnected = false;

                // stop holding on to the connection
                ConnectionHolder.RemoveConnection(connection);

                // Unhook the events.
                connection.OnLocalNodeInfo -= Stack.HandleLocalNodeInfoMessage;
                connection.OnKnownHubList -= Stack.HandleKnownHubListMessage;
                connection.OnQuery -= Stack.HandleQueryMessage;
                connection.OnQueryAcknowledgement -= Stack.HandleQueryAcknowledgementMessage;
                connection.OnQueryHashTable -= Stack.HandleQueryHashTableMessage;
                connection.OnClosed -= HandleClosedStackConnection;
                connection.OnAnyMessage -= Stack.HandleAnyMessage;
                connection.OnUnknownMessage -= Stack.HandleUnknownStackConnectionMessage;
                connection.OnSendingMessage -= Stack.HandleSendingMessage;

                // Remove the connection.
                m_pendingConnections.Remove(connection);
                if (m_temporaryConnections.Remove(connection))
                {
                    wasConnected = true;
                }
                if (m_volatileConnections.Remove(connection))
                {
                    wasConnected = true;
                }
                if (m_persistentConnections.Remove(connection))
                {
                    wasConnected = true;
                }

                return wasConnected;
            }

            /// <summary>
            /// Find an existing stack connection.
            /// </summary>
            /// <param name="info">The remote node info to search for.</param>
            /// <param name="persistentOnly">True if only persistent connections are to be searched.</param>
            /// <returns>A stack connection, or null if not found.</returns>
            public StackConnection FindConnection(NodeInfo info, bool persistentOnly)
            {
                if (null == info)
                {
                    throw new ArgumentNullException("info");
                }

                return FindConnection(info.NodeGUID, persistentOnly);
            }

            /// <summary>
            /// Find an existing stack connection by ID.
            /// </summary>
            /// <param name="nodeID">The remote node ID to search for.</param>
            /// <param name="persistentOnly">True if only persistent connections are to be searched.</param>
            /// <returns>A stack connection, or null if not found.</returns>
            public StackConnection FindConnection(System.Guid nodeID, bool persistentOnly)
            {
                Predicate<StackConnection> isMatch = delegate(StackConnection element)
                {
                    if (element == null)
                    {
                        element.Tracer.DebugWriteLine("Connection from {0} found that is null.", element.FriendlyName);
                        return false;
                    }
                    if (element.IsClosed)
                    {
                        element.Tracer.DebugWriteLine("Connection from {0} found that is closed.", element.FriendlyName);
                        return false;
                    }
                    NodeInfo info = element.RemoteNodeInfo;
                    if (null == info)
                    {
                        element.Tracer.DebugWriteLine("Connection from {0} found that has null remote node info.", element.FriendlyName);
                        return false;
                    }
                    if (info.NodeGUID.Equals(nodeID))
                    {
                        element.Tracer.DebugWriteLine("Connection from {0} to {1} found.", Stack.NodeInfo.FriendlyName, info.FriendlyName);
                        return true;
                    }
                    return false;
                };

                StackConnection foundConnection = m_persistentConnections.Find(isMatch);
                if (null == foundConnection && !persistentOnly)
                {
                    foundConnection = m_temporaryConnections.Find(isMatch);
                }
                if (null != foundConnection)
                {
                    foundConnection.Tracer.Enabled = this.Tracer.Enabled;
                }
                return foundConnection;
            }

            /// <summary>
            /// If the connection is pending, wait for it and find it afterward.
            /// </summary>
            /// <param name="nodeGUID"></param>
            /// <param name="persistentOnly"></param>
            /// <returns></returns>
            private StackConnection FindPendingConnection(Guid nodeGUID, bool persistentOnly)
            {
                if (nodeGUID == Guid.Empty)
                {
                    return null;
                }

                // Determine if the connection is already pending.
                StackConnection connection = m_pendingConnections.Find(
                    delegate(StackConnection element)
                    {
                        NodeInfo remoteNodeInfo = element.RemoteNodeInfo;
                        return remoteNodeInfo != null && remoteNodeInfo.NodeGUID.Equals(nodeGUID);
                    });
                if (null != connection)
                {
                    // The connection is already pending; wait for it to finish.
                    if (WaitForPendingConnection(connection))
                    {
                        // Return it if it has the persistence we are looking for.
                        return FindConnection(nodeGUID, persistentOnly);
                    }
                }
                return null;
            }

            /// <summary>
            /// Waits until a pending connection is no longer pending.
            /// </summary>
            /// <param name="connection">The connection to wait for.  If not a pending connection, returns true immediately.</param>
            /// <returns>False if timed out; true otherwise.</returns>
            private bool WaitForPendingConnection(StackConnection connection)
            {
                DateTime startTime = DateTime.Now;
                while (m_pendingConnections.Contains(connection))
                {
                    if (DateTime.Now - startTime > m_connectionTimeOut)
                    {
                        // It took too long.  Remove the pending connection.
                        m_pendingConnections.Remove(connection);
                        return false;
                    }

                    // Wait for a moment.
                    System.Threading.Thread.Sleep(50);
                }
                return true;
            }

            /// <summary>
            /// A lock to control adding to the m_getConnectionList member.
            /// </summary>
            private object m_getConnectionListAddingLock = new object();

            /// <summary>
            /// A list of Node ID Guids which are currently active inside the GetConnection function.
            /// </summary>
            private DynamicList<Guid> m_getConnectionList = new DynamicList<Guid>();

            /// <summary>
            /// Find, or create, a connection.
            /// </summary>
            /// <param name="info">The connection node info.</param>
            /// <param name="persistentOnly">True if persistent only.</param>
            /// <param name="timeoutForReverseConnection">
            /// Amount of time to allow for a reverse connection, or TimeSpan.Zero if no reverse connection
            /// is to be attempted.
            /// </param>
            /// <returns>The stack connection</returns>
            public StackConnection GetConnection(NodeInfo info, bool persistentOnly, TimeSpan timeoutForReverseConnection)
            {
                DateTime start = DateTime.Now;

                if (info.Equals(Stack.NodeInfo))
                {
                    this.Tracer.WriteLine("Stack {0} attempted to connect to itself.", this.ToString());
                    return null;
                }

                // Only allow one thread through at a time to connect to any single NodeID (by guid).
                if (info.NodeGUID != null)
                {
                    bool connectionIsBeingTriedByAnotherThread;
                    do
                    {
                        lock (m_getConnectionListAddingLock)
                        {
                            connectionIsBeingTriedByAnotherThread = m_getConnectionList.Contains(info.NodeGUID);
                            if (!connectionIsBeingTriedByAnotherThread)
                            {
                                m_getConnectionList.Add(info.NodeGUID);
                            }
                        }
                        if (connectionIsBeingTriedByAnotherThread)
                        {
                            System.Threading.Thread.Sleep(TimeSpan.FromSeconds(0.2));
                        }
                    } while (connectionIsBeingTriedByAnotherThread);
                }

                var now = DateTime.Now;

                if (now-start > TimeSpan.FromSeconds(1))
                {
                    System.Diagnostics.Trace.WriteLine("Waiting for another thread took " + (now-start) + " connection to node " + info);
                }

                start = now;

                // NOTE:  DO NOT do an early return from this function past this point without removing the 
                // NodeGuid from the list.

                // Find an existing connection to the originating node.
                StackConnection connection = FindConnection(info, persistentOnly);
                
                if (null == connection)
                {
                    // We couldn't find one; look in pending connections.
                    connection = FindPendingConnection(info.NodeGUID, persistentOnly);
                }

                if (connection == null)
                {
                    if (!IsUnreachableNode(info))
                    {
                        connection = TryToCreateConnection(info, persistentOnly, timeoutForReverseConnection);

                        if (connection == null)
                        {
                            MarkNodeAsUnreachable(info);
                        }
                    }
                }
                
                if (connection != null)
                {
                    connection.Tracer.Enabled = this.Tracer.Enabled;
                }
                else
                {
                    Tracer.DebugWriteLine("Failed to connect to {0}.", info.FriendlyName);
                }

                // Remove this guid from the list 
                if (info.NodeGUID != null)
                {
                    lock (m_getConnectionListAddingLock)
                    {
                        m_getConnectionList.Remove(info.NodeGUID);
                    }
                }

                now = DateTime.Now;
                var timeTook = now - start ;

                if (timeTook > TimeSpan.FromSeconds(1))
                {
                    System.Diagnostics.Trace.WriteLine("create connection to node " + info + " took " + (now - start));
                }

                if (timeTook > TimeSpan.FromSeconds(30))
                {
                    Logging.Categories.Connection.Log(String.Format("GetConnection({0}).Time", info), timeTook.ToString());
                }

                return connection;
            }

            private StackConnection TryToCreateConnection(NodeInfo info, bool persistentOnly, TimeSpan timeoutForReverseConnection)
            {
                StackConnection connection = null;

                // Used to control assignment to the connection that will be returned.
                Object lockConnection = new Object();

                // We wait on this event before trying to get a reverse connection.
                SynchronizationEvent forwardConnectionHappened = new SynchronizationEvent();

                System.Threading.ThreadStart connect =
                    delegate()
                    {
                        try
                        {
                            if (connection == null)
                            {
                                StackConnection newConnection = CreateConnection(info, persistentOnly);
                                if (connection == null && newConnection != null)
                                {
                                    lock (lockConnection)
                                    {
                                        if (connection == null)
                                        {
                                            connection = newConnection;
                                        }
                                    }
                                    forwardConnectionHappened.Pulse();
                                }
                            }
                        }
                        catch (ConnectionException exception)
                        {
                            Tracer.WriteLine("GetConnection: connecting to {0} threw {1}", info.ToString(), exception.ToString());
                        }
                    };

                if (timeoutForReverseConnection.Equals(TimeSpan.Zero))
                {
                    // Connect from this side, on the main thread.
                    Tracer.WriteLine("Attempting direct connection to {0} from {1} (no reverse).", info.FriendlyName, Stack.NodeInfo.FriendlyName);
                    connect();
                }
                else
                {
                    // Create a thread to create a connection from this side.
                    Tracer.WriteLine("Attempting background-thread connection to {0} from {1}.", info.FriendlyName, Stack.NodeInfo.FriendlyName);
                    System.Threading.ThreadPool.QueueUserWorkItem(delegate(Object unused) { connect(); });

                    // Wait for a moment to allow for a fast connection, and prevent unnecessary reverse connection.
                    // In effect we are giving the direct connection a "head start" in the race to complete.
                    // Since many well behave connections will happen in less than 0.2 seconds, many times this
                    // head start code will negate having to use the reverse connection logic at all and thus be easier
                    // on the network traffic.
                    // The forward connection happening will pulse this to let it through faster.
                    forwardConnectionHappened.AddTimedPulse(TimeSpan.FromSeconds(1));
                    forwardConnectionHappened.Wait();
                    if (null == connection)
                    {
                        // Use a message relay with an embedded connector to connect from the other side.
                        Tracer.WriteLine("Attempting reverse connection to {0} (i.e. {0} to {1}).", info.FriendlyName, Stack.NodeInfo.FriendlyName);
                        StackConnector connector = new StackConnector(Stack.NodeInfo, true);
                        MessageRelayer relayer = new MessageRelayer(Stack, new MessageRelay(connector.ToMessage(), info.NodeGUID), 1000);
                        relayer.Start();

                        // Wait for a connection, either from this side or the other.
                        DateTime startTime = DateTime.Now;
                        while (connection == null)
                        {
                            // Check to see if the connection exists.
                            StackConnection foundConnection = FindConnection(info, persistentOnly);
                            if (connection == null && foundConnection != null)
                            {
                                lock (lockConnection)
                                {
                                    if (connection == null)
                                    {
                                        connection = foundConnection;
                                    }
                                }
                                break;
                            }

                            // Wait for a moment, and break if the timeout has been passed.
                            System.Threading.Thread.Sleep(500);
                            if (timeoutForReverseConnection < (DateTime.Now - startTime))
                            {
                                break;
                            }
                        }

                        // Make sure the relayer is stopped.
                        relayer.Stop();
                    }
                }

                return connection;
            }

            private object m_notReachableNodesLockObject = new object();
            private Dictionary<NodeInfo, DateTime> m_notReachableNodes = new Dictionary<NodeInfo, DateTime>();

            private bool IsUnreachableNode(NodeInfo candidateHub)
            {
                lock (m_notReachableNodesLockObject)
                {
                    if (!m_notReachableNodes.ContainsKey(candidateHub))
                    {
                        //hub not in list
                        return false;
                    }

                    if ((DateTime.Now - m_notReachableNodes[candidateHub]) < Properties.Settings.Default.RetryConnectingHubsTimeout)
                    {
                        //failed to connect to hub is still fresh
                        return true;
                    }

                    //try to reconnect after 5 min
                    m_notReachableNodes.Remove(candidateHub);
                    return false;
                }
            }

            private void MarkNodeAsUnreachable(NodeInfo hub)
            {
                lock (m_notReachableNodesLockObject)
                {
                    m_notReachableNodes[hub] = DateTime.Now;
                }
            }

            /// <summary>
            /// Add the known hubs from this stack into the list.
            /// </summary>
            /// <param name="candidateHubs">The list to add the known hub info to.</param>
            public void GetHubs(IList<NodeInfo> hubs)
            {
                Action<StackConnection> addToList = delegate(StackConnection connection)
                {
                    // If the connection is a hub, add it to the list.
                    if (null != connection.RemoteNodeInfo &&
                        connection.RemoteNodeInfo.IsHub)
                    {
                        if (hubs.Contains(connection.RemoteNodeInfo))
                        {
                            Tracer.DebugWriteLine(
                                "Cannot add hub '{0}' to list because it's already there.",
                                connection.RemoteNodeInfo.FriendlyName);
                        }
                        else
                        {
                            Tracer.DebugWriteLine(
                                "Adding hub '{0}' to list.",
                                connection.RemoteNodeInfo.FriendlyName);

                            hubs.Add(connection.RemoteNodeInfo);
                        }
                    }
                };
                m_persistentConnections.ForEach(addToList);
                m_temporaryConnections.ForEach(addToList);
                m_volatileConnections.ForEach(addToList);
            }

            /// <summary>
            /// Handle a closed stack connection.
            /// </summary>
            /// <param name="connection">The closed connection.</param>
            public void HandleClosedStackConnection(StackConnection connection)
            {
                System.Diagnostics.Debug.Assert(connection.IsClosed,
                    "We're in Stack.HandleClosedStackConnection, but the connection isn't closed.");

                connection.Tracer.DebugWriteLine("Handling closed connection to {1} inside {0}.",
                    Stack.NodeInfo.FriendlyName, connection.ToString());

                // Remove this connection from our lists of connections if we find it in there.
                // This must be done before calling KnownHubList.Add, so that it gets removed
                // from the "connected" list.
                RemoveConnection(connection);

                NodeInfo remoteNodeInfo = connection.RemoteNodeInfo;
                if (null != remoteNodeInfo)
                {
                    lock (Stack.m_lockKnownHubList)
                    {
                        // If it's not found, adjust known hub list.
                        // Note that if there are connections with a null remote node info,
                        // we can still proceed because when the connection updates its remote node info,
                        // it will be sending a new known hub list message, and all such code is 
                        // locked on m_knownHubList ensuring that the state of the stack is always consistent.
                        if (null == FindConnection(remoteNodeInfo, true))
                        {
                            // Move connection from "connected hubs" to "known hubs" in known hub list.
                            if (remoteNodeInfo.IsHub)
                            {
                                System.Diagnostics.Debug.Assert(!Stack.NodeInfo.Equals(remoteNodeInfo),
                                    "The stack's connected hub list contained the stack itself.");

                                // This should remove it from the connected list, and add it to the known list.
                                if (Stack.m_knownHubList.Add(remoteNodeInfo))
                                {
                                    Stack.SendKnownHubListMessage();
                                }
                            }
                        }
                    }
                }

                if (connection.ConnectionState != StackConnection.State.Constructed)
                {
                    Logging.Categories.Connection.Log("State(" + connection.RemoteNodeInfo + ")", "Closed");
                }

                // Propagate the event.
                ClosedConnectionHandler handler = Stack.m_onClosedConnection;
                if (null != handler)
                {
                    handler(Stack, connection);
                }
            }

            #region Connection Request

            public void HandleConnectionRequestMessage(object sender, MessageReceivedEventArgs args)
            {
                // Get the connection.
                StackConnection connection = args.Context.Sender;

                StackConnectionRequest connectionRequest;

                // Get the connection request.
                try
                {
                    connectionRequest = new StackConnectionRequest(args.Message);
                }
                catch
                {
                    Stack.m_tracer.DebugWriteLine("{0} sent a badly formed connection request message {1}",
                        args.Context.Sender.ToString(),
                        args.Message.ToString());
                    return;
                }

                Stack.m_tracer.DebugWriteLine("Connection request received by {0} from {1} with key {2}",
                    Stack.NodeInfo, connectionRequest.FromNodeInfo,
                    DLM.PrivateKey.GetKeyDescription(connectionRequest.FromNodeInfo.PublicKey));

                // Create the connection response.
                StackConnectionResponse connectionResponse = new StackConnectionResponse(
                    connectionRequest.IsPersistent, Stack.NodeInfo, Stack.KnownHubList);

                // Remove the connection from the pending connections list.
                if (!m_pendingConnections.Remove(connection))
                {
                    // Add error to connection response, send, and return.
                    Tracer.WriteLine("Attempting to connect to non-pending.  Setting connection error.");
                    connectionResponse.Error = StackConnectionResponse.ErrorType.NodeNotPending;
                    connection.SendMessage(connectionResponse.ToMessage());
                    return;
                }

                // Ensure that the ID is a match.
                if (!connectionRequest.IsToNodeGUID(Stack.NodeInfo.NodeGUID))
                {
                    // Add error to connection response, send, and return.
                    Tracer.WriteLine("Attempting to connect to {0} and found wrong node {1}.  Setting connection error.",
                        connectionRequest.ToNodeGUID, Stack.NodeInfo.NodeGUID);
                    connectionResponse.Error = StackConnectionResponse.ErrorType.IncorrectNode;
                    connection.SendMessage(connectionResponse.ToMessage());
                    return;
                }

                // Ensure that we're not connecting to self.
                Tracer.DebugWriteLine("Checking if {0} and {1} are the same node.", connectionRequest.FromNodeInfo, Stack.NodeInfo);
                Tracer.DebugWriteLine("NodeGUID: {0}, {1}.", connectionRequest.FromNodeInfo.NodeGUID, Stack.NodeInfo.NodeGUID);
                Tracer.DebugWriteLine("NodeID: {0}, {1}.", connectionRequest.FromNodeInfo.NodeId, Stack.NodeInfo.NodeId);
                if (connectionRequest.FromNodeInfo.Equals(Stack.NodeInfo))
                {
                    // Add error to connection response, send, and return.
                    Tracer.WriteLine("Attempting to connect to self.  Setting connection error.");
                    connectionResponse.Error = StackConnectionResponse.ErrorType.SameNode;
                    connection.SendMessage(connectionResponse.ToMessage());
                    return;
                }

                // Add the connection to the proper list.
                if (connectionRequest.IsPersistent)
                {
                    // TODO: put this code in when the other side of the connection can handle it.
                    // The other side of the connection should be able to see this error, close this connection, and then
                    // look for the other persistent connection by the NodeID that is returned in the StackConnectionResponse.
                    //if (FindConnection(connectionRequest.FromNodeInfo.NodeGUID, true) != null)
                    //{
                    //    connection.Tracer.DebugWriteLine("Duplicate persistent connection to {0}'s persistent connections from {1}.  Refused.",
                    //        NodeInfo.FriendlyName, connectionRequest.FromNodeInfo.FriendlyName);
                    //    connectionResponse.Error = StackConnectionResponse.ErrorType.DuplicateConnection;
                    //    connection.SendMessage(connectionResponse.ToMessage());
                    //    return;
                    //}
                    //else
                    //{
                    // Move to persistent.
                    connection.Tracer.DebugWriteLine("Moving pending connection from {1} to {0}'s persistent connections.",
                        Stack.NodeInfo.FriendlyName, connectionRequest.FromNodeInfo.FriendlyName);
                    m_persistentConnections.Add(connection);
                    //}
                }
                else
                {
                    // Move to volatile.
                    connection.Tracer.DebugWriteLine("Moving pending connection to {0}'s volatile connections.",
                        Stack.NodeInfo.FriendlyName);
                    m_volatileConnections.Add(connection);
                }

                // Update connection's remote node info with FromNodeInfo.
                connection.RemoteNodeInfo = connectionRequest.FromNodeInfo;

                // Update connection's remote known hub list with FromKnownHubList.
                connection.RemoteKnownHubList = connectionRequest.FromKnownHubList;

                // Add the node info to our known hubs and
                // update our Known Hub List from this connections Known Hub List
                lock (Stack.m_lockKnownHubList)
                {
                    bool addedRemoteNode = Stack.m_knownHubList.Add(connection.RemoteNodeInfo);
                    if (Stack.m_knownHubList.Add(connection.RemoteKnownHubList) || addedRemoteNode)
                    {
                        Stack.SendKnownHubListMessage();
                    }
                }
                
                // Send connection response.
                connection.SendMessage(connectionResponse.ToMessage());
                
                connection.SetConnectionState(StackConnection.State.Active);

                // Let everyone know we're connected.
                Stack.OnConnected(this, connection);
            }

            #endregion

            #endregion

            internal void SendMessage(Message message)
            {
                Action<StackConnection> action = delegate(StackConnection connection)
                {
                    connection.SendMessage(message);
                };
                m_persistentConnections.ForEach(action);
                m_temporaryConnections.ForEach(action);
                m_volatileConnections.ForEach(action);
            }

            /// <summary>
            /// Send a Ping message to all attached nodes.
            /// </summary>
            internal void SendPing()
            {
                Action<StackConnection> action = delegate(StackConnection connection)
                {
                    connection.SendPing();
                };
                m_persistentConnections.ForEach(action);
                m_temporaryConnections.ForEach(action);
                m_volatileConnections.ForEach(action);
            }

        }
    }
}
