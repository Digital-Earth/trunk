using System;

namespace PyxNet.Test
{
    /// <summary>
    /// Class to create two sides of a stack connection.
    /// Derive from this class to get a stack to test with.
    /// The connection is createed in the constructor for this class.
    /// </summary>
    public class TestStackConnection
    {
        /// <summary>
        /// Used to keep track of the incoming connection.
        /// </summary>
        protected INetworkConnection m_incomingConnection;

        /// <summary>
        /// Keep track of the outgoing connection.
        /// </summary>
        protected INetworkConnection m_outgoingConnection;

        /// <summary>
        /// Set to true once we have an incoming connection.
        /// </summary>
        protected bool m_wasIncomingConnectionOpened = false;

        /// <summary>
        /// One side of the connection.
        /// </summary>
        protected StackConnection m_one;

        /// <summary>
        /// The other side of the connection.
        /// </summary>
        protected StackConnection m_two;

        /// <summary>
        /// Used to listen for a new connection.
        /// </summary>
        /// <param name="sender">The network that created the connection.</param>
        /// <param name="newConnection">The new connection.</param>
        public void HandleConnection(object sender, ConnectionEventArgs args)
        {
            if (m_incomingConnection == null)
            {
                m_incomingConnection = args.Connection;
                m_wasIncomingConnectionOpened = true;
            }
        }

        // The port number we are going to use to listen for connections.
        // This member can be incremented on connection so that we don't reuse port numbers.
        static int portNum = 34044;

        /// <summary>
        /// Default constructor that creates boths sides of the connection.
        /// </summary>
        public TestStackConnection()
        {
            System.Net.IPEndPoint listenOn = new System.Net.IPEndPoint(
                System.Net.IPAddress.Parse("127.0.0.1"), portNum);
            NetworkAddress attachToAddress = new NetworkAddress(listenOn);

            using (TcpNetwork network = new TcpNetwork())
            {
                network.ConnectionOpened += HandleConnection;
                network.SetListeners(attachToAddress);

                m_outgoingConnection = network.Connect(attachToAddress);
                NUnit.Framework.Assert.IsNotNull(m_outgoingConnection, "The outgoing connection couldn't be created.");

                // wait until the network connection is accepted
                DateTime startTime = DateTime.Now;
                while (!m_wasIncomingConnectionOpened)
                {
                    // hang out for a 1/10th of a second
                    System.Threading.Thread.Sleep(100);
                    TimeSpan elapsedTime = DateTime.Now - startTime;

                    // fail the test if it takes longer than 5 seconds to see the connection
                    NUnit.Framework.Assert.IsTrue(elapsedTime.TotalSeconds < 15,
                        "Connection not made.");
                }
            }

            m_one = new StackConnection(m_incomingConnection);
            m_one.Start();

            m_two = new StackConnection(m_outgoingConnection);
            m_two.Start();
        }

        public void Close()
        {
            if (null != m_one)
            {
                m_one.Close();
            }
            NUnit.Framework.Assert.IsTrue(m_one.IsClosed, "Connection one didn't close.");

            if (null != m_two)
            {
                m_two.Close();
            }
            NUnit.Framework.Assert.IsTrue(m_two.IsClosed, "Connection two didn't close.");

            // give the sockets a chance to clean up.
            System.Threading.Thread.Sleep(100);
        }
    }
}