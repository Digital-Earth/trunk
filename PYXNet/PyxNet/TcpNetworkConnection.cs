// If defined, allows messages to be processed out-of-order.
#define ASYNCHRONOUS_MESSAGE_HANDLING

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.Linq;

namespace PyxNet
{
    /// <summary>
    /// This is just a dumb network connection that receives messages without 
    /// knowing what they mean.
    /// </summary>
    public class TcpNetworkConnection : INetworkConnection, IDisposable
    {
#if ASYNCHRONOUS_MESSAGE_HANDLING
        public const bool AsynchronousMessageHandling = true;
#else
        public const bool AsynchronousMessageHandling = false;
#endif

        /// <summary>
        /// This is the trace tool that one should use for all things to do with this connection.
        /// </summary>
        private Pyxis.Utilities.NumberedTraceTool<INetworkConnection> m_tracer 
            = new Pyxis.Utilities.NumberedTraceTool<INetworkConnection>(Pyxis.Utilities.TraceTool.GlobalTraceLogEnabled);

        public Pyxis.Utilities.NumberedTraceTool<INetworkConnection> Tracer
        {
            get
            {
                return m_tracer;
            }
        }

        /// <summary>
        /// The size of the buffer.
        /// </summary>
        public const int BufferSize = 8192; // TODO: Verify that this is an optimal buffer size.

        /// <summary>
        /// The TCP client.
        /// </summary>
        private System.Net.Sockets.TcpClient m_client;

        /// <summary>
        /// TcpClient methods/properties are not thread-safe.
        /// (Note that Socket is thread-safe, so no lock is needed for it.)
        /// </summary>
        private readonly Object m_lockClient = new Object();

        private System.Net.Sockets.Socket Socket
        {
            get
            {
                System.Net.Sockets.TcpClient client = m_client;
                if (null != client)
                {
                    System.Net.Sockets.Socket socket;
                    lock (m_lockClient)
                    {
                        socket = client.Client;
                    }
                    if (null != socket && !socket.Connected)
                    {
                        Close();
                        return null;
                    }
                    return socket;
                }
                return null;
            }
        }

        #region Buffer

        /// <summary>
        /// The read buffer.
        /// </summary>
        private readonly byte[] m_buffer;

        /// <summary>
        /// The length of the next message.
        /// </summary>
        private int m_remainingLength = 0;

        /// <summary>
        /// storage area to collect up the length of the incoming message
        /// </summary>
        private readonly byte[] m_lengthBuffer = new byte[4];

        /// <summary>
        /// The last position used in the length buffer.
        /// </summary>
        private int m_lengthBufferIndex = 0;

        #endregion

        #region Constructors

        /// <summary>
        /// Create a connection.
        /// </summary>
        /// <param name="client">The TCP client to use for the connection.</param>
        public TcpNetworkConnection(System.Net.Sockets.TcpClient client)
        {
            if (null == client)
            {
                throw new ArgumentNullException("client");
            }

            m_client = client;

            // If a timeout isn't here, send may block for a long time if the socket is disconnected.
            m_client.SendTimeout = 10000; // If we can't send a buffer in 10 seconds, we're toast.

#if false
            // This ensures that stuff doesn't hang around in the send buffer for too long.
            m_client.NoDelay = true;

            // TODO: Revisit the utility of this, and applicability to our code.
            m_client.Client.Blocking = false;

            // TODO: Expose this timeout as a tunable parameter.
            m_client.LingerState = new System.Net.Sockets.LingerOption(true, 5);
#endif

            m_buffer = new byte[BufferSize];
        }

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
                   	Close();
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
        ~TcpNetworkConnection()
        {
            // Do not re-create Dispose clean-up code here.
            // Calling Dispose(false) is optimal in terms of
            // readability and maintainability.
            Dispose(false);
        }

        #endregion

        #region Remote Node Info

