using System;
using Pyxis.Utilities.Test;

namespace PyxNet.Test
{
    [NUnit.Framework.TestFixture]
    public class NodeInfoTests
    {
        /// <summary>
        /// Test construction and moving to/from a message.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestLocalNodeInfo()
        {
            // Create a local node info object.
            NodeInfo myInfo = new NodeInfo();
            // Generate a random public key (for testing purposes)
            myInfo.PublicKey = new PyxNet.DLM.PrivateKey().PublicKey;
            myInfo.Mode = NodeInfo.OperatingMode.Hub;
            myInfo.HubCount = 10;
            myInfo.LeafCount = 972;
            myInfo.NodeGUID = new Guid("C2BDECF2-8FEB-48f9-BEFA-FE9E76B53B92");
            myInfo.Address = new NetworkAddress(new System.Net.IPEndPoint(System.Net.IPAddress.Parse("127.0.0.1"), 33033));
            myInfo.FriendlyName = "Your Friendly Neighbourhood Hub";

            // Convert it to a message.
            Message aMessage = myInfo.ToMessage();

            // Construct a new one from the message.
            NodeInfo reconstructedInfo = new NodeInfo(aMessage);

            // Ensure that the new LNI contains the same information as the old LNI.
            NUnit.Framework.Assert.IsTrue(myInfo.Mode == reconstructedInfo.Mode);
            NUnit.Framework.Assert.IsTrue(myInfo.IsHub);
            NUnit.Framework.Assert.IsFalse(myInfo.IsLeaf);
            NUnit.Framework.Assert.IsTrue(reconstructedInfo.IsHub);
            NUnit.Framework.Assert.IsTrue(myInfo.HubCount == reconstructedInfo.HubCount);
            NUnit.Framework.Assert.IsTrue(myInfo.LeafCount == reconstructedInfo.LeafCount);
            NUnit.Framework.Assert.IsTrue(myInfo.NodeGUID == reconstructedInfo.NodeGUID);
            NUnit.Framework.Assert.IsTrue(myInfo.Address.Equals(reconstructedInfo.Address));
            NUnit.Framework.Assert.IsTrue(myInfo.FriendlyName.Equals(reconstructedInfo.FriendlyName));
            NUnit.Framework.Assert.AreEqual(myInfo.PublicKey, reconstructedInfo.PublicKey);
            NUnit.Framework.Assert.IsTrue(myInfo.Equals(reconstructedInfo));
        }

        /// <summary>
        /// Test for <see cref="NodeInfo.Find"/>.
        /// </summary>
        [NUnit.Framework.Test]
        public void FindNodeInfo()
        {
            // TODO: This should work with Topology.Two, but doesn't.  There is 
            // some difficulty with initialization.  (Which shouldn't matter in
            // production code, since all the nodes will update their QHT's as
            // they come up.
            using (StackTestHelper stacks = new StackTestHelper(StackTestHelper.Topology.Four, StackTests.ShortInterval))
            {
                // TODO: Determine why this test requires this sleep to pass, and remove it.
                // Possibly related to initialization difficulty cited above.
                System.Threading.Thread.Sleep(1000);
                stacks.WaitForHashUpdates();

                FindSingleNodeInfo(stacks.Stacks[1], stacks.Stacks[0],
                    "Error finding nodeinfo from our own hub.");
                FindSingleNodeInfo(stacks.Stacks[1], stacks.Stacks[2],
                    "Error finding nodeinfo from a node connected directly to our own hub.");
                FindSingleNodeInfo(stacks.Stacks[1], stacks.Stacks[5],
                    "Error finding nodeinfo from a distant hub.");
            }
        }

        /// <summary>
        /// Finds a single node info.  Helper for FindNodeInfo.
        /// </summary>
        /// <param name="seeker">The seeker.</param>
        /// <param name="target">The target.</param>
        /// <param name="failureMessage">The failure message.</param>
        private static void FindSingleNodeInfo(Stack seeker, Stack target, string failureMessage)
        {
            try
            {
                // Wait for the qht to update itself...
                TimedTest.Verify(delegate() { return !target.IsQueryHashTableDirty; },
                    TimeSpan.FromSeconds(10),
                    "Stack was unable to update its Query Hash Table.");

                NodeInfo found = NodeInfo.Find(seeker, target.NodeInfo.NodeId, TimeSpan.FromSeconds(15));
                NUnit.Framework.Assert.AreEqual(found, target.NodeInfo,
                    failureMessage);
            }
            catch (TimeoutException ex)
            {
                throw new ApplicationException(failureMessage, ex);
            }
        }
    }
}