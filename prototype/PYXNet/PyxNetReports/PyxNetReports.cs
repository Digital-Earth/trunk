using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace PyxNetReportsApplication
{
    public partial class PyxNetReports : Form
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="PyxNetReports"/> class.
        /// </summary>
        public PyxNetReports()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Handles the Load event of the PyxNetReports control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void PyxNetReports_Load(object sender, EventArgs e)
        {
            PyxNet.StackSingleton.Stack.NodeInfo.FriendlyName =
                PyxNet.StackSingleton.Stack.NodeInfo.FriendlyName + " (WatchDog)";
            PyxNet.StackSingleton.Stack.Connect();            
        }

        /// <summary>
        /// Handles the Shown event of the PyxNetReports control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void PyxNetReports_Shown(object sender, EventArgs e)
        {
            PyxNet.StackSingleton.Stack.NodeInfo.IsLeaf = true;

            PyxNet.StackSingleton.Stack.PersistentConnections.AddedElement +=
                delegate(object _sender, Pyxis.Utilities.DynamicList<PyxNet.StackConnection>.ElementEventArgs _e)
                {
                    new System.Threading.Thread(
                        delegate()
                        {
                            System.Threading.Thread.CurrentThread.IsBackground = true;
                            while (true)
                            {
                                try
                                {
                                    NotifyAdministrator(_e.Element.RemoteNodeInfo.FriendlyName,
                                        string.Format("Server {0} is online.", _e.Element.RemoteNodeInfo.FriendlyName));
                                    return;
                                }
                                catch (Exception)
                                {
                                }
                                System.Threading.Thread.Sleep(TimeSpan.FromSeconds(15));
                            }
                        }).Start();
                };

            PyxNet.StackSingleton.Stack.PersistentConnections.RemovedElement +=
                delegate(object _sender, Pyxis.Utilities.DynamicList<PyxNet.StackConnection>.ElementEventArgs _e)
                {
                    new System.Threading.Thread(
                        delegate()
                        {
                            System.Threading.Thread.CurrentThread.IsBackground = true;
                            NotifyAdministrator(_e.Element.RemoteNodeInfo.FriendlyName,
                                string.Format("Server {0} has gone offline.", _e.Element.RemoteNodeInfo.FriendlyName));
                        }).Start();
                };

            PyxNet.StackSingleton.Stack.Connected +=
                delegate(object _sender, PyxNet.Stack.ConnectedEventArgs _e)
                {
                    SetConnectionInfo();
                };

            #region Network connection, handlers.

            PyxNet.StackSingleton.Stack.HubConnected +=
                delegate(object _sender, EventArgs _e)
                {
                    SetStatusText("Connected");
                    SetConnectionInfo();
                };

            PyxNet.StackSingleton.Stack.HubDisconnected +=
                delegate(object _sender, EventArgs _e)
                {
                    SetStatusText("Disconnected");
                    SetConnectionInfo();
                };

            SetStatusText("Offline");
            if (PyxNet.StackSingleton.Stack.IsHubConnected)
            {
                SetStatusText("Connected");
            }

            #endregion Network connection, handlers.

            SetConnectionInfo();

            m_timer.AutoReset = true;
            m_timer.Elapsed += 
                delegate(object sender_, System.Timers.ElapsedEventArgs e_)
                {
                    SetConnectionInfo();
                };
            m_timer.Interval = 2000;
            m_timer.Start();

            // Display the DNS lookup for the server.
            System.Net.IPHostEntry host = null;
            try
            {
                host = System.Net.Dns.GetHostEntry("pyxnet.pyxisinnovation.com");

                StringBuilder result = new StringBuilder();
                foreach (System.Net.IPAddress address in host.AddressList)
                {
                    result.Append(address.ToString());
                    result.Append(" ");
                }
                this.DNSResolution.Text = result.ToString();
            }
            catch (System.Net.Sockets.SocketException ex)
            {
                this.DNSResolution.Text = ex.Message;
            }
        }

        /// <summary>
        /// Notifies the administrator.
        /// </summary>
        [System.Diagnostics.Conditional("DEBUG")]
        private static void NotifyAdministrator( string serverName, string messageText)
        {
            System.Net.Mail.SmtpClient client = null;
            string message = null;

            do
            {
                try
                {
                    System.Net.Mail.MailMessage notification = new System.Net.Mail.MailMessage(
                        "WatchDog@pyxisinnovation.com", // From:
                        "PyxNetAdmin@pyxisinnovation.com",  // To:
                        string.Format("{0} status as observed by {1}", 
                            serverName, Environment.MachineName),      // Subject:
                        string.Format("{0} This was observed at {1} by the watchdog that {2} is running on machine {3}.",
                            messageText, 
                            DateTime.Now.ToString("F"),
                            Environment.UserName,
                            Environment.MachineName)); // Message body.

                    // Send it using the settings in app.config
                    client = new System.Net.Mail.SmtpClient();
                    client.Send(notification);
                    return;
                }
                catch (Exception ex)
                {
                    StringBuilder response = new StringBuilder("Unable to send mail.  ");
                    response.Append(ex.Message);
                    response.AppendFormat(
                        "\r\nCurrently configured to use {0}:{1}.  ",
                        ((client.Host != null) && (client.Host.Length > 0)) ? client.Host : "default smtp host",
                        client.Port);
                    response.AppendFormat(
                        "You can configure your settings in\r\n{0}",
                        System.Reflection.Assembly.GetExecutingAssembly().Location + ".config");
                    message = response.ToString();
                }
            }
            while (MessageBox.Show(message,
                "WatchDog Service", MessageBoxButtons.RetryCancel) == DialogResult.Retry);
        }

        private System.Timers.Timer m_timer = new System.Timers.Timer();

        /// <summary>
        /// Sets the connection info.
        /// </summary>
        private void SetConnectionInfo()
        {
            Dictionary<PyxNet.NodeInfo, DisplayableNodeInfo> hubs = new Dictionary<PyxNet.NodeInfo, DisplayableNodeInfo>();

            hubs.Add(PyxNet.StackSingleton.Stack.NodeInfo, new DisplayableNodeInfo(PyxNet.StackSingleton.Stack.NodeInfo));

            foreach (PyxNet.NodeInfo n in PyxNet.StackSingleton.Stack.KnownHubList.KnownHubs)
            {
                if (!hubs.ContainsKey(n))
                    hubs.Add(n, new DisplayableNodeInfo(n));
                hubs[n].Known = true;
            }
            foreach (PyxNet.NodeInfo n in PyxNet.StackSingleton.Stack.KnownHubList.ConnectedHubs)
            {
                if (!hubs.ContainsKey(n))
                    hubs.Add(n, new DisplayableNodeInfo(n));
                hubs[n].Connected = true;
            }
            foreach (PyxNet.StackConnection c in PyxNet.StackSingleton.Stack.PersistentConnections)
            {
                PyxNet.NodeInfo n = c.RemoteNodeInfo;
                if (!hubs.ContainsKey(n))
                    hubs.Add(n, new DisplayableNodeInfo(n));
                hubs[n].Persistant = true;
            }
            foreach (PyxNet.StackConnection c in PyxNet.StackSingleton.Stack.TemporaryConnections)
            {
                PyxNet.NodeInfo n = c.RemoteNodeInfo;
                if (!hubs.ContainsKey(n))
                    hubs.Add(n, new DisplayableNodeInfo(n));
                hubs[n].Temporary = true;
            }

            List<DisplayableNodeInfo> list = new List<DisplayableNodeInfo>();
            foreach (DisplayableNodeInfo n in hubs.Values)
            {
                list.Add(n);
            }

            try
            {
                if (InvokeRequired)
                {

                    Invoke(new MethodInvoker(delegate()
                    {
                        if (m_timer.Enabled)
                        {
                            this.NodeInfoList.DataSource = list;
                            this.Notes.Text = PyxNet.StackSingleton.Stack.CurrentConnectionStatus;
                        }
                    }));
                }
                else
                {
                    this.NodeInfoList.DataSource = list;
                    this.Notes.Text = PyxNet.StackSingleton.Stack.CurrentConnectionStatus;
                }
            }
            catch (ObjectDisposedException)
            {
            }
        }

        /// <summary>
        /// Sets the status text.  Invokes if required.
        /// </summary>
        /// <param name="p">The p.</param>
        private void SetStatusText(string p)
        {
            if (InvokeRequired)
            {
                Invoke(new MethodInvoker(delegate()
                {
                    OnlineStatus.Text = p;
                }));
            }
            else
            {
                OnlineStatus.Text = p;
            }
        }

        /// <summary>
        /// Handles the FormClosing event of the PyxNetReports control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.Windows.Forms.FormClosingEventArgs"/> instance containing the event data.</param>
        private void PyxNetReports_FormClosing(object sender, FormClosingEventArgs e)
        {
            m_timer.Stop();
            PyxNet.StackSingleton.Stack.Close();
        }

        /// <summary>
        /// Extend the NodeInfo class to include some nice flags.
        /// </summary>
        public class DisplayableNodeInfo : PyxNet.NodeInfo
        {
            private bool m_connected = false;

            public bool Connected
            {
                get { return m_connected; }
                set { m_connected = value; }
            }

            private bool m_known = false;

            public bool Known
            {
                get { return m_known; }
                set { m_known = value; }
            }

            private bool m_persistant = false;

            public bool Persistant
            {
                get { return m_persistant; }
                set { m_persistant = value; }
            }

            private bool m_temporary = false;

            public bool Temporary
            {
                get { return m_temporary; }
                set { m_temporary = value; }
            }

            private class HistoricConnection
            {
                private DisplayableNodeInfo m_nodeInfo;

                public DisplayableNodeInfo NodeInfo
                {
                    get { return m_nodeInfo; }
                    set { m_nodeInfo = value; }
                }
                private bool? m_canConnect;
                private object m_canConnectLock = new object();
                public bool CanConnect
                {
                    get 
                    {
                        if (m_canConnect.HasValue)
                        {
                            return m_canConnect.Value;
                        }
                            
                        lock (m_canConnectLock)
                        {
                            if (!m_canConnect.HasValue)
                            {
                                m_canConnect = false;
                                new System.Threading.Thread(
                                    delegate()
                                    {
                                        System.Threading.Thread.CurrentThread.IsBackground = true;
                                        while (m_canConnect.Value == false)
                                        {
                                            if (PyxNet.StackSingleton.Stack.GetConnection(this.NodeInfo, false, TimeSpan.FromSeconds(120)) != null)
                                            {
                                                m_canConnect = true;
                                                return;
                                            }
                                            System.Threading.Thread.Sleep(TimeSpan.FromMinutes(1));
                                        }
                                    }).Start();
                            }
                            return m_canConnect.Value;
                        }
                    }
                    set { m_canConnect = value; }
                }

                private HistoricConnection(DisplayableNodeInfo node)
                {
                    m_nodeInfo = node;
                }

                static private List<HistoricConnection> s_list = new List<HistoricConnection>();
                static public bool CanConnectToNode(DisplayableNodeInfo node)
                {
                    lock (s_list)
                    {
                        foreach (HistoricConnection c in s_list)
                        {
                            if (c.NodeInfo.NodeGUID.Equals(node.NodeGUID))
                            {
                                return c.CanConnect;
                            }
                        }
                        HistoricConnection connection = new HistoricConnection(node);
                        s_list.Add(connection);
                        return connection.CanConnect;
                    }
                }
            }

            public bool CanConnect
            {
                get { return HistoricConnection.CanConnectToNode( this); }
            }

            public DisplayableNodeInfo(PyxNet.NodeInfo n) :
                base( n.ToMessage())
            {
            }
        }
    }
}
