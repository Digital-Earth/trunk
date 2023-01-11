using System;
using Pyxis.Utilities.Test;

namespace PyxNet.Test
{
    /// <summary>
    /// Test Stacks.
    /// </summary>
    [NUnit.Framework.TestFixture]
    public class StackTests
    {
        /// <summary>
        /// The interval of short tests, in milliseconds.
        /// </summary>
        public const int ShortInterval = 500;

        /// <summary>
        /// A count of how many "Ping Intervals" to run the test for.
        /// </summary>
        private const int m_intervalCount = 10;

        // Utility function to encapsulate the using of the test.
        public void RunStackTest(StackTestHelper.Topology topology, int interval)
        {
            using (StackTestHelper testHelper = new StackTestHelper(topology, interval))
            {
                testHelper.Run(m_intervalCount);
            }
        }

        // Utility function to encapsulate the using of the test.
        public void RunQHTStackTest(StackTestHelper.Topology topology, int interval, bool RunLongTesting)
        {
            using (StackTestHelper testHelper = new StackTestHelper(topology, interval))
            {
                testHelper.RunQHTTest(RunLongTesting);
            }
        }

        /// <summary>
        /// Test connection to the wrong node, and ensure that the connection fails.
        /// </summary>
        public void RunConnectionTest(StackTestHelper.Topology topology, int interval)
        {
            using (StackTestHelper testHelper = new StackTestHelper(topology, interval))
            {
                testHelper.TestConnectionHandshake();
            }
        }

        /// <summary>
        /// Test multithreaded connection to a single node.
        /// </summary>
        public void RunMultiConnectionTest(StackTestHelper.Topology topology, int interval)
        {
            using (StackTestHelper testHelper = new StackTestHelper(topology, interval))
            {
                testHelper.TestMultiConnection();
            }
        }

        /// <summary>
        /// Test relaying messages via the hubs.
        /// </summary>
        [NUnit.Framework.Test]
        public void RelayMessageTest()
        {
            using (StackTestHelper testHelper = new StackTestHelper(StackTestHelper.Topology.Two, ShortInterval))
            {
                // "Handle" the result on node 1.
                bool messageReceived = false;
                testHelper.Stacks[1].RegisterHandler("TEST",
                    delegate(object sender, MessageHandlerCollection.MessageReceivedEventArgs args)
                    {
                        messageReceived = true;
                    });

                // "Handle" the result on node 0.  Note that this node should _not_ get the message.
                bool messageIntercepted = false;
                testHelper.Stacks[0].RegisterHandler("TEST",
                    delegate(object sender, MessageHandlerCollection.MessageReceivedEventArgs args)
                    {
                        messageIntercepted = true;
                    });

                // Send the message from node 5 to node 1.
                testHelper.Stacks[5].RelayMessage(testHelper.Stacks[1].NodeInfo,
                    new Message("TEST"));

                // Stress the testHelper by garbage collecting to make sure the 
                // relayer can survive a collection.
                GC.GetTotalMemory(true);

                // Test to see if the message was received (waits up to 15 seconds)
                TimedTest.Verify(
                    delegate() { return messageReceived; }, TimeSpan.FromSeconds(15),
                    "Relayed message was not received.");

                // Test to see that the message was not intercepted.
                NUnit.Framework.Assert.IsFalse(messageIntercepted,
                    "Relayed message was received on an intermediate node.");
            }
        }

        /// <summary>
        /// Test reverse connections by trying to contact the node at the wrong
        /// address.  Then the reverse connection message should be relayed to the 
        /// node though PyxNet and the connection should still happen even though
        /// we don't know where the node is.
        /// </summary>
        [NUnit.Framework.Test]
        public void ReverseConnectionTest()
        {
            using (StackTestHelper testHelper = new StackTestHelper(StackTestHelper.Topology.Two, ShortInterval))
            {
                // get a copy of the NodeInfo for Node 1
                NodeInfo badAddressInfo = new NodeInfo(testHelper.Stacks[1].NodeInfo.ToMessage());

                // set the port to 60000 for all addresses.
                foreach (System.Net.IPEndPoint ep in badAddressInfo.Address.InternalIPEndPoints)
                {
                    NUnit.Framework.Assert.AreNotEqual(60000, ep.Port, "The port was already set this value.");
                    ep.Port = 60000;
                }
                foreach (System.Net.IPEndPoint ep in badAddressInfo.Address.ExternalIPEndPoints)
                {
                    NUnit.Framework.Assert.AreNotEqual(60000, ep.Port, "The port was already set this value.");
                    ep.Port = 60000;
                }

                StackConnection existingConnection = testHelper.Stacks[5].FindConnection(badAddressInfo, false);

                // Test to see that the connection is not already there.
                NUnit.Framework.Assert.IsNull(existingConnection,
                    "A connection exists already that is not suppose to be there.");

                StackConnection newConnection = testHelper.Stacks[5].GetConnection(badAddressInfo, true, TimeSpan.FromSeconds(10));

                // Test to see that the connection was made.
                NUnit.Framework.Assert.IsNotNull(newConnection,
                    "Did not get a connection.");
            }
        }

