using System;

namespace PyxNet.Test
{
    /// <summary>
    /// Test Class to test Sending/Receiving Local Node Information.
    /// </summary>
    //[NUnit.Framework.TestFixture]
    public class StackConnectionLocalNodeInfoTests : TestStackConnection
    {
        /// <summary>
        /// Set to true when a Local Node Info is seen from stack one.
        /// </summary>
        private bool m_wasLNIReceivedOne = false;

        /// <summary>
        /// Set to true when a Local Node Info is seen from stack two.
        /// </summary>
        private bool m_wasLNIReceivedTwo = false;

        /// <summary>
        /// Handle a Local Node Info coming from stack one.
        /// </summary>
        public void HandleLNIOne(StackConnection connection, Message message)
        {
            m_wasLNIReceivedOne = true;
        }

        /// <summary>
        /// Handle a Local Node Info coming from stack two.
        /// </summary>
        public void HandleLNITwo(StackConnection connection, Message message)
        {
            m_wasLNIReceivedTwo = true;
        }

        /// <summary>
        /// Entry point for the test code for Local Node Info.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestLNI()
        {
            m_one.OnLocalNodeInfo += HandleLNIOne;
            m_two.OnLocalNodeInfo += HandleLNITwo;

            // create some dummy info about node one.
            NodeInfo localNodeInfoOne = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 15, 942);

            // create some dummy info about node two.
            NodeInfo localNodeInfoTwo = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Leaf, 4, 0);

            m_one.SendMessage(localNodeInfoOne.ToMessage());
            DateTime startTime = DateTime.Now;
            while (!m_wasLNIReceivedTwo)
            {
                // hang out for a 1/10th of a second
                System.Threading.Thread.Sleep(100);
                TimeSpan elapsedTime = DateTime.Now - startTime;

                // fail the test if it takes longer than 5 seconds to see the connection
                NUnit.Framework.Assert.IsTrue(elapsedTime.TotalSeconds < 5,
                    "Connection two did not get an LNI message.");
            }

            // Connection two's remote Node should be the info for connection one.
            NUnit.Framework.Assert.IsTrue(localNodeInfoOne.Mode == m_two.RemoteNodeInfo.Mode);
            NUnit.Framework.Assert.IsTrue(localNodeInfoOne.HubCount == m_two.RemoteNodeInfo.HubCount);
            NUnit.Framework.Assert.IsTrue(localNodeInfoOne.LeafCount == m_two.RemoteNodeInfo.LeafCount);
            NUnit.Framework.Assert.IsTrue(localNodeInfoOne.NodeGUID == m_two.RemoteNodeInfo.NodeGUID);

            m_two.SendMessage(localNodeInfoTwo.ToMessage());
            startTime = DateTime.Now;
            while (!m_wasLNIReceivedOne)
            {
                // hang out for a 1/10th of a second
                System.Threading.Thread.Sleep(100);
                TimeSpan elapsedTime = DateTime.Now - startTime;

                // fail the test if it takes longer than 5 seconds to see the connection
                NUnit.Framework.Assert.IsTrue(elapsedTime.TotalSeconds < 5,
                    "Connection one did not get an LNI message.");
            }

            // Connection two's remote Node should be the info for connection one.
            NUnit.Framework.Assert.IsTrue(localNodeInfoTwo.Mode == m_one.RemoteNodeInfo.Mode);
            NUnit.Framework.Assert.IsTrue(localNodeInfoTwo.HubCount == m_one.RemoteNodeInfo.HubCount);
            NUnit.Framework.Assert.IsTrue(localNodeInfoTwo.LeafCount == m_one.RemoteNodeInfo.LeafCount);
            NUnit.Framework.Assert.IsTrue(localNodeInfoTwo.NodeGUID == m_one.RemoteNodeInfo.NodeGUID);

            Close();
        }
    }
}