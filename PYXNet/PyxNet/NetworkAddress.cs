using System;
using System.Collections.Generic;

using Pyxis.Utilities;

namespace PyxNet
{
    /// <summary>
    /// Identifies a network address.
    /// </summary>
    public class NetworkAddress
    {
        #region Tracer

        private readonly static Pyxis.Utilities.NumberedTraceTool<NetworkAddress> s_tracer =
            new Pyxis.Utilities.NumberedTraceTool<NetworkAddress>(Pyxis.Utilities.TraceTool.GlobalTraceLogEnabled);

        public static Pyxis.Utilities.NumberedTraceTool<NetworkAddress> Tracer
        {
            get
            {
                return s_tracer;
            }
        }

        #endregion

        #region Data

        /// <summary>
        /// External IP addresses that refer to this address.
        /// </summary>
        private readonly DynamicList<System.Net.IPEndPoint> m_externalIPEndPoints =
            new DynamicList<System.Net.IPEndPoint>();

        /// <summary>
        /// External IP addresses that refer to this address.
        /// </summary>
        public DynamicList<System.Net.IPEndPoint> ExternalIPEndPoints
        {
            get
            {
                return m_externalIPEndPoints;
            }
        }

        /// <summary>
        /// Internal IP addresses represented by this address.
        /// </summary>
        private readonly DynamicList<System.Net.IPEndPoint> m_internalIPEndPoints;

        /// <summary>
        /// Internal IP addresses represented by this address.
        /// </summary>
        public DynamicList<System.Net.IPEndPoint> InternalIPEndPoints
        {
            get
            {
                return m_internalIPEndPoints;
            }
        }

        #endregion

        #region Constructors

        /// <summary>
        /// Constructs an empty address.
        /// </summary>
        public NetworkAddress()
        {
            m_internalIPEndPoints = new DynamicList<System.Net.IPEndPoint>();
        }

        /// <summary>
        /// Construct the address from a local IP address.
        /// </summary>
        /// <param name="address">The IP end point representing the address.</param>
        public NetworkAddress(System.Net.IPEndPoint ipAddress) : this()
        {
            if (null == ipAddress)
            {
                throw new ArgumentNullException("ipAddress");
            }

            m_internalIPEndPoints.Add(ipAddress);
        }

        /// <summary>
        /// Construct the address from a list of local IP addresses.
        /// </summary>
        /// <param name="ipAddresses">The local IP addresses.</param>
        public NetworkAddress(IEnumerable<System.Net.IPEndPoint> ipAddresses)
        {
            m_internalIPEndPoints = new DynamicList<System.Net.IPEndPoint>(ipAddresses);
        }

        /// <summary>
        /// Construct the address from lists of local internal and external IP addresses.
        /// </summary>
        /// <param name="internalIPEndPoints">The local internal IP addresses.</param>
        /// <param name="externalIPEndPoints">The local external IP addresses.</param>
        public NetworkAddress(
            IEnumerable<System.Net.IPEndPoint> internalIPEndPoints,
            IEnumerable<System.Net.IPEndPoint> externalIPEndPoints) : this(internalIPEndPoints)
        {
            m_externalIPEndPoints = new DynamicList<System.Net.IPEndPoint>(externalIPEndPoints);
        }

        /// <summary>
        /// Construct from a message.
        /// </summary>
        public NetworkAddress(MessageReader reader) : this()
        {
            FromMessageReader(reader);
        }

        /// <summary>
        /// Construct from a host and port.
        /// </summary>
        /// <param name="host">The host.</param>
        /// <param name="port">The port.</param>
        /// <exception cref="ArgumentNullException">Thrown if host could not be resolved.</exception>
        public NetworkAddress(System.Net.IPHostEntry host, int port)
            : this(ResolveHost(host, port))
        {
        }
        
