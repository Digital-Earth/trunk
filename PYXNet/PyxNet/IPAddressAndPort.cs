using System;
using System.Collections.Generic;

namespace PyxNet
{
    /// <summary>
    /// This is a class that holds an IP address and port,
    /// used as a workaround for the fact that System.Net.IPEndPoint is not serializable.
    /// </summary>
    [Serializable]
    public class IPAddressAndPort
    {
        [NonSerialized]
        private readonly System.Net.IPEndPoint m_ipEndPoint;

        public System.Net.IPEndPoint IPEndPoint
        {
            get
            {
                return m_ipEndPoint;
            }
        }

        public String IPAddress
        {
            get
            {
                return m_ipEndPoint.Address.ToString();
            }
            set
            {
                m_ipEndPoint.Address = System.Net.IPAddress.Parse(value);
            }
        }

        public int Port
        {
            get
            {
                return m_ipEndPoint.Port;
            }
            set
            {
                m_ipEndPoint.Port = value;
            }
        }

        public IPAddressAndPort() : this("127.0.0.1", 40)
        {
        }

        public IPAddressAndPort(String ipAddress, int port)
        {
            m_ipEndPoint = new System.Net.IPEndPoint(System.Net.IPAddress.Parse(ipAddress), port);
        }

        public IPAddressAndPort(System.Net.IPEndPoint ipEndPoint)
        {
            m_ipEndPoint = ipEndPoint;
        }

        public static List<IPAddressAndPort> Deserialize(String serialized)
        {
            List<IPAddressAndPort> addressesAndPorts = new List<IPAddressAndPort>();
            if (serialized.Trim().Length != 0)
            {
                try
                {
                    String xmlBuffer = serialized.Trim().Trim('\0');
                    System.IO.StringReader input = new System.IO.StringReader(xmlBuffer);

                    System.Xml.Serialization.XmlSerializer s =
                        new System.Xml.Serialization.XmlSerializer(addressesAndPorts.GetType());

                    addressesAndPorts = (List<IPAddressAndPort>)s.Deserialize(input);
                }
                catch (Exception ex)
                {
                    // If anything went wrong with deserializing, then ignore.
                    System.Diagnostics.Trace.WriteLine(string.Format(
                        "Could not deserialize IP address and port: {0}", ex.Message));
                }
            }
            return addressesAndPorts;
        }

        public static String Serialize(List<IPAddressAndPort> addressesAndPorts)
        {
            System.IO.StringWriter outputStream = new System.IO.StringWriter(
                new System.Text.StringBuilder());

            System.Xml.Serialization.XmlSerializer s =
                new System.Xml.Serialization.XmlSerializer(addressesAndPorts.GetType());
            s.Serialize(outputStream, addressesAndPorts);
            return outputStream.ToString();
        }

        public static List<System.Net.IPEndPoint> DeserializeIPEndPoints(String serialized)
        {
            List<System.Net.IPEndPoint> ipEndPoints = new List<System.Net.IPEndPoint>();

            List<IPAddressAndPort> addressesAndPorts = IPAddressAndPort.Deserialize(serialized);

            // Convert addresses and ports to list of ipEndPoints.
            addressesAndPorts.ForEach(
                delegate(IPAddressAndPort addressAndPort)
                {
                    ipEndPoints.Add(addressAndPort.IPEndPoint);
                });

            return ipEndPoints;
        }

        public static String SerializeIPEndPoints(List<System.Net.IPEndPoint> ipEndPoints)
        {
            // Convert ipEndPoints list to list of IPAddressAndPort.
            List<IPAddressAndPort> addressesAndPorts = new List<IPAddressAndPort>();
            ipEndPoints.ForEach(
                delegate(System.Net.IPEndPoint endPoint)
                {
                    addressesAndPorts.Add(new IPAddressAndPort(endPoint.Address.ToString(), endPoint.Port));
                });

            return IPAddressAndPort.Serialize(addressesAndPorts);
        }
    }
}