using System;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Net.NetworkInformation;

namespace PyxisCLI.Server.Utilities
{
    static class PortFinder
    {
        static HashSet<int> ListUsedTcpPort()
        {
            var usedPorts = new HashSet<int>();
            IPGlobalProperties ipGlobalProperties = IPGlobalProperties.GetIPGlobalProperties();
            IPEndPoint[] tcpConnInfoArray = ipGlobalProperties.GetActiveTcpListeners();
            IEnumerator myEnum = tcpConnInfoArray.GetEnumerator();

            while (myEnum.MoveNext())
            {
                IPEndPoint tcpInfo = (IPEndPoint)myEnum.Current;
                usedPorts.Add(tcpInfo.Port);
            }

            return usedPorts;
        }

        public static int GetFreePort(int fromPort, int maxPort = UInt16.MaxValue)
        {
            var usedPorts = ListUsedTcpPort();

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