        /// <summary>
        /// Get the network address.
        /// </summary>
        private NetworkAddress m_address;
        public NetworkAddress Address
        {
            get
            {
                if (null == m_address)
                {
                    try
                    {
                        System.Net.EndPoint endPoint = Socket.RemoteEndPoint;
                        m_address = new NetworkAddress((System.Net.IPEndPoint)endPoint);
                    }
                    catch (NullReferenceException)
                    {
                        // It is closed.
                    }
                }

                return m_address;
            }
        }

        #endregion

        #region Messages

        /// <summary>
        /// Construct a list of two byte array segments: the byte representation of the length,
        /// and the byte representation of the message.
        /// </summary>
        /// <param name="message">The message to convert.</param>
        /// <param name="byteCount">Will contain the total number of bytes.</param>
        /// <returns>A list of byte array segments representing the message.</returns>
        internal static List<ArraySegment<byte>> ToBytes(Message message, out int byteCount)
        {
            List<ArraySegment<byte>> bytes = new List<ArraySegment<byte>>(2);

            int length = message.Bytes.Count;

            {
                byte[] lengthBytes = System.BitConverter.GetBytes(length);
                byteCount = lengthBytes.Length + length;
                bytes.Add(new ArraySegment<byte>(lengthBytes));
            }

            bytes.Add(new ArraySegment<byte>(message.Bytes.Array, 0, length));

            return bytes;
        }

        /// <summary>
        /// Send a message over the connection.
        /// </summary>
        /// <param name="message">The message to send.</param>
        /// <returns>True if the entire message was sent.</returns>
        public bool SendMessage(Message message)
        {
            if (IsClosed)
            {
                return false;
            }

            if (0 < message.Bytes.Count)
            {
                // Convert the message to bytes for sending.
                int byteCountToSend;
                List<ArraySegment<byte>> bytes = ToBytes(message, out byteCountToSend);

                // Send.
                try
                {
                    int byteCountSent = Socket.Send(bytes);
                    if (byteCountSent == byteCountToSend)
                    {
                        return true;
                    }
                }
                catch (NullReferenceException)
                {
                    // It is already closed.
                    return false;
                }
                catch (System.Net.Sockets.SocketException e)
                {
                    // The socket was closed or had trouble, so fall through to close up shop and fire closed event.
                    Tracer.DebugWriteLine(e.Message);
                }
                catch (ObjectDisposedException)
                {
                    // The socket was closed or had trouble, so fall through to close up shop and fire closed event.
                }

                // Not all the bytes were sent.  Close.
                Close();
            }

            return false;
        }

        /// <summary>
        /// Wait for the next message.
        /// </summary>
        /// <param name="message">The holder for the message.</param>
        /// <returns>True if successful; if false, the connection is closed.</returns>
        private bool WaitForMessage(Message message)
        {
            if (IsClosed)
            {
                return false;
            }

            try
            {
                // Start receiving the next message.
                System.IAsyncResult result = Socket.BeginReceive(
                    m_buffer,
                    0,
                    m_buffer.Length,
                    System.Net.Sockets.SocketFlags.None,
                    new System.AsyncCallback(ReceiveMessage),
                    message);

                return true;
            }
            catch (NullReferenceException)
            {
                // It is already closed.
            }
            catch (System.Net.Sockets.SocketException e)
            {
                // The socket had trouble.
                Tracer.DebugWriteLine(e.Message);
                Close();
            }
            catch (ObjectDisposedException)
            {
                Close();
            }

			return false;
        }

