using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;
using NetFwTypeLib;

using System.Collections;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.CustomMarshalers;

using System.Net;
using NatTraversal.Interop;


namespace NatTraversal.Interop
{
    [ComImport, Guid("624BD588-9060-4109-B0B0-1ADBBCAC32DF"), TypeLibType(4160)]
    internal interface INATEventManager
    {
        [DispId(1)]
        object ExternalIPAddressCallback { [param: In, MarshalAs(UnmanagedType.IUnknown)] [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(1)] set; }
        [DispId(2)]
        object NumberOfEntriesCallback { [param: In, MarshalAs(UnmanagedType.IUnknown)] [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(2)] set; }
    }

    [ComImport, TypeLibType(4160), Guid("6F10711F-729B-41E5-93B8-F21D0F818DF1")]
    internal interface IStaticPortMapping
    {
        [DispId(1)]
        string ExternalIPAddress { [return: MarshalAs(UnmanagedType.BStr)] [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(1)] get; }
        [DispId(2)]
        int ExternalPort { [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(2)] get; }
        [DispId(3)]
        int InternalPort { [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(3)] get; }
        [DispId(4)]
        string Protocol { [return: MarshalAs(UnmanagedType.BStr)] [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(4)] get; }
        [DispId(5)]
        string InternalClient { [return: MarshalAs(UnmanagedType.BStr)] [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(5)] get; }
        [DispId(6)]
        bool Enabled { [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(6)] get; }
        [DispId(7)]
        string Description { [return: MarshalAs(UnmanagedType.BStr)] [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(7)] get; }
        [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(8)]
        void EditInternalClient([In, MarshalAs(UnmanagedType.BStr)] string bstrInternalClient);
        [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(9)]
        void Enable([In] bool vb);
        [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(10)]
        void EditDescription([In, MarshalAs(UnmanagedType.BStr)] string bstrDescription);
        [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(11)]
        void EditInternalPort([In] int lInternalPort);
    }

    [ComImport, Guid("CD1F3E77-66D6-4664-82C7-36DBB641D0F1"), TypeLibType(4160)]
    internal interface IStaticPortMappingCollection /*: IEnumerable*/
    {
        [return: MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(EnumeratorToEnumVariantMarshaler))]
        [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(-4), TypeLibFunc(0x41)]
        IEnumerator GetEnumerator();

        [DispId(0)]
        IStaticPortMapping this[int lExternalPort, string bstrProtocol] { [return: MarshalAs(UnmanagedType.Interface)] [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(0)] get; }
        [DispId(1)]
        int Count { [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(1)] get; }
        [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(2)]
        void Remove([In] int lExternalPort, [In, MarshalAs(UnmanagedType.BStr)] string bstrProtocol);
        [return: MarshalAs(UnmanagedType.Interface)]
        [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(3)]
        IStaticPortMapping Add([In] int lExternalPort, [In, MarshalAs(UnmanagedType.BStr)] string bstrProtocol, [In] int lInternalPort, [In, MarshalAs(UnmanagedType.BStr)] string bstrInternalClient, [In] bool bEnabled, [In, MarshalAs(UnmanagedType.BStr)] string bstrDescription);
    }

    [ComImport, Guid("B171C812-CC76-485A-94D8-B6B3A2794E99"), TypeLibType(4160)]
    internal interface IUPnPNAT
    {
        [DispId(1)]
        IStaticPortMappingCollection StaticPortMappingCollection { [return: MarshalAs(UnmanagedType.Interface)] [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(1)] get; }
        [DispId(2)]
        object /*IDynamicPortMappingCollection*/ DynamicPortMappingCollection { [return: MarshalAs(UnmanagedType.Interface)] [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(2)] get; }
        [DispId(3)]
        INATEventManager NATEventManager { [return: MarshalAs(UnmanagedType.Interface)] [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime), DispId(3)] get; }
    }

    [ComImport, InterfaceType(ComInterfaceType.InterfaceIsIUnknown), Guid("9C416740-A34E-446F-BA06-ABD04C3149AE")]
    internal interface INATExternalIPAddressCallback
    {
        void NewExternalIPAddress([MarshalAs(UnmanagedType.BStr)]string newExternalIPAddress);
    }

    [ComImport, InterfaceType(ComInterfaceType.InterfaceIsIUnknown), Guid("C83A0A74-91EE-41B6-B67A-67E0F00BBD78")]
    internal interface INATNumberOfEntriesCallback
    {
        void NewNumberOfEntries(int newNumberOfEntries);
    }

    [ComImport, Guid("B171C812-CC76-485A-94D8-B6B3A2794E99"), CoClass(typeof(UPnPNATCreator))]
    internal interface UPnPNAT : IUPnPNAT { }

    [ComImport, TypeLibType(2), Guid("AE1E00AA-3FD5-403C-8A27-2BBDC30CD0E1"), ClassInterface(ClassInterfaceType.None)]
    internal class UPnPNATCreator { }
}


