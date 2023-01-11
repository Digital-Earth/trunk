using System;
using NUnit.Framework;

namespace PyxNet.Test
{
    [TestFixture]
    public class QueryResultTests
    {
        /// <summary>
        /// Test construction and moving to/from a message.
        /// </summary>
        [Test]
        public void TestQueryResult()
        {
            // Create a local node info object.
            NodeInfo originInfo = new NodeInfo();
            originInfo.Mode = NodeInfo.OperatingMode.Leaf;
            originInfo.HubCount = 8;
            originInfo.LeafCount = 17;
            originInfo.NodeGUID = new Guid("F3DFD8DF-9888-4b92-9D88-65BF71FF5246");
            originInfo.PublicKey = new DLM.PrivateKey().PublicKey;
            originInfo.Address = new NetworkAddress(new System.Net.IPEndPoint(System.Net.IPAddress.Parse("127.0.0.1"), 42046));
            originInfo.FriendlyName = "The node that started the query.";

            // Create a local node info object.
            NodeInfo myInfo = new NodeInfo();
            myInfo.Mode = NodeInfo.OperatingMode.Leaf;
            myInfo.HubCount = 3;
            myInfo.LeafCount = 0;
            myInfo.NodeGUID = new Guid("C2BDECF2-8FEB-48f9-BEFA-FE9E76B53B92");
            myInfo.PublicKey = new DLM.PrivateKey().PublicKey;
            myInfo.Address = new NetworkAddress(new System.Net.IPEndPoint(System.Net.IPAddress.Parse("127.0.0.1"), 42044));
            myInfo.FriendlyName = "The node that has what you want.";

            // Create a local node info object for the connected hub.
            NodeInfo connectedInfo = new NodeInfo();
            connectedInfo.Mode = NodeInfo.OperatingMode.Hub;
            connectedInfo.HubCount = 10;
            connectedInfo.LeafCount = 972;
            connectedInfo.NodeGUID = new Guid("2ADC5FD4-55BE-44b7-B415-18084DB59E2D");
            connectedInfo.PublicKey = new DLM.PrivateKey().PublicKey;
            connectedInfo.Address = new NetworkAddress(new System.Net.IPEndPoint(System.Net.IPAddress.Parse("127.0.0.1"), 42045));
            connectedInfo.FriendlyName = "The hub that is connected to the node that has what you want.";

            QueryResult aResult = new QueryResult(Guid.NewGuid(), originInfo, myInfo, connectedInfo);
            aResult.DataSize = 7923450345;
            aResult.HashCodeType = 1;
            aResult.HashCode = new byte[5];
            aResult.MatchingContents = "The string that matched.";
            aResult.MatchingDescription = "A data source that has lovely images.";

            // Convert it to a message.
            Message aMessage = aResult.ToMessage();

            // Construct a new one from the message.
            QueryResult reconstructedResult = new QueryResult(aMessage);

            // Ensure that the new QueryResult contains the same information as the old QueryResult.
            Assert.IsTrue(aResult.DataSize == reconstructedResult.DataSize);
            Assert.IsTrue(aResult.HashCodeType == reconstructedResult.HashCodeType);
            Assert.IsTrue(aResult.HashCode.Length == reconstructedResult.HashCode.Length);
            for (int index = 0; index < aResult.HashCode.Length; ++index)
            {
                Assert.IsTrue(aResult.HashCode[index] == reconstructedResult.HashCode[index]);
            }
            Assert.IsTrue(aResult.MatchingContents == reconstructedResult.MatchingContents);
            Assert.IsTrue(aResult.MatchingDescription == reconstructedResult.MatchingDescription);
            Assert.IsTrue(aResult.QueryGuid.Equals(reconstructedResult.QueryGuid));
            Assert.IsTrue(aResult.QueryOriginNode.Equals(reconstructedResult.QueryOriginNode));
            Assert.IsTrue(aResult.ResultNode.Equals(reconstructedResult.ResultNode));
#if INCLUDE_CONNECTED_NODE_IN_MESSAGE
            NUnit.Framework.Assert.IsTrue(aResult.ConnectedNode.Equals(reconstructedResult.ConnectedNode));
#endif
        }
    }
}