        /// <summary>
        /// Construct from a host name and port.
        /// </summary>
        /// <param name="hostName">The host name.</param>
        /// <param name="port">The port.</param>
        /// <exception cref="ArgumentNullException">Thrown if host name could not be resolved.</exception>
        public NetworkAddress(String hostName, int port)
            : this(ResolveHostName(hostName), port)
        {
        }

        #endregion

        #region Host and Host Name Resolution

        /// <summary>
        /// A utility function that resolves an IP host entry and port to a list of IP endpoints.
        /// </summary>
        /// <param name="host">The host.</param>
        /// <param name="port">The port.</param>
        /// <exception cref="ArgumentNullException">Host cannot be null.</exception>
        /// <returns>A list of IP end points.</returns>
        static public List<System.Net.IPEndPoint> ResolveHost(System.Net.IPHostEntry host, int port)
        {
            if (null == host)
            {
                throw new ArgumentNullException("host");
            }

            List<System.Net.IPEndPoint> endPoints = new List<System.Net.IPEndPoint>();
            foreach (System.Net.IPAddress a in host.AddressList)
            {
                Tracer.DebugWriteLine("Host contained address {0}.", a);

                if (a.AddressFamily.Equals(System.Net.Sockets.AddressFamily.InterNetwork))
                {
                    Tracer.DebugWriteLine(
                        "Host's address {0} is usable; adding to list.", a);

                    System.Net.IPEndPoint endPoint = new System.Net.IPEndPoint(a, port);
                    endPoints.Add(endPoint);
                }
            }
            if (endPoints.Count == 0)
            {
                Tracer.DebugWriteLine(
                    "No IP endpoints were found in host {0}.", host.ToString());
            }

            return endPoints;
        }

        /// <summary>
        /// A utility function that resolves a host name to an IP host entry.
        /// </summary>
        /// <param name="hostName">The host name.</param>
        /// <exception cref="ArgumentNullException">Host name cannot be null.</exception>
        /// <returns>The IP host entry, or null.</returns>
        static public System.Net.IPHostEntry ResolveHostName(String hostName)
        {
            if (null == hostName)
            {
                throw new ArgumentNullException("hostName");
            }

            Tracer.DebugWriteLine(
                "[{1}] Resolving host name '{0}'.", hostName, System.Threading.Thread.CurrentThread.ManagedThreadId);

            System.Net.IPHostEntry host = null;
            try
            {
                host = System.Net.Dns.GetHostEntry(hostName);
            }
            catch (System.Net.Sockets.SocketException ex)
            {
                Tracer.WriteLine(
                    "Exception when getting DNS host entry for host name '{0}': {1}",
                    hostName, ex.ToString());
            }
            if (null == host)
            {
                Tracer.WriteLine(
                    "Unable to resolve host name for '{0}'.", hostName);
            }
            else
            {
                string [] addresses = new string[host.AddressList.Length];
                for (int i = 0; i < host.AddressList.Length; ++i )
                {
                    addresses[i] = host.AddressList[i].ToString();
                }
                Tracer.DebugWriteLine(
                    "Host name '{0}' resolved to {1}.", hostName, String.Join(", ", addresses));
            }

            return host;
        }

        #endregion

        #region String Conversion

        /// <summary>
        /// Display address information.
        /// </summary>
        /// <returns>A list of address:port in clear text format.</returns>
        public override string ToString()
        {
            System.Text.StringBuilder returnVal = new System.Text.StringBuilder();
            returnVal.Append("Internal: ( ");
            m_internalIPEndPoints.ForEach(delegate(System.Net.IPEndPoint ep)
            {
                returnVal.Append(ep.ToString());
                returnVal.Append(" ");
            });
            returnVal.Append("); External: ( ");
            m_externalIPEndPoints.ForEach(delegate(System.Net.IPEndPoint ep)
            {
                returnVal.Append(ep.ToString());
                returnVal.Append(" ");
            });
            returnVal.Append(")");

            return returnVal.ToString();
        }

        #endregion

        #region Equality