namespace NatTraversal
{
    /// <summary>
    /// Wrapper for information about a specific mapped port on a UPnP Router.
    /// </summary>
    public class PortMappingInfo
    {

        #region Fields
        /// <summary>
        /// true if this mapping is enabled, otherwise false.
        /// </summary>
        private bool enabled;

        /// <summary>
        /// Text summary of what this mapping is for.
        /// </summary>
        private string description;

        /// <summary>
        /// The internal IP address (host) this mapping points to.
        /// </summary>
        private string internalHostName;

        /// <summary>
        /// Internal Port number for this mapping.
        /// </summary>
        private int internalPort;

        /// <summary>
        /// External IP address for this port mapping.
        /// </summary>
        private IPAddress externalIPAddress;

        /// <summary>
        /// External port for this mapping.
        /// </summary>
        private int externalPort;

        /// <summary>
        /// Protocol (UDP or TCP) for this port mapping.
        /// </summary>
        private string protocol;

        #endregion

        #region Instance Management

        /// <summary>
        /// Public constructor for a port mapping info object.
        /// </summary>
        /// <param name="description">Description of mapping.</param>
        /// <param name="protocol">Protocol for mapping.</param>
        /// <param name="internalHostName">Internal IP address to use for mapping.</param>
        /// <param name="internalPort">Internal port for this mapping.</param>
        /// <param name="externalIPAddress">External IP address for this mapping - NOT USED.</param>
        /// <param name="externalPort">External port for this mapping.</param>
        /// <param name="enabled">true if this mapping is enabled, otherwise false.</param>
        public PortMappingInfo( string description,
                                string protocol,
                                string internalHostName,
                                int internalPort,
                                IPAddress externalIPAddress,
                                int externalPort,
                                bool enabled)
        {

            // Initializes fields
            this.enabled = enabled;
            this.description = description;
            this.internalHostName = internalHostName;
            this.internalPort = internalPort;
            this.externalIPAddress = externalIPAddress;
            this.externalPort = externalPort;
            this.protocol = protocol;
        }

        #endregion

        #region Properties

        /// <summary>
        /// Accessor for internal IP address.
        /// </summary>
        public string InternalHostName
        {
            get
            {
                return internalHostName;
            }
        }

        /// <summary>
        /// Accessor for internal port value.
        /// </summary>
        public int InternalPort
        {
            get
            {
                return internalPort;
            }
        }

        /// <summary>
        /// Accessor for external IP address.
        /// </summary>
        public IPAddress ExternalIPAddress
        {
            get
            {
                return externalIPAddress;
            }
        }

        /// <summary>
        /// Accessor for external port number.
        /// </summary>
        public int ExternalPort
        {
            get
            {
                return externalPort;
            }
        }

        /// <summary>
        /// Accessor for protocol.
        /// </summary>
        public string Protocol
        {
            get
            {
                return protocol;
            }
        }

        /// <summary>
        /// Accessor for enabled flag.
        /// </summary>
        public bool Enabled
        {
            get
            {
                return enabled;
            }
        }

        /// <summary>
        /// Accessor for description of this port mapping.
        /// </summary>
        public string Description
        {
            get
            {
                return description;
            }
        }

        #endregion
    }

    public class PortMapping: IDisposable 
    {
        private NatTraversal.PortMappingInfo m_externalPortMapping = null;

        private System.Net.IPEndPoint m_externalIPEndPoint = null;
        public System.Net.IPEndPoint ExternalIPEndPoint
        {
            get
            {
                return m_externalIPEndPoint;
            }
        }

        public static PortMapping Create(System.Net.IPEndPoint internalIPEndPoint)
        {
            return Create(internalIPEndPoint,null);
        }

