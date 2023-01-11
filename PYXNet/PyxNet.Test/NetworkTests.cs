using System;
using NUnit.Framework;

namespace PyxNet.Test
{
    public class NetworkTests
    {
        /// <summary>
        /// Test passing a message on the network.
        /// </summary>
        /// <param name="network"></param>
        /// <param name="attachToAddress"></param>
        /// <param name="outgoingMessageSent"></param>
        /// <param name="incomingMessageSent"></param>
        public void TestAnyNetwork(
            INetwork network,
            NetworkAddress attachToAddress,
            Message outgoingMessageSent,
            Message incomingMessageSent)
        {
            INetworkConnection incomingConnection = null;
            INetworkConnection outgoingConnection = null;

            {
                EventHandler<ConnectionEventArgs> handleConnection = delegate(object sender, ConnectionEventArgs args)
                {
                    if (null == incomingConnection)
                    {
                        incomingConnection = args.Connection;
                        incomingConnection.Tracer.DebugWriteLine("Incoming connection assigned.");
                    }
                };

                // Handle connections on the network.
                network.ConnectionOpened += handleConnection;
                network.SetListeners(attachToAddress);

                // Create the outgoing connection.
                outgoingConnection = network.Connect(attachToAddress);
                Assert.IsNotNull(outgoingConnection, "Couldn't create outgoing connection.");
                outgoingConnection.Tracer.DebugWriteLine("Outgoing connection assigned.");

                // Wait until the network connection is accepted.
                for (DateTime startTime = DateTime.Now; null == incomingConnection; )
                {
                    // Hang out for a moment.
                    System.Threading.Thread.Sleep(100);
                    TimeSpan elapsedTime = DateTime.Now - startTime;

                    // Fail the test if it takes too long to see the connection.
                    Assert.IsTrue(elapsedTime.TotalSeconds < 5,
                        "Connection not made.");
                }

                // Stop handling connections on the network.
                network.RemoveListeners();
                network.ConnectionOpened -= handleConnection;

                // Make sure the incoming connection was opened.
                Assert.IsTrue(null != incomingConnection);
            }

            // Start accepting messages on the outgoing side, test message sending, 
            // and stop accepting messages.
            {
                Message outgoingMessageReceived = null;
                Message incomingMessageReceived = null;

                MessageHandler handleIncomingConnectionMessageReceived = delegate(object sender, Message message)
                {
                    incomingMessageReceived = message;
                    incomingConnection.Tracer.DebugWriteLine("Incoming connection received message '{0}'.", message.Identifier);
                };

                MessageHandler handleOutgoingConnectionMessageReceived = delegate(object sender, Message message)
                {
                    outgoingMessageReceived = message;
                    outgoingConnection.Tracer.DebugWriteLine("Outgoing connection received message '{0}'.", message.Identifier);
                };

                incomingConnection.OnMessage += handleIncomingConnectionMessageReceived;
                outgoingConnection.OnMessage += handleOutgoingConnectionMessageReceived;

                outgoingConnection.SendMessage(outgoingMessageSent);
                incomingConnection.SendMessage(incomingMessageSent);

                // Wait until both messages are received.
                DateTime startTime = DateTime.Now;
                while (null == incomingMessageReceived || null == outgoingMessageReceived)
                {
                    // hang out for a moment
                    System.Threading.Thread.Sleep(100);
                    TimeSpan elapsedTime = DateTime.Now - startTime;

                    // fail the test if it takes too long to get the messages
                    Assert.IsTrue(elapsedTime.TotalSeconds < 10, "Messages not received.");
                }

                // The incoming message received should be the same as the outgoing message sent.
                Assert.IsTrue(incomingMessageReceived.Length == outgoingMessageSent.Length,
                    String.Format("Incoming length {0} != outgoing length {1}.",
                        incomingMessageReceived.Length.ToString(),
                        outgoingMessageSent.Length.ToString()));
                Assert.IsTrue(incomingMessageReceived.Equals(outgoingMessageSent));

                // The outgoing message received should be the same as the incoming message sent.
                Assert.IsTrue(outgoingMessageReceived.Length == incomingMessageSent.Length);
                Assert.IsTrue(outgoingMessageReceived.Equals(incomingMessageSent));

                incomingConnection.OnMessage -= handleIncomingConnectionMessageReceived;
                outgoingConnection.OnMessage -= handleOutgoingConnectionMessageReceived;
            }

            // Ensure that incoming message event is fired.
            {
                bool incomingCloseEventFired = false;
                incomingConnection.OnClosed += delegate(object sender, INetworkConnection connection)
                {
                    incomingCloseEventFired = true;
                };

                Assert.IsTrue(!incomingConnection.IsClosed,
                    "The incoming connection should not be closed before we call close.");
                Assert.IsTrue(!outgoingConnection.IsClosed,
                    "The outgoing connection should not be closed before we call close.");

                // Close our connection.
                outgoingConnection.Close();

                // Wait for the close to come through.
                for (DateTime startTime = DateTime.Now;
                    !incomingConnection.IsClosed || !incomingCloseEventFired; )
                {
                    // Hang out for a moment.
                    System.Threading.Thread.Sleep(500);
                    TimeSpan elapsedTime = DateTime.Now - startTime;

                    // Fail the test if it takes too long to see the connection close.
                    if (elapsedTime.TotalSeconds > 10)
                    {
                        Assert.IsTrue(incomingConnection.IsClosed,
                            "Did not see the connection closed.");
                        Assert.IsTrue(incomingCloseEventFired,
                            "Did not see the connection closed event fire.");
                    }
                }

                Assert.IsTrue(incomingCloseEventFired,
                    "The remote end of the connection did not fire a close event.");
            }
        }

        /// <summary>
        /// Test sending an arbitrary short message.
        /// </summary>
        /// <param name="network"></param>
        /// <param name="attachToAddress"></param>
        public void TestAnyNetwork(INetwork network, NetworkAddress attachToAddress)
        {
            Message outgoingMessageSent = new Message(1024);
            outgoingMessageSent.Append("Hello there server.");

            Message incomingMessageSent = new Message(1024);
            incomingMessageSent.Append("Hello there client.");

            TestAnyNetwork(network, attachToAddress, outgoingMessageSent, incomingMessageSent);
        }
    }
}