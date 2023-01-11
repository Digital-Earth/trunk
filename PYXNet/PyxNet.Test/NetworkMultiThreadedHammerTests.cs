using System;
using NUnit.Framework;

namespace PyxNet.Test
{
    public class NetworkMultiThreadedHammerTests
    {
        private INetworkConnection m_incomingConnection;
        private INetworkConnection m_outgoingConnection;
        private bool m_wasIncomingConnectionOpened = false;

        public void HandleConnection(object sender, ConnectionEventArgs args)
        {
            if (m_incomingConnection == null)
            {
                m_incomingConnection = args.Connection;
                m_wasIncomingConnectionOpened = true;
            }
        }

        private object m_threadNumberLock = new object();
        private int m_threadNumber = 0;

        const int m_numberMessagesToSend = 500;
        const int m_threadPairsToStart = 25;

        private int m_outstandingMessageCount = 0;

        private object m_lengthLock = new object();
        private int m_maxMessageLength = 0;

        public void Send(INetworkConnection from, INetworkConnection to, string messageBase)
        {
            int messageNumber;
            lock (m_threadNumberLock)
            {
                messageNumber = m_threadNumber;
                ++m_threadNumber;
            }
            long messageCount = 0;
            string sendMessageContent = messageBase + messageNumber.ToString();
            Message sendMessage = new Message();
            sendMessage.Append(sendMessageContent);

            lock (m_lengthLock)
            {
                if (sendMessage.Length > m_maxMessageLength)
                {
                    m_maxMessageLength = sendMessage.Length;
                }
            }

            MessageHandler messageHandler = delegate(object sender, Message message)
            {
                lock (m_lengthLock)
                {
                    Assert.IsTrue(message.Length <= m_maxMessageLength);
                }
                if (message.Length > m_maxMessageLength)
                {
                    System.Diagnostics.Debug.WriteLine("Got a big message length = " + message.Length.ToString());
                }

                if (message.Equals(sendMessage))
                {
                    System.Threading.Interlocked.Increment(ref messageCount);
                    System.Threading.Interlocked.Decrement(ref m_outstandingMessageCount);
                }
            };

            to.OnMessage += messageHandler;

            // send them
            for (int index = 0; index < m_numberMessagesToSend; ++index)
            {
                if (!from.SendMessage(sendMessage))
                {
                    // we should always send messages
                    return;
                }
                System.Threading.Interlocked.Increment(ref m_outstandingMessageCount);
                System.Threading.Thread.Sleep(0);
            }

            // wait for them all to come through but only for 2 minutes
            DateTime startTime = DateTime.Now;
            while (System.Threading.Interlocked.Read(ref messageCount) < m_numberMessagesToSend)
            {
                // hang out for a moment
                System.Threading.Thread.Sleep(1000);

                if (from.IsClosed)
                {
                    // the socket should stay open until after the test.
                    break;
                }

                TimeSpan elapsedTime = DateTime.Now - startTime;
                if (elapsedTime.TotalSeconds >= 120)
                {
                    break;
                }
            }

            to.OnMessage -= messageHandler;
        }

        public void SendOut()
        {
            Send(m_outgoingConnection, m_incomingConnection, "Out the outgoing ");
        }

        public void SendIn()
        {
            Send(m_incomingConnection, m_outgoingConnection, "Out the in door ");
        }

        public void HammerAnyNetwork(
            INetwork network,
            NetworkAddress attachToAddress)
        {
            const int timeOutSeconds = 8;

            network.ConnectionOpened += HandleConnection;
            network.SetListeners(attachToAddress);

            m_outgoingConnection = network.Connect(attachToAddress);
            Assert.IsNotNull(m_outgoingConnection, "Couldn't create outgoing connection.");

            // wait until the network connection is accepted
            DateTime startTime = DateTime.Now;
            while (!m_wasIncomingConnectionOpened)
            {
                // hang out for a 1/10th of a second
                System.Threading.Thread.Sleep(100);
                TimeSpan elapsedTime = DateTime.Now - startTime;

                // fail the test if it takes too long to see the connection
                Assert.IsTrue(elapsedTime.TotalSeconds < timeOutSeconds,
                    "Connection not made.");
            }

            network.RemoveListeners();
            network.ConnectionOpened -= HandleConnection;

            Assert.IsTrue(m_wasIncomingConnectionOpened);

            // create a bunch of threads in each direction.
            System.Collections.Generic.List<System.Threading.Thread> threadList
                = new System.Collections.Generic.List<System.Threading.Thread>();
            for (int count = 0; count < m_threadPairsToStart; ++count)
            {
                System.Threading.Thread outThread = new System.Threading.Thread(SendOut);
                outThread.Name = "Out " + count.ToString();
                threadList.Add(outThread);
                System.Threading.Thread inThread = new System.Threading.Thread(SendIn);
                inThread.Name = "In " + count.ToString();
                threadList.Add(inThread);
            }

            // start them working
            threadList.ForEach(delegate(System.Threading.Thread t)
            {
                t.Start();
            });

            // wait for them to finish
            threadList.ForEach(delegate(System.Threading.Thread t)
            {
                t.Join();
            });

            // all the messages should have been recieved.
            Assert.IsTrue(m_outstandingMessageCount == 0, 
                String.Format("{0} of {1} sent messages not recieved.", m_outstandingMessageCount,
                    m_threadPairsToStart * 2 * m_numberMessagesToSend));

            // Close our connection.
            m_outgoingConnection.Close();

            // wait for the close to come through.
            startTime = DateTime.Now;
            while (!m_incomingConnection.IsClosed)
            {
                // hang out for a 1/10th of a second
                System.Threading.Thread.Sleep(100);
                TimeSpan elapsedTime = DateTime.Now - startTime;

                // fail the test if it takes too long to see the connection close
                Assert.IsTrue(elapsedTime.TotalSeconds < timeOutSeconds,
                    "Did not see the connection closed.");
            }

            m_outgoingConnection = null;
            m_incomingConnection = null;
        }
    }
}