        public static PortMapping Create(System.Net.IPEndPoint internalIPEndPoint, string uniqueId)
        {
            PYXNATTraversal.PYXNatControl pyxNatControl = new PYXNATTraversal.PYXNatControl();

            string mappingName = "PyxNet TCPIP Port";
            if (!String.IsNullOrEmpty(uniqueId))
            {
                mappingName = String.Format("PyxNet({0},TCP)", uniqueId);

                var reusedMapping = TryUseExsitingPortMapping(pyxNatControl, mappingName, internalIPEndPoint);

                if (reusedMapping != null)
                {
                    return reusedMapping;
                }
            }
            
            return CreateNewMapping(pyxNatControl, mappingName, internalIPEndPoint);
        }

        private static PortMapping TryUseExsitingPortMapping(PYXNATTraversal.PYXNatControl pyxNatControl, string mappingName, System.Net.IPEndPoint internalIPEndPoint)
        {
            int count = 0;
            foreach (var foundMapping in pyxNatControl.getPortMappings(ref count).Where(x => x.Description == mappingName))
            {
                if (foundMapping.InternalHostName == internalIPEndPoint.Address.ToString() &&
                    foundMapping.InternalPort == internalIPEndPoint.Port)
                {
                    //we can safly reuse this mapping
                    return new PortMapping(foundMapping);
                }
                else
                {
                    //TODO: this line will force to have only 1 mapping available on rounter. need to make sure this is ok
                    pyxNatControl.RemovePortMapping(foundMapping);
                }
            }
            
            //we wasn't able to use the existing mapping
            return null;
        }

        private static PortMapping CreateNewMapping(PYXNATTraversal.PYXNatControl pyxNatControl, string mappingName, System.Net.IPEndPoint internalIPEndPoint)
        {
            int externalPort = internalIPEndPoint.Port;
            IPAddress externalIPAddress = null;
            string externalIPString = null;

            NatTraversal.PortMappingInfo newPortMapping =
                new NatTraversal.PortMappingInfo(
                    mappingName,
                    "TCP",
                    internalIPEndPoint.Address.ToString(), 
                    internalIPEndPoint.Port,
                    externalIPAddress, 
                    externalPort,
                    true);
            
            if (pyxNatControl.AddPortMapping(newPortMapping, out externalIPString, out externalPort))
            {
                var result = new PortMapping(
                    new PortMappingInfo(
                        newPortMapping.Description,
                        newPortMapping.Protocol,
                        newPortMapping.InternalHostName,
                        newPortMapping.InternalPort,
                        System.Net.IPAddress.Parse(externalIPString),
                        externalPort,
                        true));

                return result;                
            }

            return null;
        }

        private PortMapping(PortMappingInfo portInfo)
        {
            //store portInfo for clean up
            m_externalPortMapping = portInfo;

            // And store the mapping for client use.
            m_externalIPEndPoint = new System.Net.IPEndPoint(
                System.Net.IPAddress.Parse(portInfo.ExternalIPAddress.ToString()), portInfo.ExternalPort);
        }

        /// <summary>
        /// Destructor
        /// </summary>
        ~PortMapping()
        {
            Dispose(false);
        }

        #region IDisposable
        private bool m_disposed = false;

        /// <summary>
        /// Dispose of this object (as per IDisposable)
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Implementation of Dispose - will be called from Dispose or destructor.
        /// </summary>
        /// <remarks>Do NOT touch member variables if disposing is false!</remarks>
        /// <param name="disposing"></param>
        private void Dispose(bool disposing)
        {
            if (!this.m_disposed)
            {
                if (disposing)
                {                   
                    if (m_externalPortMapping != null)
                    {
                        try
                        {
                            PYXNATTraversal.PYXNatControl control = new PYXNATTraversal.PYXNatControl();
                            if (control.RemovePortMapping(m_externalPortMapping) == false)
                            {
                                System.Diagnostics.Trace.WriteLine("Error disposing of a port mapping.");
                            }
                        }
                        catch (Exception ex)
                        {
                            // Eat the exception, but send some output to the trace window.
                            System.Diagnostics.Trace.WriteLine(
                                String.Format("Error disposing of a port mapping: {0}.", ex.ToString()));
                        }
                    }
                }
                else
                {
                    // If we don't dispose of the port mapping, the router will eventually become overloaded.
                    System.Diagnostics.Trace.WriteLine( 
                        "Warning: NatTraversal.PortMapping was not disposed.");
                }
            }
            m_disposed = true;
        }
        #endregion /* IDispose */
    }
}

