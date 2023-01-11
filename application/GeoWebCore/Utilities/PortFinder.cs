using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.NetworkInformation;
using System.Text;
using System.Threading.Tasks;

namespace GeoWebCore.Utilities
{
    static class PortFinder
    {
        static HashSet<int> ListUsedTCPPort()
        {
            var usedPorts = new HashSet<int>();
            IPGlobalProperties ipGlobalProperties = IPGlobalProperties.GetIPGlobalProperties();
            IPEndPoint[] tcpConnInfoArray = ipGlobalProperties.GetActiveTcpListeners();
            IEnumerator myEnum = tcpConnInfoArray.GetEnumerator();

            while (myEnum.MoveNext())
            {
                IPEndPoint TCPInfo = (IPEndPoint)myEnum.Current;
                usedPorts.Add(TCPInfo.Port);
            }

            return usedPorts;
        }

        static public int GetFreePort(int fromPort, int maxPort = UInt16.MaxValue)
        {
            var usedPorts = ListUsedTCPPort();

            for (var port = fromPort; port < maxPort; port++)
            {
                if (!usedPorts.Contains(port))
                {
                    return port;
                }
            }

            throw new Exception(String.Format("Failed to find free port in range {0}..{1}",fromPort,maxPort));
        }
    }
}