        /// <summary>
        /// The call-back function called when a message is received.
        /// </summary>
        /// <param name="ar">The asynchronous result.</param>
        private void ReceiveMessage(System.IAsyncResult ar)
        {
            if (IsClosed)
            {
                return;
            }

            // End the read of the stream, and get the remaining length in the buffer.
            int remainingLengthInBuffer = 0;
            try
            {
                // End the read of the stream.
                remainingLengthInBuffer = Socket.EndReceive(ar);
            }
            catch (NullReferenceException)
            {
                // It is already closed.
                return;
            }
            catch (System.Net.Sockets.SocketException)
            {
                // Fall through to close the connection and return.
            }
            catch (ObjectDisposedException)
            {
                // Fall through to close the connection and return.
            }
            if (0 == remainingLengthInBuffer)
            {
                // If EndReceive returned 0, we're done.
                Close();
                return;
            }

            // If there is a handler for the message, handle it and continue to 
            // wait for more messages.
            MessageHandler onMessage = m_onMessage;
            if (null != onMessage)
            {
                // Populate a queue of messages from the buffer and continue waiting asynchronously.
                Queue<Message> messageQueue = new Queue<Message>();

                // Get the next message.
                Message message = (Message)ar.AsyncState;
                System.Diagnostics.Debug.Assert(null != message, "The message shouldn't be null.");

                // Append bytes to the message as long as there is remaining data in the buffer.
                for (int bufferOffset = 0; 0 < remainingLengthInBuffer; )
                {
                    if (0 == m_remainingLength)
                    {
                        // If there isn't enough data left in the buffer to complete an int...
                        if (remainingLengthInBuffer < (sizeof(int) - m_lengthBufferIndex))
                        {
                            // We only have part of a length.
                            while (remainingLengthInBuffer > 0)
                            {
                                m_lengthBuffer[m_lengthBufferIndex] = m_buffer[bufferOffset];
                                ++bufferOffset;
                                ++m_lengthBufferIndex;
                                --remainingLengthInBuffer;
                            }
                        }
                        else // Get the length, and read into m_remainingLength.
                        {
                            while (m_lengthBufferIndex < sizeof(int))
                            {
                                m_lengthBuffer[m_lengthBufferIndex] = m_buffer[bufferOffset];
                                ++bufferOffset;
                                ++m_lengthBufferIndex;
                                --remainingLengthInBuffer;
                            }

                            // Convert the length from the length buffer.
                            m_remainingLength = System.BitConverter.ToInt32(m_lengthBuffer, 0);
                            m_lengthBufferIndex = 0;
                        }
                    }
                    else if (remainingLengthInBuffer < m_remainingLength)
                    {
                        // Read message from buffer.
                        message.Append(m_buffer, bufferOffset, remainingLengthInBuffer);
                        bufferOffset += remainingLengthInBuffer;
                        m_remainingLength -= remainingLengthInBuffer;

                        // There's nothing left in the buffer.
                        remainingLengthInBuffer = 0;
                    }
                    else
                    {
                        // Read message from buffer.
                        message.Append(m_buffer, bufferOffset, m_remainingLength);
                        bufferOffset += m_remainingLength;
                        remainingLengthInBuffer -= m_remainingLength;

                        // We're done with this message.
                        m_remainingLength = 0;

                        // Push it into the queue.
                        messageQueue.Enqueue(message);

                        // Start a new message.
                        message = new Message(m_buffer.Length);
                    }
                }

#if ASYNCHRONOUS_MESSAGE_HANDLING
                // Wait for more data.  This is a non-blocking call.
                WaitForMessage(message);

                //handle all message in parallel                
                Parallel.ForEach(messageQueue, (messageObject) => RaiseOnMessage(onMessage, messageObject));

#else
                // Raise message events in order.
                while (0 < messageQueue.Count)
                {
                    RaiseOnMessage(onMessage, messageQueue.Dequeue());
                }

                // Wait for more data.  This is a non-blocking call.
                WaitForMessage(message);
#endif
            }
        }