namespace PYXNATTraversal
{
    /// <summary>
    /// Provide access to generic information about the active network adapter in this machine.
    /// Currently only active IP address and Host Name are supported.
    /// </summary>
    public class PYXAdapterInfo
    {
        /// <summary>
        /// IP Address accessor.
        /// </summary>
        /// <returns>First active IP address for this computer as returned by DNS.</returns>
        public string getIPAddress()
        {
            //  String strHostName = Dns.GetHostName();

            // Find host by name
            IPHostEntry iphostentry = Dns.GetHostEntry("");

            // Enumerate IP addresses
            foreach (IPAddress ipaddress in iphostentry.AddressList)
            {
                // Return the first IPv4 address.
                if (ipaddress.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)
                {
                    return ipaddress.ToString();
                }
            }

            return "";
        }

        /// <summary>
        /// Host name accessor.
        /// </summary>
        /// <returns>Host name of this computer as reurned by DNS.</returns>
        public string getHostName()
        {
            return Dns.GetHostName();
        }
    }

    /// <summary>
    /// Wrapper for values returned from PYXNatControl completion notifications.
    /// Values and variables present are based on which function notification is being used.  Unused values are null.
    /// If success is false, all other values returned will be null.
    /// </summary>
    public class PYXNatProperty
    {
        /// <summary>
        /// Returned IP Address.
        /// </summary>
        private string m_strIPAddress;

        /// <summary>
        /// Returned port number.
        /// </summary>
        private int m_nPort;

        /// <summary>
        /// true if function completed successfully, otherwise false.
        /// </summary>
        private bool m_bSuccess;

        /// <summary>
        /// Array of PortMappingInfo objects (1 for getmapping, multiple for getMappings)
        /// </summary>
        private NatTraversal.PortMappingInfo[] m_portMappings;

        /// <summary>
        /// Number of items contained in the above "m_portMappings" variable.
        /// </summary>
        private int m_nCount;

        /// <summary>
        /// Create a PYXNatProperty obejct with an IP address and port with a
        /// boolean specifying if the function succeeded or not.
        /// </summary>
        /// <param name="strIPAddress">IP address from returned function.  Function determines exact meaning of address.</param>
        /// <param name="nPort">Port number returned from completed function.  Function determines exact meaning of port.</param>
        /// <param name="bSuccess">true if function completed successfully, otherwise false.</param>
        public PYXNatProperty(string strIPAddress, int nPort, bool bSuccess)
        {
            m_bSuccess = bSuccess;
            m_nPort = nPort;
            m_strIPAddress = strIPAddress;
            m_nCount = 0;
            m_portMappings = null;
        }

        /// <summary>
        /// Create a PYXNatProperty obejct with an array of portMappings,
        /// count of items in the array and a boolean specifying if the
        /// function succeeded or not.
        /// </summary>
        /// <param name="portMappings"></param>
        /// <param name="nCount"></param>
        /// <param name="bSuccess"></param>
        public PYXNatProperty(NatTraversal.PortMappingInfo[] portMappings, int nCount, bool bSuccess)
        {
            m_bSuccess = bSuccess;
            m_nPort = 0;
            m_strIPAddress = null;
            m_nCount = nCount;
            m_portMappings = portMappings;
        }

        /// <summary>
        /// Accessor for IP address.
        /// </summary>
        public string ipAddress
        {
            get { return m_strIPAddress; }
        }

        /// <summary>
        /// Accessor for Port number.
        /// </summary>
        public int port
        {
            get { return m_nPort; }
        }

        /// <summary>
        /// Accessor for count of items in portMappingsInfo array.
        /// </summary>
        public int count
        {
            get { return m_nCount; }
        }

        /// <summary>
        /// true if function succeeded, otherwise false.
        /// </summary>
        public bool success
        {
            get { return m_bSuccess; }
        }

        /// <summary>
        /// Array of port mappings for "getPortMappings" function and 1 item for "getPortMapping"
        /// </summary>
        public NatTraversal.PortMappingInfo[] mappings
        {
            get { return m_portMappings; }
        }
    }

