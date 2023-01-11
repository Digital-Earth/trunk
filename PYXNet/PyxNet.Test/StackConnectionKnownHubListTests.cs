using System;
using System.Collections.Generic;

namespace PyxNet.Test
{
    /// <summary>
    /// Test Class to test Sending/Receiving Known Hub lists.
    /// </summary>
    //[NUnit.Framework.TestFixture]
    public class StackConnectionKnownHubListTests : TestStackConnection
    {
        /// <summary>
        /// Set to true when a message is seen from stack one.
        /// </summary>
        private bool m_wasReceivedOne = false;

        /// <summary>
        /// Set to true when a message is seen from stack two.
        /// </summary>
        private bool m_wasReceivedTwo = false;

        /// <summary>
        /// Handle a message coming from stack one.
        /// </summary>
        public void HandleOne(StackConnection connection, Message message)
        {
            m_wasReceivedOne = true;
        }

        /// <summary>
        /// Handle a message coming from stack two.
        /// </summary>
        public void HandleTwo(StackConnection connection, Message message)
        {
            m_wasReceivedTwo = true;
        }

        /// <summary>
        /// Entry point for the test code.
        /// </summary>
        [NUnit.Framework.Test]
        public void Test()
        {
            m_one.OnKnownHubList += HandleOne;
            m_two.OnKnownHubList += HandleTwo;

            NodeInfo oneInfo1 = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 12, 150);
            NodeInfo oneInfo2 = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 14, 160);
            NodeInfo oneInfo3 = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 16, 170);
            NodeInfo oneInfo4 = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 18, 180);
            NodeInfo oneInfo5 = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 20, 190);

            List<NodeInfo> connectedOne = new List<NodeInfo>();
            connectedOne.Add(oneInfo1);
            connectedOne.Add(oneInfo2);
            connectedOne.Add(oneInfo3);

            List<NodeInfo> knownOne = new List<NodeInfo>();
            knownOne.Add(oneInfo4);
            knownOne.Add(oneInfo5);

            KnownHubList payloadOne = new KnownHubList(connectedOne, knownOne);

            NodeInfo twoInfo1 = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 12, 150);
            NodeInfo twoInfo2 = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 14, 160);
            NodeInfo twoInfo3 = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 16, 170);
            NodeInfo twoInfo4 = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 18, 180);
            NodeInfo twoInfo5 = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 20, 190);

            List<NodeInfo> connectedTwo = new List<NodeInfo>();
            connectedTwo.Add(twoInfo1);
            connectedTwo.Add(twoInfo2);

            List<NodeInfo> knownTwo = new List<NodeInfo>();
            knownTwo.Add(twoInfo3);
            knownTwo.Add(twoInfo4);
            knownTwo.Add(twoInfo5);

            KnownHubList payloadTwo = new KnownHubList(connectedTwo, knownTwo);

            m_one.SendMessage(payloadOne.ToMessage());
            DateTime startTime = DateTime.Now;
            while (!m_wasReceivedTwo)
            {
                // hang out for a 1/10th of a second
                System.Threading.Thread.Sleep(100);
                TimeSpan elapsedTime = DateTime.Now - startTime;

                // fail the test if it takes longer than 5 seconds to see the connection
                NUnit.Framework.Assert.IsTrue(elapsedTime.TotalSeconds < 5,
                    "Connection two did not get a message.");
            }

            // Compare what was sent with the result.
            {
                IList<NodeInfo> connected = payloadOne.ConnectedHubs;
                IList<NodeInfo> remoteConnected = m_two.RemoteKnownHubList.ConnectedHubs;
                for (int count = 0; count < connected.Count; ++count)
                {
                    NUnit.Framework.Assert.IsTrue(
                        connected[count].LeafCount == remoteConnected[count].LeafCount);
                    NUnit.Framework.Assert.IsTrue(
                        connected[count].HubCount == remoteConnected[count].HubCount);
                }
            }

            m_two.SendMessage(payloadTwo.ToMessage());
            startTime = DateTime.Now;
            while (!m_wasReceivedOne)
            {
                // hang out for a 1/10th of a second
                System.Threading.Thread.Sleep(100);
                TimeSpan elapsedTime = DateTime.Now - startTime;

                // fail the test if it takes longer than 5 seconds to see the connection
                NUnit.Framework.Assert.IsTrue(elapsedTime.TotalSeconds < 5,
                    "Connection one did not get a message.");
            }

            // Compare what was sent with the result.
            {
                IList<NodeInfo> connected = payloadTwo.ConnectedHubs;
                IList<NodeInfo> remoteConnected = m_one.RemoteKnownHubList.ConnectedHubs;
                for (int count = 0; count < connected.Count; ++count)
                {
                    NUnit.Framework.Assert.IsTrue(
                        connected[count].LeafCount == remoteConnected[count].LeafCount);
                    NUnit.Framework.Assert.IsTrue(
                        connected[count].HubCount == remoteConnected[count].HubCount);
                }
            }

            Close();
        }
    }
}