        /// <summary>
        /// A private helper function that raises the "on message" handler, which is passed in.
        /// </summary>
        /// <param name="onMessage">The handler to raise.</param>
        /// <param name="theMessage">The message to pass to the handler.</param>
        private void RaiseOnMessage(MessageHandler onMessage, Message message)
        {
            try
            {
                onMessage(this, message);
            }
            catch (IndexOutOfRangeException ex)
            {
                // Bad message.
                if (Tracer.Enabled)
                {
                    Tracer.WriteLine("Bad message received: {0}", ex.Message);
                }
                else
                {
                    System.Diagnostics.Trace.WriteLine(string.Format(
                        "Bad message received: {0}", ex.Message));
                }

                Logging.Categories.Connection.Error(ex);

                // Close the connection.
                Close();
            }
            catch (ArgumentException ex)
            {
                // Bad message.
                if (Tracer.Enabled)
                {
                    Tracer.WriteLine("Bad message received: {0}", ex.Message);
                }
                else
                {
                    System.Diagnostics.Trace.WriteLine(string.Format(
                        "Bad message received: {0}", ex.Message));
                }

                Logging.Categories.Connection.Error(ex);

                // Close the connection.
                Close();
            }
            catch (Exception ex)
            {
                if (Tracer.Enabled)
                {
                    Tracer.WriteLine("Ignoring exception in message handler: {0}", ex.Message);
                }
                else
                {
                    System.Diagnostics.Trace.WriteLine(string.Format(
                        "Ignoring exception in message handler: {0}", ex.Message));
                }

                Logging.Categories.Connection.Error(ex);
            }
        }

        /// <summary>
        /// This event is triggered when a message has been received.
        /// </summary>
        private event MessageHandler m_onMessage;
        private object m_messageEventLock = new object();
        public event MessageHandler OnMessage
        {
            add
            {
                lock (m_messageEventLock)
                {
                    if (null == m_onMessage)
                    {
                        m_onMessage = value;

                        // Now that we have an event handler, start receiving the next message.
                        // TODO: maybe we should only start this once.
                        WaitForMessage(new Message(m_buffer.Length));
                    }
                    else
                    {
                        m_onMessage += value;
                    }
                }
            }

            remove
            {
                lock (m_messageEventLock)
                {
                    // TODO: should we close down the WaitForMessage logic here?
                    m_onMessage -= value;
                }
            }
        }

        #endregion

        #region Close

        /// <summary>
        /// Close the connection.
        /// </summary>
        public void Close()
        {
            System.Net.Sockets.TcpClient client = m_client;
            m_client = null;

            if (null != client)
            {
                System.Net.Sockets.Socket socket;
                lock (m_lockClient)
                {
                    socket = client.Client;
                    client.Client = null;
                }

                if (null != socket)
                {
                    if (socket.Connected)
                    {
                        try
                        {
                            // See if data is still available to be read.
                            Tracer.DebugWriteLine("Closing with {0} bytes still available to be read.", socket.Available.ToString());
                        }
                        catch (Exception)
                        {
                            // Ignore any exception, since we're closing anyway.
                        }

                        try
                        {
                            // Shut down the socket.
                            socket.Shutdown(System.Net.Sockets.SocketShutdown.Both);
                        }
                        catch (Exception)
                        {
                            // Ignore any exception, since we're closing anyway.
                        }

                        try
                        {
                            // Close the socket.
                            socket.Close();
                        }
                        catch (Exception)
                        {
                            // Ignore any exception, since we're closing anyway.
                        }
                    }

                    try
                    {
                        // Close the TcpClient (which disposes it).
                        lock (m_lockClient)
                        {
                            client.Close();
                        }
                    }
                    catch (Exception)
                    {
                        // Ignore any exception, since we're closing anyway.
                    }

                    if (null != m_onClosed)
                    {
                        // Trigger OnClosed event.
                        m_onClosed(this, this);
                    }
                }
            }
        }

        /// <summary>
        /// This event is triggered when the connection has been closed.
        /// </summary>
        private event ClosedConnectionHandler m_onClosed;
        private object m_closeEventLock = new object();
        public event ClosedConnectionHandler OnClosed
        {
            add 
            {
                lock (m_closeEventLock)
                {
                    m_onClosed += value;
                }
            }

            remove
            {
                lock (m_closeEventLock)
                {
                    m_onClosed -= value;
                }
            }
        }

        /// <summary>
        /// This is true if the connection is closed; otherwise, false.
        /// </summary>
        public bool IsClosed
        {
            get
            {
                return (null == Socket);
            }
        }

        #endregion
    }
}