    /// <summary>
    /// Provide functions for adding, removing and obtaining port mappings for a UPnP router.
    /// </summary>
    public class PYXNatControl
    {
        /// <summary>
        /// Internal parameter storage for "addPortMapping" function.
        /// </summary>
        private NatTraversal.PortMappingInfo m_addPortMapping;

        /// <summary>
        /// Internal parameter storage for "removePortMapping" function.
        /// </summary>
        private NatTraversal.PortMappingInfo m_removePortMapping;

        /// <summary>
        /// Internal parameter storage for "getPortMapping" function.
        /// </summary>
        private NatTraversal.PortMappingInfo m_getPortMapping;

        /// <summary>
        /// Internal result storage for "addPortMapping" function.
        /// </summary>
        private PYXNatProperty m_pyxNatAddProperty;

        /// <summary>
        /// Internal result storage for "removePortMapping" function.
        /// </summary>
        private PYXNatProperty m_pyxNatRemoveProperty;

        /// <summary>
        /// Internal result storage for "getPortMapping" function.
        /// </summary>
        private PYXNatProperty m_pyxNatGetMappingProperty;

        /// <summary>
        /// Internal result storage for "getPortMappings" function.
        /// </summary>
        private PYXNatProperty m_pyxNatGetMappingsProperty;

        /// <summary>
        /// Instance storage of UPnPNAT object so it is only created once per PYXNatControl object.
        /// </summary>
        private UPnPNAT m_nat;

        /// <summary>
        /// Internal lock to allow thread safety between various functions.
        /// </summary>
        private Object thisLock = new Object();

        /// <summary>
        /// Accessor to get (and if not already done, create) a UPnPNAT object.
        /// </summary>
        /// <returns>UPnPNat object.</returns>
        UPnPNAT getUPnPNat()
        {
            lock (thisLock)
            {
                if (m_nat == null)
                {
                    m_nat = new UPnPNAT();

                    if (m_nat == null || m_nat.StaticPortMappingCollection == null)
                    {
                        System.Threading.Thread.Sleep(4000);
                        m_nat = new UPnPNAT();
                    }
                }
                return m_nat;
            }
        }

        #region PYXNatFunctionComplete Event
        /// <summary>
        /// Class which will be passed as the second argument to a PYXNatFunctionCompleteHandler which 
        /// wraps a PYXNatProperty object.
        /// </summary>
        public class PYXNatFunctionCompleteEventArgs : EventArgs
        {
            private PYXNatProperty m_PYXNatProperty;

            public PYXNatProperty PYXNatProperty
            {
                get { return m_PYXNatProperty; }
                set { m_PYXNatProperty = value; }
            }

            /// <summary>
            /// Wrapper for PYXNatProperty for events.
            /// </summary>
            /// <param name="thePYXNatProperty">The PYXNatProperty</param>
            internal PYXNatFunctionCompleteEventArgs(PYXNatProperty thePYXNatProperty)
            {
                m_PYXNatProperty = thePYXNatProperty;
            }
        }

        /// <summary>
        /// Event which is fired when addMapping has completed.
        /// </summary>
        public event EventHandler<PYXNatFunctionCompleteEventArgs> OnPYXNatAddComplete;

        /// <summary>
        /// Event which is fired when removeMapping has completed.
        /// </summary>
        public event EventHandler<PYXNatFunctionCompleteEventArgs> OnPYXNatRemoveComplete;

        /// <summary>
        /// Event which is fired when getMapping has completed.
        /// </summary>
        public event EventHandler<PYXNatFunctionCompleteEventArgs> OnPYXNatGetMappingComplete;

        /// <summary>
        /// Event which is fired when getMappings has completed.
        /// </summary>
        public event EventHandler<PYXNatFunctionCompleteEventArgs> OnPYXNatGetMappingsComplete;

        /// <summary>
        /// Method to safely raise event when the "addPortMappingWithThread" function completes.
        /// </summary>
        protected void OnPYXNatAddCompleteRaise()
        {
            EventHandler<PYXNatFunctionCompleteEventArgs> handler = OnPYXNatAddComplete;
            if (null != handler)
            {
                handler(this, new PYXNatFunctionCompleteEventArgs(m_pyxNatAddProperty));
            }
        }

