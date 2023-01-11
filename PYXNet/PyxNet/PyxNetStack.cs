using System;
using System.Collections.Generic;
using System.Management;
using System.Net.NetworkInformation;
using System.Text;
using Pyxis.Utilities;

namespace PyxNet
{
    // TODO: Rename to LocalNode, replace derivation from stack with composition,
    // and modify members accordingly (e.g. IsLocalNodeStarted/StartLocalNode/StopLocalNode to
    // IsStarted/Start/Stop).
    /// <summary>
    /// A stack that will remember its GUID between runs, and publishes a 
    /// method to enable connection to the PyxNet network.  It also monitors
    /// network events and restarts the stack's listeners when network events
    /// are fired.
    /// </summary>
    public class PyxNetStack : Stack
    {
        #region Tracer

        private readonly NumberedTraceTool<PyxNetStack> m_tracer =
            new NumberedTraceTool<PyxNetStack>(TraceTool.GlobalTraceLogEnabled);

        new public Pyxis.Utilities.NumberedTraceTool<PyxNetStack> Tracer
        {
            get { return m_tracer; }
        }

        #endregion

        public PyxNetStack(int initialPort, int numberOfPortsToTry)
            : this(initialPort, numberOfPortsToTry, new PyxNetStackDefaultConfiguration())
        {
        }

        public PyxNetStack(int initialPort, int numberOfPortsToTry, IPyxNetStackConfiguration configuration)
            : base(configuration.GetPrivateKey(), configuration.GetNodeID())
        {
            InitialPort = initialPort;
            NumberOfPortsToTry = numberOfPortsToTry;
            Configuration = configuration;

            StartNetwork();
        }

        #region IDisposable

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                StopNetwork();

