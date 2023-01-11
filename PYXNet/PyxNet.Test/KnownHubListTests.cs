using System.Collections.Generic;

namespace PyxNet.Test
{
    /// <summary>
    /// Testing for Known Hub List class.
    /// </summary>
    [NUnit.Framework.TestFixture]
    public class KnownHubListTests
    {
        /// <summary>
        /// Create a known hub list for testing.
        /// </summary>
        /// <returns>A new known hub list.</returns>
        static public KnownHubList CreateKnownHubList()
        {
            NodeInfo info1 = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 12, 150);
            NodeInfo info2 = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 14, 160);
            NodeInfo info3 = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 16, 170);
            NodeInfo info4 = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 18, 180);
            NodeInfo info5 = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 20, 190);

            List<NodeInfo> connected = new List<NodeInfo>();
            connected.Add(info1);
            connected.Add(info2);
            connected.Add(info3);

            List<NodeInfo> known = new List<NodeInfo>();
            known.Add(info4);
            known.Add(info5);

            KnownHubList testList = new KnownHubList(connected, known);

            IList<NodeInfo> khlConnected = testList.ConnectedHubs;
            NUnit.Framework.Assert.IsTrue(khlConnected.Count == 3);
            NUnit.Framework.Assert.IsTrue(khlConnected.Contains(info1));
            NUnit.Framework.Assert.IsTrue(khlConnected.Contains(info2));
            NUnit.Framework.Assert.IsTrue(khlConnected.Contains(info3));
            NUnit.Framework.Assert.IsFalse(khlConnected.Contains(info4));
            NUnit.Framework.Assert.IsFalse(khlConnected.Contains(info5));

            IList<NodeInfo> khlKnown = testList.KnownHubs;
            NUnit.Framework.Assert.IsTrue(khlKnown.Count == 2);
            NUnit.Framework.Assert.IsFalse(khlKnown.Contains(info1));
            NUnit.Framework.Assert.IsFalse(khlKnown.Contains(info2));
            NUnit.Framework.Assert.IsFalse(khlKnown.Contains(info3));
            NUnit.Framework.Assert.IsTrue(khlKnown.Contains(info4));
            NUnit.Framework.Assert.IsTrue(khlKnown.Contains(info5));

            return testList;
        }

        /// <summary>
        /// Test construction and moving to/from a message.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestKnownHubList()
        {
            // Create a KnownHubList.
            KnownHubList testList = CreateKnownHubList();

            // Create a message from the KnownHubList.
            Message message = testList.ToMessage();

            // Recreate the KnownHubList from the message.
            KnownHubList reconstructedList = new KnownHubList(message);

            // Check the original against the reconstructed.
            NUnit.Framework.Assert.IsTrue(testList.Equals(reconstructedList));
        }

        /// <summary>
        /// Test finding Nodes in the list.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestKnownHubListContains()
        {
            // Create a KnownHubList.
            KnownHubList testList = CreateKnownHubList();

            // Create a message from the KnownHubList.
            Message message = testList.ToMessage();

            // Recreate the KnownHubList from the message.
            KnownHubList reconstructedList = new KnownHubList(message);

            // Check that the original contains the reconstructed.
            {
                IList<NodeInfo> test = testList.ConnectedHubs;
                foreach (NodeInfo element in reconstructedList.ConnectedHubs)
                {
                    NUnit.Framework.Assert.IsTrue(test.Contains(element));
                }
            }
            {
                IList<NodeInfo> test = testList.KnownHubs;
                foreach (NodeInfo element in reconstructedList.KnownHubs)
                {
                    NUnit.Framework.Assert.IsTrue(test.Contains(element));
                }
            }
        }
    }
}