        /// <summary>
        /// Method to safely raise event when the "removeMappingWithThread" function completes.
        /// </summary>
        protected void OnPYXNatRemoveCompleteRaise()
        {
            EventHandler<PYXNatFunctionCompleteEventArgs> handler = OnPYXNatRemoveComplete;
            if (null != handler)
            {
                handler(this, new PYXNatFunctionCompleteEventArgs(m_pyxNatRemoveProperty));
            }
        }

        /// <summary>
        /// Method to safely raise event when the "getMapingWithThread" function completes.
        /// </summary>
        protected void OnPYXNatGetMappingCompleteRaise()
        {
            EventHandler<PYXNatFunctionCompleteEventArgs> handler = OnPYXNatGetMappingComplete;
            if (null != handler)
            {
                handler(this, new PYXNatFunctionCompleteEventArgs(m_pyxNatGetMappingProperty));
            }
        }

        /// <summary>
        /// Method to safely raise event when the "getMappingsWithThread" function completes.
        /// </summary>
        protected void OnPYXNatGetMappingsCompleteRaise()
        {
            EventHandler<PYXNatFunctionCompleteEventArgs> handler = OnPYXNatGetMappingsComplete;
            if (null != handler)
            {
                handler(this, new PYXNatFunctionCompleteEventArgs(m_pyxNatGetMappingsProperty));
            }
        }

        #endregion PYXNatFunctionComplete Event

        /// <summary>
        /// Function to perform addPortMapping.  Run by stand alone function or thread.
        /// Will find the first free port starting with the provided port number and map it to
        /// the routers external IP address.  If a port is already mapped to the specified
        /// protocol, IP address and the description matches the supplied description, that
        /// port will be returned.
        /// </summary>
        private void addPortMappingThread()
        {
            m_pyxNatAddProperty = null;

            UPnPNAT nat = getUPnPNat();

            if (nat != null && nat.StaticPortMappingCollection != null)
            {
                // Get first port starting at specified number that is either already mapped to the IP address
                // specified, or is not already mapped.

                int nExternalPort = m_addPortMapping.ExternalPort;
                string strExternalIP = null;

                IStaticPortMapping mapping = null;

                bool bAlreadyMapped = false;

                do
                {
                    try
                    {
                        if ((nat != null) && (nat.StaticPortMappingCollection != null))
                        {
                            mapping = nat.StaticPortMappingCollection[nExternalPort, m_addPortMapping.Protocol];
                        }

                        if (mapping != null)
                        {
                            if (m_addPortMapping.InternalHostName != mapping.InternalClient)
                            {
                                ++nExternalPort;
                            }
                            else
                            {
                                if (m_addPortMapping.Description == mapping.Description)
                                {
                                    strExternalIP = mapping.ExternalIPAddress.ToString();
                                    mapping = null;
                                    bAlreadyMapped = true;
                                }
                                else
                                {
                                    ++nExternalPort;
                                }
                            }
                        }
                    }
                    catch (Exception e)
                    {
                        System.Diagnostics.Debug.WriteLine(
                            String.Format("First block threw exception {0}.", e.Message));

                        mapping = null;
                    }
                }
                while (mapping != null);

                if (!bAlreadyMapped)
                {
                    try
                    {
                        IStaticPortMapping staticPortMap = nat.StaticPortMappingCollection.Add(nExternalPort, m_addPortMapping.Protocol, m_addPortMapping.InternalPort, m_addPortMapping.InternalHostName, m_addPortMapping.Enabled, m_addPortMapping.Description);

                        if (staticPortMap != null)
                        {
                            m_pyxNatAddProperty = new PYXNatProperty(staticPortMap.ExternalIPAddress.ToString(), nExternalPort, true);
                        }
                    }
                    catch (Exception e)
                    {
                        System.Diagnostics.Debug.WriteLine(
                            String.Format("Second block threw exception {0}.", e.Message));

                        m_pyxNatAddProperty = new PYXNatProperty("", 0, false);
                    }
                }
                else
                {
                    m_pyxNatAddProperty = new PYXNatProperty(strExternalIP, nExternalPort, true);
                }
            }

            if (m_pyxNatAddProperty == null)
            {
                m_pyxNatAddProperty = new PYXNatProperty("", 0, false);
            }

            OnPYXNatAddCompleteRaise();
        }

