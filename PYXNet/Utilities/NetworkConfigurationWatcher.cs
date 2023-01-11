/******************************************************************************
NetworkConfigurationWatcher.cs

begin      : September 29, 2008
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Net.NetworkInformation;
using System.Management;

namespace PyxNet.Utilities
{
    /// <summary>
    /// A class that raises an event when the network configuration changes.
    /// </summary>
    public class NetworkConfigurationWatcher
    {
        #region Tracer

        /// <summary>
        /// The diagnostic tracer.
        /// </summary>
        public readonly NumberedTraceTool<NetworkConfigurationWatcher> Tracer =
            new NumberedTraceTool<NetworkConfigurationWatcher>(TraceTool.GlobalTraceLogEnabled);

        #endregion

        #region Fields

        /// <summary>
        /// The WMI event watcher used to get Windows events.
        /// </summary>
        private readonly ManagementEventWatcher m_wmiEventWatcher;

        #endregion

        #region Construction

        /// <summary>
        /// Constructs an instance.
        /// </summary>
        public NetworkConfigurationWatcher()
        {
            // Set up WMI events.  Bind to local machine.
            ManagementScope scope = new ManagementScope("root\\CIMV2");
            scope.Options.EnablePrivileges = true; //set required privilege

            // TODO: Does this catch too many events?  Can it be more specific?
            WqlEventQuery wqlEventQuery = new WqlEventQuery(
                "__InstanceModificationEvent",
                new TimeSpan(0, 0, 3),
                "TargetInstance ISA 'Win32_NetworkAdapterConfiguration'");

            m_wmiEventWatcher = new ManagementEventWatcher(scope, wqlEventQuery);
        }

        #endregion

        #region Events

        /// <summary>
        /// The event fired when the network configuration changes.
        /// </summary>
        public event EventArrivedEventHandler EventArrived
        {
            add
            {
                m_wmiEventWatcher.EventArrived += value;
            }
            remove
            {
                m_wmiEventWatcher.EventArrived -= value;
            }
        }

        #endregion

        #region Start and Stop

        /// <summary>
        /// Start watching for network configuration events.
        /// </summary>
        public void Start()
        {
            m_wmiEventWatcher.Start();
        }

        /// <summary>
        /// Stop watching for network configuration events.
        /// </summary>
        public void Stop()
        {
            m_wmiEventWatcher.Stop();
        }

        #endregion
    }
}