                base.Dispose(disposing);
            }
        }

        #endregion
        
        #region Diagnostics

        private static string NullSafeToString(object o)
        {
            if (o == null)
            {
                return "null value";
            }
            return o.ToString();
        }

        private static void DumpQualifiers(StringBuilder output, QualifierDataCollection collection, string indent)
        {
            foreach (QualifierData q in collection)
            {
                output.Append(indent);
                output.Append("Qualifier: ");
                output.Append(q.Name);
                output.Append(", ");
                output.AppendLine(NullSafeToString(q.Value));
            }
        }

        private static void DumpProperties(StringBuilder output, PropertyDataCollection properties, string indent)
        {
            if (indent.Length > 15)
            {
                output.AppendLine (indent + "**** Recursion too deep: ABORTED ****");
                return;
            }

            foreach (PropertyData property in properties)
            {
                output.Append(indent);
                output.Append("Property Name: ");
                output.AppendLine(property.Name);
                output.Append(indent);
                output.Append("  Type: ");
                output.AppendLine(property.Type.ToString());
                output.Append(indent);
                output.Append("  Origin: ");
                output.AppendLine(property.Origin);
                output.Append(indent);
                output.Append("  Value: ");
                output.AppendLine(NullSafeToString(property.Value));
                DumpQualifiers(output, property.Qualifiers, indent + "  ");

                ManagementBaseObject valueObject = property.Value as ManagementBaseObject;
                if (valueObject != null)
                {
                    DumpManagementBaseObject(output, valueObject, indent + "  ");
                }
            }
        }

        private static void DumpManagementBaseObject(StringBuilder output, ManagementBaseObject o, string indent)
        {
            output.Append(indent);
            output.Append("Management Base Object Class Name: ");
            output.AppendLine(o.ClassPath.ClassName);
            DumpQualifiers(output, o.Qualifiers, indent);
            DumpProperties(output, o.Properties, indent);
        }

        #endregion

        #region Network Events

        private readonly Object m_networkEventMutex = new Object();

        void HandleNetworkAvailabilityChanged(object sender, NetworkAvailabilityEventArgs e)
        {
            lock (m_networkEventMutex)
            {
                if (e.IsAvailable)
                {
                    Tracer.WriteLine("The network has become available; starting local node.");
                    StartLocalNode();
                }
                else
                {
                    Tracer.WriteLine("The network has become unavailable; stopping local node.");
                    StopLocalNode();
                }
            }
        }

        void HandleNetworkAddressChanged(object sender, EventArgs e)
        {
            lock (m_networkEventMutex)
            {
                Tracer.WriteLine("An network address has changed; restarting local node.");
                StopLocalNode();
                StartLocalNode();
            }
        }

        public void RestartLocalNode()
        {
            lock (m_networkEventMutex)
            {
                Tracer.WriteLine("restarting local node.");
                StopLocalNode();
                StartLocalNode();
            }
        }

        /// <summary>
        /// Starts the local network node, and starts receiving network events.
        /// Does not throw.
        /// </summary>
        private void StartNetwork()
        {
            lock (m_networkEventMutex)
            {
                // Start local network node (i.e. listening and connecting).
                StartLocalNode();

                // Start handling network address change and connection events.
                NetworkChange.NetworkAvailabilityChanged += HandleNetworkAvailabilityChanged;
                NetworkChange.NetworkAddressChanged += HandleNetworkAddressChanged;
            }
        }

        /// <summary>
        /// Stops receiving network events, and stops the local network node.
        /// Does not throw.
        /// </summary>
        private void StopNetwork()
        {
            lock (m_networkEventMutex)
            {
                // Stop handling network address change and connection events.
                NetworkChange.NetworkAddressChanged -= HandleNetworkAddressChanged;
                NetworkChange.NetworkAvailabilityChanged -= HandleNetworkAvailabilityChanged;

                // Stop local network node (i.e. listening and connecting).
                StopLocalNode();
            }
        }

        #endregion

        #region Configuraiton

        public IPyxNetStackConfiguration Configuration { get; private set; }

        #endregion

        #region Port

        /// <summary>
        /// The default number of ports to try.
        /// </summary>
        public const int DefaultNumberOfPortsToTry = 10;

        /// <summary>
        /// The port that Pyxnet stacks connect to, and  the
        /// default pyxnet hubs listen on.
        /// </summary>
        public const int HubPort = 44044;

        /// <summary>
        /// The port that nodes try and listen on.
        /// </summary>
        public const int NodePort = 44017;

        /// <summary>
        /// The license server port
        /// </summary>
        public const int LicensePort = 44064;

        /// <summary>
        /// The initial port to try to listen on.
        /// </summary>
        public int InitialPort
        {
            set
            {
                m_initialPort = value;
            }
            get
            {
                return m_initialPort;
            }
        }

        private int m_initialPort = NodePort;

        /// <summary>
        /// The port being listened on.
        /// </summary>
        public int Port
        {
            get
            {
                return m_port;
            }
        }

        private int m_port = NodePort;

        #endregion

        #region Local Addresses

        /// <summary>
        /// Get all the IP addresses for this computer.
        /// </summary>
        /// <returns>The local IP addresses.</returns>
        private List<System.Net.IPAddress> GetLocalAddresses()
        {
            List<System.Net.IPAddress> addresses = new List<System.Net.IPAddress>();
            NetworkInterface[] nics = NetworkInterface.GetAllNetworkInterfaces();
            foreach (NetworkInterface adapter in nics)
            {
                if (adapter.OperationalStatus == OperationalStatus.Up &&
                    adapter.Supports(NetworkInterfaceComponent.IPv4))
                {
                    IPInterfaceProperties ipProps = adapter.GetIPProperties();
                    UnicastIPAddressInformationCollection uniCast = ipProps.UnicastAddresses;
                    // the loopback address will not return IP V4r properties.
                    IPv4InterfaceProperties v4Props = ipProps.GetIPv4Properties();
                    if (uniCast != null && v4Props != null)
                    {
                        foreach (UnicastIPAddressInformation uni in uniCast)
                        {
                            if (uni.Address.AddressFamily.Equals(System.Net.Sockets.AddressFamily.InterNetwork))
                            {
                                addresses.Add(uni.Address);
                            }
                        }
                    }
                }
            }

            return addresses;
        }

        #endregion

        #region Start and Stop

        private bool m_isLocalNodeStarted = false;
        private string m_localNodeStartingError = "";

        /// <summary>
        /// Returns true if the local network node is started.
        /// Start and stop are transactional, so this blocks until each is finished.
        /// </summary>
        public bool IsLocalNodeStarted
        {
            get
            {
                lock (m_networkEventMutex)
                {
                    return m_isLocalNodeStarted;
                }
            }
        }

        public string LocalNodeStartingErrorMessage
        {
            get
            {
                lock (m_networkEventMutex)
                {
                    return m_localNodeStartingError;
                }
            }
        }

        private int m_numberOfPortsToTry = DefaultNumberOfPortsToTry;

        /// <summary>
        /// The number of ports to try and open a listening socket on.
        /// This only kicks in if we can not listen on the port we want,
        /// and have to try subsequent ports.
        /// </summary>
        public int NumberOfPortsToTry
        {
            get { return m_numberOfPortsToTry; }
            set { m_numberOfPortsToTry = value; }
        }

        /// <summary>
        /// Listens on the first free port >= initialPort.
        /// </summary>
        /// <param name="stack">The stack.</param>
        /// <param name="initialPort">The initial port to try listening on.</param>
        /// <param name="port">The first free port >= initialPort chosen to listen on.</param>
        /// <returns>True if successful.</returns>
        private bool Listen(int initialPort, out int port)
        {
            port = initialPort;

            // Get all the internal IP addresses for this computer.
            List<System.Net.IPAddress> addresses = GetLocalAddresses();
            if (0 == addresses.Count)
            {
                return false;
            }

            // Get external IP addresses for this computer from setting.
            List<System.Net.IPEndPoint> externalEndPoints = Configuration.GetExternalIPEndPoints();

            // Try to start listening on a port offset; if exception, repeat for the next port offset.
            for (; port - initialPort < NumberOfPortsToTry; ++port)
            {
                // Construct network address.
                List<System.Net.IPEndPoint> internalEndPointsToListenOn = new List<System.Net.IPEndPoint>();
                foreach (System.Net.IPAddress address in addresses)
                {
                    internalEndPointsToListenOn.Add(new System.Net.IPEndPoint(address, port));
                }
                NetworkAddress listenOnAddress = new NetworkAddress(internalEndPointsToListenOn, externalEndPoints);

                try
                {
                    if (base.Start(listenOnAddress))
                    {
                        return true;
                    }
                }
                catch (System.Net.Sockets.SocketException)
                {
                }
            }

            return false;
        }

        /// <summary>
        /// Starts listening and connecting on the local network node.
        /// </summary>
        private bool StartLocalNode()
        {
            lock (m_networkEventMutex)
            {
                Tracer.WriteLine("Starting Local Node");
                m_localNodeStartingError = "";
                if (m_isLocalNodeStarted)
                {
                    return false;
                }

                if (!NetworkInterface.GetIsNetworkAvailable())
                {
                    m_localNodeStartingError = "Network is not available, failed to start local node";
                    Tracer.WriteLine("Network is not available, failed to start local node");
                    return false;
                }

                // Start listening.
                if (!Listen(m_initialPort, out m_port))
                {
                    m_localNodeStartingError = "Failed to obtain a port, failed to start local node";
                    Tracer.WriteLine("Failed to obtain a port, failed to start local node");
                    return false;
                }

                // Get external port mappings for the stack.
                GetExternalPortMappings();

                if (NodeInfo.Address.ExternalIPEndPoints.Count == 0)
                {
                    m_localNodeStartingError = "Failed to external port, contiune without external port";
                    Tracer.WriteLine("Failed to external port, contiune without external port");
                }

                if (NodeInfo.FriendlyName == "")
                {
                    // Set the friendly name of the stack.
                    String stackName = Configuration.GetDefaultStackName();
                    if (stackName.Trim().Length == 0)
                    {
                        stackName = IPGlobalProperties.GetIPGlobalProperties().HostName;
                    }
                    NodeInfo.FriendlyName = stackName;
                }

                Pyxis.Utilities.Logging.LogRepository.SetLocalNodeInfo(NodeInfo.FriendlyName, NodeInfo.NodeGUID);

                // Iterate through persistent connections and tell each to refresh.
                lock (m_persistentConnections)
                {
                    foreach (PersistentConnection connection in m_persistentConnections)
                    {
                        connection.Clear();
                        connection.Reconnect = true;
                    }
                }

                Publisher.Start();

                Tracer.WriteLine("The local node has been started.");
                m_isLocalNodeStarted = true;
            }

            return true;
        }

        /// <summary>
        /// Stops listening and connecting on the local network node.
        /// </summary>
        private bool StopLocalNode()
        {
            lock (m_networkEventMutex)
            {
                if (!m_isLocalNodeStarted)
                {
                    return false;
                }

                // Iterate through persistent connections and tell each to stop trying to connect.
                lock (m_persistentConnections)
                {
                    foreach (PersistentConnection connection in m_persistentConnections)
                    {
                        connection.Reconnect = false;
                        connection.Close();
                    }
                }
                StackSingleton.Stack.ConnectionManager.TemporaryConnections.Clear();
                StackSingleton.Stack.ConnectionManager.VolatileConnections.Clear();

                if (!base.Stop())
                {
                    return false;
                }

                Tracer.WriteLine("The local node has been stopped.");
                m_isLocalNodeStarted = false;
            }

            return true;
        }

        #endregion

        #region Connecting

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

        private TimeSpan m_timeBetweenRetries = PersistentConnection.DefaultTimeBetweenRetries;

        private readonly LinkedList<PersistentConnection> m_persistentConnections =
            new LinkedList<PersistentConnection>();

        /// <summary>
        /// Add the address to the pending address connection queue.
        /// </summary>
        /// <param name="connectToNetworkAddress">The network address to connect to.</param>
        private void Connect(NetworkAddress connectToNetworkAddress)
        {
            lock (m_persistentConnections)
            {
                PersistentConnection connection =
                    new PersistentConnection(this, connectToNetworkAddress, TimeBetweenRetries);
                m_persistentConnections.AddLast(connection);
            }
        }

        /// <summary>
        /// Add the host name to the pending host name connection queue.
        /// </summary>
        /// <param name="hostName"></param>
        public void Connect(String hostName)
        {
            lock (m_persistentConnections)
            {
                PersistentConnection connection =
                    new PersistentConnection(this, hostName, HubPort, TimeBetweenRetries);
                m_persistentConnections.AddLast(connection);
            }
        }

        private List<String> m_defaultHubs = null;

        /// <summary>
        /// Gets the default hubs.  (These are the dns names of the hubs that PyxNet 
        /// connects to "automatically")
        /// </summary>
        /// <value>The default hubs.</value>
        /// <remarks>
        /// This could be read from a configuration file, and might eventually include 
        /// an option to save known hubs for reconnection on restart.
        /// </remarks>
        public List<String> DefaultHubs
        {
            get
            {
                if (m_defaultHubs == null)
                {
                    m_defaultHubs = new List<string> 
                    { 
                        "pyxnet.pyxisinnovation.com", 
                        "pyxnet2.pyxisinnovation.com"
                    };
                }
                return m_defaultHubs;
            }
        }

        /// <summary>
        /// Connect to the default hub(s).
        /// </summary>
        public void Connect()
        {
            foreach (var hub in DefaultHubs)
            {
                Connect(hub);
            }
        }

        #endregion
    }
}

// TODO:  unit test this now that it is not all static.