        /// <summary>
        /// Method to perform addPortMapping using a thread.  Event fired when complete.
        /// </summary>
        /// <param name="portMapping">Parameters of port to map.</param>
        public void AddPortMappingWithThread(NatTraversal.PortMappingInfo portMapping)
        {
            m_addPortMapping = portMapping;

            System.Threading.Thread pThread = new System.Threading.Thread(addPortMappingThread);
            pThread.IsBackground = true;
            pThread.Start();
        }

        /// <summary>
        /// Perform addPortMapping in a blocking call.  Can take several seconds to execute.
        /// </summary>
        /// <param name="portMapping">Parameters of port to map</param>
        /// <param name="strExternalIP">Returns external IP address of mapped port.</param>
        /// <param name="nPort">Returns external mapped port.</param>
        /// <returns>true if successful, otherwise false.</returns>
        public bool AddPortMapping(NatTraversal.PortMappingInfo portMapping, out string strExternalIP, out int nPort)
        {
            m_addPortMapping = portMapping;

            addPortMappingThread();

            strExternalIP = m_pyxNatAddProperty.ipAddress;
            nPort = m_pyxNatAddProperty.port;

            return m_pyxNatAddProperty.success;
        }

        /// <summary>
        /// Main routine called by stand alone fuction or thread to
        /// re move a port mapping from the router.
        /// </summary>
        private void removePortMappingThread()
        {
            m_pyxNatRemoveProperty = null;

            UPnPNAT nat = getUPnPNat();

            if (nat != null && nat.StaticPortMappingCollection != null)
            {
                nat.StaticPortMappingCollection.Remove(m_removePortMapping.ExternalPort, m_removePortMapping.Protocol);
                m_pyxNatRemoveProperty = new PYXNatProperty("", m_removePortMapping.ExternalPort, true);
            }
            else
            {
                m_pyxNatRemoveProperty = new PYXNatProperty("", 0, false);
            }

            OnPYXNatRemoveCompleteRaise();
        }

        /// <summary>
        /// Remove a port mapping from the router using a thread.  Event fired when compelte.
        /// </summary>
        /// <param name="portMapping">Parameters specifying which mapping to remove.</param>
        public void RemovePortMappingWithThread(NatTraversal.PortMappingInfo portMapping)
        {
            m_removePortMapping = portMapping;

            System.Threading.Thread pThread = new System.Threading.Thread(removePortMappingThread);
            pThread.IsBackground = true;
            pThread.Start();
        }

        /// <summary>
        /// Remove port mapping - blocking call.  Can take several seconds to execute.
        /// </summary>
        /// <param name="portMapping">Parmaeters of port to remove mapping for</param>
        /// <returns>true if successfully removed, otherwise false.</returns>
        public bool RemovePortMapping(NatTraversal.PortMappingInfo portMapping)
        {
            m_removePortMapping = portMapping;

            removePortMappingThread();

            return m_pyxNatRemoveProperty.success;
        }

        /// <summary>
        /// Get list of all mapped ports for the router.  Blocking call.  Can take
        /// 10-30 seconds to execute - depending on number of mappings (1-3 seconds
        /// per mapping) depending on router.
        /// </summary>
        /// <param name="nCount">Number of items in the returned portMappingInfo structure.</param>
        /// <returns>Array of port mapping info objects - one for each mapped port.</returns>
        public NatTraversal.PortMappingInfo[] getPortMappings(ref int nCount)
        {
            getPortMappingsThread();

            return m_pyxNatGetMappingsProperty.mappings;
        }

        /// <summary>
        /// Get all port mappings using a thread.  Event fired when complete.
        /// </summary>
        public void getPortMappingsWithThread()
        {
            System.Threading.Thread pThread = new System.Threading.Thread(getPortMappingsThread);
            pThread.IsBackground = true;
            pThread.Start();
        }

