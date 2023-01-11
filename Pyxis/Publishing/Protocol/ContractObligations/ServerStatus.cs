/******************************************************************************
ServerStatus.cs

begin		: Oct. 8, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Diagnostics;
using System.IO;
using Pyxis.Contract.Services.GeoWebStreamService;

namespace Pyxis.Publishing.Protocol.ContractObligations
{
    public class ServerStatus : IServerStatus
    {
        //private static PerformanceCounter s_cpuCounter = new PerformanceCounter("Processor", "% Processor Time", "_Total");

        public static IServerStatus CurrentStatus()
        {
            ServerStatus status = new ServerStatus();

            //Get DiskSpace
            DriveInfo driveInfo = new DriveInfo(Environment.CurrentDirectory.Substring(0, 1));
            status.AvailableDiskSpaceMB = driveInfo.AvailableFreeSpace / (1024 * 1024);

            //CPU
           status.AverageCPU = 0;
           

            //Network
            status.NetworkStatus = Protocol.ContractObligations.NetworkStatus.CurrentStatus();

            //TODO Version
            status.Version = Properties.Resources.Version;
            return status;
        }

        public double AvailableDiskSpaceMB { get; set; }

        public float AverageCPU { get; set; }

        public string Version { get; set; }

        public INetworkStatus NetworkStatus { get; set; }
    }
}