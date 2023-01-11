//#define CHECK_LISTENERS

using System;
using System.Collections.Generic;
using System.Net.Sockets;

namespace PyxNet
{
    /// <summary>
    /// A TCP server.
    /// </summary>
    public class TcpNetwork : INetwork, IDisposable
    {
        #region Tracer

        /// <summary>
        /// This is the trace tool that one should use for all things to do with this object.
        /// </summary>
        private Pyxis.Utilities.NumberedTraceTool<INetwork> m_tracer
            = new Pyxis.Utilities.NumberedTraceTool<INetwork>(Pyxis.Utilities.TraceTool.GlobalTraceLogEnabled);

        /// <summary>
        /// This is the trace tool that one should use for all things to do with this object.
        /// </summary>
        public Pyxis.Utilities.NumberedTraceTool<INetwork> Tracer
        {
            get
            {
                return m_tracer;
            }
        }

        #endregion

        #region Helper Functions

        /// <summary>
        /// Closes the TCP client after shuting down and closing the
        /// underlying socket.  This is because TCPClient.Close() does
        /// not close the underlying socket.
        /// </summary>
        /// <param name="client">The client to close.</param>
        public static void CloseTCPClient(TcpClient client)
        {
            System.Net.Sockets.Socket socket = client.Client;
            client.Client = null;
            try
            {
                socket.Shutdown(System.Net.Sockets.SocketShutdown.Both);
            }
            catch
            {
            }
            try
            {
                socket.Close();
            }
            catch
            {
            }
            try
            {
                client.Close();
            }
            catch
            {
            }
        }

        #endregion

        #region Fields

#if CHECK_LISTENERS
        /// <summary>
        /// Checks that listeners are working, and re-creates if necessary.
        /// </summary>
        private readonly System.Timers.Timer m_checkingTimer;
#endif

        /// <summary>
        /// The TCP listeners.
        /// </summary>
        private readonly Dictionary<System.Net.IPEndPoint, TcpListener> m_listeners =
            new Dictionary<System.Net.IPEndPoint, TcpListener>();

        #endregion

        #region Construction

        /// <summary>
        /// Construct a TcpNetwork with no listeners.
        /// </summary>
        public TcpNetwork()
        {
#if CHECK_LISTENERS
            m_checkingTimer = new System.Timers.Timer(TimeSpan.FromMinutes(1).TotalMilliseconds);
            m_checkingTimer.Elapsed +=
                delegate(object sender, System.Timers.ElapsedEventArgs e)
                {
                    CheckListeners();
                };
            m_checkingTimer.AutoReset = true;
            m_checkingTimer.Start();
#endif
        }

        #endregion

        #region Listeners

        /// <summary>
        /// Removes all listeners.
        /// </summary>
        public void RemoveListeners()
        {
            lock (m_listeners)
            {
                // Stop each listener.
                foreach (TcpListener listener in m_listeners.Values)
                {
                    StopListener(listener);
                }

                // Clear the dictionary.
                m_listeners.Clear();
            }
        }

        /// <summary>
        /// Stops all listeners.
        /// </summary>
        public void StopListeners()
        {
            lock (m_listeners)
            {
                // Stop each listener.
                foreach (TcpListener listener in m_listeners.Values)
                {
                    StopListener(listener);
                }
            }
        }

        /// <summary>
        /// Resets all listeners.
        /// </summary>
        public void ResetListeners()
        {
            lock (m_listeners)
            {
                StopListeners();

                // Start each listener.
                foreach (TcpListener listener in m_listeners.Values)
                {
                    try
                    {
                        StartListener(listener);
                    }
                    catch (Exception e)
                    {
                        Tracer.WriteLine("ResetListeners: Exception {0} when  starting listener at {1}",
                            e, listener.LocalEndpoint);
                    }
                }
            }
        }