        /// <summary>
        /// Internal routine to perform getPortMappings function from either thread or
        /// stand alone function.
        /// </summary>
        private void getPortMappingsThread()
        {
            m_pyxNatGetMappingsProperty = null;

            UPnPNAT nat = getUPnPNat();

            if (nat != null && nat.StaticPortMappingCollection != null)
            {
                IEnumerator enumerator = nat.StaticPortMappingCollection.GetEnumerator();
                enumerator.Reset();

                // Builds port mappings list
                ArrayList portMappings = new ArrayList();

                IStaticPortMapping mapping = null;

                do
                {
                    try
                    {
                        mapping = null;

                        if (enumerator.MoveNext())
                            mapping = (IStaticPortMapping)enumerator.Current;
                    }
                    catch
                    {
                    }

                    if (mapping != null)
                    {
                        portMappings.Add(new NatTraversal.PortMappingInfo(mapping.Description,
                                                                mapping.Protocol.ToUpper(),
                                                                mapping.InternalClient,
                                                                mapping.InternalPort,
                                                                IPAddress.Parse(mapping.ExternalIPAddress),
                                                                mapping.ExternalPort,
                                                                mapping.Enabled));
                    }
                }
                while (mapping != null);

                // Now copies the ArrayList to an array of PortMappingInfo.
                NatTraversal.PortMappingInfo[] portMappingInfos = new NatTraversal.PortMappingInfo[portMappings.Count];
                portMappings.CopyTo(portMappingInfos);

                m_pyxNatGetMappingsProperty = new PYXNatProperty(portMappingInfos, portMappings.Count, true);
            }

            if (m_pyxNatGetMappingsProperty == null)
            {
                m_pyxNatGetMappingsProperty = new PYXNatProperty("", 0, false);
            }

            OnPYXNatGetMappingsCompleteRaise();
        }

        /// <summary>
        /// Get mapping for a single port using a thread.  Fires event when compelte.
        /// </summary>
        /// <param name="nExternalPort">Port number of mapping to return.</param>
        /// <param name="strProtocol">Protocol (TCP/UDP) of port to return mapping for.</param>
        public void getPortMappingWithThread(int nExternalPort, string strProtocol)
        {
            m_getPortMapping = new NatTraversal.PortMappingInfo(null, strProtocol, null, nExternalPort, null, nExternalPort, false);

            System.Threading.Thread pThread = new System.Threading.Thread(getPortMappingThread);
            pThread.IsBackground = true;
            pThread.Start();
        }

        /// <summary>
        /// Blocking call to get port mapping.  Can take several seconds to execute.
        /// </summary>
        /// <param name="nExternalPort">Port to get mapping information for.</param>
        /// <param name="strProtocol">Protocol "UDP/TCP" to get mapping for.</param>
        /// <returns>portMappingInfo structure containing information about mapped port if 'success' member is true.</returns>
        public NatTraversal.PortMappingInfo getPortMapping(int nExternalPort, string strProtocol)
        {
            m_getPortMapping = new NatTraversal.PortMappingInfo(null, strProtocol, null, nExternalPort, null, nExternalPort, false);

            getPortMappingThread();

            if (m_pyxNatGetMappingProperty != null)
            {
                try
                {
                    return m_pyxNatGetMappingProperty.mappings[0];
                }
                catch (NullReferenceException)
                {
                }
            }

            return null;
        }

        /// <summary>
        /// Internal routine to perform getMapping function. Called by thread and
        /// stand alone blocking function.
        /// </summary>
        private void getPortMappingThread()
        {
            m_pyxNatGetMappingProperty = null;

            UPnPNAT nat = getUPnPNat();

            if (nat != null && nat.StaticPortMappingCollection != null)
            {
                try
                {
                    IStaticPortMapping mapping = nat.StaticPortMappingCollection[m_getPortMapping.ExternalPort, m_getPortMapping.Protocol];

                    if (mapping != null)
                    {
                        NatTraversal.PortMappingInfo[] portMappingInfos = new NatTraversal.PortMappingInfo[1];
                        portMappingInfos[0] = new NatTraversal.PortMappingInfo(mapping.Description,
                                                                mapping.Protocol.ToUpper(),
                                                                mapping.InternalClient,
                                                                mapping.InternalPort,
                                                                IPAddress.Parse(mapping.ExternalIPAddress),
                                                                mapping.ExternalPort,
                                                                mapping.Enabled);

                        m_pyxNatGetMappingProperty = new PYXNatProperty(portMappingInfos, 1, true);
                    }
                }
                catch
                {
                    m_pyxNatGetMappingProperty = new PYXNatProperty("", 0, false);
                }
            }

            if (m_pyxNatGetMappingProperty == null)
            {
                m_pyxNatGetMappingProperty = new PYXNatProperty("", 0, false);
            }

            OnPYXNatGetMappingCompleteRaise();
        }
    }
}