        /// <summary>
        /// Check for equality of contents.
        /// </summary>
        /// <param name="addressObject"></param>
        /// <returns>True if the addresses are equal.</returns>
        public override bool Equals(object addressObject)
        {
            if (null == addressObject)
            {
                return false;
            }

            NetworkAddress address = addressObject as NetworkAddress;
            if (null != address)
            {
                return Equals(address);
            }

            return false;
        }

        /// <summary>
        /// Check for equality of contents.
        /// </summary>
        /// <param name="address"></param>
        /// <returns>True if the addresses are equal.</returns>
        public bool Equals(NetworkAddress address)
        {
            {
                int length = m_internalIPEndPoints.Count;
                if (length != address.m_internalIPEndPoints.Count)
                {
                    return false;
                }
                for (int index = length - 1; 0 <= index; --index)
                {
                    if (!m_internalIPEndPoints[index].Equals(address.m_internalIPEndPoints[index]))
                    {
                        return false;
                    }
                }
            }

            {
                int length = m_externalIPEndPoints.Count;
                if (length != address.m_externalIPEndPoints.Count)
                {
                    return false;
                }
                for (int index = length - 1; 0 <= index; --index)
                {
                    if (!m_externalIPEndPoints[index].Equals(address.m_externalIPEndPoints[index]))
                    {
                        return false;
                    }
                }
            }

            return true;
        }

        /// <summary>
        /// Need to override GetHashCode() when you override Equals.  
        /// </summary>
        /// <returns>the hash code of the contained ip addresses.</returns>
        public override int GetHashCode()
        {
            throw new NotImplementedException();
        }

        #endregion

        #region Message Conversion

        /// <summary>
        /// Append the address to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            // Number of details.
            UInt16 detailCount = (UInt16)(m_internalIPEndPoints.Count + m_externalIPEndPoints.Count);
            message.Append(detailCount);

            // Do internal addresses (type 1).
            byte type = 1;
            m_internalIPEndPoints.ForEach(delegate(System.Net.IPEndPoint address)
            {
                message.Append(type);

                byte[] addressBytes = address.Address.GetAddressBytes();
                UInt16 port = (UInt16)address.Port;
                UInt16 length = (UInt16)(sizeof(UInt16) + addressBytes.Length);

                message.Append(length);
                message.Append(addressBytes);
                message.Append(port);
            });

            // Do external addresses (type 2).
            type = 2;
            m_externalIPEndPoints.ForEach(delegate(System.Net.IPEndPoint address)
            {
                message.Append(type);

                byte[] addressBytes = address.Address.GetAddressBytes();
                UInt16 port = (UInt16)address.Port;
                UInt16 length = (UInt16)(sizeof(UInt16) + addressBytes.Length);

                message.Append(length);
                message.Append(addressBytes);
                message.Append(port);
            });
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of an address.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            UInt16 detailCount = reader.ExtractUInt16();

            for (; detailCount > 0; --detailCount)
            {
                // Read detail.
                byte type = reader.ExtractByte();

                switch (type)
                {
                    case 1: // Internal address.
                        {
                            UInt16 length = reader.ExtractUInt16();
                            byte[] addressBytes = reader.ExtractBytes(length - sizeof(UInt16));
                            UInt16 port = reader.ExtractUInt16();

                            m_internalIPEndPoints.Add(new System.Net.IPEndPoint(new System.Net.IPAddress(addressBytes), port));
                        }

                        break;

                    case 2: // External address.
                        {
                            UInt16 length = reader.ExtractUInt16();
                            byte[] addressBytes = reader.ExtractBytes(length - sizeof(UInt16));
                            UInt16 port = reader.ExtractUInt16();

                            m_externalIPEndPoints.Add(new System.Net.IPEndPoint(new System.Net.IPAddress(addressBytes), port));
                        }

                        break;

                    default:
                        {
                            // Extract the length here, and then extract length bytes to discard.
                            UInt16 length = reader.ExtractUInt16();
                            byte[] bytesToDiscard = reader.ExtractBytes(length);
                        }

                        break;
                }
            }
        }

        #endregion
    }
}
