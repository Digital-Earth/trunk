/******************************************************************************
DDEServer.cs

begin      : Dec 14, 2007
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace PyxNet.Utilities
{
    /// <summary>    
    /// The application DDE Server that listens for pyxis:// protocol and 
    /// PPL file clicks.
    /// </summary>
    public class DDEServer : NDde.Server.DdeServer
    {
        /// <summary>
        /// The event to raise when a command is received from a DDE client.
        /// </summary>
        public event EventHandler<DDECommandReceivedEventArgs> DDECommandReceived;

        public DDEServer(string service)
            : base(service)
        {
        }

        /// <summary>
        /// This is invoked when a client attempts to establish a 
        /// conversation.
        /// </summary>
        /// <param name="topic">The topic name is a logical context for 
        /// data and is defined by the server application.</param>
        /// <returns>
        /// We return true to indicate we accept all connections.
        /// </returns>
        protected override bool OnBeforeConnect(string topic)
        {
            return true;
        }

        /// <summary>
        /// This is invoked when a client sends a DDE command.  This method 
        /// raises an event and passes the command as an argument for handlers 
        /// to deal with it as they see fit.
        /// </summary>
        /// <param name="conversation">
        /// The conversation associated with this event.
        /// </param>
        /// <param name="command">
        /// The command to execute.
        /// </param>
        /// <returns>
        /// Whether the command is successfully processed or not.
        /// </returns>
        protected override ExecuteResult OnExecute(
            NDde.Server.DdeConversation conversation, string command)
        {
            DDECommandReceivedEventArgs e =
                new DDECommandReceivedEventArgs(command);

            EventHandler<DDECommandReceivedEventArgs> handler =
                DDECommandReceived;

            // event will be null if there are no subscribers
            if (handler != null)
            {
                try
                {
                    // raise the event
                    handler(this, e);
                }
                catch (Exception ex)
                {
                    DDEManager.Tracer.WriteLine(ex.ToString());
                    return ExecuteResult.NotProcessed;
                }
            }

            return ExecuteResult.Processed;
        }
    }

    /// <summary>
    /// The argument to pass when a DDECommandReceived event is raised.
    /// </summary>
    public class DDECommandReceivedEventArgs : EventArgs
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="command">
        /// The command received from the DDE client.
        /// </param>
        public DDECommandReceivedEventArgs(string command)
        {
            m_command = command;
        }

        /// <summary>
        /// The command received from the DDE client.
        /// </summary>
        public string Command
        {
            get
            {
                return m_command;
            }
        }
        private string m_command;
    }

    public class DDEManager: IDisposable 
    {
        public static PyxNet.TraceTool Tracer = new PyxNet.TraceTool(false);

        /// <summary>
        /// An object that represents the server side of DDE conversations.  
        /// We use DDE to communicate between WorldView and the client that 
        /// initiates a DDE conversion using pyxis://[argument].
        /// </summary>
        private PyxNet.Utilities.DDEServer m_ddeServer = null;

        /// <summary>
        /// The event to raise when a command is received from a DDE client.
        /// </summary>
        public event EventHandler<DDECommandReceivedEventArgs> DDECommandReceived;

        #region EnableDDE
        private const int MSGFLT_ADD = 1;
        //private const int WM_DROPFILES = 0x0233;

        [System.Runtime.InteropServices.DllImport("user32.dll", SetLastError = true)]
        static extern IntPtr ChangeWindowMessageFilter(uint message, uint dwFlag);

        static private bool s_ddeIsEnabled = false;
        /// <summary>
        /// Enables DDE and drag and drop between this app and other 
        /// (non-elevated) apps.  Note that we aren't usually running
        /// as an elevated app, but if we are, we're still safe.
        /// (Fixes Vista UAC compatability.)
        /// </summary>
        static public void EnableDDE()
        {
            if (!s_ddeIsEnabled && (Environment.OSVersion.Version.Major >= 6))
            {
                s_ddeIsEnabled = true;
                ChangeWindowMessageFilter((uint)Windows.Core.WindowsMessages.WM_DROPFILES, MSGFLT_ADD);
                ChangeWindowMessageFilter((uint)Windows.Core.WindowsMessages.WM_DDE_INITIATE, MSGFLT_ADD);
                ChangeWindowMessageFilter((uint)Windows.Core.WindowsMessages.WM_DDE_ACK, MSGFLT_ADD);
                ChangeWindowMessageFilter((uint)Windows.Core.WindowsMessages.WM_DDE_POKE, MSGFLT_ADD);
                ChangeWindowMessageFilter((uint)Windows.Core.WindowsMessages.WM_DDE_EXECUTE, MSGFLT_ADD);
                ChangeWindowMessageFilter((uint)Windows.Core.WindowsMessages.WM_DDE_DATA, MSGFLT_ADD);
                ChangeWindowMessageFilter((uint)Windows.Core.WindowsMessages.WM_DDE_ADVISE, MSGFLT_ADD);
                ChangeWindowMessageFilter((uint)Windows.Core.WindowsMessages.WM_DDE_UNADVISE, MSGFLT_ADD);
                ChangeWindowMessageFilter((uint)Windows.Core.WindowsMessages.WM_DDE_INITIATE, MSGFLT_ADD);
                ChangeWindowMessageFilter((uint)Windows.Core.WindowsMessages.WM_DDE_REQUEST, MSGFLT_ADD);
            }
        }
        #endregion

        private System.Threading.Mutex m_mutex;
        private bool m_mutexWasCreated;

        public DDEManager()
        {
            // Enable DDE between elevated and non-elevated clients.
            EnableDDE();

            m_mutex = new System.Threading.Mutex(
                true,
                System.Reflection.Assembly.GetEntryAssembly().GetName().Name,
                out m_mutexWasCreated);
        }

        /// <summary>
        /// Destructor
        /// </summary>
        ~DDEManager()
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
                    IDisposable disposableMutex = m_mutex as IDisposable;
                    if (disposableMutex != null)
                    {
                        disposableMutex.Dispose();
                    }
                    m_mutex = null;
                }
            }
            m_disposed = true;
        }
        #endregion /* IDispose */

        /// <summary>
        /// Initializes the specified args.
        /// </summary>
        /// <param name="args">The args.</param>
        /// <returns>
        /// False if we found another server running, true otherwise.
        /// </returns>
        public bool Initialize(string[] args)
        {
            // If we're able to connect to a DDE server with the specified 
            // service name, it means an instance of WorldView is already 
            // running.  We pass any command line arguments we receive to the 
            // DDE server and shut ourselves down.  An exception is thrown if 
            // we're unable to connect to a DDE server, meaning there are no 
            // instances of WorldView already running.
            try
            {
                if (m_mutexWasCreated == false)
                {
                    SendMessageToServer(
                        System.Reflection.Assembly.GetEntryAssembly().GetName().Name, 
                        args);

                    // Signal to the caller that we did not finish initialization.
                    return false;
                }
            }
            catch (Exception)
            {
                // an exception is thrown if no instance exists already
            }

            new System.Threading.Thread(
                delegate()
                {
                    System.Threading.Thread.CurrentThread.IsBackground = true;

                    try
                    {
                        // create a server that will register the service 
                        // name 'PYXIS WorldView'
                        m_ddeServer = new DDEServer(
                            System.Reflection.Assembly.GetEntryAssembly().GetName().Name);

                        // Register the service name.
                        m_ddeServer.Register();

                        // Pass on any DDE Commands.
                        m_ddeServer.DDECommandReceived +=
                            delegate(object sender,
                            PyxNet.Utilities.DDECommandReceivedEventArgs e)
                            {
                                DDECommandReceived(sender, e);
                            };

                    }
                    catch (ArgumentException)
                    {
                        Tracer.WriteLine("The service name provided to the DDE Server " +
                            "cannot exceed 255 characters.");
                    }
                    catch (NDde.DdeException ex)
                    {
                        Tracer.WriteLine("Unable to create dde server: " + ex.ToString());
                    }
                    catch (ObjectDisposedException ex)
                    {
                        Tracer.WriteLine("Unable to create dde server: " + ex.ToString());
                    }
                }).Start();

            return true;
        }

        /// <summary>
        /// Sends a message to server.
        /// </summary>
        /// <param name="serverName">Name of the server.</param>
        /// <param name="args">The message.</param>
        public static bool SendMessageToServer(string serverName, string[] args)
        {
            EnableDDE();

            try
            {
                System.Threading.Mutex mutex = System.Threading.Mutex.OpenExisting(
                    serverName);
                if (mutex == null)
                {
                    return false;
                }
            }
            catch (System.Threading.WaitHandleCannotBeOpenedException)
            {
                return false;
            }
            NDde.Client.DdeClient ddeClient =
                new NDde.Client.DdeClient( serverName, "system");
            ddeClient.Connect();
            IAsyncResult result = ddeClient.BeginExecute(
                string.Join(" ", args), null, null);
            ddeClient.EndExecute(result);
            return true;
        }
    }
}
