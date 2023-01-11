using System;
using System.Collections.Generic;
using NUnit.Framework;

namespace PyxNet.Test
{
    public class NetworkSimplifiedMultiThreadedHammerTests
    {
        private INetworkConnection m_connection1;
        private INetworkConnection m_connection2;

        private const int m_numberMessagesToSend = 500;
        private const int m_threadPairsToStart = 25;

        private int m_messagesSentBy1 = 0;
        private int m_messagesSentBy2 = 0;

        private int m_messagesReceivedBy1 = 0;
        private int m_messagesReceivedBy2 = 0;

        private void HandleConnection(object sender, ConnectionEventArgs args)
        {
            m_connection2 = args.Connection;
        }

        private void TestConnect(INetwork network, NetworkAddress attachToAddress)
        {
            // This should only be called before the connections are established.
            System.Diagnostics.Debug.Assert(null == m_connection1);
            System.Diagnostics.Debug.Assert(null == m_connection2);

            network.ConnectionOpened += HandleConnection;
            network.SetListeners(attachToAddress);

            m_connection1 = network.Connect(attachToAddress);
            Assert.IsNotNull(m_connection1, "Couldn't create connection 1.");

            // Wait until the network connection is accepted.
            const int timeOutSeconds = 8;
            DateTime startTime = DateTime.Now;
            while (null == m_connection2 && (DateTime.Now - startTime).TotalSeconds < timeOutSeconds)
            {
                // Sleep for a moment.
                System.Threading.Thread.Sleep(500);
            }

            Assert.IsNotNull(m_connection2, "Couldn't create connection 2.");

            network.RemoveListeners();
            network.ConnectionOpened -= HandleConnection;
        }

        private void TestDisconnect()
        {
            // This should only be called after the connections are established.
            System.Diagnostics.Debug.Assert(null != m_connection1);
            System.Diagnostics.Debug.Assert(null != m_connection2);

            // Close our connection.
            m_connection1.Close();

            // Wait for the close to propagate to connection 2, and close it.
            const int timeOutSeconds = 8;
            DateTime startTime = DateTime.Now;
            while (!m_connection2.IsClosed && (DateTime.Now - startTime).TotalSeconds < timeOutSeconds)
            {
                // Sleep for a moment.
                System.Threading.Thread.Sleep(500);
            }

            Assert.IsTrue(m_connection2.IsClosed, "Connection 2 didn't see that connection 1 closed, and remains open.");

            m_connection1 = null;
            m_connection2 = null;
        }

        private Message ConstructMessage(int number, string content)
        {
            Message message = new Message();
            message.Append(String.Format("{0}: {1}", content, number.ToString()));
            return message;
        }

        public void SendFrom1()
        {
            for (int count = 0; count < m_numberMessagesToSend; ++count)
            {
                bool sent = m_connection1.SendMessage(ConstructMessage(m_messagesSentBy1, "From 1 to 2."));
                Assert.IsTrue(sent);
                System.Threading.Interlocked.Increment(ref m_messagesSentBy1);
            }
        }

        public void SendFrom2()
        {
            for (int count = 0; count < m_numberMessagesToSend; ++count)
            {
                bool sent = m_connection2.SendMessage(ConstructMessage(m_messagesSentBy2, "From 2 to 1."));
                Assert.IsTrue(sent);
                System.Threading.Interlocked.Increment(ref m_messagesSentBy2);
            }
        }

        private void HandleMessageTo1(object sender, Message message)
        {
            System.Threading.Interlocked.Increment(ref m_messagesReceivedBy1);
        }

        private void HandleMessageTo2(object sender, Message message)
        {
            System.Threading.Interlocked.Increment(ref m_messagesReceivedBy2);
        }

        private void TestSend()
        {
            // This should only be called after the connections are established.
            System.Diagnostics.Debug.Assert(null != m_connection1);
            System.Diagnostics.Debug.Assert(null != m_connection2);

            // Create a bunch of threads that send in both directions.
            List<System.Threading.Thread> threadList = new List<System.Threading.Thread>();
            for (int count = 0; count < m_threadPairsToStart; ++count)
            {
                threadList.Add(new System.Threading.Thread(new System.Threading.ThreadStart(SendFrom1)));
                threadList.Add(new System.Threading.Thread(new System.Threading.ThreadStart(SendFrom2)));
            }

            // Connect message handlers.
            m_connection1.OnMessage += HandleMessageTo1;
            m_connection2.OnMessage += HandleMessageTo2;

            // Start them.
            threadList.ForEach(delegate(System.Threading.Thread t)
            {
                t.Start();
            });

            // Wait for them to finish.
            threadList.ForEach(delegate(System.Threading.Thread t)
            {
                t.Join();
            });

            // Verify that all messages were sent.
            int nTotalMessages = m_threadPairsToStart * m_numberMessagesToSend;
            Assert.AreEqual(nTotalMessages, m_messagesSentBy1);
            Assert.AreEqual(nTotalMessages, m_messagesSentBy2);

            // Give the messages time to be received.
            const int timeOutSeconds = 120;
            DateTime startTime = DateTime.Now;
            while ((m_messagesSentBy1 != m_messagesReceivedBy2 || m_messagesSentBy2 != m_messagesReceivedBy1) &&
                   (DateTime.Now - startTime).TotalSeconds < timeOutSeconds)
            {
                // Sleep for a moment.
                System.Threading.Thread.Sleep(500);
            }

            // Verify that all messages were received.
            Assert.AreEqual(m_messagesSentBy1, m_messagesReceivedBy2);
            Assert.AreEqual(m_messagesSentBy2, m_messagesReceivedBy1);

            // Disconnect message handlers.
            m_connection1.OnMessage -= HandleMessageTo1;
            m_connection2.OnMessage -= HandleMessageTo2;
        }

        public void HammerAnyNetwork(INetwork network, NetworkAddress attachToAddress)
        {
            TestConnect(network, attachToAddress);
            Assert.IsNotNull(m_connection1, "Connection 1 is null.");
            Assert.IsNotNull(m_connection2, "Connection 2 is null.");

            TestSend();

            TestDisconnect();
            Assert.IsNull(m_connection1, "Connection 1 is not null.");
            Assert.IsNull(m_connection2, "Connection 2 is not null.");
        }
    }
}