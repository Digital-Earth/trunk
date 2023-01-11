using System;
using System.Collections.Generic;
using System.Text;
using NetFwTypeLib;

namespace PYXNatTraversal
{
    /// <summary>
    /// Wrapper class for functions related to Windows Firewall (should work with other firewalls?)
    /// NOTE: Administrator privelidges are required to perform these functions successfully.
    /// </summary>
    /// <remarks>
    /// Currently unused.
    /// </remarks>
    public class PYXFireWall
    {
        /// <summary>
        /// Return state of firewall.
        /// </summary>
        /// <returns>1 if on, 0 if off, -1 on fail.</returns>
        public int isFireWallOn()
        {
            Moah.WinXPSP2FireWall fw = new Moah.WinXPSP2FireWall();
            Moah.WinXPSP2FireWall.FW_ERROR_CODE result = fw.Initialize();

            if (result == Moah.WinXPSP2FireWall.FW_ERROR_CODE.FW_NOERROR)
            {
                bool bOn = false;
                result = fw.IsWindowsFirewallOn(ref bOn);

                if (result == Moah.WinXPSP2FireWall.FW_ERROR_CODE.FW_NOERROR)
                {
                    return bOn ? 1 : 0;
                }
            }
            return -1;
        }

        /// <summary>
        /// Return state of specified port in firewall for UDP protocol.
        /// </summary>
        /// <param name="nPort">Port to check state of.</param>
        /// <returns>1 if open, 0 if closed, -1 on fail.</returns>
        public int isUDPPortOpen(int nPort)
        {
            Moah.WinXPSP2FireWall fw = new Moah.WinXPSP2FireWall();
            Moah.WinXPSP2FireWall.FW_ERROR_CODE result = fw.Initialize();

            if (result == Moah.WinXPSP2FireWall.FW_ERROR_CODE.FW_NOERROR)
            {
                bool bIsPortEnabled = false;
                result = fw.IsPortEnabled(nPort, NET_FW_IP_PROTOCOL_.NET_FW_IP_PROTOCOL_UDP, ref bIsPortEnabled);
                if (result == Moah.WinXPSP2FireWall.FW_ERROR_CODE.FW_NOERROR)
                {
                    return bIsPortEnabled ? 1 : 0;
                }
            }
            return -1;
        }

        /// <summary>
        /// Return state of specified port in firewall for TCP protocol.
        /// </summary>
        /// <param name="nPort">Port to check state of.</param>
        /// <returns>1 if open, 0 if closed, -1 on fail.</returns>
        public int isTCPPortOpen(int nPort)
        {
            Moah.WinXPSP2FireWall fw = new Moah.WinXPSP2FireWall();
            Moah.WinXPSP2FireWall.FW_ERROR_CODE result = fw.Initialize();

            bool bIsPortEnabled = false;

            if (result == Moah.WinXPSP2FireWall.FW_ERROR_CODE.FW_NOERROR)
            {
                result = fw.IsPortEnabled(nPort, NET_FW_IP_PROTOCOL_.NET_FW_IP_PROTOCOL_TCP, ref bIsPortEnabled);

                if (result == Moah.WinXPSP2FireWall.FW_ERROR_CODE.FW_NOERROR)
                {
                    return bIsPortEnabled ? 1 : 0;
                }
            }
            return -1;
        }

        /// <summary>
        /// Open UDP port through the fire wall.
        /// </summary>
        /// <param name="nPort">Port to open</param>
        /// <param name="strDescription">Description of this ports function.</param>
        /// <returns></returns>
        public bool addUDPPort(int nPort, string strDescription)
        {
            Moah.WinXPSP2FireWall fw = new Moah.WinXPSP2FireWall();
            Moah.WinXPSP2FireWall.FW_ERROR_CODE result = fw.Initialize();

            if (result == Moah.WinXPSP2FireWall.FW_ERROR_CODE.FW_NOERROR)
            {
                result = fw.AddPort(nPort, NET_FW_IP_PROTOCOL_.NET_FW_IP_PROTOCOL_UDP, strDescription);
            }

            return result == Moah.WinXPSP2FireWall.FW_ERROR_CODE.FW_NOERROR;
        }

        /// <summary>
        /// Open TCP port through the fire wall.
        /// </summary>
        /// <param name="nPort">Port to open.</param>
        /// <param name="strDescription">Description of this ports function.</param>
        /// <returns></returns>
        public bool addTCPPort(int nPort, string strDescription)
        {
            Moah.WinXPSP2FireWall fw = new Moah.WinXPSP2FireWall();
            Moah.WinXPSP2FireWall.FW_ERROR_CODE result = fw.Initialize();

            if (result == Moah.WinXPSP2FireWall.FW_ERROR_CODE.FW_NOERROR)
            {
                result = fw.AddPort(nPort, NET_FW_IP_PROTOCOL_.NET_FW_IP_PROTOCOL_TCP, strDescription);
            }

            return (result == Moah.WinXPSP2FireWall.FW_ERROR_CODE.FW_NOERROR) || (result == Moah.WinXPSP2FireWall.FW_ERROR_CODE.FW_ERR_SAME_PORT_EXIST);
        }

        /// <summary>
        /// Remove (close) this port in the firewall for UDP protocol.
        /// </summary>
        /// <param name="nPort">Port to remove mapping for.</param>
        /// <returns>true if successful, otherwise false.</returns>
        public bool removeUDPPort(int nPort)
        {
            Moah.WinXPSP2FireWall fw = new Moah.WinXPSP2FireWall();
            Moah.WinXPSP2FireWall.FW_ERROR_CODE result = fw.Initialize();

            if (result == Moah.WinXPSP2FireWall.FW_ERROR_CODE.FW_NOERROR)
            {
                result = fw.RemovePort(nPort, NET_FW_IP_PROTOCOL_.NET_FW_IP_PROTOCOL_UDP);
            }

            return result == Moah.WinXPSP2FireWall.FW_ERROR_CODE.FW_NOERROR;
        }

        /// <summary>
        /// Remove (close) this port in the firewall for TCP protocol.
        /// </summary>
        /// <param name="nPort">Port to remove mapping for.</param>
        /// <returns>true if successful, otherwise false.</returns>
        public bool removeTCPPort(int nPort)
        {
            Moah.WinXPSP2FireWall fw = new Moah.WinXPSP2FireWall();
            Moah.WinXPSP2FireWall.FW_ERROR_CODE result = fw.Initialize();

            if (result == Moah.WinXPSP2FireWall.FW_ERROR_CODE.FW_NOERROR)
            {
                result = fw.RemovePort(nPort, NET_FW_IP_PROTOCOL_.NET_FW_IP_PROTOCOL_TCP);
            }

            return result == Moah.WinXPSP2FireWall.FW_ERROR_CODE.FW_NOERROR;
        }
    }
}