        /// <summary>
        /// Creates and adds listeners for addresses in the internal address list in 'localAddress'.
        /// This can throw a SocketException.
        /// </summary>
        /// <param name="localAddress">The local address.</param>
        public void SetListeners(NetworkAddress localAddress)
        {
            if (null == localAddress)
            {
                RemoveListeners();
                return;
            }

            lock (m_listeners)
            {
                // Remove all existing listeners not in the address.
                Queue<System.Net.IPEndPoint> removeQueue = new Queue<System.Net.IPEndPoint>();
                foreach (System.Net.IPEndPoint ipEndPoint in m_listeners.Keys)
                {
                    if (!localAddress.InternalIPEndPoints.Contains(ipEndPoint))
                    {
                        TcpListener listener;
                        if (m_listeners.TryGetValue(ipEndPoint, out listener))
                        {
	                        Tracer.WriteLine("SetListeners: Removing listener on {0}", ipEndPoint.ToString());
                            removeQueue.Enqueue(ipEndPoint);

	                        Tracer.WriteLine("SetListeners: Stopping listener on {0}", ipEndPoint.ToString());
                            StopListener(listener);
                        }
                    }
                }
                while (0 < removeQueue.Count)
                {
                    System.Net.IPEndPoint ipEndPoint = removeQueue.Dequeue();
                    m_listeners.Remove(ipEndPoint);
                }

                // Add new listeners.
                foreach (System.Net.IPEndPoint ipEndPoint in localAddress.InternalIPEndPoints)
                {
                    if (!m_listeners.ContainsKey(ipEndPoint))
                    {
                        Tracer.WriteLine("SetListeners: Adding listener on {0}", ipEndPoint.ToString());

                        try
                        {
                            TcpListener listener = new System.Net.Sockets.TcpListener(ipEndPoint);
                            if (!ipEndPoint.Equals(listener.LocalEndpoint))
                            {
                                this.Tracer.WriteLine(
                                    "SetListeners: The listener's IPEndPoint {0} doesn't match the specified IP end point {1}.",
                                    listener.LocalEndpoint.ToString(), ipEndPoint.ToString());
                            }

	                        Tracer.WriteLine("SetListeners: Starting listener on {0}", ipEndPoint.ToString());
                            StartListener(listener);

	                        Tracer.WriteLine("SetListeners: Adding listener on {0}", ipEndPoint.ToString());
                            m_listeners[ipEndPoint] = listener;
                        }
                        catch (Exception exception)
                        {
                            Tracer.WriteLine("SetListeners: Couldn't create listener at {0}: {1}", ipEndPoint, exception);
                            throw exception;
                        }
                    }
                }
            }
        }

#if CHECK_LISTENERS
        /// <summary>
        /// Check a listener to see if it is still listening.
        /// </summary>
        /// <param name="listener">The listener.</param>
        /// <param name="ipEndPoint">The IP end point it is supposed to be listening on.</param>
        /// <returns>False if demonstrably not listening.</returns>
        private bool CheckListener(TcpListener listener, System.Net.IPEndPoint ipEndPoint)
        {
            Tracer.WriteLine("CheckListener: Checking listener on {0}.", ipEndPoint.ToString());

            // Create a connection to the listener.
            TcpClient client = null;
            try
            {
                client = new TcpClient(ipEndPoint);

                if (client.Connected)
                {
                    Tracer.WriteLine("CheckListener: The listener is working; connected to address {0}.", ipEndPoint);
                    return true;
                }
            }
            catch (SocketException exception)
            {
                // If "address in use", return true.
                // TODO: Why does this happen?
                if (exception.SocketErrorCode == SocketError.AddressAlreadyInUse)
                {
                    Tracer.WriteLine("CheckListener: The listener's address {0} is in use, so that probably means it is working.", ipEndPoint);
                    return true;
                }
                Tracer.WriteLine("CheckListener: Couldn't connect to the listener on {0}: {1}", ipEndPoint, exception);
            }
            catch (Exception exception)
            {
                Tracer.WriteLine("CheckListener: Couldn't connect to the listener on {0}: {1}", ipEndPoint, exception);
            }
            finally
            {
                if (null != client)
                {
                    try
                    {
                        client.Close();
                    }
                    catch
                    {
                    }
                }
            }

            Tracer.WriteLine("CheckListener: The listener on {0} is not working.", ipEndPoint);
            return false;
        }

        /// <summary>
        /// Check all listeners and deal with the ones not listening in an appropriate manner.
        /// </summary>
        private void CheckListeners()
        {
            // Keep a list of bad listeners.
            Queue<System.Net.IPEndPoint> removeQueue = new Queue<System.Net.IPEndPoint>();

            // Lock listeners and check.
            lock (m_listeners)
            {
                foreach (KeyValuePair<System.Net.IPEndPoint, TcpListener> keyValuePair in this.m_listeners)
                {
                    System.Net.IPEndPoint ipEndPoint = keyValuePair.Key;
                    TcpListener listener = keyValuePair.Value;
                    if (!CheckListener(listener, ipEndPoint))
                    {
                        // Stop the listener.
                        StopListener(listener);

                        // Add to "remove" queue.
                        removeQueue.Enqueue(ipEndPoint);
                    }
                }

                // Go through remove queue and remove listeners.
                while (0 < removeQueue.Count)
                {
                    System.Net.IPEndPoint ipEndPoint = removeQueue.Dequeue();
                    m_listeners.Remove(ipEndPoint);
                }
            }
        }
#endif