        /// <summary>
        /// Simplest test: create one Stack and let it live for 2 seconds
        /// and then close it.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestConstructionAndLiveForTwoSeconds()
        {
            using (StackTestHelper testHelper = new StackTestHelper(StackTestHelper.Topology.One, ShortInterval))
            {
                // hang out for 2 seconds
                System.Threading.Thread.Sleep(2000);
            }
        }

        /// <summary>
        /// Run short topology 1 test.
        /// </summary>
        [NUnit.Framework.Test]
        public void TopologyOneShort()
        {
            RunStackTest(StackTestHelper.Topology.One, ShortInterval);
        }

        /// <summary>
        /// Run short topology 2 test.
        /// </summary>
        [NUnit.Framework.Test]
        public void TopologyTwoShort()
        {
            RunStackTest(StackTestHelper.Topology.Two, ShortInterval);
        }

        /// <summary>
        /// Run short topology 3 test.
        /// </summary>
        [NUnit.Framework.Test]
        public void TopologyThreeShort()
        {
            RunStackTest(StackTestHelper.Topology.Three, ShortInterval);
        }

        /// <summary>
        /// Run short topology 4 test.
        /// </summary>
        [NUnit.Framework.Test]
        public void TopologyFourShort()
        {
            RunStackTest(StackTestHelper.Topology.Four, ShortInterval);
        }

        /// <summary>
        /// Run topology 1 test on the query hash table propagation.
        /// </summary>
        [NUnit.Framework.Test]
        public void TopologyOneQueryHashTableTest()
        {
            RunQHTStackTest(StackTestHelper.Topology.One, ShortInterval, true);
        }

        /// <summary>
        /// Run topology 2 test on the query hash table propagation.
        /// </summary>
        [NUnit.Framework.Test]
        public void TopologyTwoQueryHashTableTest()
        {
            RunQHTStackTest(StackTestHelper.Topology.Two, ShortInterval, false);
        }

        /// <summary>
        /// Run topology 3 test on the query hash table propagation.
        /// </summary>
        [NUnit.Framework.Test]
        public void TopologyThreeQueryHashTableTest()
        {
            RunQHTStackTest(StackTestHelper.Topology.Three, ShortInterval, false);
        }

        /// <summary>
        /// Run topology 4 test on the query hash table propagation.
        /// </summary>
        [NUnit.Framework.Test]
        public void TopologyFourQueryHashTableTest()
        {
            RunQHTStackTest(StackTestHelper.Topology.Four, ShortInterval, false);
        }

        /// <summary>
        /// Run topology 1 connection test.
        /// </summary>
        [NUnit.Framework.Test]
        public void TopologyOneConnectionTest()
        {
            RunConnectionTest(StackTestHelper.Topology.One, ShortInterval);
        }

        /// <summary>
        /// Run topology 2 connection test.
        /// </summary>
        [NUnit.Framework.Test]
        public void TopologyTwoConnectionTest()
        {
            RunConnectionTest(StackTestHelper.Topology.Two, ShortInterval);
        }

        /// <summary>
        /// Run topology 3 connection test.
        /// </summary>
        [NUnit.Framework.Test]
        public void TopologyThreeConnectionTest()
        {
            RunConnectionTest(StackTestHelper.Topology.Three, ShortInterval);
        }

        /// <summary>
        /// Run topology 4 connection test.
        /// </summary>
        [NUnit.Framework.Test]
        public void TopologyFourConnectionTest()
        {
            RunConnectionTest(StackTestHelper.Topology.Four, ShortInterval);
        }

        /// <summary>
        /// Run topology 2 multi connection test.
        /// </summary>
        [NUnit.Framework.Test]
        public void TopologyTwoMultiConnectionTest()
        {
            RunMultiConnectionTest(StackTestHelper.Topology.Two, ShortInterval);
        }
    }
}