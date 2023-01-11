using NUnit.Framework;
using PyxNet.Publishing;

namespace PyxNet.Test.Publishing
{
    /// <summary>
    /// Unit tests for PublishedNodeInfo
    /// </summary>
    [TestFixture]
    public class PublishedNodeInfoTests
    {
        [Test]
        public void Matches()
        {
            Stack dummyStack = PyxNet.Test.StackTestHelper.CreateStack("Dummy", 1, false);

            NodeInfo nodeInfo = PyxNet.Test.NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Leaf, 0, 0);
            PublishedNodeInfo publishedNodeInfo = new PublishedNodeInfo(nodeInfo);
            Query query = NodeInfo.CreateQuery(nodeInfo, nodeInfo.NodeId);
            Assert.IsNotNull(publishedNodeInfo.Matches(query, dummyStack));

            // Test failure case.
            NodeInfo anotherNodeInfo = PyxNet.Test.NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Leaf, 0, 0);
            Query anotherQuery = NodeInfo.CreateQuery(nodeInfo, anotherNodeInfo.NodeId);
            Assert.IsNull(publishedNodeInfo.Matches(anotherQuery, dummyStack));
        }
    }
}