        #endregion

        #region Dispose

        /// <summary>
        /// Track whether Dispose has been called.
        /// </summary>
        private bool m_disposed = false;

        /// <summary>
        /// Implement IDisposable.
        /// </summary>
        /// <remarks>
        /// Do not make this method virtual.
        /// A derived class should not be able to override this method. 
        /// </remarks>
        public void Dispose()
        {
            Dispose(true);

            // This object will be cleaned up by the Dispose method.
            // Therefore, you should call GC.SupressFinalize to
            // take this object off the finalization queue 
            // and prevent finalization code for this object
            // from executing a second time.
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Dispose(bool disposing) executes in two distinct scenarios, 
        /// indicated by the "disposing" argument.
        /// </summary>
        /// <param name="disposing">
        /// If disposing equals true, the method has been called directly
        /// or indirectly by a user's code. Managed and unmanaged resources
        /// can be disposed.
        /// If disposing equals false, the method has been called by the 
        /// runtime from inside the finalizer and you should not reference 
        /// other objects. Only unmanaged resources can be disposed.
        /// </param>
        private void Dispose(bool disposing)
        {
            // Check to see if Dispose has already been called.
            if (!m_disposed)
            {
                // If disposing equals true, dispose all managed 
                // and unmanaged resources.
                if (disposing)
                {
                    // Dispose managed resources.
#if CHECK_LISTENERS
                    m_checkingTimer.Stop();
#endif
                    m_ConnectionOpened = null; // Or perhaps just close all listeners?
                    RemoveListeners();
                }

                // Call the appropriate methods to clean up 
                // unmanaged resources here.
                // If disposing is false, 
                // only the following code is executed.
            }
            m_disposed = true;
        }

        /// <summary>
        /// The finalization code.
        /// </summary>
        /// <remarks>
        /// Use C# destructor syntax for finalization code.
        /// This destructor will run only if the Dispose method 
        /// does not get called.
        /// It gives your base class the opportunity to finalize.
        /// Do not provide destructors in types derived from this class.
        /// </remarks>
        ~TcpNetwork()
        {
            // Do not re-create Dispose clean-up code here.
            // Calling Dispose(false) is optimal in terms of
            // readability and maintainability.
            Dispose(false);
        }

        #endregion

        #region Connect

        /// <summary>
        /// Connect to a TCP network address.
        /// Uses the default .Net timeout for connections.
        /// </summary>
        /// <param name="address">The TCP network address to connect to.</param>
        /// <param name="tryInternal">True if internal addresses are to be tried.</param>
        /// <returns>The resulting TCP network connection.</returns>
        public TcpNetworkConnection Connect(NetworkAddress address, bool tryInternal)
        {
            if (null == address)
            {
                throw new System.ArgumentNullException("address");
            }

            TcpNetworkConnection connection = null;

            Pyxis.Utilities.SynchronizationEvent operationCompleted =
                new Pyxis.Utilities.SynchronizationEvent();

            Pyxis.Utilities.ThreadSafeInt threadCount = new Pyxis.Utilities.ThreadSafeInt();

            Pyxis.Utilities.ObservableObject<int> threadCountObserver =
                new Pyxis.Utilities.ObservableObject<int>(0);

            threadCountObserver.Changed +=
                delegate(object sender,
                    Pyxis.Utilities.ChangedEventArgs<int> e)
                {
                    if (e.NewValue == 0)
                    {
                        operationCompleted.Pulse();
                    }
                };

            Object lockConnection = new Object();

            AsyncCallback endConnect =
                delegate(IAsyncResult ar)
                {
                    // Make this a background thread.
                    System.Threading.Thread.CurrentThread.IsBackground = true;

                    TcpClient client = ar.AsyncState as TcpClient;
                    System.Diagnostics.Debug.Assert(client != null);

                    try
                    {
                        // Finish the connection.
                        // This throws System.Net.Sockets.SocketException if unsuccessful.
                        client.EndConnect(ar);

                        if (connection == null)
                        {
                            lock (lockConnection)
                            {
                                if (connection == null)
                                {
                                    connection = new TcpNetworkConnection(client);
                                    operationCompleted.Pulse();
                                    return;
                                }
                            }
                        }
                        CloseTCPClient(client);
                    }
                    catch (Exception e)
                    {
                        CloseTCPClient(client);
                        Tracer.DebugWriteLine("Caught exception when trying to connect: {0}", e);
                    }
                    finally
                    {
                        threadCountObserver.Value = --threadCount;
                    }
                };

            DateTime startTime = DateTime.Now;
            if (tryInternal)
            {
                foreach (System.Net.IPEndPoint ipEndPoint in address.InternalIPEndPoints)
                {
                    System.Diagnostics.Debug.Assert(ipEndPoint != null);

                    if (connection != null)
                    {
                        return connection;
                    }

                    threadCountObserver.Value = ++threadCount;
                    TcpClient client = new TcpClient();
                    client.BeginConnect(ipEndPoint.Address, ipEndPoint.Port, endConnect, client);
                }
            }
            foreach (System.Net.IPEndPoint ipEndPoint in address.ExternalIPEndPoints)
            {
                System.Diagnostics.Debug.Assert(ipEndPoint != null);

                if (connection != null)
                {
                    return connection;
                }

                threadCountObserver.Value = ++threadCount;
                TcpClient client = new TcpClient();
                client.BeginConnect(ipEndPoint.Address, ipEndPoint.Port, endConnect, client);
            }

            operationCompleted.Wait();

            TimeSpan elapsedTime = DateTime.Now - startTime;
            if (connection == null)
            {
                Tracer.WriteLine("Failed connection to {1} attempt took {0} seconds.", elapsedTime.TotalSeconds, address);
            }
            else
            {
                Tracer.DebugWriteLine("Successful connection to {1} took {0} seconds.", elapsedTime.TotalSeconds, connection.Address);
            }

            return connection;
        }

        /// <summary>
        /// Connect to a TCP network address.
        /// </summary>
        /// <param name="address">The TCP network address to connect to.</param>
        /// <returns>The resulting TCP network connection.</returns>
        public INetworkConnection Connect(NetworkAddress address)
        {
            return Connect(address, true);
        }

        /// <summary>
        /// Create a new connection to the network, from a specific address.
        /// </summary>
        /// <param name="to">The address of the computer to connect to.</param>
        /// <param name="from">The address of the computer to connect from.</param>
        /// <returns>The new network connection.</returns>
        public INetworkConnection Connect(NetworkAddress to, NetworkAddress from)
        {
            if (null == from)
            {
                return Connect(to, true);
            }
            if (null == to)
            {
                throw new System.ArgumentNullException("to");
            }

            // If the 'to' address doesn't have any external endpoints, try the internals
            // as our only option.
            if (0 == to.ExternalIPEndPoints.Count)
            {
                return Connect(to, true);
            }

            // If they share an external address, connect by internal address first.
            // Otherwise, connect only by external address.
            foreach (System.Net.IPEndPoint fromEndPoint in from.ExternalIPEndPoints)
            {
                Predicate<System.Net.IPEndPoint> match = delegate(System.Net.IPEndPoint toEndPoint)
                {
                    return fromEndPoint.Address.Equals(toEndPoint.Address);
                };
                if (null != to.ExternalIPEndPoints.Find(match))
                {
                    return Connect(to, true);
                }
            }
            return Connect(to, false);
        }

        #endregion

        #region Start and Stop

        /// <summary>
        /// Process the client connection.
        /// This is a call-back function, called when a client is accepted.
        /// </summary>
        /// <param name="ar">The asynchronous result.</param>
        private void AcceptClient(System.IAsyncResult ar)
        {
            try
            {
                TcpListener listener = ar.AsyncState as TcpListener;
                if (null == listener)
                {
                    this.Tracer.ForcedWriteLine("AcceptClient: The listener was null.");
                    return;
                }

                this.Tracer.WriteLine("AcceptClient: Listener accepting a client.");

                // End the asynchronous connection accept.
                System.Net.Sockets.TcpClient client = listener.EndAcceptTcpClient(ar);
                this.Tracer.WriteLine("AcceptClient: Listener accepted a client.");

                // Listen for the next client in the incoming connection queue.
                listener.BeginAcceptTcpClient(AcceptClient, listener);
                this.Tracer.WriteLine("AcceptClient: Listener accepting connections from its connection queue.");

                if (null == client)
                {
                    this.Tracer.WriteLine("AcceptClient: The accepted client was null.");
                }
                else
                {
                    this.Tracer.WriteLine("AcceptClient: The accepted client was not null.");

                    // Do OnConnect callback, passing the new connection.
                    // Create TcpNetworkConnection.
                    this.Tracer.WriteLine("AcceptClient: Creating connection wrapper for client.");
                    TcpNetworkConnection connection = new TcpNetworkConnection(client);
                    this.Tracer.WriteLine("AcceptClient: Created connection wrapper for client.");

                    System.Timers.Timer connectTakesTooLong =
                        new System.Timers.Timer(TimeSpan.FromSeconds(20).TotalMilliseconds);
                    connectTakesTooLong.Elapsed +=
                        delegate(object o, System.Timers.ElapsedEventArgs a)
                        {
                            System.Diagnostics.Trace.WriteLine("AcceptClient: OnConnect has taken too long.");
                        };
                    connectTakesTooLong.Start();
                    m_ConnectionOpened.Invoke(this, new ConnectionEventArgs() { Connection = connection });
                    connectTakesTooLong.Stop();

                    this.Tracer.WriteLine("AcceptClient: The connection has been handled.");
                }
            }
            catch (System.ObjectDisposedException exception)
            {
                // TODO: deal with this exception appropriately.
                this.Tracer.ForcedWriteLine("AcceptClient caught ObjectDisposedException {0}", exception.ToString());
            }
            catch (System.NullReferenceException exception)
            {
                // This happens when the connection is shutting down, but a new connection came in.
                // Just ignore this one so we shut down gracefully.
                this.Tracer.ForcedWriteLine("AcceptClient caught NullReferenceException {0}", exception.ToString());
            }
            catch (System.Net.Sockets.SocketException exception)
            {
                // This one can occur too.
                this.Tracer.ForcedWriteLine("AcceptClient caught SocketException {0}", exception.ToString());
            }
            catch (InvalidOperationException exception)
            {
                // If the listener has already been stopped, BeginAcceptTcpClient will throw this.
                this.Tracer.ForcedWriteLine("AcceptClient caught InvalidOperationException {0}", exception.ToString());
            }
            catch (Exception exception)
            {
                this.Tracer.ForcedWriteLine("AcceptClient caught exception {0}", exception.ToString());
            }
        }

        /// <summary>
        /// Start accepting connections on the listener.
        /// Note that this may throw an exception, most likely a SocketException.
        /// </summary>
        /// <param name="listener">The listener to accept connections on.</param>
        private void StartListener(TcpListener listener)
        {
            try
            {
                this.Tracer.WriteLine("StartListener: Starting listener.");
                try
                {
                    listener.Start(25);
                }
                catch
                {
                    StopListener(listener);
                    listener.Start();
                }
                this.Tracer.WriteLine("StartListener: Listener started.");

                // Accept the first client from the incoming connection queue.
                listener.BeginAcceptTcpClient(AcceptClient, listener);
                this.Tracer.WriteLine("StartListener: Listener accepting connections from its connection queue.");
            }
            catch (Exception exception)
            {
                this.Tracer.WriteLine("StartListener: Exception starting listener: {0}", exception.ToString());

                // Stop the listener.
                StopListener(listener);

                // Rethrow;
                throw;
            }
        }

        /// <summary>
        /// Stop accepting connections on the listener.
        /// </summary>
        /// <param name="listener">The listener to stop accepting connections on.</param>
        private void StopListener(TcpListener listener)
        {
            try
            {
                this.Tracer.WriteLine("StopListener: Stopping listener.");
                listener.Stop();
                this.Tracer.WriteLine("StopListener: Listener stopped.");
            }
            catch (Exception exception)
            {
                // Ignore exceptions, since we're stopping.
                this.Tracer.WriteLine("StopListener: Exception stopping listener: {0}", exception.ToString());
            }
        }

        #endregion

        #region Events

        private Pyxis.Utilities.EventHelper<ConnectionEventArgs> m_ConnectionOpened =
            new Pyxis.Utilities.EventHelper<ConnectionEventArgs>();

        /// <summary>
        /// This event is triggered when a connection happens.
        /// A ConnectionHandler is a delegate that takes the new connection and does something with it.
        /// This should be handled before adding listeners, to make sure that nothing
        /// falls through the cracks.
        /// </summary>
        public event EventHandler<ConnectionEventArgs> ConnectionOpened
        {
            add
            {
                m_ConnectionOpened.Add(value);
            }
            remove
            {
                m_ConnectionOpened.Remove(value);
            }
        }

        #endregion
    }
}